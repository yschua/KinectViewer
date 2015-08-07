#include "StdHuffmanCompressor.h"

StdHuffmanCompressor::StdHuffmanCompressor()
{
  //std::string s = "100111111111";
  //std::reverse(s.begin(), s.end());
  //Bitset bs = Bitset(s);
  //int value = extractCode(bs, 0, 4);
  //std::cout << value << std::endl;

  
  longestCodeLength = 0;
  constructHistogram();
  constructTable(histogramDepth, "depth");

  // test
  INT16 values[] = { 1989, -1080, 0,  16, 248, -506};
  std::string compressed("");
  for (int i = 0; i < 6; i++) {
    compressed += huffmanCodes[values[i]];
  }
  std::cout << "compressed: " << compressed << std::endl;
  std::reverse(compressed.begin(), compressed.end());
  Bitset transmitData = Bitset(compressed);
  //decode
  INT16 decodedValue;
  int transmitIndex = 0;
  for (int i = 0; i < 6; i++) {
    int index = extractCode(transmitData, transmitIndex, longestCodeLength);
    
    INT16 value = huffmanCodesLookup[index].value;
    std::cout << "extracted: " << index << " value: " << value << " index: " << transmitIndex << std::endl;
    transmitIndex += huffmanCodesLookup[index].length;
  }

}

StdHuffmanCompressor::~StdHuffmanCompressor()
{
}

void StdHuffmanCompressor::constructHistogram()
{
  // Generate for all values
  for (int i = -4500; i <= 4500; i++) {
    ++histogramDepth[i];
    ++histogramCombined[i];
  }
  for (int i = -256; i <= 256; i++) {
    ++histogramColor[i];
  }

  // Build histogram from dataset
  std::ifstream file;
  file.open("depth-differential.txt", std::ios::in);
  INT16 value;
  while (file >> value) {
    ++histogramDepth[value];
    ++histogramCombined[value];
  }
  file.close();

  file.open("color-differential.txt", std::ios::in);
  while (file >> value) {
    ++histogramColor[value];
    ++histogramCombined[value];
  }
  file.close();
}

void StdHuffmanCompressor::constructTable(const std::unordered_map<INT16, int> &histogram, std::string name)
{
  // Generate node in min heap
  for (auto it = histogram.begin(); it != histogram.end(); ++it) {
    Node *node = new Node(it->first, it->second);
    minHeap.push(node);
  }

  // TODO: deallocate memory
  // Generate Huffman tree
  while (minHeap.size() != 1) {
    Node *right = minHeap.top();
    minHeap.pop();
    Node *left = minHeap.top();
    minHeap.pop();
    minHeap.push(new Node(left->frequency + right->frequency, left, right));
  }
  Node *huffmanTree = minHeap.top();

  // Get Huffman codes
  std::string code("");
  std::string dummy("");
  getHuffmanCode(huffmanTree, code, dummy); // dummy field unused for this case

  // TODO: write table as bytes for smaller filesize
  // Write compress table to file
  std::ofstream file;
  file.open("huffman-compress-" + name + ".txt");
  for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
    file << it->first << "\t" << it->second << std::endl;
    longestCodeLength = max(longestCodeLength, (int)it->second.size());
  }
  file.close();

  // Generate lookup table and write to file
  file.open("temp.txt");
  for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
    Bitset code = Bitset(it->second);
    int index = code.to_ulong();
    int length = code.size();
    index = index << (longestCodeLength - length);
    huffmanCodesLookup[index].value = it->first;
    huffmanCodesLookup[index].length = length;
    file << index << '\t' << it->first << '\t' << length << std::endl;
  }
  file.close();
  INT16 currentValue;
  BYTE currentLength;
  file.open("huffman-decompress-" + name + ".txt");
  file << longestCodeLength << std::endl;
  for (int i = 0; i < (2 << longestCodeLength); i++) { // may cause problem
    if (huffmanCodesLookup[i].length != 0) {
      currentValue = huffmanCodesLookup[i].value;
      currentLength = huffmanCodesLookup[i].length;
    } else {
      huffmanCodesLookup[i].value = currentValue;
      huffmanCodesLookup[i].length = currentLength;
    }
    file << i << '\t' << huffmanCodesLookup[i].value << '\t' << (int)huffmanCodesLookup[i].length << std::endl;
  }
  file.close();
}

int StdHuffmanCompressor::extractCode(Bitset &bitset, int index, int length)
{
  int value = 0;
  for (int i = index; i < index + length; i++) {
    value <<= 1;
    if (i < bitset.size()) {
      value |= bitset[i];
    }
  }
  return value;
}