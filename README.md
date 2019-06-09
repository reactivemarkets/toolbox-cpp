# Toolbox: The Reactive C++ Toolbox

## What is Toolbox?

The Reactive C++ Toolbox is an open source library of C++17 components designed for efficient
aynchronous IO network applications on the Linux platform. Machine architectures currently supported
are: AMD64, ARM and ARM32.

### Dependencies

To build Toolbox from source, you will need:

- [CMake](http://www.cmake.org/) for Makefile generation;
- [GCC](http://gcc.gnu.org/) or [Clang](http://clang.llvm.org/) with support for C++17;
- [Boost](http://www.boost.org/) for additional library dependencies;

## Quick Start

```bash
$ git clone git@github.com:reactivemarkets/toolbox-cpp.git toolbox-cpp
$ mkdir toolbox-cpp/build
$ cd toolbox-cpp/build
$ cmake ..
$ make -j all test package
```

## Building from Source

The following sections assume that `TOOLBOX_SOURCE` is equal to `SOURCE_DIR/toolbox-cpp`, and that
`TOOLBOX_BUILD` is equal to `BUILD_DIR/toolbox-cpp`.

### Clone Repository

Clone the repository from the [master repository](https://github.com/reactivemarkets/toolbox-cpp) or
your own personal fork:

``` bash
$ cd $SOURCE_DIR
$ git clone git@github.com:reactivemarkets/toolbox-cpp.git toolbox-cpp
```

The `SOURCE_DIR/toolbox-cpp` directory should now contain the source.

### Configure Build

Configure using `cmake` as follows from the build directory:

``` bash
$ cd $BUILD_DIR
$ mkdir -p toolbox-cpp
$ cd toolbox-cpp
$ cmake $SOURCE_DIR/toolbox-cpp -DCMAKE_INSTALL_PREFIX=$TOOLBOX_HOME
```

This step will produce Unix Makefiles by default. The build-type and compiler can be set using the
following options:

``` bash
$ cmake $SOURCE_DIR/toolbox-cpp -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DCMAKE_INSTALL_PREFIX=$TOOLBOX_HOME \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++
```

Build times can be reduced during development by disabling shared libraries and compiler
optimisations:

``` bash
$ cmake $SOURCE_DIR/toolbox-cpp -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=DEBUG \
  -DTOOLBOX_BUILD_SHARED=OFF
```

Note that both shared and static libraries are built by default.

Architecture-specific optimisation flags can be specified for release builds:

``` bash
$ cmake $SOURCE_DIR/toolbox-cpp -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DCMAKE_C_FLAGS_RELEASE="-march=native -O3" \
  -DCMAKE_CXX_FLAGS_RELEASE="-march=native -O3"
```

See [CMake Documentation](https://cmake.org/documentation) for further information.

### Build Binaries

Compile the code in the build directory using the `make` command:

``` bash
$ cd $BUILD_DIR/toolbox-cpp
$ make
```

### Install Artefacts

Install to `CMAKE_INSTALL_PREFIX`:

``` bash
$ make install
```
