#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <string>
#include "Blockchain.h"
#include "crow.h"
#include "json.hpp"

std::string uuidGen()
{
    boost::uuids::uuid uuidb = boost::uuids::random_generator()();
    auto uuid = boost::uuids::to_string(uuidb);

    uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
    return uuid;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "\nplease enter the port(e.g., 6000)\n";
    }

    crow::SimpleApp app;

    std::string uuid = uuidGen();

    Blockchain blockchain;

    CROW_ROUTE(app, "/mine")([&](){
        auto lastBlock = blockchain.getLastBlock();
        auto proof = blockchain.proofOfWork(lastBlock);
        blockchain.createNewTransaction("0", uuid, 1);
        auto prevHash = blockchain.getHash(lastBlock);
        auto block = blockchain.createNewBlock(proof, prevHash);

        nlohmann::json jObj;
        jObj["message"] = "new block forged";
        jObj["index"] = block.getIndex();

        nlohmann::json arr;
        auto trxs = block.getTransactions();
        for (const auto& trx : trxs) {
            nlohmann::json jObj;
            jObj["sender"] = trx.getSender();
            jObj["recipient"] = trx.getRecipient();
            jObj["amount"] = trx.getAmount();
            arr.emplace_back(jObj);
        }
        jObj["transactions"] = arr;
        jObj["proof"] = block.getProof();
        jObj["previous_hash"] = block.getPreviousHash();
        jObj["ts"] = block.getTimestamp();
        
        return crow::response(jObj.dump());
    });

    CROW_ROUTE(app, "/transaction/new")
    .methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400);
        }

        blockchain.createNewTransaction(body["sender"].s(),
                                        body["recipient"].s(),
                                        body["amount"].i());

        std::ostringstream os;
        os << "transaction will be added to new block";
        return crow::response{os.str()};
    });

    CROW_ROUTE(app, "/chain")([&](){

        nlohmann::json jObj;
        nlohmann::json arr;
        const auto& chain = blockchain.getChain();
        for (const auto& block : chain) {
            nlohmann::json arr1;
            nlohmann::json arr2;
            arr2["index"] = block.getIndex();
            const auto& trxs = block.getTransactions();
            for (const auto& trx : trxs) {
                nlohmann::json jObj;
                jObj["sender"] = trx.getSender();
                jObj["recipient"] = trx.getRecipient();
                jObj["amount"] = trx.getAmount();
                arr1.emplace_back(jObj);
            }
            arr2["transactions"] = arr1;
            arr2["proof"] = block.getProof();
            arr2["previous_hash"] = block.getPreviousHash();
            arr2["ts"] = block.getTimestamp();
            arr.emplace_back(arr2);
        }
        jObj["chain"] = arr;
        jObj["length"] = blockchain.getChainSize();

        return crow::response(jObj.dump());
    });

    CROW_ROUTE(app, "/nodes/register")
    .methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400);
        }

        auto nodesss = body["nodes"];
        for (auto n : nodesss) {
            blockchain.registerNode(n.s());
        }

        std::ostringstream os;
        os << "new nodes have been added\n";
        return crow::response{os.str()};
    });

    CROW_ROUTE(app, "/nodes/resolve")([&](){
        auto replaced = blockchain.resolveConflicts();

        nlohmann::json jObj;
        nlohmann::json arr;
        const auto& chain = blockchain.getChain();
        for (const auto& block : chain) {
            nlohmann::json arr1;
            nlohmann::json arr2;
            arr2["index"] = block.getIndex();
            const auto& trxs = block.getTransactions();
            for (const auto& trx : trxs) {
                nlohmann::json jObj;
                jObj["sender"] = trx.getSender();
                jObj["recipient"] = trx.getRecipient();
                jObj["amount"] = trx.getAmount();
                arr1.emplace_back(jObj);
            }
            arr2["transactions"] = arr1;
            arr2["proof"] = block.getProof();
            arr2["previous_hash"] = block.getPreviousHash();
            arr2["ts"] = block.getTimestamp();
            arr.emplace_back(arr2);
        }
        jObj["chain"] = arr;
        jObj["length"] = blockchain.getChainSize();

        if (replaced) {
            jObj["message"] = "our chain was replaced";
        } else {
            jObj["message"] = "our chain is authoritative";
        }

        return crow::response{jObj.dump()};
    });

    app.port(atoi(argv[1])).multithreaded().run();

    return 0;
}
