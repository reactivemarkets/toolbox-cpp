# Build from Source

The following sections assume that `TOOLBOX_SOURCE` is equal to `SOURCE_DIR/toolbox-cpp`, and that
`TOOLBOX_BUILD` is equal to `BUILD_DIR/toolbox-cpp`.

## Clone

Clone the repository from the [master repository](https://github.com/reactivemarkets/toolbox-cpp) or
your own personal fork:

``` bash
$ cd $SOURCE_DIR
$ git clone git@github.com:reactivemarkets/toolbox-cpp.git toolbox-cpp
```

The `SOURCE_DIR/toolbox-cpp` directory should now contain the source.

## Configure

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
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$TOOLBOX_HOME \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++
```

Build times can be reduced during development by disabling shared libraries and compiler
optimisations:

``` bash
$ cmake $SOURCE_DIR/toolbox-cpp -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DTOOLBOX_BUILD_SHARED=OFF
```

Note that both shared and static libraries are built by default.

Architecture-specific optimisation flags can be specified for release builds:

``` bash
$ cmake $SOURCE_DIR/toolbox-cpp -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS_RELEASE="-march=native -O3" \
  -DCMAKE_CXX_FLAGS_RELEASE="-march=native -O3"
```

See [CMake Documentation](https://cmake.org/documentation) for further information.

## Make

Compile the code in the build directory using the `make` command:

``` bash
$ cd $BUILD_DIR/toolbox-cpp
$ make
```

### Install

Install to `CMAKE_INSTALL_PREFIX`:

``` bash
$ make install
```
