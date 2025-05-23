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

#include <toolbox/io.hpp>
#include <toolbox/net.hpp>
#include <toolbox/sys.hpp>
#include <toolbox/util.hpp>

using namespace toolbox;

namespace {

using Path = FileWatcher::Path;

/// Example configuration.
struct Config {
    Path path;
};

using ConfigPtr = std::unique_ptr<Config>;
using ConfigFuture = std::future<ConfigPtr>;

class ConfigLoader {
    using Task = std::packaged_task<ConfigPtr()>;

  public:
    explicit ConfigLoader(const Path& path)
    : path_{path}
    {
    }
    ~ConfigLoader() = default;

    // Copy.
    ConfigLoader(const ConfigLoader& rhs) = delete;
    ConfigLoader& operator=(const ConfigLoader& rhs) = delete;

    // Move.
    ConfigLoader(ConfigLoader&&) = delete;
    ConfigLoader& operator=(ConfigLoader&&) = delete;

    const Path& path() const noexcept { return path_; }
    bool run()
    {
        return tq_.run([](Task&& t) { t(); });
    }
    void stop() { tq_.stop(); }
    void clear()
    {
        // This will unblock waiters by throwing a "broken promise" exception.
        return tq_.clear();
    }
    ConfigFuture load()
    {
        Task task{[this]() -> ConfigPtr {
            // TODO: load config from filesystem.
            return ConfigPtr{new Config{.path = path_}};
        }};
        auto future{task.get_future()};
        tq_.push(std::move(task));
        return future;
    }

  private:
    const Path path_;
    TaskQueue<Task> tq_;
};

class App {
  public:
    App(CyclTime now, Reactor& reactor, Inotify& inotify, ConfigLoader& config_loader)
    : config_loader_{config_loader}
    , file_watcher_{reactor, inotify}
    // Immediate and then at 5s intervals.
    , tmr_{reactor.timer(now.mono_time(), 5s, Priority::Low, bind<&App::on_timer>(this))}
    {
        // Bind on_config_update slot to the foo.conf configuration file.
        file_watcher_.watch(config_loader.path(), bind<&App::on_config_update>(this),
                            IN_CLOSE_WRITE | IN_DELETE_SELF);
        // Trigger initial config load.
        config_future_ = config_loader_.load();
    }
    ~App() = default;

  private:
    void on_config_update(const Path& path, int /*wd*/, std::uint32_t event_mask)
    {
        if (path != config_loader_.path()) {
            return;
        }
        if ((event_mask & IN_CLOSE_WRITE) == IN_CLOSE_WRITE) {
            TOOLBOX_INFO << "config " << path << " updated (IN_CLOSE_WRITE)";
        }
        if ((event_mask & IN_DELETE_SELF) == IN_DELETE_SELF) {
            TOOLBOX_INFO << "config " << path << " updated (IN_DELETE_SELF)";

            // On Kubernetes, inotify only receives the IN_DELETE_SELF event on config maps.
            // This deletion event breaks the inotify watch and so code needs to handle
            // re-establishing the watch every time the file is updated.

            file_watcher_.watch(path, bind<&App::on_config_update>(this),
                                IN_CLOSE_WRITE | IN_DELETE_SELF);
        }
        config_future_ = config_loader_.load();
    }
    void on_timer(CyclTime /*now*/, Timer& /*tmr*/)
    {
        if (config_future_.valid()) {
            if (!is_ready(config_future_)) {
                TOOLBOX_INFO << "config pending";
                return;
            }
            try {
                config_ = config_future_.get();
            } catch (const std::exception& e) {
                TOOLBOX_ERROR << "could not load config: " << e.what();
                config_future_ = config_loader_.load();
                return;
            }
            TOOLBOX_INFO << "config loaded: " << config_->path;
        }
    }

    ConfigLoader& config_loader_;
    FileWatcher file_watcher_;
    Timer tmr_;
    ConfigFuture config_future_;
    ConfigPtr config_;
};
} // namespace

int main()
{
    using namespace std::literals::string_literals;
    int ret = 1;
    try {

        const auto start_time{CyclTime::now()};
        Reactor reactor{};
        Inotify inotify{IN_NONBLOCK};
        ConfigLoader config_loader{"/tmp/inotify_test/files/foo.txt"};
        App app{start_time, reactor, inotify, config_loader};

        // Start service threads.
        pthread_setname_np(pthread_self(), "main");
        ReactorRunner reactor_runner{reactor, 100, "reactor"s};
        Runner config_loader_runner{config_loader, "config_loader"s};

        // Wait for termination.
        SigWait sig_wait;
        for (;;) {
            switch (const auto sig = sig_wait()) {
            case SIGHUP:
                TOOLBOX_INFO << "received SIGHUP";
                continue;
            case SIGINT:
                TOOLBOX_INFO << "received SIGINT";
                break;
            case SIGTERM:
                TOOLBOX_INFO << "received SIGTERM";
                break;
            default:
                TOOLBOX_INFO << "received signal: " << sig;
                continue;
            }
            break;
        }
        ret = 0;

    } catch (const std::exception& e) {
        TOOLBOX_ERROR << "exception on main thread: " << e.what();
    }
    return ret;
}
