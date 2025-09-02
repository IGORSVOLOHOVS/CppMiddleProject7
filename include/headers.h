#pragma once

#include <string>
#include <functional>
#include <optional>

struct HostPort {
    std::string host;
    std::string port;
};

using Callback = std::function<void(std::string_view, std::string_view)>;

void iterHeaders(std::string_view req, Callback&& callback);

HostPort findHostPort(std::string_view req);

std::optional<size_t> findContentLength(std::string_view rsp);
