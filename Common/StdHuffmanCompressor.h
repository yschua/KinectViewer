#pragma once
#include "HuffmanCompressor.h"

enum DataType {
  DATA_DEPTH = 1,
  DATA_COLOR = 2,
  DATA_COMBINED = 3,
};

struct LookupValue {
  INT16 value;
  BYTE length;
};

typedef std::unordered_map<INT16, int> Histogram;
typedef std::unordered_map<INT16, std::string> MapTable;
typedef std::unordered_map<int, LookupValue> UnmapTable;

class StdHuffmanCompressor : public HuffmanCompressor {
  Histogram depthHistogram;
  Histogram colorHistogram;
  Histogram combinedHistogram;

  MapTable depthMapTable;
  MapTable colorMapTable;
  MapTable combinedMapTable;

  UnmapTable depthUnmapTable;
  UnmapTable colorUnmapTable;
  UnmapTable combinedUnmapTable;

  int depthCodeLength;
  int colorCodeLength;
  int combinedCodeLength;
public:
  StdHuffmanCompressor();
  ~StdHuffmanCompressor();
  Bitset compress(DataType dataType, const INT16 *data);
  void decompress(DataType dataType, const Bitset &transmitData, UINT16 *dataOut); // change dataOut to INT16
private:
  void constructHistogram();
  void constructTable(DataType dataType, const Histogram &histogram);
  int extractCode(const Bitset &bitset, int index, int length);
};

