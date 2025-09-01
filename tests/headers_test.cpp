#include <gtest/gtest.h>
#include "headers.h"

TEST(findHostPort, Simple) {
    const char* req = "GET / HTTP/1.1\r\nHost: example.com:8080\r\n\r\n";
    auto [host, port] = findHostPort(req);
    EXPECT_EQ(host, "example.com");
    EXPECT_EQ(port, "8080");
}

TEST(findHostPort, NoPort) {
    const char* req = "GET / HTTP/1.1\r\nHost: yandex.ru\r\n\r\n";
    auto [host, port] = findHostPort(req);
    EXPECT_EQ(host, "yandex.ru");
    EXPECT_EQ(port, "80");
}

TEST(findHostPort, NoHost) {
    const char* req = "GET / HTTP/1.1\r\n\r\n";
    auto [host, port] = findHostPort(req);
    EXPECT_TRUE(host.empty());
    EXPECT_TRUE(port.empty());
}

TEST(findContentLength, Simple) {
    const char* rsp = "HTTP/1.1 200 OK\r\nContent-Length: 1234\r\n\r\n";
    auto len = findContentLength(rsp);
    ASSERT_TRUE(len.has_value());
    EXPECT_EQ(*len, 1234);
}

TEST(findContentLength, NoContentLength) {
    const char* rsp = "HTTP/1.1 200 OK\r\n\r\n";
    auto len = findContentLength(rsp);
    EXPECT_FALSE(len.has_value());
}