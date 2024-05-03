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

#include "Inotify.hpp"

namespace toolbox {
inline namespace io {

FileWatcher::FileWatcher(Reactor& r, Inotify& inotify, const Path& dir_name, std::uint32_t mask)
: watch_dir_{inotify.add_watch(dir_name.c_str(), mask)}
, sub_{r.subscribe(inotify.fd(), EpollIn, toolbox::bind<&FileWatcher::on_inotify>(this))}
{
}

void FileWatcher::on_inotify(CyclTime /*now*/, int fd, unsigned events)
{
    if (events & (EpollIn | EpollHup)) {
        char buf[1024 * (sizeof(struct inotify_event) + NAME_MAX + 1)];
        const auto size{os::read(fd, buf, sizeof(buf))};
        if (size == 0) {
            // FIXME: a zero return normally indicates end of file.
            //  Can this happen on an inotify descriptor?
            //  And if so, how should the application behave?
            watch_dir_.reset();
            sub_.reset();
            return;
        }
        for (std::size_t i{0}; i < size;) {
            inotify_event* event{reinterpret_cast<inotify_event*>(&buf[i])};
            const Path file_name{event->name};
            const auto it{slot_map_.find(file_name)};
            if (it != slot_map_.end()) {
                it->second(file_name, event->mask);
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }
}

} // namespace io
} // namespace toolbox
