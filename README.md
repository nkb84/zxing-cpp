# ZXing C++ Port

[ZXing](https://github.com/zxing/zxing) is/was a Java library.

At some point a complete C++ port/rewrite was created and maintained in the official [ZXing](https://github.com/zxing/zxing) repo. However, at the time of writing the C++ port is no longer maintained and has been removed from the official ZXing repo.

This project was forked from the [zxing-cpp](https://github.com/glassechidna/zxing-cpp) to contain the C++ project, with try to keep zxing-cpp up-to-date with zxing-java version. Current progress:
  1. Migrated PDF417
  2. Migrated DataMatrix
  ... To be continue


# Building using CMake

CMake is a tool, that generates native makefiles and workspaces. It integrates well with a number of IDEs including Qt Creator and Visual Studio.

Usage with CLion or Qt Creator:

  1. Simply open `CMakeLists.txt` as a new project
  2. Additional command line arguments can be specified (see below)

Usage with Makefiles, Visual Studio, etc. (see `cmake --help` for a complete list of generators):

  1. `mkdir build`
  2. `cd` to `build`
  3. Unix: run `cmake -G "Unix Makefiles" ..`
  3. Windows: run `cmake -G "Visual Studio 10" ..`
  
You can switch between build modes by specifying:

  - `-DCMAKE_BUILD_TYPE=Debug` or
  - `-DCMAKE_BUILD_TYPE=Release`

# Development tips

To profile the code (very useful to optimize the code):

  1. Install Valgrind
  2. Run `valgrind --tool=callgrind build/zxing - path/to/test/data/*.jpg > report.html`
  3. Analyze output using KCachegrind
