#pragma once
#include "net.h"
#include "beast.h"
#include "shared_state.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "messanger.h"
// Forward declaration
class shared_state;

/** Represents an active WebSocket connection to the server
*/
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    beast::flat_buffer buffer_;
    websocket::stream<tcp::socket> ws_;
    std::shared_ptr<shared_state> state_;
    std::vector<std::shared_ptr<std::string const>> queue_;
    messanger& messanger_;

    void fail(error_code ec, char const* what);
    void on_accept(error_code ec);
    void on_read(error_code ec, std::size_t bytes_transferred);
    void on_write(error_code ec, std::size_t bytes_transferred);

public:
    websocket_session(
        tcp::socket socket,
        std::shared_ptr<shared_state> const& state,
        messanger& msgr);

    ~websocket_session();

    template<class Body, class Allocator>
    void
        run(http::request<Body, http::basic_fields<Allocator>> req);

    // Send a message
    void
        send(std::shared_ptr<std::string const> const& ss);
};

template<class Body, class Allocator>
void
websocket_session::
run(http::request<Body, http::basic_fields<Allocator>> req)
{
    // Accept the websocket handshake
    ws_.async_accept(
        req,
        std::bind(
            &websocket_session::on_accept,
            shared_from_this(),
            std::placeholders::_1));
}