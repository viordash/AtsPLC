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
    int32_t view_top_index;

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

    EditableElement::ElementState GetDesignState(int selected_network);
    int GetSelectedNetwork();
    bool RemoveNetworkIfEmpty(int network_id);

  public:
    const size_t MinNetworksCount = 1;
    const size_t MaxNetworksCount = 80;
    const size_t MaxViewPortCount = 2;

    explicit Ladder();
    ~Ladder();

    bool DoAction();
    void Render(FrameBuffer *fb);
    void AtLeastOneNetwork();
    void Append(Network *network);
    void Duplicate(int network_id);
    void Delete(int network_id);
    bool CanScrollAuto();
    void AutoScroll();
    bool ScrollUp(int *selected_network);
    bool ScrollDown(int *selected_network);
    void HandleButtonUp();
    void HandleButtonPageUp();
    void HandleButtonDown();
    void HandleButtonPageDown();
    void HandleButtonSelect();
    void HandleButtonOption();

    void Load();
    void Store();
    static void DeleteStorage();

    void SetViewTopIndex(int16_t index);
    void SetSelectedNetworkIndex(int16_t index);
};
