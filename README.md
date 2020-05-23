# The Reactive C++ Toolbox

## What is Toolbox?

The Reactive C++ Toolbox is an open source library of C++20 components designed for efficient
aynchronous IO network applications on the Linux platform. Machine architectures currently supported
are: AMD64, ARM and ARM32.

## Prerequisites

To build Toolbox from source, you will need:

- [CMake](http://www.cmake.org/) for Makefile generation;
- [GCC](http://gcc.gnu.org/) or [Clang](http://clang.llvm.org/) with support for C++20;
- [Boost](http://www.boost.org/) for additional library dependencies.

## Getting Started

Clone the repository, build and run the unit-tests as follows:

```bash
$ git clone git@github.com:reactivemarkets/toolbox-cpp.git toolbox-cpp
$ mkdir toolbox-cpp/build
$ cd toolbox-cpp/build
$ cmake ..
$ make -j all test install
```

## Documentation

The [latest Doxygen documentation](https://toolbox-cpp.reactivemarkets.com) is available online.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details
on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](https://semver.org/) for versioning. For the versions available, see the [releases
page](https://github.com/reactivemarkets/toolbox-cpp/releases).

## License

This project is licensed under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0).
A copy of the license is available in the [LICENSE.md](LICENSE.md) file.
