#include "headers.h"

#include <algorithm>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

using Callback = std::function<void(std::string_view, std::string_view)>;

void iterHeaders(std::string_view req, Callback&& callback) {
    auto header_pairs = req 
                      | std::views::split("\r\n"sv)
                      | std::views::drop(1)
                      | std::views::take_while([](const auto& range) { 
                          return !range.empty(); 
                        })
                      | std::views::transform([](const auto& range) {
                          std::string_view line(range.begin(), range.end());
                          auto pos = line.find(": ");
                          if (pos != std::string_view::npos) {
                              return std::make_pair(line.substr(0, pos), line.substr(pos + 2));
                          }
                          return std::make_pair(std::string_view{}, std::string_view{});
                        })
                      | std::views::filter([](const auto& pair){
                          return !pair.first.empty();
                      });

    std::ranges::for_each(header_pairs, [&callback](const auto& pair) {
        callback(pair.first, pair.second);
    });
}

std::pair<std::string, std::string> findHostPort(std::string_view req) {
    std::pair<std::string, std::string> host_port;
    iterHeaders(req, [&host_port](std::string_view key, std::string_view value){
        if (key == "Host") {
            auto pos = value.find(':');
            if (pos != std::string_view::npos) {
                host_port.first = value.substr(0, pos);
                host_port.second = value.substr(pos + 1);
            } else {
                host_port.first = value;
                host_port.second = "80"; 
            }
        }
    });
    return host_port;
}

std::optional<size_t> findContentLength(std::string_view rsp) {
    std::optional<size_t> content_length;
    iterHeaders(rsp, [&content_length](std::string_view key, std::string_view value){
        if (key == "Content-Length") {
            size_t length = 0;
            auto result = std::from_chars(value.data(), value.data() + value.size(), length);
            if (result.ec == std::errc()) {
                content_length = length;
            }
        }
    });
    return content_length;
}
