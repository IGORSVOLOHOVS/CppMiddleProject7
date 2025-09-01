#include "headers.h"

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/tcp_stream.hpp>

#include <chrono>
#include <print>
#include <ranges>
#include <string_view>
#include <iostream>


using boost::asio::io_service;
using boost::asio::co_spawn;
using boost::asio::async_read_until;
using boost::asio::awaitable;
using boost::asio::use_awaitable;
using boost::system::error_code;
using boost::asio::buffer;
using boost::asio::dynamic_buffer;
using boost::asio::transfer_at_least;
using boost::asio::ip::tcp;


constexpr std::string_view delimiter = "\r\n\r\n";

awaitable<void> session(tcp::socket client_socket, io_service& io_service)
{
    auto server_socket = std::make_shared<tcp::socket>(io_service);
    try
    {
        boost::asio::streambuf client_buffer;
        auto bytes_read = co_await boost::asio::async_read_until(client_socket, client_buffer, delimiter, use_awaitable);

        std::string_view client_request(
            (const char*)client_buffer.data().data(),
            bytes_read
        );

        auto [host, port] = findHostPort(client_request);
        if (host.empty()) {
            std::println("Could not find host in request");
            co_return; 
        }

        tcp::resolver resolver(io_service);
        auto endpoints = co_await resolver.async_resolve(host, port, use_awaitable);
        co_await server_socket->async_connect(*endpoints.begin(), use_awaitable);

        co_await boost::asio::async_write(*server_socket, client_buffer, use_awaitable);

        boost::asio::streambuf server_buffer;
        bytes_read = co_await boost::asio::async_read_until(*server_socket, server_buffer, delimiter, use_awaitable);

        std::string_view server_response(
            (const char*)server_buffer.data().data(),
            bytes_read
        );
        
        co_await boost::asio::async_write(client_socket, server_buffer, use_awaitable);

        if (auto content_length = findContentLength(server_response))
        {
            size_t bytes_to_transfer = *content_length;
            std::array<char, 8192> buffer;

            while (bytes_to_transfer > 0)
            {
                size_t n = co_await server_socket->async_read_some(boost::asio::buffer(buffer), use_awaitable);
                co_await boost::asio::async_write(client_socket, boost::asio::buffer(buffer, n), use_awaitable);
                bytes_to_transfer -= n;
            }
        }
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "Session error: " << e.what() << std::endl;
        std::println("Session error: {}", e.what());
    }

    boost::beast::error_code ec;
    client_socket.shutdown(tcp::socket::shutdown_both, ec);
    client_socket.close(ec);
    server_socket->shutdown(tcp::socket::shutdown_both, ec);
    server_socket->close(ec);
}

class Server
{
public:
  Server(io_service& io_service, short port)
    : io_service_(io_service)
    , acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
      [this](error_code ec)
      {
        if(!ec){
          co_spawn(io_service_,
            session(std::move(socket_), io_service_),
            boost::asio::detached);
        }
        do_accept();
      }
    );
  }

  io_service& io_service_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {

  try {
    if (argc != 2) {
      std::cerr << "Usage: proxy_server";
      std::cerr << " <listen_port>\n";
      return 1;
    }
    io_service io_service(1);
    Server server(io_service, std::atoi(argv[1]));
    io_service.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
