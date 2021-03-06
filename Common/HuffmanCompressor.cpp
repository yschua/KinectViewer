#include "HuffmanCompressor.h"

HuffmanCompressor::HuffmanCompressor()
{
}

HuffmanCompressor::~HuffmanCompressor()
{
}

void HuffmanCompressor::compress(int size, const INT16 *data, Bitset &transmitData)
{
  // Find frequency of each value
  for (UINT i = 0; i < size; ++i)
    ++dataFrequency[data[i]];
  dataFrequency[INT16_MAX] = 1; // use max value as the pseudo EOF

  // Store each frequency as a (leaf) node in minimum heap
  for (auto it = dataFrequency.begin(); it != dataFrequency.end(); ++it) {
    Node *node = new Node(it->first, it->second);
    minHeap.push(node);
  }

  // Generate Huffman tree
  while (minHeap.size() != 1) {
    Node *right = minHeap.top();
    minHeap.pop();
    Node *left = minHeap.top();
    minHeap.pop();
    minHeap.push(new Node(left->frequency + right->frequency, left, right)); // Create internal node
  }
  Node *huffmanTree = minHeap.top();

  // Get Huffman codes for each value and generate encoded Huffman tree
  std::string code("");
  std::string encodedHuffmanTree("");
  getHuffmanCode(huffmanTree, code, encodedHuffmanTree);
  std::reverse(encodedHuffmanTree.begin(), encodedHuffmanTree.end()); // TODO: try encodedHuffmanTree = bit + encodedHuffmanTree to see if performance is better

  // Encode data values
  std::string encodedData("");
  for (UINT i = 0; i < size; ++i) {
    encodedData += huffmanCodes[data[i]];
  }
  encodedData += huffmanCodes[INT16_MAX]; // add pseudo EOF
  std::reverse(encodedData.begin(), encodedData.end());

  // Convert data to bit array
  transmitData = Bitset(encodedData + encodedHuffmanTree);

  // Deallocate memory
  deallocateTree(huffmanTree);
  minHeap.pop();
  dataFrequency.clear();
  huffmanCodes.clear();
}

void HuffmanCompressor::decompress(int size, const Bitset &transmitData, INT16 *dataOut)
{
  transmitDataIndex = 0;
  this->transmitData = transmitData;
  // Reconstruct Huffman tree
  Node *huffmanTree = NULL;
  reconstructHuffmanTree(huffmanTree);
  ++transmitDataIndex; // dispose last bit

  // Decode data values
  UINT i = 0; // temp
  Node *head = huffmanTree;
  Node *current = head;
  bool pseudoEOF = false;
  while (!pseudoEOF) {
    //timer.startTimer();
    INT16 value = (transmitData[transmitDataIndex] == 0) ? current->left->value : current->right->value;
    if (value == INT16_MAX) {
      pseudoEOF = true;
    } else if (value != INT16_MIN) {
      dataOut[i++] = value;
      current = head;
    } else {
      current = (transmitData[transmitDataIndex] == 0) ? current->left : current->right;
    }
    ++transmitDataIndex;
  }

  // Deallocate memory
  deallocateTree(huffmanTree);
}

void HuffmanCompressor::getHuffmanCode(Node *node, std::string code, std::string &encodedHuffmanTree)
{
  if (node->left == NULL && node->right == NULL) {
    // Leaf node
    encodedHuffmanTree += "1";
    encodedHuffmanTree += getBinary(node->value);
    huffmanCodes[node->value] = code;
  } else {
    // Internal node
    encodedHuffmanTree += "0";
    if (node->left != NULL) 
      getHuffmanCode(node->left, code + "0", encodedHuffmanTree);
    if (node->right != NULL)
      getHuffmanCode(node->right, code + "1", encodedHuffmanTree);
  }
}

void HuffmanCompressor::reconstructHuffmanTree(Node *&node)
{
  node = new Node();
  if (transmitData[transmitDataIndex] == 0) {
    if (node->left == NULL) {
      ++transmitDataIndex;
      reconstructHuffmanTree(node->left);
    }
    if (node->right == NULL) {
      ++transmitDataIndex;
      reconstructHuffmanTree(node->right);
    }
    return;
  } else {
    INT16 value = 0;
    for (UINT i = 0; i < 16; ++i) {
      value <<= 1;
      ++transmitDataIndex;
      value = value & 0xfffe;
      value = value | transmitData[transmitDataIndex];
    }
    node->value = value;
    return;
  }
}

std::string HuffmanCompressor::getBinary(INT16 value)
{
  std::bitset<16> bits(value);
  return bits.to_string();
}

void HuffmanCompressor::deallocateTree(Node *node)
{
  if (node != NULL) {
    deallocateTree(node->left);
    deallocateTree(node->right);
    delete node;
  }
}
