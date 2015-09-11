#pragma once
#include "HuffmanCompressor.h"

enum DataType {
  DATA_DEPTH = 0,
  DATA_COLOR = 1,
  DATA_COMBINED = 2,
};

struct LookupValue {
  INT16 value;
  BYTE length;
};

typedef std::unordered_map<INT16, int> Histogram;
typedef std::vector<std::string> MapTable;
typedef std::unordered_map<int, LookupValue> UnmapTable;

class StdHuffmanCompressor : public HuffmanCompressor {
  Histogram depthHistogram;
  Histogram colorHistogram;
  Histogram combinedHistogram;

  MapTable mapTable[3];
  UnmapTable unmapTable[3];
  int codeLength[3];
  int *dataSize = new int[3]{ DEPTH_SIZE, COLOR_SIZE, COMBINED_SIZE };
public:
  StdHuffmanCompressor();
  ~StdHuffmanCompressor();
  void compress(DataType dataType, const INT16 *data, Bitset &transmitData);
  void decompress(DataType dataType, const Bitset &transmitData, INT16 *dataOut);
private:
  void loadTable();
  void loadMapTable(DataType dataType, std::string name);
  void loadUnmapTable(DataType dataType, std::string name);
  void constructHistogram();
  void constructTable(DataType dataType, const Histogram &histogram);
  int extractCode(const Bitset &bitset, int index, int length);
};

