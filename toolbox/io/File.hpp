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

#ifndef TOOLBOX_IO_FILE_HPP
#define TOOLBOX_IO_FILE_HPP

#include <toolbox/io/Buffer.hpp>
#include <toolbox/io/Handle.hpp>
#include <toolbox/sys/Error.hpp>

#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

namespace toolbox {
namespace os {

/// Open and possibly create a file.
inline FileHandle open(const char* path, int flags, mode_t mode, std::error_code& ec) noexcept
{
    const auto fd = ::open(path, flags, mode);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Open and possibly create a file.
inline FileHandle open(const char* path, int flags, mode_t mode)
{
    const auto fd = ::open(path, flags, mode);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "open"};
    }
    return fd;
}

/// Open and possibly create a file.
inline FileHandle open(const char* path, int flags, std::error_code& ec) noexcept
{
    const auto fd = ::open(path, flags);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Open and possibly create a file.
inline FileHandle open(const char* path, int flags)
{
    const auto fd = ::open(path, flags);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "open"};
    }
    return fd;
}

/// Create pipe.
inline std::pair<FileHandle, FileHandle> pipe2(int flags, std::error_code& ec) noexcept
{
    int pipefd[2];
    if (::pipe2(pipefd, flags) < 0) {
        ec = make_sys_error(errno);
    }
    return {FileHandle{pipefd[0]}, FileHandle{pipefd[1]}};
}

/// Create pipe.
inline std::pair<FileHandle, FileHandle> pipe2(int flags)
{
    int pipefd[2];
    if (::pipe2(pipefd, flags) < 0) {
        throw std::system_error{make_sys_error(errno), "pipe2"};
    }
    return {FileHandle{pipefd[0]}, FileHandle{pipefd[1]}};
}

/// Get file status.
inline void fstat(int fd, struct stat& statbuf, std::error_code& ec) noexcept
{
    const auto ret = ::fstat(fd, &statbuf);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
}

/// Get file status.
inline void fstat(int fd, struct stat& statbuf)
{
    const auto ret = ::fstat(fd, &statbuf);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "fstat"};
    }
}

/// Truncate a file to a specified length.
inline void ftruncate(int fd, off_t length, std::error_code& ec) noexcept
{
    const auto ret = ::ftruncate(fd, length);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
}

/// Truncate a file to a specified length.
inline void ftruncate(int fd, off_t length)
{
    const auto ret = ::ftruncate(fd, length);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "ftruncate"};
    }
}

/// Read from a file descriptor.
inline ssize_t read(int fd, void* buf, std::size_t len, std::error_code& ec) noexcept
{
    const auto ret = ::read(fd, buf, len);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Read from a file descriptor.
inline std::size_t read(int fd, void* buf, std::size_t len)
{
    const auto ret = ::read(fd, buf, len);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "read"};
    }
    return ret;
}

/// Read from a file descriptor.
inline ssize_t read(int fd, MutableBuffer buf, std::error_code& ec) noexcept
{
    return read(fd, buffer_cast<void*>(buf), buffer_size(buf), ec);
}

/// Read from a file descriptor.
inline std::size_t read(int fd, MutableBuffer buf) noexcept
{
    return read(fd, buffer_cast<void*>(buf), buffer_size(buf));
}

/// Write to a file descriptor.
inline ssize_t write(int fd, const void* buf, std::size_t len, std::error_code& ec) noexcept
{
    const auto ret = ::write(fd, buf, len);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Write to a file descriptor.
inline std::size_t write(int fd, const void* buf, std::size_t len)
{
    const auto ret = ::write(fd, buf, len);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "write"};
    }
    return ret;
}

/// Write to a file descriptor.
inline ssize_t write(int fd, ConstBuffer buf, std::error_code& ec) noexcept
{
    return write(fd, buffer_cast<const void*>(buf), buffer_size(buf), ec);
}

/// Write to a file descriptor.
inline std::size_t write(int fd, ConstBuffer buf)
{
    return write(fd, buffer_cast<const void*>(buf), buffer_size(buf));
}

/// File control.
inline int fcntl(int fd, int cmd, std::error_code& ec) noexcept
{
    const auto ret = ::fcntl(fd, cmd);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// File control.
template <typename ArgT>
inline int fcntl(int fd, int cmd, ArgT arg, std::error_code& ec) noexcept
{
    const auto ret = ::fcntl(fd, cmd, arg);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// File control.
inline int fcntl(int fd, int cmd)
{
    const auto ret = ::fcntl(fd, cmd);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "fcntl"};
    }
    return ret;
}

/// File control.
template <typename ArgT>
inline int fcntl(int fd, int cmd, ArgT arg)
{
    const auto ret = ::fcntl(fd, cmd, arg);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "fcntl"};
    }
    return ret;
}

} // namespace os
inline namespace io {

/// Get file size.
inline std::size_t file_size(int fd)
{
    struct stat st;
    os::fstat(fd, st);
    return st.st_size;
}

/// Get current file mode.
inline mode_t file_mode() noexcept
{
    mode_t mode{umask(0)};
    umask(mode);
    return mode;
}

inline void set_non_block(int fd, std::error_code& ec) noexcept
{
    os::fcntl(fd, F_SETFL, O_NONBLOCK, ec);
}

inline void set_non_block(int fd)
{
    os::fcntl(fd, F_SETFL, O_NONBLOCK);
}

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_FILE_HPP
