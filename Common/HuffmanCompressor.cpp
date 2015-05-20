#include "HuffmanCompressor.h"

HuffmanCompressor::HuffmanCompressor()
{
}

HuffmanCompressor::~HuffmanCompressor()
{
}

void HuffmanCompressor::compress(UINT size, const INT16 *data)
{
  diffData = data;
  // Find frequency of each value
  // TODO: Use unordered map
  timer.startTimer();
  for (UINT i = 0; i < size; ++i)
    ++dataFrequency[data[i]];
  dataFrequency[INT16_MAX] = 1; // use max value as the pseudo EOF
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;
  

  // Store each frequency as a (leaf) node in minimum heap
  timer.startTimer();
  for (auto it = dataFrequency.begin(); it != dataFrequency.end(); ++it) {
    Node *node = new Node(it->first, it->second);
    minHeap.push(node);
  }
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;

  // Generate Huffman tree
  timer.startTimer();
  while (minHeap.size() != 1) {
    Node *right = minHeap.top();
    minHeap.pop();
    Node *left = minHeap.top();
    minHeap.pop();
    minHeap.push(new Node(left->frequency + right->frequency, left, right)); // Create internal node
  }
  Node *huffmanTree = minHeap.top();
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;
  //minHeap.pop(); // check that this does not erase huffmanTree root node

  // Get Huffman codes for each value and generate encoded Huffman tree
  timer.startTimer();
  std::string code("");
  std::string encodedHuffmanTree("");
  getHuffmanCode(huffmanTree, code, encodedHuffmanTree);
  std::reverse(encodedHuffmanTree.begin(), encodedHuffmanTree.end()); // TODO: try encodedHuffmanTree = bit + encodedHuffmanTree to see if performance is better
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;

  // Encode data values
  timer.startTimer();
  std::string encodedData("");
  for (UINT i = 0; i < size; ++i) {
    encodedData += huffmanCodes[data[i]];
  }
  encodedData += huffmanCodes[INT16_MAX]; // add pseudo EOF
  std::reverse(encodedData.begin(), encodedData.end());
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;


  // Convert data to bit array
  timer.startTimer();
  transmitData = Bitset(encodedData + encodedHuffmanTree);
  timer.stopTimer();
  std::cout << timer.getElapsedTime() / 1000 << std::endl;

  // Deallocate memory

  std::cout << std::endl;
}

void HuffmanCompressor::decompress()
{
  transmitDataIndex = 0;

  // Reconstruct Huffman tree
  Node *huffmanTree = NULL;
  timer.startTimer();
  reconstructHuffmanTree(huffmanTree);
  ++transmitDataIndex; // dispose last bit
  //transmitData >>= 1; // dispose last bit
  //transmitData.pop_back();
  timer.stopTimer();
  std::cout << "Reconstruct tree: " << timer.getElapsedTime() / 1000 << std::endl;

  // Generate lookup table

  // Decode data values
  timer.startTimer();
  INT16 depthDiff[512 * 424]; // temp
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
      depthDiff[i++] = value;
      //std::cout << "i: " << i << " value: " << value << std::endl;
      current = head;
    } else {
      current = (transmitData[transmitDataIndex] == 0) ? current->left : current->right;
    }
    ++transmitDataIndex;
    //transmitData >>= 1;
    //transmitData.pop_back();
    //timer.stopTimer();
    //std::cout << "Read bit: " << timer.getElapsedTime() << std::endl;
  }
  timer.stopTimer();
  std::cout << "Decode: " << timer.getElapsedTime() / 1000 << std::endl;

  // Deallocate memory

  std::cout << std::endl;
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
      //transmitData >>= 1;
      //transmitData.pop_back();
      reconstructHuffmanTree(node->left);
    }
    if (node->right == NULL) {
      ++transmitDataIndex;
      //transmitData >>= 1;
      //transmitData.pop_back();
      reconstructHuffmanTree(node->right);
    }
    return;
  } else {
    INT16 value = 0;
    for (UINT i = 0; i < 16; ++i) {
      value <<= 1;
      ++transmitDataIndex;
      //transmitData >>= 1;
      //transmitData.pop_back();
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