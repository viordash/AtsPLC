#pragma once
#include "Display/EditableElement.h"
#include "LogicProgram/Network.h"
#include "LogicProgram/WorkMode.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "partitions.h"
#include "redundant_storage.h"

#define TAG_Ladder "Ladder"

#define LADDER_VERSION ((uint32_t)0x20260822)

class Ladder {
  protected:
    std::vector<Network *> items;
    int32_t view_top_index;
    bool frame_buffer_req_render;
    WorkMode work_mode;


    EditableElement::ElementState GetDesignState(int selected_network);
    int GetSelectedNetwork();
    bool RemoveNetworkIfEmpty(int network_id);

  public:
    static constexpr size_t MinNetworksCount = 1;
    static constexpr size_t MaxNetworksCount = 80;
    static constexpr size_t MaxViewPortCount = 2;

    explicit Ladder();
    ~Ladder();

    bool DoAction();
    void Render(FrameBuffer *fb);
    void AtLeastOneNetwork();
    void Append(Network *network);
    void Duplicate(int network_id);
    void Delete(int network_id);
    bool ScrollUp(int *selected_network);
    bool ScrollDown(int *selected_network);
    void HandleButtonUp();
    void HandleButtonPageUp();
    void HandleButtonDown();
    void HandleButtonPageDown();
    void HandleButtonSelect();
    void HandleButtonOption();

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);
    void RemoveAll();

    void Load();
    void Store();
    static void DeleteStorage();

    void SetViewTopIndex(int32_t index);
    void SetSelectedNetworkIndex(int32_t index);

    WorkMode GetWorkMode();
    void ChangeWorkMode(WorkMode new_mode, bool enable_debug);
};
