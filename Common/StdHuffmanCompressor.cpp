#include "StdHuffmanCompressor.h"

StdHuffmanCompressor::StdHuffmanCompressor()
{
  //constructHistogram();
  //std::cout << "Finish histogram" << std::endl;
  //constructTable(DATA_DEPTH, depthHistogram);
  //std::cout << "Finish depth table" << std::endl;
  //constructTable(DATA_COLOR, colorHistogram); 
  //std::cout << "Finish color table" << std::endl;
  //constructTable(DATA_COMBINED, combinedHistogram);
  //std::cout << "Finish combined table" << std::endl;

  loadTable();
  std::cout << "Huffman table loaded." << std::endl;
}

StdHuffmanCompressor::~StdHuffmanCompressor()
{
  delete[] dataSize;
}

void StdHuffmanCompressor::compress(DataType dataType, const INT16 *data, Bitset &transmitData)
{ 
  std::string encodedData("");
  timer.startTimer();
  for (int i = 0; i < dataSize[dataType]; i++)
    encodedData += mapTable[dataType][data[i] + 4500];
  timer.stopTimer();
  //std::cout << "Compress-encode: " << timer.getElapsedTime() << std::endl;
  std::reverse(encodedData.begin(), encodedData.end());
  transmitData = Bitset(encodedData);
}

void StdHuffmanCompressor::decompress(DataType dataType, const Bitset &transmitData, INT16 *dataOut)
{
  timer.startTimer();
  for (int i = 0, transmitDataIndex = 0; i < dataSize[dataType]; i++) {
    int code = extractCode(transmitData, transmitDataIndex, codeLength[dataType]);
    dataOut[i] = unmapTable[dataType][code].value;
    transmitDataIndex += unmapTable[dataType][code].length;
  }
  timer.stopTimer();
  //std::cout << "Decompress: " << timer.getElapsedTime() << std::endl << std::endl;
}

void StdHuffmanCompressor::constructHistogram()
{
  // Generate for all values
  for (int i = -4500; i <= 4500; i++) {
    ++depthHistogram[i];
    ++combinedHistogram[i];
  }
  for (int i = -256; i <= 256; i++) {
    ++colorHistogram[i];
  }

  // Build histogram from dataset
  std::ifstream file;
  file.open("depth-differential.txt", std::ios::in);
  INT16 value;
  while (file >> value) {
    ++depthHistogram[value];
    ++combinedHistogram[value];
  }
  file.close();

  file.open("color-differential.txt", std::ios::in);
  while (file >> value) {
    ++colorHistogram[value];
    ++combinedHistogram[value];
  }
  file.close();
}

void StdHuffmanCompressor::loadTable()
{
  for (int i = 0; i < 3; i++) {
    mapTable[i] = MapTable(10000);
  }

  // Load map tables
  loadMapTable(DATA_DEPTH, "depth");
  loadMapTable(DATA_COLOR, "color");
  loadMapTable(DATA_COMBINED, "combined");

  // Load unmap tables
  loadUnmapTable(DATA_DEPTH, "depth");
  loadUnmapTable(DATA_COLOR, "color");
  loadUnmapTable(DATA_COMBINED, "combined");
}

void StdHuffmanCompressor::loadMapTable(DataType dataType, std::string name)
{
  std::ifstream file;
  int value;
  std::string code;

  file.open("maptbl-" + name + ".txt", std::ios::in);
  while (file >> value >> code)
    mapTable[dataType][value + 4500] = code;
  file.close();
}

void StdHuffmanCompressor::loadUnmapTable(DataType dataType, std::string name)
{
  std::ifstream file;
  int value, length;

  file.open("unmaptbl-" + name + ".txt", std::ios::in);
  file >> codeLength[dataType];
  for (int i = 0; file >> value >> length; i++) {
    unmapTable[dataType][i].value = value;
    unmapTable[dataType][i].length = length;
  }
  file.close();
}

void StdHuffmanCompressor::constructTable(DataType dataType, const Histogram &histogram)
{
  std::string name;
  switch (dataType) {
    case DATA_DEPTH: name = "depth"; break;   
    case DATA_COLOR: name = "color"; break;
    case DATA_COMBINED: name = "combined"; break;    
  }

  // Generate node in min heap
  for (auto it = histogram.begin(); it != histogram.end(); ++it) {
    Node *node = new Node(it->first, it->second);
    minHeap.push(node);
  }

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
  // Write map (compress) table to file
  int longestCodeLength = 0;
  std::ofstream file;
  file.open("maptbl-" + name + ".txt");
  for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
    file << it->first << "\t" << it->second << std::endl;
    longestCodeLength = max(longestCodeLength, (int)it->second.size());
  }
  file.close();

  // Generate unmap (decompress) table and write to file
  //file.open("temp.txt");
  UnmapTable unmapTable;
  for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
    Bitset code = Bitset(it->second);
    int index = code.to_ulong();
    int length = code.size();
    index = index << (longestCodeLength - length);
    unmapTable[index].value = it->first;
    unmapTable[index].length = length;
    //file << index << '\t' << it->first << '\t' << length << std::endl;
  }
  //file.close();
  INT16 currentValue;
  BYTE currentLength;
  file.open("unmaptbl-" + name + ".txt");
  file << longestCodeLength << std::endl;
  for (int i = 0; i < (2 << (longestCodeLength - 1)); i++) { // may cause problem
    if (unmapTable[i].length != 0) {
      currentValue = unmapTable[i].value;
      currentLength = unmapTable[i].length;
    } else {
      unmapTable[i].value = currentValue;
      unmapTable[i].length = currentLength;
    }
    file << unmapTable[i].value << '\t' << (int)unmapTable[i].length << std::endl;
  }
  file.close();

  // Load table to data structure
  //switch (dataType) {
  //  case DATA_DEPTH: {
  //    depthCodeLength = longestCodeLength;
  //    depthMapTable = huffmanCodes;
  //    depthUnmapTable = unmapTable;
  //    break;
  //  }
  //  case DATA_COLOR: {
  //    colorCodeLength = longestCodeLength;
  //    colorMapTable = huffmanCodes;
  //    colorUnmapTable = unmapTable;
  //    break;
  //  }
  //  case DATA_COMBINED: {
  //    combinedCodeLength = longestCodeLength;
  //    combinedMapTable = huffmanCodes;
  //    combinedUnmapTable = unmapTable;
  //    break;
  //  }
  //}

  // Deallocate memory
  deallocateTree(huffmanTree);
  minHeap.pop();
  unmapTable.clear();
  huffmanCodes.clear();
}

int StdHuffmanCompressor::extractCode(const Bitset &bitset, int index, int length)
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