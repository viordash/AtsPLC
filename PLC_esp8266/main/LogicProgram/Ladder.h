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
    void SafeLoad(redundant_storage *storage);

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

  public:
    const static size_t MinNetworksCount = 1;
    const static size_t MaxNetworksCount = 42;

    Ladder();
    ~Ladder();

    void Append(Network *network);

    void Load();
    void Store();
};
