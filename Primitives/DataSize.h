#pragma once

enum DataSize {
  DEPTH_WIDTH = 512,
  DEPTH_HEIGHT = 424,
  COLOR_WIDTH = 1920,
  COLOR_HEIGHT = 1080,

  DEPTH_SIZE = DEPTH_WIDTH * DEPTH_HEIGHT,
  COLOR_SIZE = DEPTH_SIZE * 3,
  COMBINED_SIZE = DEPTH_SIZE * 4,
  FRAME_SIZE = COMBINED_SIZE,

  MAX_DEPTH = 4500, // mm
  ICP_ROW_START = 100,
  ICP_ROW_END = 200,
  ICP_COL_START = 100,
  ICP_COL_END = 200,
};