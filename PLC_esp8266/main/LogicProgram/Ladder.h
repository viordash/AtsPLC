#pragma once
#include "Display/EditableElement.h"
#include "LogicProgram/Network.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "partitions.h"
#include "redundant_storage.h"

#define TAG_Ladder "Ladder"

#define LADDER_VERSION ((uint32_t)0x20240905)

class Ladder : public std::vector<Network *> {
  protected:
    TEditableElementState design_state;
    size_t view_top_index;
    size_t selected_network;

    void InitialLoad();

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

  public:
    const size_t MinNetworksCount = 1;
    const size_t MaxNetworksCount = 42;
    const size_t MaxViewPortCount = 2;

    Ladder();
    ~Ladder();

    bool DoAction();
    bool Render(uint8_t *fb);

    void Append(Network *network);
    bool CanScrollAuto();
    void AutoScroll();
    void ScrollUp();
    void ScrollDown();
    void SwitchDesign();
    void SwitchEditing();

    void Load();
    void Store();
};
