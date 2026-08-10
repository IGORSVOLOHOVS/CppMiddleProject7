#include "headers.h"

#include <algorithm>
// std::from_chars и std::errc. В libstdc++ <charconv> приезжает попутно через
// <string_view>, в MSVC STL - нет, и файл падал на C2039: 'from_chars': is not
// a member of 'std'.
#include <charconv>
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

HostPort findHostPort(std::string_view req) {
    HostPort result;
    iterHeaders(req, [&result](std::string_view key, std::string_view value){
        if (key == "Host") {
            auto pos = value.find(':');
            if (pos != std::string_view::npos) {
                result.host = value.substr(0, pos);
                result.port = value.substr(pos + 1);
            } else {
                result.host = value;
                result.port = "80"; 
            }
        }
    });
    return result;
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
