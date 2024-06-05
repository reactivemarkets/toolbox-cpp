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

FileWatcher::FileWatcher(Reactor& r, Inotify& inotify)
: inotify_{&inotify}
, sub_{r.subscribe(inotify.fd(), EpollIn, toolbox::bind<&FileWatcher::on_inotify>(this))}
{
}

void FileWatcher::watch(const Path& path, Slot slot, std::uint32_t mask)
{
    auto new_wh{inotify_->add_watch(path.c_str(), mask)};
    auto& watch{path_index_[path]};
    if (watch.wh == new_wh) {
        // Update entries for existing watch descriptor.
        watch.slot = slot;
        return;
    }
    wd_index_[new_wh.get().wd] = &watch;
    // Update watch.
    // N.B. in the unlikely event that an exception is thrown, the dangling entry in wd_index_
    // would be cleaned-up by the garbage collection logic in on_inotify function below.
    watch = Watch{.path = path, .slot = slot, .wh = std::move(new_wh)};
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
            wd_index_.clear();
            path_index_.clear();
            sub_.reset();
            return;
        }
        for (std::size_t i{0}; i < size;) {
            inotify_event* event{reinterpret_cast<inotify_event*>(&buf[i])};
            const auto it{wd_index_.find(event->wd)};
            if (it != wd_index_.end()) {
                it->second->slot(it->second->path, event->wd, event->mask);
            }
            i += sizeof(struct inotify_event) + event->len;
        }
        // Perform garbage collection on dangling entries in wd_index_. These entries refer to
        // watch handles in path_index_ that are now associated with different watch descriptors.
        // This situation typically arises when a watch is updated, causing the path to point to
        // a new inode.
        //
        // Note: This garbage collection occurs after all inotify events have been dispatched.
        // This ensures that events for old watch descriptors are still dispatched, even if the
        // path was rebound during the iteration.
        //
        for (auto it{wd_index_.cbegin()}; it != wd_index_.cend();) {
            if (it->first != it->second->wh.get().wd) {
                it = wd_index_.erase(it);
            } else {
                ++it;
            }
        }
    }
}

} // namespace io
} // namespace toolbox
