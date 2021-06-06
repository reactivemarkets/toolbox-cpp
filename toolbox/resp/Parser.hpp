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

#ifndef TOOLBOX_RESP_PARSER_HPP
#define TOOLBOX_RESP_PARSER_HPP

#include <toolbox/resp/Exception.hpp>

#include <toolbox/util/Enum.hpp>
#include <toolbox/util/Finally.hpp>

#include <boost/container/small_vector.hpp>

#include <stack>

namespace toolbox {
inline namespace resp {

/// Type is an enumeration of the RESP data-types currently supported.
enum class Type : char {
    None = '\0',
    CommandLine = '\1',
    /// For Simple Strings the first byte of the reply is "+".
    SimpleString = '+',
    /// For Errors the first byte of the reply is "-".
    Error = '-',
    /// For Integers the first byte of the reply is ":".
    Integer = ':',
    /// For Bulk Strings the first byte of the reply is "$".
    BulkString = '$',
    /// For Arrays the first byte of the reply is "*".
    Array = '*',
};

/// BasicParser is a class template for RESP (REdis Serialization Protocol) parsers.
template <typename DerivedT>
class BasicParser {
    using Stack = std::stack<int, boost::container::small_vector<int, 8>>;

  public:
    BasicParser() = default;
    ~BasicParser() = default;

    // Copy.
    BasicParser(const BasicParser&) = default;
    BasicParser& operator=(const BasicParser&) = default;

    // Move.
    BasicParser(BasicParser&&) noexcept = default;
    BasicParser& operator=(BasicParser&&) noexcept = default;

  protected:
    void put(char c)
    {
        switch (type_) {
        case Type::None:
            switch (c) {
            case unbox(Type::SimpleString):
            case unbox(Type::Error):
            case unbox(Type::Integer):
            case unbox(Type::BulkString):
            case unbox(Type::Array):
                type_ = Type{c};
                break;
            default:
                type_ = Type::CommandLine;
                tok_ += c;
                break;
            }
            return;
        case Type::CommandLine:
            put_command_line(c);
            break;
        case Type::SimpleString:
            put_simple_string(c);
            break;
        case Type::Error:
            put_error(c);
            break;
        case Type::Integer:
            put_integer(c);
            break;
        case Type::BulkString:
            put_bulk_string(c);
            break;
        case Type::Array:
            put_array(c);
            break;
        }
    }

  private:
    void put_command_line(char c)
    {
        put_string(c, [this]() { static_cast<DerivedT*>(this)->on_resp_command_line(tok_); });
    }
    void put_simple_string(char c)
    {
        put_string(c, [this]() { static_cast<DerivedT*>(this)->on_resp_string(tok_); });
    }
    void put_error(char c)
    {
        put_string(c, [this]() { static_cast<DerivedT*>(this)->on_resp_error(tok_); });
    }
    void put_integer(char c)
    {
        if (sign_ == 0) {
            if (c == '+') {
                sign_ = 1;
                return;
            }
            if (c == '-') {
                sign_ = -1;
                return;
            }
            // Imply positive sign.
            sign_ = 1;
        }
        if (c == '\r') {
            // Ignore.
            return;
        }
        if (c != '\n') {
            if (c < '0' || c > '9') {
                // Fatal protocol exception.
                throw Exception{"invalid integer"};
            }
            num_ = num_ * 10 + (c - '0');
            return;
        }
        flush([this]() { static_cast<DerivedT*>(this)->on_resp_integer(sign_ * num_); });
    }
    void put_bulk_string(char c)
    {
        // TODO: add support for null bulk strings.
        if (sign_ == 0) {
            switch (c) {
            case '\r':
                // Ignore.
                break;
            case '\n':
                sign_ = 1;
                break;
            default:
                if (c < '0' || c > '9') {
                    // Fatal protocol exception.
                    throw Exception{"invalid length"};
                }
                num_ = num_ * 10 + (c - '0');
                break;
            }
            return;
        }
        if (num_ > 0) {
            tok_ += c;
            --num_;
            return;
        }
        // End of line.
        if (c == '\r') {
            // Ignore.
            return;
        }
        if (c != '\n') {
            // Fatal protocol exception.
            throw Exception{"invalid bulk string"};
        }
        flush([this]() { static_cast<DerivedT*>(this)->on_resp_string(tok_); });
    }
    void put_array(char c)
    {
        if (c == '\r') {
            // Ignore.
            return;
        }
        if (c != '\n') {
            if (c < '0' || c > '9') {
                // Fatal protocol exception.
                throw Exception{"invalid length"};
            }
            num_ = num_ * 10 + (c - '0');
            return;
        }
        bool ok{false};
        int popped{0};
        if (num_ > 0) {
            stack_.push(num_);
        } else {
            popped = 1 + pop_if_end();
        }
        const auto finally = make_finally([&]() noexcept {
            if (!ok) {
                if (!is_top_level()) {
                    bad_ = true;
                } else if (popped > 0) {
                    assert(!ok && is_top_level() && popped > 0);
                    // Callback must be noexcept.
                    static_cast<DerivedT*>(this)->on_resp_reset(); // noexcept
                }
            }
            clear_tok();
        });
        if (bad_) [[unlikely]] {
            if (is_top_level() && popped > 0) {
                bad_ = false;
                static_cast<DerivedT*>(this)->on_resp_reset(); // noexcept
            }
        } else {
            static_cast<DerivedT*>(this)->on_resp_array_begin(num_);
            for (int i{0}; i < popped; ++i) {
                static_cast<DerivedT*>(this)->on_resp_array_end();
            }
        }
        ok = true;
    }
    template <typename FnT>
    void put_string(char c, FnT fn)
    {
        if (c != '\n') {
            tok_ += c;
            return;
        }
        if (!tok_.empty() && tok_.back() == '\r') {
            tok_.pop_back();
        }
        flush(fn);
    }
    template <typename FnT>
    void flush(FnT fn)
    {
        bool ok{false};
        const int popped{pop_if_end()};
        const auto finally = make_finally([&]() noexcept {
            if (!ok) {
                if (!is_top_level()) {
                    bad_ = true;
                } else if (popped > 0) {
                    assert(!ok && is_top_level() && popped > 0);
                    // Callback must be noexcept.
                    static_cast<DerivedT*>(this)->on_resp_reset(); // noexcept
                }
            }
            clear_tok();
        });
        if (bad_) [[unlikely]] {
            if (is_top_level() && popped > 0) {
                bad_ = false;
                static_cast<DerivedT*>(this)->on_resp_reset(); // noexcept
            }
        } else {
            fn();
            for (int i{0}; i < popped; ++i) {
                static_cast<DerivedT*>(this)->on_resp_array_end();
            }
        }
        ok = true;
    }
    bool is_top_level() const noexcept { return stack_.empty(); }
    void clear_tok() noexcept
    {
        type_ = Type::None;
        sign_ = 0;
        num_ = 0;
        tok_.clear();
    }
    /// Returns the number of levels in the stack that were popped or unwound.
    int pop_if_end() noexcept
    {
        int n{0};
        while (!stack_.empty() && --stack_.top() == 0) {
            stack_.pop();
            ++n;
        }
        return n;
    }
    /// The bad flag is set when an application exception occurs while processing an array.
    /// The bad flag is reset when the parser has finished processing the array.
    bool bad_{false};
    Type type_{Type::None};
    int sign_{0};
    std::int64_t num_{0};
    std::string tok_;
    Stack stack_;
};

} // namespace resp
} // namespace toolbox

#endif // TOOLBOX_RESP_PARSER_HPP
