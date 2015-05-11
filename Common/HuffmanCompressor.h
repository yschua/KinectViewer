#pragma once
#include "Core\Timer.h"
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <bitset>
#include <algorithm>
#include <boost\dynamic_bitset.hpp>
#include <Kinect.h>


// Huffman tree node only works for depth differential values at the moment.
// The differential values don't use up all 16 bits so perhaps the encoded tree
// does not need to store "value" as the full 16 bits.
// For the colour BYTE (8-bit) data, in order to introduce BYTE differentials,
// 9 bits will be required to account for the negative values.
struct Node {
  INT16 value;
  int frequency;
  Node *left;
  Node *right;
  Node() :
    value(INT16_MIN), left(NULL), right(NULL) {}
  Node(INT16 value, int frequency) :
    value(value), frequency(frequency), left(NULL), right(NULL) {}
  Node(int frequency, Node *left, Node *right) :
    value(INT16_MIN), frequency(frequency), left(left), right(right) {}
};

// Compare function used for generating minimum heap
struct CompareNodePtr {
  bool operator()(const Node *a, const Node *b) const { return a->frequency > b->frequency; }
};

typedef boost::dynamic_bitset<BYTE> Bitset;

class HuffmanCompressor {
  Core::Timer timer;
  std::map<INT16, int> dataFrequency;
  std::priority_queue<Node *, std::vector<Node *>, CompareNodePtr> minHeap;
  std::map<INT16, std::string> huffmanCodes;
  Bitset transmitData;
  const INT16 *diffData;
public:
  HuffmanCompressor();
  ~HuffmanCompressor();
  void compress(UINT size, const INT16 *data);
  void decompress();
private:
  void getHuffmanCode(Node *node, std::string code, std::string &encodedHuffmanTree);
  void reconstructHuffmanTree(Node *&node);
  std::string getBinary(INT16 value);
};

