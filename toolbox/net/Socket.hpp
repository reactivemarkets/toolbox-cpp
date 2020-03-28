// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2020 Reactive Markets Limited
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

#ifndef TOOLBOX_NET_SOCKET_HPP
#define TOOLBOX_NET_SOCKET_HPP

#include <toolbox/net/Error.hpp>

#include <toolbox/io/File.hpp>

#include <memory>

#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace toolbox {
inline namespace net {
using AddrInfoPtr = std::unique_ptr<addrinfo, void (*)(addrinfo*)>;
} // namespace net
namespace os {

/// Get network address from Internet host and service.
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, const addrinfo& hints,
                               std::error_code& ec) noexcept
{
    addrinfo* ai{nullptr};
    const auto err = ::getaddrinfo(node, service, &hints, &ai);
    if (err != 0) {
        ec = make_gai_error_code(err);
    }
    return {ai, freeaddrinfo};
}

/// Get network address from Internet host and service.
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, const addrinfo& hints)
{
    addrinfo* ai{nullptr};
    const auto err = ::getaddrinfo(node, service, &hints, &ai);
    if (err != 0) {
        throw std::system_error{make_gai_error_code(err), "getaddrinfo"};
    }
    return {ai, freeaddrinfo};
}

/// Get network address from Internet host and service.
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, int family, int type,
                               int protocol, std::error_code& ec) noexcept
{
    addrinfo hints{};
    // If node is not specified, then return a wildcard address suitable for bind()ing.
    hints.ai_flags = node ? 0 : AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    return getaddrinfo(node, service, hints, ec);
}

/// Get network address from Internet host and service.
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, int family, int type,
                               int protocol)
{
    addrinfo hints{};
    // If node is not specified, then return a wildcard address suitable for bind()ing.
    hints.ai_flags = node ? 0 : AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    return getaddrinfo(node, service, hints);
}

/// Get network address from Internet host and service.
template <typename ProtocolT>
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, ProtocolT protocol,
                               std::error_code& ec) noexcept
{
    return getaddrinfo(node, service, protocol.family(), protocol.type(), protocol.protocol(), ec);
}

/// Get network address from Internet host and service.
template <typename ProtocolT>
inline AddrInfoPtr getaddrinfo(const char* node, const char* service, ProtocolT protocol)
{
    return getaddrinfo(node, service, protocol.family(), protocol.type(), protocol.protocol());
}

/// Returns the index of the network interface corresponding to the name ifname.
inline unsigned if_nametoindex(const char* ifname, std::error_code& ec) noexcept
{
    unsigned ifindex{0};
    if (ifname) {
        if (!(ifindex = ::if_nametoindex(ifname))) {
            ec = make_sys_error(errno);
        }
    }
    return ifindex;
}

/// Returns the index of the network interface corresponding to the name ifname.
inline unsigned if_nametoindex(const char* ifname)
{
    unsigned ifindex{0};
    if (ifname) {
        if (!(ifindex = ::if_nametoindex(ifname))) {
            throw std::system_error{make_sys_error(errno), "if_nametoindex"};
        }
    }
    return ifindex;
}

/// Create an endpoint for communication.
inline FileHandle socket(int family, int type, int protocol, std::error_code& ec) noexcept
{
    const auto sockfd = ::socket(family, type, protocol);
    if (sockfd < 0) {
        ec = make_sys_error(errno);
    }
    return sockfd;
}

/// Create an endpoint for communication.
inline FileHandle socket(int family, int type, int protocol)
{
    const auto sockfd = ::socket(family, type, protocol);
    if (sockfd < 0) {
        throw std::system_error{make_sys_error(errno), "socket"};
    }
    return sockfd;
}

/// Create an endpoint for communication.
template <typename ProtocolT>
inline FileHandle socket(ProtocolT protocol, std::error_code& ec) noexcept
{
    return socket(protocol.family(), protocol.type(), protocol.protocol(), ec);
}

/// Create an endpoint for communication.
template <typename ProtocolT>
inline FileHandle socket(ProtocolT protocol)
{
    return socket(protocol.family(), protocol.type(), protocol.protocol());
}

/// Create a pair of connected sockets.
inline std::pair<FileHandle, FileHandle> socketpair(int family, int type, int protocol,
                                                    std::error_code& ec) noexcept
{
    int sv[2];
    if (::socketpair(family, type, protocol, sv) < 0) {
        ec = make_sys_error(errno);
    }
    return {FileHandle{sv[0]}, FileHandle{sv[1]}};
}

/// Create a pair of connected sockets.
inline std::pair<FileHandle, FileHandle> socketpair(int family, int type, int protocol)
{
    int sv[2];
    if (::socketpair(family, type, protocol, sv) < 0) {
        throw std::system_error{make_sys_error(errno), "socketpair"};
    }
    return {FileHandle{sv[0]}, FileHandle{sv[1]}};
}

/// Create a pair of connected sockets.
template <typename ProtocolT>
inline std::pair<FileHandle, FileHandle> socketpair(ProtocolT protocol,
                                                    std::error_code& ec) noexcept
{
    return socketpair(protocol.family(), protocol.type(), protocol.protocol(), ec);
}

/// Create a pair of connected sockets.
template <typename ProtocolT>
inline std::pair<FileHandle, FileHandle> socketpair(ProtocolT protocol)
{
    return socketpair(protocol.family(), protocol.type(), protocol.protocol());
}

/// Accept a connection on a socket.
inline FileHandle accept(int sockfd, sockaddr& addr, socklen_t& addrlen,
                         std::error_code& ec) noexcept
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    const auto fd = ::accept(sockfd, &addr, &addrlen);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Accept a connection on a socket.
inline FileHandle accept(int sockfd, sockaddr& addr, socklen_t& addrlen)
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    const auto fd = ::accept(sockfd, &addr, &addrlen);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "accept"};
    }
    return fd;
}

/// Accept a connection on a socket.
inline FileHandle accept(int sockfd, std::error_code& ec) noexcept
{
    const auto fd = ::accept(sockfd, nullptr, nullptr);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Accept a connection on a socket.
inline FileHandle accept(int sockfd)
{
    const auto fd = ::accept(sockfd, nullptr, nullptr);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "accept"};
    }
    return fd;
}

/// Accept a connection on a socket.
template <typename EndpointT>
inline FileHandle accept(int sockfd, EndpointT& ep, std::error_code& ec) noexcept
{
    socklen_t addrlen = ep.capacity();
    FileHandle fh{accept(sockfd, *ep.data(), addrlen, ec)};
    if (!ec) {
        ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
    }
    return fh;
}

/// Accept a connection on a socket.
template <typename EndpointT>
inline FileHandle accept(int sockfd, EndpointT& ep)
{
    socklen_t addrlen = ep.capacity();
    FileHandle fh{accept(sockfd, *ep.data(), addrlen)};
    ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
    return fh;
}

/// Bind a name to a socket.
inline void bind(int sockfd, const sockaddr& addr, socklen_t addrlen, std::error_code& ec) noexcept
{
    if (::bind(sockfd, &addr, addrlen) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Bind a name to a socket.
inline void bind(int sockfd, const sockaddr& addr, socklen_t addrlen)
{
    if (::bind(sockfd, &addr, addrlen) < 0) {
        throw std::system_error{make_sys_error(errno), "bind"};
    }
}

/// Bind a name to a socket.
template <typename EndpointT>
inline void bind(int sockfd, const EndpointT& ep, std::error_code& ec) noexcept
{
    bind(sockfd, *ep.data(), ep.size(), ec);
}

/// Bind a name to a socket.
template <typename EndpointT>
inline void bind(int sockfd, const EndpointT& ep)
{
    bind(sockfd, *ep.data(), ep.size());
}

/// Initiate a connection on a socket.
inline void connect(int sockfd, const sockaddr& addr, socklen_t addrlen,
                    std::error_code& ec) noexcept
{
    if (::connect(sockfd, &addr, addrlen) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Initiate a connection on a socket.
inline void connect(int sockfd, const sockaddr& addr, socklen_t addrlen)
{
    if (::connect(sockfd, &addr, addrlen) < 0) {
        throw std::system_error{make_sys_error(errno), "connect"};
    }
}

/// Initiate a connection on a socket.
template <typename EndpointT>
inline void connect(int sockfd, const EndpointT& ep, std::error_code& ec) noexcept
{
    connect(sockfd, *ep.data(), ep.size(), ec);
}

/// Initiate a connection on a socket.
template <typename EndpointT>
inline void connect(int sockfd, const EndpointT& ep)
{
    connect(sockfd, *ep.data(), ep.size());
}

/// Listen for connections on a socket.
inline void listen(int sockfd, int backlog, std::error_code& ec) noexcept
{
    if (::listen(sockfd, backlog) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Listen for connections on a socket.
inline void listen(int sockfd, int backlog)
{
    if (::listen(sockfd, backlog) < 0) {
        throw std::system_error{make_sys_error(errno), "listen"};
    }
}

/// Shut-down part of a full-duplex connection
inline void shutdown(int sockfd, int how, std::error_code& ec) noexcept
{
    if (::shutdown(sockfd, how) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Shut-down part of a full-duplex connection
inline void shutdown(int sockfd, int how)
{
    if (::shutdown(sockfd, how) < 0) {
        throw std::system_error{make_sys_error(errno), "shutdown"};
    }
}

/// Receive a message from a socket.
inline ssize_t recv(int sockfd, void* buf, std::size_t len, int flags, std::error_code& ec) noexcept
{
    const auto ret = ::recv(sockfd, buf, len, flags);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Receive a message from a socket.
inline std::size_t recv(int sockfd, void* buf, std::size_t len, int flags)
{
    const auto ret = ::recv(sockfd, buf, len, flags);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "recv"};
    }
    return ret;
}

/// Receive a message from a socket.
inline ssize_t recv(int sockfd, MutableBuffer buf, int flags, std::error_code& ec) noexcept
{
    return recv(sockfd, buffer_cast<void*>(buf), buffer_size(buf), flags, ec);
}

/// Receive a message from a socket.
inline std::size_t recv(int sockfd, MutableBuffer buf, int flags)
{
    return recv(sockfd, buffer_cast<void*>(buf), buffer_size(buf), flags);
}

/// Receive a message from a socket.
inline ssize_t recvfrom(int sockfd, void* buf, std::size_t len, int flags, sockaddr& addr,
                        socklen_t& addrlen, std::error_code& ec) noexcept
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    const auto ret = ::recvfrom(sockfd, buf, len, flags, &addr, &addrlen);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Receive a message from a socket.
inline std::size_t recvfrom(int sockfd, void* buf, std::size_t len, int flags, sockaddr& addr,
                            socklen_t& addrlen)
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    const auto ret = ::recvfrom(sockfd, buf, len, flags, &addr, &addrlen);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "recvfrom"};
    }
    return ret;
}

/// Receive a message from a socket.
template <typename EndpointT>
inline ssize_t recvfrom(int sockfd, void* buf, std::size_t len, int flags, EndpointT& ep,
                        std::error_code& ec) noexcept
{
    socklen_t addrlen = ep.capacity();
    const auto ret = recvfrom(sockfd, buf, len, flags, *ep.data(), addrlen, ec);
    if (!ec) {
        ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
    }
    return ret;
}

/// Receive a message from a socket.
template <typename EndpointT>
inline std::size_t recvfrom(int sockfd, void* buf, std::size_t len, int flags, EndpointT& ep)
{
    socklen_t addrlen = ep.capacity();
    const auto ret = recvfrom(sockfd, buf, len, flags, *ep.data(), addrlen);
    ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
    return ret;
}

/// Receive a message from a socket.
template <typename EndpointT>
inline ssize_t recvfrom(int sockfd, MutableBuffer buf, int flags, EndpointT& ep,
                        std::error_code& ec) noexcept
{
    return recvfrom(sockfd, buffer_cast<void*>(buf), buffer_size(buf), flags, ep, ec);
}

/// Receive a message from a socket.
template <typename EndpointT>
inline std::size_t recvfrom(int sockfd, MutableBuffer buf, int flags, EndpointT& ep)
{
    return recvfrom(sockfd, buffer_cast<void*>(buf), buffer_size(buf), flags, ep);
}

/// Send a message on a socket.
inline ssize_t send(int sockfd, const void* buf, std::size_t len, int flags,
                    std::error_code& ec) noexcept
{
    const auto ret = ::send(sockfd, buf, len, flags);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Send a message on a socket.
inline std::size_t send(int sockfd, const void* buf, std::size_t len, int flags)
{
    const auto ret = ::send(sockfd, buf, len, flags);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "send"};
    }
    return ret;
}

/// Send a message on a socket.
inline ssize_t send(int sockfd, ConstBuffer buf, int flags, std::error_code& ec) noexcept
{
    return send(sockfd, buffer_cast<const void*>(buf), buffer_size(buf), flags, ec);
}

/// Send a message on a socket.
inline std::size_t send(int sockfd, ConstBuffer buf, int flags)
{
    return send(sockfd, buffer_cast<const void*>(buf), buffer_size(buf), flags);
}

/// Send a message on a socket.
inline ssize_t sendto(int sockfd, const void* buf, std::size_t len, int flags, const sockaddr& addr,
                      socklen_t addrlen, std::error_code& ec) noexcept
{
    const auto ret = ::sendto(sockfd, buf, len, flags, &addr, addrlen);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Send a message on a socket.
inline std::size_t sendto(int sockfd, const void* buf, std::size_t len, int flags,
                          const sockaddr& addr, socklen_t addrlen)
{
    const auto ret = ::sendto(sockfd, buf, len, flags, &addr, addrlen);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "sendto"};
    }
    return ret;
}

/// Send a message on a socket.
template <typename EndpointT>
inline ssize_t sendto(int sockfd, const void* buf, std::size_t len, int flags, const EndpointT& ep,
                      std::error_code& ec) noexcept
{
    return sendto(sockfd, buf, len, flags, *ep.data(), ep.size());
}

/// Send a message on a socket.
template <typename EndpointT>
inline std::size_t sendto(int sockfd, const void* buf, std::size_t len, int flags,
                          const EndpointT& ep)
{
    return sendto(sockfd, buf, len, flags, *ep.data(), ep.size());
}

/// Send a message on a socket.
template <typename EndpointT>
inline ssize_t sendto(int sockfd, ConstBuffer buf, int flags, const EndpointT& ep,
                      std::error_code& ec) noexcept
{
    return sendto(sockfd, buffer_cast<const void*>(buf), buffer_size(buf), flags, *ep.data(),
                  ep.size(), ec);
}

/// Send a message on a socket.
template <typename EndpointT>
inline std::size_t sendto(int sockfd, ConstBuffer buf, int flags, const EndpointT& ep)
{
    return sendto(sockfd, buffer_cast<const void*>(buf), buffer_size(buf), flags, *ep.data(),
                  ep.size());
}

/// Get the socket name.
inline void getsockname(int sockfd, sockaddr& addr, socklen_t& addrlen,
                        std::error_code& ec) noexcept
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    if (::getsockname(sockfd, &addr, &addrlen) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Get the socket name.
inline void getsockname(int sockfd, sockaddr& addr, socklen_t& addrlen)
{
    // The addrlen argument is updated to contain the actual size of the source address.
    // The returned address is truncated if the buffer provided is too small.
    if (::getsockname(sockfd, &addr, &addrlen) < 0) {
        throw std::system_error{make_sys_error(errno), "getsockname"};
    }
}

/// Get the socket name.
template <typename EndpointT>
inline void getsockname(int sockfd, EndpointT& ep, std::error_code& ec) noexcept
{
    socklen_t addrlen = ep.capacity();
    getsockname(sockfd, *ep.data(), addrlen, ec);
    if (!ec) {
        ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
    }
}

/// Get the socket name.
template <typename EndpointT>
inline void getsockname(int sockfd, EndpointT& ep)
{
    socklen_t addrlen = ep.capacity();
    getsockname(sockfd, *ep.data(), addrlen);
    ep.resize(std::min<std::size_t>(addrlen, ep.capacity()));
}

/// Get socket option.
inline void getsockopt(int sockfd, int level, int optname, void* optval, socklen_t& optlen,
                       std::error_code& ec) noexcept
{
    if (::getsockopt(sockfd, level, optname, optval, &optlen) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Get socket option.
inline void getsockopt(int sockfd, int level, int optname, void* optval, socklen_t& optlen)
{
    if (::getsockopt(sockfd, level, optname, optval, &optlen) < 0) {
        throw std::system_error{make_sys_error(errno), "getsockopt"};
    }
}

/// Set socket option.
inline void setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen,
                       std::error_code& ec) noexcept
{
    if (::setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        ec = make_sys_error(errno);
    }
}

/// Set socket option.
inline void setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    if (::setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        throw std::system_error{make_sys_error(errno), "setsockopt"};
    }
}

} // namespace os
inline namespace net {

/// Create an addrinfo structure for a Unix domain address.
TOOLBOX_API AddrInfoPtr get_unix_addrinfo(std::string_view path, int type);

template <typename EndpointT>
inline void get_sock_name(int sockfd, EndpointT& ep, std::error_code& ec) noexcept
{
    os::getsockname(sockfd, ep, ec);
}

template <typename EndpointT>
inline void get_sock_name(int sockfd, EndpointT& ep)
{
    os::getsockname(sockfd, ep);
}

inline std::error_code get_so_error(int sockfd, std::error_code& ec) noexcept
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, optlen, ec);
    return make_sys_error(optval);
}

inline std::error_code get_so_error(int sockfd)
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, optlen);
    return make_sys_error(optval);
}

inline int get_so_rcv_buf(int sockfd, std::error_code& ec) noexcept
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &optval, optlen, ec);
    return optval;
}

inline int get_so_rcv_buf(int sockfd)
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &optval, optlen);
    return optval;
}

inline int get_so_snd_buf(int sockfd, std::error_code& ec) noexcept
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &optval, optlen, ec);
    return optval;
}

inline int get_so_snd_buf(int sockfd)
{
    int optval{};
    socklen_t optlen{sizeof(optval)};
    os::getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &optval, optlen);
    return optval;
}

inline void set_so_rcv_buf(int sockfd, int size, std::error_code& ec) noexcept
{
    os::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size), ec);
}

inline void set_so_rcv_buf(int sockfd, int size)
{
    os::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

inline void set_so_reuse_addr(int sockfd, bool enabled, std::error_code& ec) noexcept
{
    int optval{enabled ? 1 : 0};
    os::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval), ec);
}

inline void set_so_reuse_addr(int sockfd, bool enabled)
{
    int optval{enabled ? 1 : 0};
    os::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

inline void set_so_snd_buf(int sockfd, int size, std::error_code& ec) noexcept
{
    os::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size), ec);
}

inline void set_so_snd_buf(int sockfd, int size)
{
    os::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

/// Enable or disable the Nagle algorithm.
///
/// This means that segments are always sent as soon as possible, even if there is only a small
/// amount of data. When not set, data is buffered until there is a sufficient amount to send out,
/// thereby avoiding the frequent sending of small packets, which results in poor utilization of the
/// network.
///
/// \param sockfd The socket descriptor.
/// \param enabled Boolean switch to enable or disable.
/// \param ec Error-code set on failure.
inline void set_tcp_no_delay(int sockfd, bool enabled, std::error_code& ec) noexcept
{
    int optval{enabled ? 1 : 0};
    os::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval), ec);
}

/// Enable or disable the Nagle algorithm.
///
/// This means that segments are always sent as soon as possible, even if there is only a small
/// amount of data. When not set, data is buffered until there is a sufficient amount to send out,
/// thereby avoiding the frequent sending of small packets, which results in poor utilization of the
/// network.
///
/// \param sockfd The socket descriptor.
/// \param enabled Boolean switch to enable or disable.
inline void set_tcp_no_delay(int sockfd, bool enabled)
{
    int optval{enabled ? 1 : 0};
    os::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

/// Set the number of SYN retransmits that TCP should send before aborting the attempt to connect.
///
/// The number of retransmits cannot exceed 255.
///
/// \param sockfd The socket descriptor.
/// \param retrans The number of retransmits.
/// \param ec Error-code set on failure.
inline void set_tcp_syn_nt(int sockfd, int retrans, std::error_code& ec) noexcept
{
    os::setsockopt(sockfd, IPPROTO_TCP, TCP_SYNCNT, &retrans, sizeof(retrans), ec);
}

/// Set the number of SYN retransmits that TCP should send before aborting the attempt to connect.
///
/// The number of retransmits cannot exceed 255.
///
/// \param sockfd The socket descriptor.
/// \param retrans The number of retransmits.
inline void set_tcp_syn_nt(int sockfd, int retrans)
{
    os::setsockopt(sockfd, IPPROTO_TCP, TCP_SYNCNT, &retrans, sizeof(retrans));
}

struct Sock : FileHandle {
    Sock(FileHandle&& sock, int family)
    : FileHandle{std::move(sock)}
    , family_{family}
    {
    }
    Sock() noexcept = default;

    int family() const noexcept { return family_; }
    bool is_ip_family() const noexcept { return family_ == AF_INET || family_ == AF_INET6; }

    // Logically const.
    std::error_code get_error(std::error_code& ec) const noexcept
    {
        return toolbox::get_so_error(get(), ec);
    }
    std::error_code get_error() const { return toolbox::get_so_error(get()); }

    int get_rcv_buf(std::error_code& ec) const noexcept
    {
        return toolbox::get_so_rcv_buf(get(), ec);
    }
    int get_rcv_buf() const { return toolbox::get_so_rcv_buf(get()); }

    int get_snd_buf(std::error_code& ec) const noexcept
    {
        return toolbox::get_so_snd_buf(get(), ec);
    }
    int get_snd_buf() const { return toolbox::get_so_snd_buf(get()); }

    void close() { reset(); }

    void set_non_block(std::error_code& ec) noexcept { toolbox::set_non_block(get(), ec); }
    void set_non_block() { toolbox::set_non_block(get()); }

    void set_rcv_buf(int size, std::error_code& ec) noexcept
    {
        toolbox::set_so_rcv_buf(get(), size, ec);
    }
    void set_rcv_buf(int size) { toolbox::set_so_rcv_buf(get(), size); }

    void set_reuse_addr(bool enabled, std::error_code& ec) noexcept
    {
        toolbox::set_so_reuse_addr(get(), enabled, ec);
    }
    void set_reuse_addr(bool enabled) { toolbox::set_so_reuse_addr(get(), enabled); }

    void set_snd_buf(int size, std::error_code& ec) noexcept
    {
        toolbox::set_so_snd_buf(get(), size, ec);
    }
    void set_snd_buf(int size) { toolbox::set_so_snd_buf(get(), size); }

  private:
    int family_{};
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_SOCKET_HPP
