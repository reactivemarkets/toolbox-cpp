// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Parser.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
/// Parser for test-cases.
class Parser : BasicParser<Parser> {
    friend class BasicParser<Parser>;

  public:
    /// The parse method uses the following special syntax to express callback sequences:
    ///
    /// | Char | Description            |
    /// |------+------------------------|
    /// | !    | Application exception. |
    /// | +    | String token.          |
    /// | ,    | Token separator.       |
    /// | -    | Error token.           |
    /// | :    | Integer token.         |
    /// | [    | Array begin.           |
    /// | ]    | Array end.             |
    /// | ~    | State reset.           |
    ///
    std::string parse(string_view data)
    {
        for (const auto c : data) {
            // Intercept special character that sets the except flag.
            if (c == '!') {
                except_ = true;
                continue;
            }
            try {
                put(c);
            } catch (const resp::Exception&) {
                throw;
            } catch (const exception& e) {
                result_ += '!';
            }
        }
        return result_;
    }

  private:
    void on_resp_command_line(const string& line)
    {
        throw_if_except();
        result_ = line;
    }
    void on_resp_string(const string& s)
    {
        if (!result_.empty() && result_.back() != '[') {
            result_ += ',';
        }
        throw_if_except();
        result_ += '+';
        result_ += s;
    }
    void on_resp_error(const string& e)
    {
        if (!result_.empty() && result_.back() != '[') {
            result_ += ',';
        }
        throw_if_except();
        result_ += '-';
        result_ += e;
    }
    void on_resp_integer(int64_t i)
    {
        if (!result_.empty() && result_.back() != '[') {
            result_ += ',';
        }
        throw_if_except();
        result_ += ':';
        result_ += to_string(i);
    }
    void on_resp_array_begin(int /*n*/)
    {
        if (!result_.empty() && result_.back() != '[') {
            result_ += ',';
        }
        throw_if_except();
        result_ += '[';
    }
    void on_resp_array_end()
    {
        throw_if_except();
        result_ += ']';
    }
    void on_resp_reset() noexcept { result_ += '~'; }
    void throw_if_except()
    {
        if (!except_) {
            return;
        }
        except_ = false;
        throw exception{};
    }
    // Setting to true causes the next handler to throw.
    bool except_{false};
    string result_;
};

std::string parse(string_view data)
{
    Parser p;
    return p.parse(data);
}

} // namespace

BOOST_AUTO_TEST_SUITE(ParserSuite)

BOOST_AUTO_TEST_CASE(SimpleStringCase)
{
    BOOST_TEST(parse("+\r\n"sv) == "+");
    BOOST_TEST(parse("+OK\r\n"sv) == "+OK");
    BOOST_TEST(parse("+OK\r"sv) == "");
    BOOST_TEST(parse("+foo bar\r\n"sv) == "+foo bar");
    BOOST_TEST(parse("+foo bar!\r\n+OK\r\n"sv) == "!,+OK");
}

BOOST_AUTO_TEST_CASE(ErrorCase)
{
    BOOST_TEST(parse("-\r\n"sv) == "-");
    BOOST_TEST(parse("-Error message\r\n"sv) == "-Error message");
    BOOST_TEST(parse("-Error message\r"sv) == "");
    BOOST_TEST(parse("-ERR unknown command 'foobar'\r\n"sv) == "-ERR unknown command 'foobar'");
    BOOST_TEST(parse("-ERR unknown command 'foobar'!\r\n+OK\r\n"sv) == "!,+OK");
}

BOOST_AUTO_TEST_CASE(IntegerCase)
{
    BOOST_TEST(parse(":\r\n"sv) == ":0");
    BOOST_TEST(parse(":0\r\n"sv) == ":0");
    BOOST_TEST(parse(":0\r"sv) == "");
    BOOST_TEST(parse(":123\r\n"sv) == ":123");
    BOOST_TEST(parse(":+123\r\n"sv) == ":123");
    BOOST_TEST(parse(":-123\r\n"sv) == ":-123");
    BOOST_TEST(parse(":-123!\r\n+OK\r\n"sv) == "!,+OK");

    BOOST_CHECK_THROW(parse(":1x"sv), resp::Exception);
}

BOOST_AUTO_TEST_CASE(BulkStringCase)
{
    BOOST_TEST(parse("$0\r\n\r\n"sv) == "+");
    BOOST_TEST(parse("$6\r\nfoobar\r\n"sv) == "+foobar");
    BOOST_TEST(parse("$7\r\nfoo bar\r\n"sv) == "+foo bar");
    BOOST_TEST(parse("$7\r\nfoo bar!\r\n+OK\r\n"sv) == "!,+OK");
    BOOST_TEST(parse("$7\r\nfoo bar!\r\n+OK\r\n"sv) == "!,+OK");

    BOOST_CHECK_THROW(parse("$6x\r\nfoobar\r\n"sv), resp::Exception);
}

BOOST_AUTO_TEST_CASE(MultiValueCase)
{
    BOOST_TEST(parse("+OK\r\n-ERR\r\n:123\r\n$6\r\nfoobar\r\n"sv) == "+OK,-ERR,:123,+foobar");
}

BOOST_AUTO_TEST_CASE(ArrayCase)
{
    BOOST_TEST(parse("*0\r\n"sv) == "[]");
    BOOST_TEST(parse("*0!\r\n+OK\r\n"sv) == "~!,+OK");
    BOOST_TEST(parse("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n"sv) == "[+foo,+bar]");
    BOOST_TEST(parse("*2\r\n$3\r\nfoo!\r\n$3\r\nbar\r\n+OK\r\n"sv) == "[!~,+OK");
    BOOST_TEST(parse("*3\r\n:1\r\n:2\r\n:3\r\n"sv) == "[:1,:2,:3]");
    BOOST_TEST(parse("*3\r\n:1\r\n:2\r\n:3!\r\n+OK\r\n"sv) == "[:1,:2,~!,+OK");
    BOOST_TEST(parse("*5\r\n:1\r\n:2\r\n:3\r\n:4\r\n$6\r\nfoobar\r\n"sv)
               == "[:1,:2,:3,:4,+foobar]");
    BOOST_TEST(parse("*5\r\n:1\r\n:2\r\n:3!\r\n:4\r\n$6\r\nfoobar\r\n+OK\r\n"sv)
               == "[:1,:2,!~,+OK");

    BOOST_CHECK_THROW(parse("*1x\r\n$3\r\nfoo\r\n"sv), resp::Exception);
}

BOOST_AUTO_TEST_CASE(MultiArrayCase)
{
    BOOST_TEST(parse("*1\r\n*0\r\n"sv) == "[[]]");
    BOOST_TEST(parse("*2\r\n*0\r\n*0\r\n"sv) == "[[],[]]");
    BOOST_TEST(parse("*1\r\n*1\r\n*0\r\n"sv) == "[[[]]]");
    BOOST_TEST(parse("*1\r\n*1\r\n+OK\r\n"sv) == "[[+OK]]");
    BOOST_TEST(parse("*1\r\n*1\r\n+OK!\r\n+OK\r\n"sv) == "[[~!,+OK");
    BOOST_TEST(parse("*2\r\n*1\r\n+OK\r\n*1\r\n-ERR\r\n"sv) == "[[+OK],[-ERR]]");
    BOOST_TEST(parse("*2\r\n*1\r\n+OK\r\n*1\r\n-ERR!\r\n+OK\r\n"sv) == "[[+OK],[~!,+OK");
    BOOST_TEST(parse("*3\r\n:1\r\n*3\r\n:11\r\n*2\r\n:111\r\n:222\r\n:22\r\n:2\r\n"sv)
               == "[:1,[:11,[:111,:222],:22],:2]");
    BOOST_TEST(parse("*3\r\n:1\r\n*3\r\n:11\r\n*2\r\n:111!\r\n:222\r\n:22\r\n:2\r\n+OK\r\n"sv)
               == "[:1,[:11,[!~,+OK");
}

BOOST_AUTO_TEST_SUITE_END()
