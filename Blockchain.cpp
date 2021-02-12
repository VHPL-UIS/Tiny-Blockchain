#include <iostream>
#include <sstream>
#include "Blockchain.h"
#include "sha256.h"
#include "HTTPRequest.hpp"
#include "json.hpp"

Blockchain::Blockchain()
{
    nodes.clear();
    chain.clear();
    currentTransactions.clear();

    createNewBlock(100, "1"); // creates the genesis block
}

void Blockchain::registerNode(const std::string& address)
{
    // adds a node to the set of nodes with address of node. Eg. 'http://192.168.0.5:5000'
    nodes.emplace(address);
}

bool Blockchain::isChainValid(const std::vector<Block>& chain)
{
    // determine if a given blockchain is valid
    // returns true if valid, false if not

    Block lastBlock = chain[0];
    int currentIndex = 1;

    while (currentIndex < getChainSize()) {
        auto block = chain[currentIndex];

        // check that the hash of the block is correct
        auto lastBlockHash = getHash(lastBlock);
        if (block.getPreviousHash() != lastBlockHash) {
            return false;
        }

        if (!isValidProof(lastBlock.getProof(), block.getProof(), lastBlockHash)) {
            return false;
        }

        lastBlock = block;
        currentIndex += 1;
    }

    return true;
}

bool Blockchain::resolveConflicts()
{
    // this is our consensus algorithm, it resolves conflicts 
    // by replacing our chain with the longest one in the network
    // returns true if our chain was replaced, false if not

    auto neighbours = nodes;
    std::vector<Block> newChain;
    bool changed = false;
    
    // we're only looking for chains longer than ours

    // grab and verify the chains from all the nodes in our network
    for (const std::string& node : nodes) {
        int maxLength = getChainSize();
        std::string url = "http://" + node + "/chain";
        http::Request request(url);
        const http::Response response = request.send("GET");
        if (response.status == http::Response::STATUS_OK) {
            nlohmann::json jObj = nlohmann::json::parse(response.body);
            
            auto length = jObj["length"];
            auto jchain = jObj["chain"];
            for (const auto& bl : jchain) {
                auto index = static_cast<int>(bl["index"]);
                auto ts = static_cast<int64_t>(bl["ts"]);
                auto trxs = bl["transactions"];
                std::vector<Transaction> trx;
                for (const auto& tr : trxs) {
                    auto sender = static_cast<std::string>(tr["sender"]);
                    auto recip = static_cast<std::string>(tr["recipient"]);
                    auto amount = static_cast<double>(tr["amount"]);
                    Transaction t(sender, recip, amount);
                    trx.emplace_back(t);
                }
                auto proof = static_cast<int>(bl["proof"]);
                auto phash = static_cast<std::string>(bl["previous_hash"]);
                Block block(index, ts, trx, proof, phash);
                newChain.emplace_back(block);
            }

            if (length > maxLength && isChainValid(newChain)) {
                // set chain to new_chain
                clearChain();                
                for (const auto& blk : newChain) {
                    pushToChain(blk);
                }
                newChain.clear();
                changed = true;
            }
        }
    }

    return changed;
}

Block Blockchain::createNewBlock(int proof, std::optional<std::string> hashStr)
{
    // creates a new block in the blockchain, returns a new block,
    // with proof from proof of work and hash of previous block

    std::string hStr;
    if (hashStr) {
        hStr = hashStr.value();
    } else {
        hStr = getHash(chain[getChainSize() - 1]);
    }
    
    auto ts = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    Block block((getChainSize() + 1), ts, getCurrentTransactions(), proof, hStr);
    
    currentTransactions.clear();
    chain.emplace_back(block);

    return block;
}

void Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, const double amount)
{
    // creates a new transaction to go into the next mined block

    Transaction trx(sender, recipient, amount);
    currentTransactions.emplace_back(trx);
}

const std::string Blockchain::getHash(const Block& block) const
{
    // creates a SHA-256 hash of a block
    return sha256(block.getBlockStr());
}

int Blockchain::proofOfWork(const Block& lastBlock)
{
    // simple Proof of Work Algorithm:
    //   - Find a number p' such that hash(pp') contains leading 4 zeroes
    //   - Where p is the previous proof, and p' is the new proof

    int lastProof = lastBlock.getProof();
    std::string lastHash = getHash(lastBlock);
    int proof = 0;

    while (!isValidProof(lastProof, proof, lastHash)) {
        proof += 1;
    }

    return proof;
}

bool Blockchain::isValidProof(int lastProof, int proof, const std::string& lastHash)
{
    // validates the proof, true if correct, false if not

    std::string guess = std::to_string(lastProof) + std::to_string(proof) + lastHash;
    std::string guessHash = sha256(guess);

    return guessHash.substr(guessHash.size() - 4) == "0000";
}