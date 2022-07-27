#include "httpclient.h"
#include <iostream>
#include <sstream>

namespace ipc {
namespace net {

using boost::asio::ip::tcp;

HttpClient::HttpClient(const std::string &server, const std::string &path)
    : resolver_(io_context_), socket_(io_context_)
{
    tcp::resolver::results_type endpoints = resolver_.resolve(server, "http");
    boost::asio::connect(socket_, endpoints);

    std::ostringstream request_stream;
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_ = request_stream.str();
}

void HttpClient::Run()
{
    io_context_.run();
}

void HttpClient::SendRequest()
{
    boost::asio::write(socket_, boost::asio::buffer(request_));
    boost::asio::streambuf response;
    boost::asio::read_until(socket_, response, "\r\n");

    std::istream response_stream(&response);
    std::string http_version;
    unsigned int status_code;
    std::string status_message;
    response_stream >> http_version >> status_code;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Invalid response\n";
        return;
    }

    if (status_code != 200)
    {
        std::cout << "Response returned with status code " << status_code << "\n";
        return;
    }

    boost::asio::read_until(socket_, response, "\r\n\r\n");
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
    {
        std::cout << header << "\n";
    }
    std::cout << "\n";

    std::ostringstream ss;
    ss << &response;
    std::cout << ss.str();

    boost::system::error_code error;
    while (boost::asio::read(socket_, response, boost::asio::transfer_at_least(1), error))
    {
        ss << &response;
    }

    if (error != boost::asio::error::eof)
    {
        throw boost::system::system_error(error);
    }

    std::cout << ss.str();
}

} // namespace net
} // namespace ipc