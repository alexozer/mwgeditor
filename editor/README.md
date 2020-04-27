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

You could open the `editor` directory as a CMake project in CLion like I do. 
You could also do this in Visual Studio apparently, but I tried and you might need to set some additional runtime path settings.
