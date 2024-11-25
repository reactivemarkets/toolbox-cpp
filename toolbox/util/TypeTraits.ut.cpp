// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2024 Reactive Markets Limited
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

#include "TypeTraits.hpp"
#include <vector>

using namespace toolbox::util;

static_assert(is_string_v<std::string>);
static_assert(is_string_v<const std::string>);
static_assert(is_string_v<volatile std::string>);
static_assert(is_string_v<const volatile std::string>);
static_assert(!is_string_v<std::string*>);
static_assert(!is_string_v<const std::string*>);
static_assert(!is_string_v<volatile std::string*>);
static_assert(!is_string_v<const volatile std::string*>);
static_assert(!is_string_v<std::string&>);
static_assert(!is_string_v<const std::string&>);
static_assert(!is_string_v<volatile std::string&>);
static_assert(!is_string_v<const volatile std::string&>);
static_assert(!is_string_v<void>);
static_assert(!is_string_v<int>);
static_assert(!is_string_v<std::vector<std::string>>);
static_assert(!is_string_v<char*>);
static_assert(!is_string_v<const char*>);
static_assert(!is_string_v<std::string_view>);

static_assert(is_string_view_v<std::string_view>);
static_assert(is_string_view_v<const std::string_view>);
static_assert(is_string_view_v<volatile std::string_view>);
static_assert(is_string_view_v<const volatile std::string_view>);
static_assert(!is_string_view_v<std::string_view*>);
static_assert(!is_string_view_v<const std::string_view*>);
static_assert(!is_string_view_v<volatile std::string_view*>);
static_assert(!is_string_view_v<const volatile std::string_view*>);
static_assert(!is_string_view_v<std::string_view&>);
static_assert(!is_string_view_v<const std::string_view&>);
static_assert(!is_string_view_v<volatile std::string_view&>);
static_assert(!is_string_view_v<const volatile std::string_view&>);
static_assert(!is_string_view_v<void>);
static_assert(!is_string_view_v<int>);
static_assert(!is_string_view_v<std::vector<std::string>>);
static_assert(!is_string_view_v<char*>);
static_assert(!is_string_view_v<const char*>);
static_assert(!is_string_view_v<std::string>);

static_assert(is_decay_to_cstring_v<char[]>);
static_assert(is_decay_to_cstring_v<const char[]>);
static_assert(is_decay_to_cstring_v<volatile char[]>);
static_assert(is_decay_to_cstring_v<const volatile char[]>);
static_assert(is_decay_to_cstring_v<char[5]>);
static_assert(is_decay_to_cstring_v<const char[5]>);
static_assert(is_decay_to_cstring_v<volatile char[5]>);
static_assert(is_decay_to_cstring_v<const volatile char[5]>);
static_assert(!is_decay_to_cstring_v<int[]>);
static_assert(!is_decay_to_cstring_v<const int[]>);
static_assert(!is_decay_to_cstring_v<volatile int[]>);
static_assert(!is_decay_to_cstring_v<const volatile int[]>);
static_assert(!is_decay_to_cstring_v<int[5]>);
static_assert(!is_decay_to_cstring_v<const int[5]>);
static_assert(!is_decay_to_cstring_v<volatile int[5]>);
static_assert(!is_decay_to_cstring_v<const volatile int[5]>);
static_assert(!is_decay_to_cstring_v<unsigned char[]>);
static_assert(!is_decay_to_cstring_v<const unsigned char[]>);
static_assert(!is_decay_to_cstring_v<volatile unsigned char[]>);
static_assert(!is_decay_to_cstring_v<const volatile unsigned char[]>);
static_assert(!is_decay_to_cstring_v<unsigned char[5]>);
static_assert(!is_decay_to_cstring_v<const unsigned char[5]>);
static_assert(!is_decay_to_cstring_v<volatile unsigned char[5]>);
static_assert(!is_decay_to_cstring_v<const volatile unsigned char[5]>);
static_assert(!is_decay_to_cstring_v<signed char[]>);
static_assert(!is_decay_to_cstring_v<const signed char[]>);
static_assert(!is_decay_to_cstring_v<volatile signed char[]>);
static_assert(!is_decay_to_cstring_v<const volatile signed char[]>);
static_assert(!is_decay_to_cstring_v<signed char[5]>);
static_assert(!is_decay_to_cstring_v<const signed char[5]>);
static_assert(!is_decay_to_cstring_v<volatile signed char[5]>);
static_assert(!is_decay_to_cstring_v<const volatile signed char[5]>);
static_assert(!is_decay_to_cstring_v<char>);
static_assert(!is_decay_to_cstring_v<signed char>);
static_assert(!is_decay_to_cstring_v<unsigned char>);
static_assert(!is_decay_to_cstring_v<int>);
static_assert(!is_decay_to_cstring_v<void>);
static_assert(!is_decay_to_cstring_v<double>);
static_assert(!is_decay_to_cstring_v<std::string>);
static_assert(!is_decay_to_cstring_v<std::string_view>);

static_assert(is_string_type_v<std::string>);
static_assert(!is_string_type_v<std::string*>);
static_assert(!is_string_type_v<std::string&>);

static_assert(is_string_type_v<std::string_view>);
static_assert(!is_string_type_v<std::string_view*>);
static_assert(!is_string_type_v<std::string_view&>);

static_assert(is_string_type_v<char*>);
static_assert(is_string_type_v<char* const>);
static_assert(is_string_type_v<char* volatile>);
static_assert(is_string_type_v<char* const volatile>);
static_assert(is_string_type_v<const char*>);
static_assert(is_string_type_v<const char* const>);
static_assert(is_string_type_v<const char* volatile>);
static_assert(is_string_type_v<const char* const volatile>);
static_assert(is_string_type_v<volatile char*>);
static_assert(is_string_type_v<volatile char* const>);
static_assert(is_string_type_v<volatile char* volatile>);
static_assert(is_string_type_v<volatile char* const volatile>);
static_assert(is_string_type_v<const volatile char*>);
static_assert(is_string_type_v<const volatile char* const>);
static_assert(is_string_type_v<const volatile char* volatile>);
static_assert(is_string_type_v<const volatile char* const volatile>);
static_assert(is_string_type_v<char[]>);
static_assert(is_string_type_v<const char[]>);
static_assert(is_string_type_v<volatile char[]>);
static_assert(is_string_type_v<const volatile char[]>);
static_assert(is_string_type_v<char[5]>);
static_assert(is_string_type_v<const char[5]>);
static_assert(is_string_type_v<volatile char[5]>);
static_assert(is_string_type_v<const volatile char[5]>);
static_assert(!is_string_type_v<void>);
static_assert(!is_string_type_v<int>);
static_assert(!is_string_type_v<float>);
static_assert(!is_string_type_v<unsigned char*>);
static_assert(!is_string_type_v<unsigned char* const>);
static_assert(!is_string_type_v<unsigned char* volatile>);
static_assert(!is_string_type_v<unsigned char* const volatile>);
static_assert(!is_string_type_v<const unsigned char*>);
static_assert(!is_string_type_v<const unsigned char* const>);
static_assert(!is_string_type_v<const unsigned char* volatile>);
static_assert(!is_string_type_v<const unsigned char* const volatile>);
static_assert(!is_string_type_v<volatile unsigned char*>);
static_assert(!is_string_type_v<volatile unsigned char* const>);
static_assert(!is_string_type_v<volatile unsigned char* volatile>);
static_assert(!is_string_type_v<volatile unsigned char* const volatile>);
static_assert(!is_string_type_v<const volatile unsigned char*>);
static_assert(!is_string_type_v<const volatile unsigned char* const>);
static_assert(!is_string_type_v<const volatile unsigned char* volatile>);
static_assert(!is_string_type_v<const volatile unsigned char* const volatile>);
static_assert(!is_string_type_v<unsigned char[]>);
static_assert(!is_string_type_v<const unsigned char[]>);
static_assert(!is_string_type_v<volatile unsigned char[]>);
static_assert(!is_string_type_v<const volatile unsigned char[]>);
static_assert(!is_string_type_v<unsigned char[5]>);
static_assert(!is_string_type_v<const unsigned char[5]>);
static_assert(!is_string_type_v<volatile unsigned char[5]>);
static_assert(!is_string_type_v<const volatile unsigned char[5]>);
static_assert(!is_string_type_v<signed char[]>);
static_assert(!is_string_type_v<const signed char[]>);
static_assert(!is_string_type_v<volatile signed char[]>);
static_assert(!is_string_type_v<const volatile signed char[]>);
static_assert(!is_string_type_v<signed char[5]>);
static_assert(!is_string_type_v<const signed char[5]>);
static_assert(!is_string_type_v<volatile signed char[5]>);
static_assert(!is_string_type_v<const volatile signed char[5]>);
