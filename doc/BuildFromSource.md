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

### Documentation

The `tb-doc` target is not part of the default build; build it explicitly to generate the Doxygen
documentation, requiring Doxygen and Graphviz (`dot`) to be found by `cmake`:

``` bash
$ make tb-doc
```

`tb-doc` depends on `tb-image`, which renders the module dependency diagrams
(`tb-util.png`, `tb-sys.png`, `tb-io.png`, `tb-net.png`, `tb-http.png`) referenced by
[Dependencies.md](Dependencies.md), so both are generated automatically. To generate just the
diagrams without running Doxygen:

``` bash
$ make tb-image
```

### Install

Install to `CMAKE_INSTALL_PREFIX`:

``` bash
$ make install
```

The dependency diagrams install as part of the `doc` component if `tb-image` was built beforehand;
otherwise they are silently omitted, since their install entries are optional. To include them,
build `tb-doc` (or just `tb-image`) before installing:

``` bash
$ make tb-doc install
```
