#include "LogicProgram/Ladder.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Ladder = "Ladder";

Ladder::Ladder() {
}

Ladder::~Ladder() {
    RemoveAll();
}

void Ladder::RemoveAll() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        ESP_LOGD(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
}

bool Ladder::DoAction() {
    bool any_changes = false;
    for (auto it = begin(); it != end(); ++it) {
        any_changes |= (*it)->DoAction();
    }
    return any_changes;
}

bool Ladder::Render(uint8_t *fb) {
    bool res = true;
    for (auto it = begin(); it != end(); ++it) {
        res &= (*it)->Render(fb);
    }
    return res;
}

void Ladder::Append(Network *network) {
    ESP_LOGI(TAG_Ladder, "append network: %p", network);
    push_back(network);
}

void Ladder::Load() {
    RemoveAll();
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       ladder_storage_name);

    ESP_LOGI(TAG_Ladder, "Load ver: 0x%X, size:%u", storage.version, (uint32_t)storage.size);

    if (storage.version != LADDER_VERSION //
        || Deserialize(storage.data, storage.size) == 0) {
        ESP_LOGI(TAG_Ladder, "load initial networks");
        InitialLoad();
    }

    delete[] storage.data;
}

void Ladder::Store() {
    redundant_storage storage;
    storage.size = Serialize(NULL, 0);
    if (storage.size == 0) {
        ESP_LOGE(TAG_Ladder, "serialization setup error");
        return;
    }
    storage.data = new uint8_t[storage.size];
    storage.version = LADDER_VERSION;

    if (Serialize(storage.data, storage.size) != storage.size) {
        ESP_LOGE(TAG_Ladder, "serialize error");
        delete[] storage.data;
        return;
    }

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);
    delete[] storage.data;
}

size_t Ladder::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;

    uint16_t networks_count;
    if (!Record::Read(&networks_count, sizeof(networks_count), buffer, buffer_size, &readed)) {
        ESP_LOGE(TAG_Ladder, "Deserialize, count read error");
        return 0;
    }
    if (networks_count < Ladder::MinNetworksCount) {
        ESP_LOGE(TAG_Ladder, "Deserialize, count min limit");
        return 0;
    }
    if (networks_count > Ladder::MaxNetworksCount) {
        ESP_LOGE(TAG_Ladder, "Deserialize, count max limit");
        return 0;
    }

    reserve(networks_count);
    for (size_t i = 0; i < networks_count; i++) {
        auto network = new Network();
        size_t network_readed = network->Deserialize(&buffer[readed], buffer_size - readed);
        if (network_readed == 0) {
            delete network;
            ESP_LOGE(TAG_Ladder, "Deserialize, network read error");
            return 0;
        }
        readed += network_readed;
        Append(network);
    }
    return readed;
}

size_t Ladder::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    uint16_t networks_count = size();
    if (networks_count < Ladder::MinNetworksCount) {
        return 0;
    }
    if (networks_count > Ladder::MaxNetworksCount) {
        return 0;
    }

    if (!Record::Write(&networks_count, sizeof(networks_count), buffer, buffer_size, &writed)) {
        return 0;
    }

    for (auto it = begin(); it != end(); ++it) {
        auto *network = *it;
        uint8_t *p;
        bool just_obtain_size = buffer == NULL;
        if (!just_obtain_size) {
            p = &buffer[writed];
        } else {
            p = NULL;
        }

        size_t network_writed = network->Serialize(p, buffer_size - writed);
        bool network_serialize_error = network_writed == 0;
        if (network_serialize_error) {
            return 0;
        }
        writed += network_writed;
    }

    return writed;
}