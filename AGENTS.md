# AGENTS.md

This file provides guidance when working with code in this repository.

## What this is

The Reactive C++ Toolbox: a C++26 library of low-level components for building efficient,
asynchronous, single-threaded-reactor-style network applications on Linux (AMD64, ARM, ARM32).
It is a foundational library used by other Reactive Markets services, not an application itself.

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make -j all test install
```

Requires CMake, GCC or Clang with C++20/23 support, and Boost >= 1.88 (`date_time`,
`unit_test_framework` components). CMake defaults `CMAKE_BUILD_TYPE` to `RelWithDebInfo` if unset.

Useful CMake options (pass as `-D<option>=<value>`):
- `REACTIVE_BUILD_SHARED` (default `ON`) — build shared libs in addition to static.
- `REACTIVE_BUILD_ARCH` (default `native`) — `-march` target.
- `ENABLE_CLANG_TIDY` (default `ON`) — clang-tidy runs as part of the build when available; set
  `OFF` to disable (CI disables it for the build matrix and runs formatting as a separate lint job).
- Non-default `CMAKE_BUILD_TYPE` values beyond the usual ones: `Perf` (release + frame pointers, for
  `perf`), `Coverage` (adds gcovr target), `Asan`, `Tsan`.

Everything links against Boost with `-DBOOST_ASIO_HEADER_ONLY=1 -DBOOST_ASIO_DISABLE_THREADS=1`.

`tb-doc` (Doxygen documentation) and `tb-image` (Graphviz module dependency diagrams, a dependency of
`tb-doc`) are not part of the default build — build them explicitly (`make tb-doc install`) to
generate and install them; see `doc/BuildFromSource.md`.

## Testing

Tests use Boost.Test and are built into a single `tb-core-test` binary (one executable, one
`Boost.Test` suite per source file under `toolbox/**/*.ut.cpp`). CTest registers one test per suite.

```bash
cd build
make test                                    # run full suite via ctest
ctest -R 'util/String::StringSuite'          # run one suite (name is dir::NameSuite)
./lib/tb-core-test -l error -t StringSuite   # run one suite directly against the test binary
./lib/tb-core-test --run_test=StringSuite/some_case   # run one case
```

To add a test for `toolbox/xxx/Yyy.cpp`, create `toolbox/xxx/Yyy.ut.cpp` and add it to both
`test_SOURCES` and (if it's not test-only) `lib_SOURCES` in `toolbox/CMakeLists.txt` — CTest
registration and header install rules are generated from these lists via `foreach`, so nothing else
needs updating.

## Formatting and linting

```bash
cmake --build build --target clang-format   # reformat entire tree in place (or: make clang-format)
```

CI runs `clang-format` as a required lint gate (fails if it produces a diff) and separately builds
with both GCC and Clang in Debug and Release. `.clang-tidy` enables a narrow, deliberate checklist
(`bugprone-use-after-move`, `modernize-use-equals-default/delete`, `modernize-use-override`,
`readability-braces-around-statements`, `readability-deleted-default`,
`readability-identifier-naming`) treated as `WarningsAsErrors`; don't add checks beyond this list
without reason. clang-tidy runs automatically during the CMake build when the compiler is Clang and
`ENABLE_CLANG_TIDY=ON`.

## Coding conventions

Full detail in `CONTRIBUTING.md`; the essentials:

- Filenames: CamelCase; `.hpp` headers, `.cpp` sources, `.ut.cpp` unit tests.
- Types and integer constants: CamelCase, upper-case first letter. Functions, variables, non-integer
  constants: snake_case. Private data members: trailing underscore (`fd_`).
- Don't prefix type names with the namespace name — disambiguate via namespace instead (`http::Serv`,
  not `HttpServ`).
- Class layout: `public` before `protected` before `private`; within a section, order is big-six ctors/
  dtor/assignment, then const member functions, then non-const, then data.
- Use `assert` to document contracts/invariants.
- Exception messages: lower-case, no trailing punctuation (`throw runtime_error{"something bad"}`),
  since they're printed after other context (see `TOOLBOX_ERROR << "..." << e.what();`).
- Library code should rarely log; logging is for long-running services reporting exceptional
  conditions and lifecycle events, not for tracing normal control flow.
- 4-space indent, spaces not tabs, 100-column limit — enforced by `clang-format`, don't hand-format.

## Architecture

Code lives under `toolbox/<module>/`, one directory per module, each with a top-level aggregator
header `toolbox/<module>.hpp` that includes every public header in that module (e.g. `toolbox/io.hpp`
pulls in all of `toolbox/io/*.hpp`). When adding a new public header to a module, add it to the
module's aggregator header and it will automatically be picked up for install by the `foreach` loop
in `toolbox/CMakeLists.txt` (as long as a same-named `.cpp` is also listed in `lib_SOURCES`).

Modules, roughly bottom-up in dependency terms:

- **`util`** — foundation: `Slot` (fixed-size type-erased callback used everywhere instead of
  `std::function`, built by binding a free function, lambda, or member function pointer via
  `bind<...>()`), `Struct`/`Variant`/`Traits`/`TypeTraits`, `RingBuffer`, `RobinHood` (open-addressing
  hash map, wraps `contrib/robin_hood.h`), `Allocator`/`Storage` (custom allocation), `Tokeniser`,
  `String`/`StringBuf`/`Stream`, `TaskQueue`, `Config`/`Options`/`Argv` (config/CLI parsing), `Enum`,
  `Math`, `Alarm`.
- **`sys`** — OS/process concerns: `Time`/`Date` (monotonic vs wall clock types — see `MonoTime`,
  `CyclTime` used throughout the reactor), `Log`/`Logger`/`Trace` (logging, `TOOLBOX_ERROR` et al.),
  `Signal`, `Daemon`, `PidFile`, `Thread`, `System`, `Runner`.
- **`io`** — the reactor: `Reactor` (single-threaded epoll-based event loop; owns per-fd subscriptions
  via `Reactor::Handle`, a timer-wheel-like `TimerQueue`/`Timer` per `Priority`, and `Hook`s that run
  at defined points in the poll cycle), `Epoll`, `Waker`/`EventFd` (cross-thread wakeup), `Buffer`,
  `Handle` (generic RAII fd wrapper distinct from `Reactor::Handle`), `Disposer`, `Inotify`, `File`.
  I/O callbacks are `IoSlot = BasicSlot<void(CyclTime, int, unsigned)>`; timer callbacks are
  `TimerSlot`. Two I/O priorities (`High`/`Low`) let latency-sensitive fds be dispatched preferentially.
- **`net`** — sockets built on top of `io`: `Socket`, `StreamSock`/`DgramSock`/`McastSock`,
  `StreamAcceptor`/`StreamConnector`, `Endpoint`/`IpAddr`, `Resolver`, `RateLimit`, `Frame` (framing
  for stream protocols), `Protocol`.
- **`http`** — HTTP server built on `io`/`net`: `Serv`/`Conn`/`App` (server, connection, request-handler
  app abstraction), `Parser` (wraps `contrib/http_parser.c`), `Request`/`Types`/`Url`.
- **`resp`** — RESP (Redis serialization protocol) `Parser`.
- **`hdr`** — HDR histogram implementation (`Histogram`, `Iterator`, `Utility`) for latency recording.
- **`bm`** — a tiny in-house microbenchmark harness (`Benchmark`, `Suite`, `Context`, `Range`,
  `Record`) built as its own library (`tb-bm`) on top of `tb-core`; used by everything under `bench/`.

`toolbox/contrib/` holds vendored third-party single-file dependencies (http_parser, robin_hood,
fast_float, magic_enum, flopen, pidfile) — treat as read-only/upstream, not house style.

Two library targets come out of the build: `tb-core` (everything except `bm`) and `tb-bm` (the
benchmark harness, linked against `tb-core`). Each is built both `-static` and `-shared` when
`TOOLBOX_BUILD_SHARED` is on; `${tb_core_LIBRARY}`/`${tb_bm_LIBRARY}` in CMake resolve to whichever
variant is preferred.

`example/` contains small standalone programs (`EchoServ`/`EchoClnt`, `HttpServ`, `Inotify`) that
double as usage references for the reactor/net/http APIs. `bench/` contains one microbenchmark binary
per area of interest (`tb-<name>-bench`), all built on the `bm` harness.

## Key idioms to know before editing

- **Reactor pattern, not threads-per-connection.** `io::Reactor` is a single-threaded epoll loop.
  Handlers are non-owning `Slot`s bound to fd events or timers; RAII `Handle`s (both
  `Reactor::Handle` for subscriptions and the generic `io::Handle` for fds) unsubscribe/close on
  destruction — prefer scoping these rather than manual unsubscribe/close calls.
- **`Slot` over `std::function`.** Callbacks throughout the codebase are `BasicSlot<Sig>` —
  non-owning, allocation-free type erasure over a `(void*, Fn*)` pair. Use `bind<Fn>()`,
  `bind<&Class::method>(obj)`, or `bind(lambda_obj)` rather than introducing `std::function` in new
  code.
- **Two clocks.** `MonoTime`/`Duration` (steady clock, used for timers/reactor internals) versus wall
  clock time in `sys/Date.hpp`/`sys/Time.hpp` — don't mix them up; the reactor's cycle time
  (`CyclTime`) is a thread-local monotonic timestamp updated once per poll, not re-read per event.
