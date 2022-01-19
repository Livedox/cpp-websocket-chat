#include "listener.h"
#include "shared_state.h"
#include "messanger.h"
#include <boost/asio/signal_set.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    auto address = net::ip::make_address("127.0.0.1");
    auto port = static_cast<unsigned short>(8083);
    auto doc_root = ".";

    mongocxx::instance instance{};
    mongocxx::uri uri("You uri");
    mongocxx::client client(uri);
    mongocxx::database db = client["You db"];
    mongocxx::collection collection = db["You coll"];
    messanger msgr(collection);

    // The io_context is required for all I/O
    net::io_context ioc;

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{ address, port },
        std::make_shared<shared_state>(doc_root),
        msgr)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc](boost::system::error_code const&, int)
        {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            ioc.stop();
        });

    // Run the I/O service on the main thread
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    return EXIT_SUCCESS;
}