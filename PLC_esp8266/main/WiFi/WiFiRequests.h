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
    union {
        struct {
            const char *ssid;
        } Scanner;
        struct {
            const char *ssid;
            const char *password;
            const char *mac;
        } AccessPoint;
    } Payload;
};

class WiFiRequests : public std::list<RequestItem> {
  protected:
    std::mutex lock_mutex;
    bool Equals(const RequestItem *a, const RequestItem *b) const;
    std::list<RequestItem>::iterator Find(RequestItem *request);

  public:
    bool Contains(RequestItem *request);
    bool OneMoreInQueue();

    bool Scan(const char *ssid);
    bool RemoveScanner(const char *ssid);

    bool AccessPoint(const char *ssid, const char *password, const char *mac);
    bool RemoveAccessPoint(const char *ssid);

    bool Station();
    bool RemoveStation();

    bool Pop(RequestItem *request);
};
