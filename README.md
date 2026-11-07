# cpp-middle-project7

An asynchronous HTTP proxy in C++23: one thread, Boost.Asio coroutines, and a small HTTP header parser at its core. It began as the sprint 7 project of the Yandex Practicum "Middle C++ developer" course. The parser depends on nothing but the standard library; only the proxy needs Boost and `<print>`.

## What it does

- **The proxy**: `AsyncHttpProxy <listen_port>` accepts a client, reads its request up to the blank line, finds the `Host` header, connects to that host, and passes the request on and the response back. A `Content-Length` says how much of the body to carry.
- **The parser**: `iterHeaders` walks the header lines of a request or response and calls back with each name and value, `findHostPort` answers the host and port with `80` as the default, and `findContentLength` answers an optional length.

The proxy reports through Boost.Asio's `use_awaitable`, which throws, so `src/main.cpp` holds a `try` around each session and around `main`. `.\recipe.ps1 test` pins those lines.

## Layout

```text
include/  headers.h, the parser's interface
src/      headers.cpp, the parser, and main.cpp, the proxy
tests/    GoogleTest cases for the parser
```

## Build and test

`.\recipe.ps1 test` builds the parser and its tests, runs `ctest`, `clang-tidy` and `clang-format`, and searches `include` and `src` for locks and for `throw`, `try` and `catch`. It needs mingw g++, cmake, ninja and LLVM, and no Boost: it configures with `-DASYNC_HTTP_PROXY_APP=OFF`, so no gate builds the proxy. `docs/install.md` has the details, and `docs/manual.md` covers the parser and the proxy.

MIT licensed: `LICENSE` is the course's, copyright Yandex Practicum.

## Lab requirements

This repository complies with [revision 4.0.0](http://127.0.0.1:3000/igors-lab/software-requirements/src/tag/v4.0.0/BEST_REQUIREMENTS.md) of the lab requirements, except R-02 (#8), R-09 (#9), R-12 (#10), R-13 (#11), R-13 (#12), R-13 (#13), R-14 (#14), R-37 (#15), R-41 (#16), R-42 (#17).
