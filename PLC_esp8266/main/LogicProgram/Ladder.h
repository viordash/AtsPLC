#pragma once
#include "Display/EditableElement.h"
#include "LogicProgram/Network.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "partitions.h"
#include "redundant_storage.h"

#define TAG_Ladder "Ladder"

typedef void (*f_UIStateChanged)(int16_t view_top_index, int16_t selected_network);

#define LADDER_VERSION ((uint32_t)0x20240905)

class Ladder : public std::vector<Network *> {
  protected:
    int16_t view_top_index;

    f_UIStateChanged cb_UI_state_changed;

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

    EditableElement::ElementState GetDesignState(int selected_network);
    int GetSelectedNetwork();
    bool RemoveNetworkIfEmpty(int network_id);

  public:
    const size_t MinNetworksCount = 1;
    const size_t MaxNetworksCount = 42;
    const size_t MaxViewPortCount = 2;

    explicit Ladder(f_UIStateChanged cb_UI_state_changed);
    ~Ladder();

    bool DoAction();
    bool Render(uint8_t *fb);
    void AtLeastOneNetwork();
    void Append(Network *network);
    bool CanScrollAuto();
    void AutoScroll();
    void HandleButtonUp();
    void HandleButtonPageUp();
    void HandleButtonDown();
    void HandleButtonPageDown();
    void HandleButtonSelect();
    void HandleButtonOption();
    bool ForcePeriodicRendering();

    void Load();
    void Store();
    static void DeleteStorage();

    void SetViewTopIndex(int16_t index);
    void SetSelectedNetworkIndex(int16_t index);
};
