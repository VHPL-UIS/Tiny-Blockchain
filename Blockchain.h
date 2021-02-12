#ifndef _BLOCKCHAIN_H_
#define _BLOCKCHAIN_H_

#include <set>
#include <string>
#include <chrono>
#include <vector>
#include <optional>
#include "json.hpp"

class Transaction
{
    std::string sender;
    std::string recipient;
    double      amount;

public:
    Transaction(const std::string& sen, const std::string& recip, const double amo) :
            sender(sen), recipient(recip), amount(amo) {}

    const std::string& getSender() const { return sender; }
    const std::string& getRecipient() const { return recipient; }
    const double getAmount() const { return amount; }

    const std::string getTransactionStr() const 
    {
        nlohmann::json jObj = {
            {"sender", sender}, {"recipient", recipient}, {"amount", amount}
        };
        std::string ret = jObj.dump();
        return ret;
    }
};

class Block
{
    int                                   index;
    int64_t                               timeStamp;
    std::vector<Transaction>              transaction;
    int                                   proof;
    std::string                           previousHash;

public:
    Block(int ind, const int64_t t, const std::vector<Transaction>& trx, const int& prf, const std::string& pHash) :
            index(ind), timeStamp(t), transaction(trx), proof(prf), previousHash(pHash) {}

    const int& getIndex() const { return index; }
    const int64_t getTimestamp() const { return timeStamp; }
    const std::vector<Transaction>& getTransactions() const { return transaction; }
    const std::vector<std::string> getTransactionsStr() const 
    {
        std::vector<std::string> strTrx;
        for (const Transaction& trx : transaction) {
            strTrx.emplace_back(trx.getTransactionStr());
        }

        return strTrx;
    }
    const int& getProof() const { return proof; }
    const std::string& getPreviousHash() const { return previousHash; }

    const std::string getBlockStr() const
    {
        std::string trxs = "";
        for (const Transaction& t : transaction) {
            trxs += t.getTransactionStr();
        }

        nlohmann::json jObj = {
            {"index", index}, {"timestamp", timeStamp}, {"transaction", trxs}, {"proof", proof}, {"previousHash", previousHash}
        };

        std::string ret = jObj.dump();
        return ret;
    }
};

class Blockchain
{
    std::set<std::string>    nodes;
    std::vector<Block>       chain;
    std::vector<Transaction> currentTransactions;

public:
    Blockchain(); // ctor
    void registerNode(const std::string& address); // adds a node to the set of nodes
    bool isChainValid(const std::vector<Block>& chain); // Is the chain valid?
    bool resolveConflicts(); // consensus algorithm
    Block createNewBlock(int proof, std::optional<std::string> hashStr); // creates a new block in the blockchain
    void createNewTransaction(const std::string& sender, const std::string& recipient, const double amount); // creates a new transaction 
    const Block& getLastBlock() const { return chain[getChainSize() - 1]; } // returns the last block in chain
    const std::string getHash(const Block& block) const; // return a SHA-256 hash of a block
    int proofOfWork(const Block& lastBlock); // simple proof of work 
    bool isValidProof(int lastProof, int proof, const std::string& lastHash); // checks for proof validity


    const std::vector<Block>& getChain() const { return chain; } // returns the chain
    //const std::vector<std::string> getChainStr() const;
    int getChainSize() const { return chain.size(); } // returns the chain size

    const std::set<std::string>& getNodes() const { return nodes; } // returns the nodes

    const std::vector<Transaction>& getCurrentTransactions() const { return currentTransactions; } // returns the current transactions

    void clearChain() { chain.clear(); }
    void pushToChain(const Block& blk) { chain.emplace_back(blk); }
};

#endif
