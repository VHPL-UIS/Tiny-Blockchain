# Tiny-Blockchain
This is a small blockchain based on **"Bitcoin and Cryptocurrency Technologies"** book from [here](https://d28rh4a8wq0iu5.cloudfront.net/bitcointech/readings/princeton_bitcoin_book.pdf). It uses **crow** library for rest apis. [crow](https://github.com/ipkn/crow). It is a very fast and easy to use C++ micro web framework (inspired by Python Flask). For the JSON manipulation, I used JSON for Modern C++, **nlohmann/json**, [nlohmann/json](https://github.com/nlohmann/json). And for calculating sha256, I used **sha256.h** from [Zedwood](http://zedwood.com/).

### How to compile:
```
mkdir build
cd build
cmake ..
make
```

### To run a node on port 6000:
```
./bc 6000
```

### To get chain from current node:
```
localhost:6000/chain
```

### To mine from current node:
```
localhost:6000/mine
```

New transaction is a **POST** method, make sure to add to its body like this:

```
{
    "sender": "x",
    "recipient": "y", 
    "amount":1
}
```
### To add a new transaction:
```
localhost:6000/transaction/new
```

Register nodes is a **POST** method, make sure to add to its body like this:

```
{
    "nodes":["localhost:6001", "localhost:6002"]
}
```
### To register other nodes in the network:
```
localhost:6000/nodes/register
```

### To solve conflicts between current nodes, use resolve nodes, it keeps the longest chain:
```
localhost:6000/nodes/resolve
```

Each node, at first, creates the genesis block with no transactions. Then it keeps the current transactions until a mine happens. Then it adds a new block to the chain with transactions.

#### Here is a complete example:
Run an instance with port 6000, ```./bc 6000```, then add some transactions, then a mine, some other transactions and some other mines, now check the chain with ```localhost:6000/chain```. Now, it's time to add another instance with port 6001, and register other nodes to it. Right now, this one has a smaller chain, so we expect that its chained replaced with the others chain. Run resolve request and see the result.



