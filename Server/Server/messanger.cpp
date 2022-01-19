#include "messanger.h"
messanger::messanger() {}
messanger::messanger(mongocxx::collection collection)
{
    this->collection = collection;
}

mongocxx::cursor messanger::get_messages(int limit = 10)
{
    if (limit < 1) auto e = std::exception("limit < 1");
    mongocxx::options::find find;
    find.limit(limit);
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value = builder
        << "_id" << -1
        << finalize;
    bsoncxx::v_noabi::document::view_or_value ordering(doc_value);
    find.sort(ordering);
    return collection.find({}, find);
}

void messanger::send_message(std::string message, bsoncxx::v_noabi::oid oid)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value = builder
        << "_id" << oid
        << "message" << message
        << finalize;
    bsoncxx::document::view view = doc_value.view();
    collection.insert_one(view);
}

void messanger::delete_message(bsoncxx::v_noabi::oid oid)
{
    collection.delete_one(document{} << "_id" << oid << finalize);
}