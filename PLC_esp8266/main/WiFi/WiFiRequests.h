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
    RequestItemType type;
    union {
        struct {
            const char *ssid;
            bool status;
        } Scanner;
        struct {
            const char *ssid;
        } AccessPoint;
    } Payload;
};

class WiFiRequests : public std::list<RequestItem> {
  protected:
    std::mutex lock_mutex;
    bool Equals(const RequestItem *a, const RequestItem *b) const;

  public:
    bool Add(RequestItem *new_request);
    bool AddOrReAddIfStatus(RequestItem *new_request, bool *status);
    RequestItem Pop();
    void StationDone();
    void ScannerDone(const char *ssid);
    void AccessPointDone(const char *ssid);
};
