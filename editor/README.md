# Milky Way Gourmet Level Editor

The level editor can be built on Windows, macOS, and Linux.

## macOS and Linux

Create a build directory (you only need to do this once):

```
cd editor
mkdir build
cd build
```

Build (necessary each pull, make sure you're inside `editor/build` already):

```
cmake ..
make
```

Run the level editor (inside `editor/build`):

```
./mwgeditor
```

## Windows

### Visual Studio

* Make sure you have "Desktop development with C++" and "Linux Development with C++" workloads installed. See here: https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#installation
* Open the `editor` folder, open the CMakeLists.txt, and click the run button near the top. Video: https://streamable.com/uxhy24

### CLion

* Open the `editor` directory as a CMake project.
