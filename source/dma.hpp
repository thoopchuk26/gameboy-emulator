#pragma once

#include <common.hpp>;

class CPU;

struct DMAContext
{
  bool active;
  u8 byte;
  u8 value;
  u8 start_delay;
};

class DMA
{
public:
  DMA(CPU& c)
      : cpu(c) {};

  void dma_start(u8 start);
  void dma_tick();
  bool dma_transferring();

private:
  DMAContext context;
  CPU& cpu;
};
