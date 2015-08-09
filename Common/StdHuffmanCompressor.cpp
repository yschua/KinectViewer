#include "StdHuffmanCompressor.h"

StdHuffmanCompressor::StdHuffmanCompressor()
{
  //std::string s = "100111111111";
  //std::reverse(s.begin(), s.end());
  //Bitset bs = Bitset(s);
  //int value = extractCode(bs, 0, 4);
  //std::cout << value << std::endl;
  
  constructHistogram();
  std::cout << "Finish histogram" << std::endl;

 
  constructTable(DATA_DEPTH, depthHistogram);

  std::cout << "Finish depth table" << std::endl;

  constructTable(DATA_COLOR, colorHistogram);
  std::cout << "Finish color table" << std::endl;
  constructTable(DATA_COMBINED, combinedHistogram);

  exit(EXIT_SUCCESS);

  // test
  //INT16 values[] = { 1989, -1080, 0,  16, 248, -506};
  //std::string compressed("");
  //for (int i = 0; i < 6; i++) {
  //  compressed += huffmanCodes[values[i]];
  //}
  //std::cout << "compressed: " << compressed << std::endl;
  //std::reverse(compressed.begin(), compressed.end());
  //Bitset transmitData = Bitset(compressed);
  ////decode
  //INT16 decodedValue;
  //int transmitIndex = 0;
  //for (int i = 0; i < 6; i++) {
  //  int index = extractCode(transmitData, transmitIndex, longestCodeLength);
  //  
  //  INT16 value = huffmanCodesLookup[index].value;
  //  std::cout << "extracted: " << index << " value: " << value << " index: " << transmitIndex << std::endl;
  //  transmitIndex += huffmanCodesLookup[index].length;
  //}
}

StdHuffmanCompressor::~StdHuffmanCompressor()
{
}

void StdHuffmanCompressor::compress(DataType dataType, Bitset &transmitData, const INT16 *data)
{
  timer.startTimer();
  // TODO: How to use a mapTable variable to assign to references depth,color map tables??
  int size;
  std::string encodedData("");
  switch (dataType) {
    case DATA_DEPTH: {
      size = 512 * 424;
      for (int i = 0; i < size; i++)
        encodedData += depthMapTable[data[i]];
      break;
    }
    case DATA_COLOR: {
      size = 512 * 424 * 3;
      for (int i = 0; i < size; i++)
        encodedData += colorMapTable[data[i]];
      break;
    }
    case DATA_COMBINED: {
      size = 512 * 424 * 4;
      for (int i = 0; i < size; i++)
        encodedData += combinedMapTable[data[i]];
      break;
    }
  }
  std::reverse(encodedData.begin(), encodedData.end());
  timer.stopTimer();
  transmitData = Bitset(encodedData);
  //std::cout << timer.getElapsedTime() << std::endl;
}

void StdHuffmanCompressor::decompress(DataType dataType, const Bitset &transmitData, UINT16 *dataOut)
{
  timer.startTimer();
  int size;
  std::unordered_map<int, LookupValue> unmapTable;
  int longestCodeLength;
  switch (dataType) {
    case DATA_DEPTH: {
      longestCodeLength = depthCodeLength;
      size = 512 * 424;
      for (int i = 0, transmitDataIndex = 0; i < size; i++) {
        int code = extractCode(transmitData, transmitDataIndex, longestCodeLength);
        dataOut[i] = depthUnmapTable[code].value;
        transmitDataIndex += depthUnmapTable[code].length;
      }
      break;
    }
    case DATA_COLOR: {
      longestCodeLength = colorCodeLength;
      size = 512 * 424 * 3;
      for (int i = 0, transmitDataIndex = 0; i < size; i++) {
        int code = extractCode(transmitData, transmitDataIndex, longestCodeLength);
        dataOut[i] = colorUnmapTable[code].value;
        transmitDataIndex += colorUnmapTable[code].length;
      }
      break;
    }
    case DATA_COMBINED: {
      longestCodeLength = combinedCodeLength;
      size = 512 * 424 * 4;
      for (int i = 0, transmitDataIndex = 0; i < size; i++) {
        int code = extractCode(transmitData, transmitDataIndex, longestCodeLength);
        dataOut[i] = combinedUnmapTable[code].value;
        transmitDataIndex += combinedUnmapTable[code].length;
      }
      break;
    }
  }
  timer.stopTimer();
  //std::cout << timer.getElapsedTime() << std::endl << std::endl;
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
  file.open("huffmaptbl-" + name + ".txt");
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
  file.open("huffunmaptbl-" + name + ".txt");
  file << longestCodeLength << std::endl;
  for (int i = 0; i < (2 << longestCodeLength); i++) { // may cause problem
    if (unmapTable[i].length != 0) {
      currentValue = unmapTable[i].value;
      currentLength = unmapTable[i].length;
    } else {
      unmapTable[i].value = currentValue;
      unmapTable[i].length = currentLength;
    }
    file << i << '\t' << unmapTable[i].value << '\t' << (int)unmapTable[i].length << std::endl;
  }
  file.close();

  // Load table to data structure
  switch (dataType) {
    case DATA_DEPTH: {
      depthCodeLength = longestCodeLength;
      depthMapTable = huffmanCodes;
      depthUnmapTable = unmapTable;
      break;
    }
    case DATA_COLOR: {
      colorCodeLength = longestCodeLength;
      colorMapTable = huffmanCodes;
      colorUnmapTable = unmapTable;
      break;
    }
    case DATA_COMBINED: {
      combinedCodeLength = longestCodeLength;
      combinedMapTable = huffmanCodes;
      combinedUnmapTable = unmapTable;
      break;
    }
  }

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