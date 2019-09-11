// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Resolver2.hpp"

#include "Endpoint.hpp"
#include "Socket.hpp"

#include <toolbox/net/DgramSock.hpp>

#include <toolbox/util/Debug.hpp>
#include <toolbox/util/Stream.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>

namespace toolbox {
inline namespace net {

char* to_dns(const std::string& host, char* buff)
{
    std::size_t pos{0};
    std::string_view view{host};
    while ((pos = view.find_first_of('.'), pos) != std::string_view::npos) {
        *buff = static_cast<char>(pos);
        std::memcpy(++buff, view.data(), pos);
        view.remove_prefix(pos + 1); // Skip the dot
        buff += pos;
    }
    *buff = static_cast<char>(view.size());
    std::memcpy(++buff, view.data(), view.size() + 1); // Include null terminator
    return buff += view.size() + 1;
}

DnsServers::DnsServers(std::istream& resolv_conf, std::vector<std::string> additional_nameservers)
: servers_{std::move(additional_nameservers)}
{
    std::string line;
    const std::string nameserver{"nameserver"};
    while (std::getline(resolv_conf, line)) {
        // C++20 Replace with starts_with
        if (line.compare(0, nameserver.size(), nameserver) == 0) {
            auto pos = line.find_first_not_of(' ', nameserver.size() + 1);
            if (pos != std::string::npos) {
                servers_.emplace_back(line.substr(pos));
            }
        }
    }

    if (servers_.empty()) {
        throw std::runtime_error("No servers available");
    }
}

const DnsServers::Servers& DnsServers::servers() const noexcept
{
    return servers_;
}

const std::string& DnsServers::server() const
{
    return servers_[0];
}

#pragma pack(push, 1)

class Dns {
  public:
    Dns(const std::string& hostname, DnsRequest::Type query_type)
    {
        auto res = to_dns(hostname, query_name());
        hostname_size_ = res - query_name();
        question().qtype = htons(query_type);
        question().class_ = htons(1); // Internet
    }

    class Header {

      public:
        Header()
        : id_{htons(getpid())}
        , recursion_desired_{1}
        , truncated_{0}
        , authorative_answer_{0}
        , opcode_{0} // Standard query
        , qr_{0}     // Query
        , rcode_{0}
        , cd_{0}
        , ad_{0}
        , z_{0}
        , recursion_available_{0}
        , questions_{htons(1)} // Single question
        , ans_count_{0}
        , auth_count_{0}
        , add_count_{0}
        {
        }

        uint16_t id() const noexcept { return ntohs(id_); }
        bool recursion_desired() const noexcept { return recursion_desired_; }
        bool truncated() const noexcept { return truncated_; }
        bool authorative_answer() const noexcept { return authorative_answer_; }
        auto opcode() const noexcept { return opcode_; }
        bool qr() const noexcept { return qr_; }
        auto rcode() const noexcept { return rcode_; }
        bool cd() const noexcept { return cd_; }
        bool ad() const noexcept { return ad_; }
        bool z() const noexcept { return z_; }
        bool recursion_available() const noexcept { return recursion_available_; }

        auto questions() const noexcept { return ntohs(questions_); }
        auto ans_count() const noexcept { return ntohs(ans_count_); }
        auto auth_count() const noexcept { return ntohs(auth_count_); }
        auto add_count() const noexcept { return ntohs(add_count_); }

      private:
        // C++20 will allow bitfield initialization on declaration
        uint16_t id_;                    // message id
        uint8_t recursion_desired_ : 1;  // recursion desired
        uint8_t truncated_ : 1;          // truncated message
        uint8_t authorative_answer_ : 1; // authoritive answer
        uint8_t opcode_ : 4;             // purpose of message
        uint8_t qr_ : 1;                 // query/response flag

        uint8_t rcode_ : 4;               // response code
        uint8_t cd_ : 1;                  // disable checking
        uint8_t ad_ : 1;                  // authenticated data
        uint8_t z_ : 1;                   // its z! reserveddns_servers
        uint8_t recursion_available_ : 1; // recursion available

        uint16_t questions_;  // number of question entries
        uint16_t ans_count_;  // number of answer entries
        uint16_t auth_count_; // number of authority entries
        uint16_t add_count_;  // number of resource entries
    };

    struct Question {
        uint16_t qtype;
        uint16_t class_;
    };

    class RData {
      public:
        enum class Type : uint16_t {
            A_RECORD = 0x0001,
            NAME_SERVER = 0x0002,
            CNAME = 0x0005,
            MAIL_SERVER = 0x000f
        };

        Type type() const noexcept { return static_cast<Type>(ntohs(type_)); }
        auto class_() const noexcept { return ntohs(class__); }
        auto ttl() const noexcept { return ntohs(ttl_); }
        auto length() const noexcept { return ntohs(length_); }

        char* data() noexcept { return reinterpret_cast<char*>(&length_) + sizeof(length_); }
        const char* data() const noexcept
        {
            return reinterpret_cast<const char*>(&length_) + sizeof(length_);
        }

      private:
        uint16_t type_;
        uint16_t class__;
        uint32_t ttl_;
        uint16_t length_;
    };

    const auto& header() const noexcept { return header_; }
    auto& header() noexcept { return header_; }

    char* query_name() noexcept { return reinterpret_cast<char*>(&header_) + sizeof(Header); }

    int hostname_size() const noexcept { return hostname_size_; }

    Question& question() noexcept
    {
        return *reinterpret_cast<Question*>(query_name() + hostname_size_);
    }

    int size() const noexcept { return sizeof(Header) + hostname_size_ + sizeof(Question); }

    static constexpr int offset() { return sizeof(hostname_size_); }

    void bookkeep() noexcept { hostname_size_ = strlen(query_name()) + 1; }

    char* reader() noexcept { return reinterpret_cast<char*>(&header_) + size(); }
    const char* reader() const noexcept { return reader(); }

  private:
    // bookkeeping
    int16_t hostname_size_;

    Header header_;
};
#pragma pack(pop)

/// Convert strings like www.google.com to 3www6google3com, where numbers are bytes that follow
std::string dns_format(const std::string& host)
{
    std::size_t pos = 0;
    std::stringstream out;
    std::string_view view{host};
    while ((pos = view.find_first_of('.'), pos) != std::string_view::npos) {
        out << static_cast<char>(pos) << view.substr(0, pos);
        view.remove_prefix(pos + 1);
    }
    out << static_cast<char>(view.size()) << view;
    return out.str();
}

int send_dns_request(const std::string& nameserver, const std::string& hostname,
                     DnsRequest::Type query_type)
{
    int sckt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP packet for DNS queries
    sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(nameserver.c_str());

    //Set the DNS structure to standard queries
    unsigned char send_buffer[1024];
    Dns* dns = new (send_buffer) Dns{hostname, query_type};

    os::sendto(sckt, &dns->header(), dns->size(), 0, reinterpret_cast<struct sockaddr&>(dest),
               sizeof(sockaddr_in));

    return sckt;
}

toolbox::net::IpAddrV4 get_dns_request(int fd, UdpEndpoint& endpoint)
{
    unsigned char recv_buffer[2048];

    os::recvfrom(fd, recv_buffer + Dns::offset(), sizeof(recv_buffer) - Dns::offset(), 0, endpoint);

    // Adjust dns to struct
    auto& dns = *reinterpret_cast<Dns*>(&recv_buffer);
    dns.bookkeep();

    char* reader = dns.reader();

    //Start reading answers
    for (int i = 0; i < dns.header().ans_count(); i++) {
        if (*reinterpret_cast<unsigned char*>(reader) >= 192) // Name is in the header (skip)
        {
            reader += 2;
        } else {
            throw std::runtime_error("Implement full name decoding");
        }

        const auto& r_data = *reinterpret_cast<Dns::RData*>(reader);
        reader += sizeof(Dns::RData);

        assert(r_data.class_() == 1);

        switch (r_data.type()) {
        case Dns::RData::Type::A_RECORD: {
            const auto& bytes
                = *reinterpret_cast<const boost::asio::ip::address_v4::bytes_type*>(r_data.data());
            return boost::asio::ip::make_address_v4(bytes);
        }
        default: { //Skip all other records
            reader += r_data.length();
        }
        }
    }
    throw std::runtime_error{"Could not resolve addr"};
}

} // namespace net
} // namespace toolbox
