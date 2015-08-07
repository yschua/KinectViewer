#pragma once
#include "HuffmanCompressor.h"

struct LookupValue {
  INT16 value;
  BYTE length;
};

class StdHuffmanCompressor : public HuffmanCompressor {
  std::unordered_map<INT16, int> histogramDepth;
  std::unordered_map<INT16, int> histogramColor;
  std::unordered_map<INT16, int> histogramCombined;
  int longestCodeLength;
  std::unordered_map<int, LookupValue> huffmanCodesLookup;
public:
  StdHuffmanCompressor();
  ~StdHuffmanCompressor();
private:
  void constructHistogram();
  void constructTable(const std::unordered_map<INT16, int> &histogram, std::string name);
  int extractCode(Bitset &bitset, int index, int length);
};

