#include "websocket_session.h"

websocket_session::
websocket_session(
    tcp::socket socket,
    std::shared_ptr<shared_state> const& state,
    messanger& msgr)
    : ws_(std::move(socket))
    , state_(state)
    , messanger_(msgr)
{
}

websocket_session::
~websocket_session()
{
    // Remove this session from the list of active sessions
    state_->leave(*this);
}

void
websocket_session::
fail(error_code ec, char const* what)
{
    // Don't report these
    if (ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
websocket_session::
on_accept(error_code ec)
{
    // Handle the error, if any
    if (ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(*this);

    // Read a message
    ws_.async_read(
        buffer_,
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
    {
        sp->on_read(ec, bytes);
    });
}

void
websocket_session::
on_read(error_code ec, std::size_t)
{
    // Handle the error, if any
    if (ec)
        return fail(ec, "read");

    
    auto out = beast::buffers_to_string(buffer_.data());
    std::cout << out << std::endl;
    auto value = bsoncxx::from_json(out);
    auto element = value.view()["head"];
    auto command = element.get_utf8().value.to_string();

    if (command == "get")
    {
        try {
            auto cursor = messanger_.get_messages(10);

            bsoncxx::builder::stream::document builder{};
            auto in_array = builder << "messages" << bsoncxx::builder::stream::open_array;
            for (auto doc : cursor) {
                in_array = in_array << doc;
            }
            auto after_array = in_array << bsoncxx::builder::stream::close_array;
            after_array = after_array << "head" << "accept_messages";
            bsoncxx::document::value doc = after_array << finalize;

            auto res = bsoncxx::to_json(doc);
            auto const ss = std::make_shared<std::string const>(std::move(res));
            this->send(ss);
        } 
        catch (std::exception e)
        {
            std::cout << "unknown error: " << e.what() << std::endl;
        }
        
    } 
    else if (command == "send")
    {
        auto view = value.view()["message"];
        auto message = view.get_utf8().value.to_string();
        if (message == "") return;
        auto oid = bsoncxx::oid::oid();
        bsoncxx::builder::stream::document builder{};
        bsoncxx::document::value doc = builder
            << "head" << "accept_message"
            << "_id" << oid
            << "message" << message
            << finalize;
        state_->send(bsoncxx::to_json(doc));
        messanger_.send_message(message, oid);
    }
    else if (command == "delete")
    {
        auto view = value.view()["_id"];
        auto oid = bsoncxx::oid::oid(view.get_utf8().value.to_string());
        bsoncxx::builder::stream::document builder{};
        bsoncxx::document::value doc = builder
            << "head" << "delete"
            << "_id" << oid
            << finalize;
        state_->send(bsoncxx::to_json(doc));
        messanger_.delete_message(oid);
    }
    else
        std::cout << "unknown command: " << command << "\n";

    buffer_.consume(buffer_.size());

    ws_.async_read(
        buffer_,
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
    {
        sp->on_read(ec, bytes);
    });
}

void
websocket_session::
send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if (queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(*queue_.front()),
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
    {
        sp->on_write(ec, bytes);
    });
}

void
websocket_session::
on_write(error_code ec, std::size_t)
{
    // Handle the error, if any
    if (ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if (!queue_.empty())
        ws_.async_write(
            net::buffer(*queue_.front()),
            [sp = shared_from_this()](
                error_code ec, std::size_t bytes)
    {
        sp->on_write(ec, bytes);
    });
}