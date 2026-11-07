# Manual

## The parser

Each function takes the raw text of a request or response:

```cpp
#include "headers.h"

auto [host, port] = findHostPort("GET / HTTP/1.1\r\nHost: example.com:8080\r\n\r\n");
// host is "example.com", port is "8080". With no port in the header, port is "80".

auto length = findContentLength("HTTP/1.1 200 OK\r\nContent-Length: 1234\r\n\r\n");
// length holds 1234. A missing, malformed or too large value leaves it empty.

iterHeaders(text, [](std::string_view name, std::string_view value) { /* each header line */ });
```

`findHostPort` answers two empty strings when the request has no `Host` header.

## The tests

`.\recipe.ps1 run` builds and runs the test binary, and hands it any further arguments:

| command | what it does |
| --- | --- |
| `.\recipe.ps1 run` | every test case |
| `.\recipe.ps1 run --gtest_list_tests` | names the test cases |
| `.\recipe.ps1 run --gtest_filter=findHostPort.*` | runs one suite |

## The proxy

Built as `docs/install.md` says, `AsyncHttpProxy 5555` listens on port 5555. Point a client at it as an HTTP proxy: `curl -x 127.0.0.1:5555 http://127.0.0.1:8000/`. It takes one port and no other argument, and prints its usage otherwise. No gate builds or checks it yet.
