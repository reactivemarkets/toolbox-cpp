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

#ifndef TOOLBOX_IO_INOTIFY_HPP
#define TOOLBOX_IO_INOTIFY_HPP

#include <toolbox/io/Reactor.hpp>

#include <sys/inotify.h>

#include <filesystem>
#include <map>

namespace toolbox {
inline namespace io {

struct WatchFile {
    int fd{-1}, wd{-1};
};

constexpr bool operator==(WatchFile lhs, WatchFile rhs)
{
    return lhs.fd == rhs.fd && lhs.wd == rhs.wd;
}

constexpr bool operator!=(WatchFile lhs, WatchFile rhs)
{
    return !(lhs == rhs);
}

struct WatchFilePolicy {
    using Id = WatchFile;
    static constexpr WatchFile invalid() noexcept { return WatchFile{}; }
    static void close(WatchFile wf) noexcept { ::inotify_rm_watch(wf.fd, wf.wd); }
};

using WatchFileHandle = BasicHandle<WatchFilePolicy>;

/// Initialise an inotify instance.
inline FileHandle inotify_init(int flags, std::error_code& ec) noexcept
{
    const auto fd = ::inotify_init1(flags);
    if (fd < 0) {
        ec = make_error(errno);
    }
    return fd;
}

/// Initialise an inotify instance.
inline FileHandle inotify_init(std::error_code& ec) noexcept
{
    return inotify_init(0, ec);
}

/// Initialise an inotify instance and returns a file descriptor associated with a new inotify
/// event queue.
inline FileHandle inotify_init(int flags = 0)
{
    const auto fd = ::inotify_init1(flags);
    if (fd < 0) {
        throw std::system_error{make_error(errno), "inotify_init1"};
    }
    return fd;
}

/// Add a watch to an initialised inotify instance.
inline WatchFileHandle inotify_add_watch(int fd, const char* path, std::uint32_t mask,
                                         std::error_code& ec) noexcept
{
    const auto wd = ::inotify_add_watch(fd, path, mask);
    if (wd < 0) {
        ec = make_error(errno);
        return {};
    }
    return WatchFile{.fd = fd, .wd = wd};
}

/// Add a watch to an initialised inotify instance.
inline WatchFileHandle inotify_add_watch(int fd, const char* path, std::uint32_t mask)
{
    const auto wd = ::inotify_add_watch(fd, path, mask);
    if (wd < 0) {
        throw std::system_error{make_error(errno), "inotify_add_watch"};
    }
    return WatchFile{.fd = fd, .wd = wd};
}

/// Inotify provides a simplified interface to an inotify instance.
class TOOLBOX_API Inotify {
  public:
    explicit Inotify(int flags = 0)
    : fh_{inotify_init(flags)}
    {
    }
    ~Inotify() = default;

    // Copy.
    Inotify(const Inotify&) = delete;
    Inotify& operator=(const Inotify&) = delete;

    // Move.
    Inotify(Inotify&&) noexcept = default;
    Inotify& operator=(Inotify&&) noexcept = default;

    int fd() const noexcept { return fh_.get(); }

    /// Add a watch to an initialised inotify instance.
    [[nodiscard]] WatchFileHandle add_watch(const char* path, std::uint32_t mask,
                                            std::error_code& ec) noexcept
    {
        return inotify_add_watch(fh_.get(), path, mask, ec);
    }
    /// Add a watch to an initialised inotify instance.
    [[nodiscard]] WatchFileHandle add_watch(const char* path, std::uint32_t mask)
    {
        return inotify_add_watch(fh_.get(), path, mask);
    }

  private:
    FileHandle fh_;
};

/// FileWatcher watches for changes to files.
class TOOLBOX_API FileWatcher {
  public:
    using Path = std::filesystem::path;
    using Slot = BasicSlot<const Path&, int, std::uint32_t>;

    FileWatcher(Reactor& r, Inotify& inotify);
    ~FileWatcher() = default;

    // Copy.
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    // Move.
    FileWatcher(FileWatcher&&) noexcept = default;
    FileWatcher& operator=(FileWatcher&&) noexcept = default;

    void watch(const Path& path, Slot slot, std::uint32_t mask = IN_ALL_EVENTS);

  private:
    struct Watch {
        Path path;
        Slot slot;
        WatchFileHandle wh;
    };

    void on_inotify(CyclTime /*now*/, int fd, unsigned events);

    Inotify* inotify_;
    Reactor::Handle sub_;
    std::map<Path, Watch> path_index_;
    std::map<int, const Watch*> wd_index_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_INOTIFY_HPP
