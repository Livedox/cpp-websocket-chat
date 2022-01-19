#pragma once
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>


using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;


class messanger
{
    mongocxx::collection collection;
public:
    messanger();
    messanger(mongocxx::collection collection);

    mongocxx::cursor get_messages(int limit);

    void send_message(std::string message, bsoncxx::v_noabi::oid oid);

    void delete_message(bsoncxx::v_noabi::oid);
};