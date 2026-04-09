#pragma once

#include <list>
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
  private:
    std::list<RequestItem> items;

  protected:
    mutable std::mutex lock_mutex;
    bool Equals(const RequestItem *a, const RequestItem *b) const;
    std::list<RequestItem>::iterator Find(RequestItem *request);

    size_t GetSize() const;
    const RequestItem &GetBack() const;
    void PopBack();
    std::list<RequestItem>::iterator GetBegin();
    std::list<RequestItem>::iterator GetEnd();
    std::list<RequestItem>::const_iterator GetBegin() const;
    std::list<RequestItem>::const_iterator GetEnd() const;

  public:
    bool Contains(RequestItem *request);
    bool HasAnother(RequestItem *current);

    bool Scan(const char *ssid);
    bool AccessPoint(const char *ssid, const char *password, const char *mac);
    bool Station();
    bool Pop(RequestItem *request);

    size_t Size() const;
    std::list<RequestItem>::const_iterator Begin() const;
    std::list<RequestItem>::const_iterator End() const;
};
