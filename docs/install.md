# Install

The parser needs no installing: put `include` on your include path, compile `src/headers.cpp` with your project, and build with C++23.

To build and test it here:

1. Install mingw g++ 13 or newer, CMake 3.28 or newer, Ninja, and LLVM for `clang-tidy` and `clang-format`.
2. Clone this repository.
3. Run `.\recipe.ps1 test`. The first run downloads GoogleTest, whose archive `CMakeLists.txt` pins by hash.

`recipe.ps1` looks for g++ in `C:\ProgramData\mingw64\mingw64\bin` and for LLVM in `C:\Program Files\LLVM\bin`, and adds those folders to its own PATH.

`.\recipe.ps1 build` compiles the parser and its tests into `build\`. `.\recipe.ps1 install` does nothing: there is no installer.

## The proxy

`AsyncHttpProxy` needs Boost and `<print>`, so it needs g++ 14 or newer or MSVC, and `recipe.ps1` does not build it. `ASYNC_HTTP_PROXY_APP` is on by default, so a plain configure looks for Boost:

```powershell
cmake -S . -B build-proxy -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-proxy
```

`conanfile.py` supplies Boost to an MSVC build, and `scripts/build_windows.ps1` drives one. These, `CMakePresets.json` and `.devcontainer` are what the course and the old GitHub route left. No gate runs them.
