#pragma once

#include "LogicProgram/Network.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "partitions.h"
#include "redundant_storage.h"

#define LADDER_VERSION ((uint32_t)0x20240905)

class Ladder : public std::vector<Network *> {
  protected:
    size_t view_top_index;

    void InitialLoad();

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

  public:
    const static size_t MinNetworksCount = 1;
    const static size_t MaxNetworksCount = 42;
    const static size_t MaxViewPortCount = 2;

    Ladder();
    ~Ladder();

    bool DoAction();
    bool Render(uint8_t *fb);

    void Append(Network *network);
    bool CanScrollAuto();
    void AutoScroll();
    void ScrollUp();
    void ScrollDown();

    void Load();
    void Store();
};
