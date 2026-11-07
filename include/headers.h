#ifndef ASYNC_HTTP_PROXY_HEADERS_H
#define ASYNC_HTTP_PROXY_HEADERS_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

struct HostPort {
    std::string host;
    std::string port;
};

using Callback = std::function<void(std::string_view, std::string_view)>;

void iterHeaders(std::string_view req, Callback &&callback);

HostPort findHostPort(std::string_view req);

std::optional<std::size_t> findContentLength(std::string_view rsp);

#endif  // ASYNC_HTTP_PROXY_HEADERS_H
