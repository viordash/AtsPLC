#pragma once

#include "config.h"
#include <mutex>
#include <stdlib.h>

enum RequestItemType { //
    wqi_Station = 0,
    wqi_Scanner,
    wqi_AccessPoint
};

struct RequestItem {
    RequestItemType Type;
    union uPayload {
        struct {
            const char *ssid;
        } Scanner;
        struct {
            const char *ssid;
            const char *password;
            const char *mac;
        } AccessPoint;
    } Payload;

    RequestItem() = default;
    RequestItem(RequestItemType type, uPayload payload) : Type{ type }, Payload{ payload } {
    }
    RequestItem(RequestItem &) = delete;
    RequestItem &operator=(RequestItem &) = delete;

    RequestItem(RequestItem &&other) noexcept : RequestItem(other.Type, other.Payload) {
    }

    RequestItem &operator=(RequestItem &&other) noexcept {
        if (this != &other) {
            Type = other.Type;
            Payload = other.Payload;
        }
        return *this;
    }
};

class WiFiRequests {
  protected:
    RequestItem items[WiFi_RequestsLimit];
    size_t count;
    mutable std::mutex lock_mutex;
    bool Equals(const RequestItem *a, const RequestItem *b) const;
    bool Find(const RequestItem *request) const;
    bool Add(RequestItem *request);

  public:
    WiFiRequests();

    bool Contains(const RequestItem *request);
    bool HasAnother(const RequestItem *current);

    bool Scan(const char *ssid);
    bool AccessPoint(const char *ssid, const char *password, const char *mac);
    bool Station();
    bool Pop(RequestItem *request);

    size_t Size() const;
    const RequestItem *First() const;
};
