#include "LogicProgram/Ladder.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Ladder::Load() {
    RemoveAll();
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       ladder_storage_name);

    ESP_LOGI(TAG_Ladder,
             "Load ver: 0x%X, size:%u",
             (unsigned int)storage.version,
             (unsigned int)storage.size);

    if (storage.version != LADDER_VERSION //
        || Deserialize(storage.data, storage.size) == 0) {
        ESP_LOGI(TAG_Ladder, "Nothing to load");
        RemoveAll();
        ChangeWorkMode(work_mode, hotreload->enable_debug);
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

    WorkMode _work_mode;
    if (!Record::Read(&_work_mode, sizeof(_work_mode), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateWorkMode(_work_mode)) {
        return 0;
    }

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

    work_mode = _work_mode;
    items.reserve(networks_count);
    for (size_t i = 0; i < networks_count; i++) {
        auto network = work_mode == WorkMode::Stop ? new Network(NetworkState::nsStopFromPassive)
                                                   : new Network(NetworkState::nsPassive);
        size_t network_readed = network->Deserialize(&buffer[readed], buffer_size - readed);
        if (network_readed == 0) {
            delete network;
            ESP_LOGE(TAG_Ladder, "Deserialize, network read error");
            return 0;
        }

        switch (network->GetState()) {
            case NetworkState::nsPassive:
            case NetworkState::nsActive:
                if (work_mode != WorkMode::Run) {
                    ESP_LOGE(TAG_Ladder,
                             "Deserialize, network state error, ladder mode:%u, network state:%u",
                             (unsigned int)work_mode,
                             (unsigned int)network->GetState());
                    delete network;
                    return 0;
                }
                break;

            case NetworkState::nsStopFromPassive:
            case NetworkState::nsStopFromActive:
                if (work_mode != WorkMode::Stop) {
                    ESP_LOGE(TAG_Ladder,
                             "Deserialize, network state error, ladder mode:%u, network state:%u",
                             (unsigned int)work_mode,
                             (unsigned int)network->GetState());
                    delete network;
                    return 0;
                }
                break;
        }

        readed += network_readed;
        Append(network);
    }
    return readed;
}

size_t Ladder::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    uint16_t networks_count = items.size();
    if (networks_count < Ladder::MinNetworksCount) {
        return 0;
    }
    if (networks_count > Ladder::MaxNetworksCount) {
        return 0;
    }

    if (!Record::Write(&work_mode, sizeof(work_mode), buffer, buffer_size, &writed)) {
        return 0;
    }

    if (!Record::Write(&networks_count, sizeof(networks_count), buffer, buffer_size, &writed)) {
        return 0;
    }

    for (auto it = items.begin(); it != items.end(); ++it) {
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

void Ladder::DeleteStorage() {
    ESP_LOGI(TAG_Ladder, "Delete storage");
    redundant_storage_delete(storage_0_partition,
                             storage_0_path,
                             storage_1_partition,
                             storage_1_path,
                             ladder_storage_name);
}