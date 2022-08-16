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

#include "String.hpp"

#include <boost/test/unit_test.hpp>

#include <filesystem>

namespace std {
template <typename T, typename U>
ostream& operator<<(ostream& os, const pair<T, U>& p)
{
    return os << '(' << p.first << ',' << p.second << ')';
}
} // namespace std

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(StringSuite)

BOOST_AUTO_TEST_CASE(StringViewCase)
{
    // Make sure the string is not optimised away.
    const auto s = to_string(time(nullptr));

    // Ensure that std::string_view iterators are equivalent across copies of the std::string_view.
    const string_view a{s};
    const string_view b{a};

    // Best effort to detect broken assumption.
    const void* aptr = a.data();
    const void* bptr = b.data();
    BOOST_TEST(aptr == bptr);
}

BOOST_AUTO_TEST_CASE(ToStringCase)
{
    BOOST_TEST(to_string("foo"sv) == "foo"s);
    BOOST_TEST(stoi(to_string(12345)) == 12345);
    BOOST_TEST(stod(to_string(12345.67)) == 12345.67);
}

BOOST_AUTO_TEST_CASE(LtrimCopyCase)
{
    BOOST_TEST(ltrim_copy(""sv) == ""sv);
    BOOST_TEST(ltrim_copy(" \t\n\v\f\r"sv) == ""sv);
    BOOST_TEST(ltrim_copy(" \t\n\v\f\rfoo "sv) == "foo "sv);
    BOOST_TEST(ltrim_copy("foo"sv) == "foo"sv);

    BOOST_TEST(ltrim_copy(""s) == ""s);
    BOOST_TEST(ltrim_copy(" \t\n\v\f\r"s) == ""s);
    BOOST_TEST(ltrim_copy(" \t\n\v\f\rfoo "s) == "foo "s);
    BOOST_TEST(ltrim_copy("foo"s) == "foo"s);
}

BOOST_AUTO_TEST_CASE(RtrimCopyCase)
{
    BOOST_TEST(rtrim_copy(""sv) == ""sv);
    BOOST_TEST(rtrim_copy(" \t\n\v\f\r"sv) == ""sv);
    BOOST_TEST(rtrim_copy(" foo \t\n\v\f\r"sv) == " foo"sv);
    BOOST_TEST(rtrim_copy("foo"sv) == "foo"sv);

    BOOST_TEST(rtrim_copy(""s) == ""s);
    BOOST_TEST(rtrim_copy(" \t\n\v\f\r"s) == ""s);
    BOOST_TEST(rtrim_copy(" foo \t\n\v\f\r"s) == " foo"s);
    BOOST_TEST(rtrim_copy("foo"s) == "foo"s);
}

BOOST_AUTO_TEST_CASE(TrimCopyCase)
{
    BOOST_TEST(trim_copy(""sv) == ""sv);
    BOOST_TEST(trim_copy(" \t\n\v\f\r"sv) == ""sv);
    BOOST_TEST(trim_copy(" \t\n\v\f\rfoo \t\n\v\f\r"sv) == "foo"sv);
    BOOST_TEST(trim_copy("foo"sv) == "foo"sv);

    BOOST_TEST(trim_copy(""s) == ""s);
    BOOST_TEST(trim_copy(" \t\n\v\f\r"s) == ""s);
    BOOST_TEST(trim_copy(" \t\n\v\f\rfoo \t\n\v\f\r"s) == "foo"s);
    BOOST_TEST(trim_copy("foo"s) == "foo"s);
}

BOOST_AUTO_TEST_CASE(SplitPairCase)
{
    BOOST_TEST(split_pair(""sv, '=') == make_pair(""sv, ""sv));
    BOOST_TEST(split_pair("="sv, '=') == make_pair(""sv, ""sv));
    BOOST_TEST(split_pair("a"sv, '=') == make_pair("a"sv, ""sv));
    BOOST_TEST(split_pair("a="sv, '=') == make_pair("a"sv, ""sv));
    BOOST_TEST(split_pair("=b"sv, '=') == make_pair(""sv, "b"sv));
    BOOST_TEST(split_pair("a=b"sv, '=') == make_pair("a"sv, "b"sv));
    BOOST_TEST(split_pair("a:b"sv, ':') == make_pair("a"sv, "b"sv));
    BOOST_TEST(split_pair(" a = b "sv, '=') == make_pair(" a "sv, " b "sv));

    BOOST_TEST(split_pair(""s, '=') == make_pair(""s, ""s));
    BOOST_TEST(split_pair("="s, '=') == make_pair(""s, ""s));
    BOOST_TEST(split_pair("a"s, '=') == make_pair("a"s, ""s));
    BOOST_TEST(split_pair("a="s, '=') == make_pair("a"s, ""s));
    BOOST_TEST(split_pair("=b"s, '=') == make_pair(""s, "b"s));
    BOOST_TEST(split_pair("a=b"s, '=') == make_pair("a"s, "b"s));
    BOOST_TEST(split_pair("a:b"s, ':') == make_pair("a"s, "b"s));
    BOOST_TEST(split_pair(" a = b "s, '=') == make_pair(" a "s, " b "s));
}

BOOST_AUTO_TEST_CASE(PstrlenCase)
{
    constexpr char ZeroPad[] = "foo";
    BOOST_TEST(pstrlen<'\0'>(ZeroPad, ~0) == 3UL);
    BOOST_TEST(pstrlen<'\0'>(ZeroPad, 2) == 2UL);
    BOOST_TEST(pstrlen<'\0'>(ZeroPad) == 3UL);

    constexpr char SpacePad[] = "foo ";
    BOOST_TEST(pstrlen<' '>(SpacePad, ~0) == 3UL);
    BOOST_TEST(pstrlen<' '>(SpacePad, 2) == 2UL);
    BOOST_TEST(pstrlen<' '>(SpacePad) == 3UL);
}

BOOST_AUTO_TEST_CASE(PstrcpyCase)
{
    struct TOOLBOX_PACKED {
        char data[8];
        char canary;
    } buf{};

    auto clear = [&]() { memset(buf.data, '#', sizeof(buf.data) + 1); };

    clear();
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / C-String / Explicit Length.
    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobar", sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobarbaz", sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / C-String / Implied Length.
    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobar") == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobarbaz") == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / String View / Explicit Length.
    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobar"sv, sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobarbaz"sv, sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / String View / Implied Length.
    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobar"sv) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<'\0'>(buf.data, "foobarbaz"sv) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / C-String / Explicit Length.
    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobar", sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobarbaz", sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / C-String / Implied Length.
    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobar") == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobarbaz") == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / String View / Explicit Length.
    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobar"sv, sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobarbaz"sv, sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / String View / Implied Length.
    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobar"sv) == 6UL);
    BOOST_TEST(memcmp(buf.data, "foobar  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpy<' '>(buf.data, "foobarbaz"sv) == 8UL);
    BOOST_TEST(memcmp(buf.data, "foobarba", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');
}

BOOST_AUTO_TEST_CASE(PstrcpyidCase)
{
    struct TOOLBOX_PACKED {
        char data[8];
        char canary;
    } buf{};

    auto clear = [&]() { memset(buf.data, '#', sizeof(buf.data) + 1); };

    clear();
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / Explicit Length.
    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, 123456, sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "123456\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, -1234567, sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "-1234567", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, 123456789, sizeof(buf.data)) == 0UL);
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / Explicit Length.
    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, 123456, sizeof(buf.data)) == 6UL);
    BOOST_TEST(memcmp(buf.data, "123456  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, -1234567, sizeof(buf.data)) == 8UL);
    BOOST_TEST(memcmp(buf.data, "-1234567", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, 123456789, sizeof(buf.data)) == 0UL);
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Zero / Implied Length.
    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, 123456) == 6UL);
    BOOST_TEST(memcmp(buf.data, "123456\0\0", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, -1234567) == 8UL);
    BOOST_TEST(memcmp(buf.data, "-1234567", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<'\0'>(buf.data, 123456789) == 0UL);
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    // Space / Implied Length.
    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, 123456) == 6UL);
    BOOST_TEST(memcmp(buf.data, "123456  ", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, -1234567) == 8UL);
    BOOST_TEST(memcmp(buf.data, "-1234567", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');

    clear();
    BOOST_TEST(pstrcpyid<' '>(buf.data, 123456789) == 0UL);
    BOOST_TEST(memcmp(buf.data, "########", sizeof(buf.data)) == 0);
    BOOST_TEST(buf.canary == '#');
}

BOOST_AUTO_TEST_CASE(MakeStringCase)
{
    auto res = strconcat(1, 1.2, "foo");
    BOOST_TEST(res == "11.2foo");
}

BOOST_AUTO_TEST_CASE(FromString)
{
    BOOST_TEST(from_string<int>("42") == 42);
    BOOST_TEST(from_string<std::string>("42") == "42");
    BOOST_TEST(from_string<std::string>("42"sv) == "42");
    BOOST_TEST(from_string<std::filesystem::path>("/toolbox") == std::filesystem::path("/toolbox"));
    BOOST_TEST(from_string<std::filesystem::path>("/toolbox"sv)
               == std::filesystem::path("/toolbox"));
}

BOOST_AUTO_TEST_SUITE_END()
