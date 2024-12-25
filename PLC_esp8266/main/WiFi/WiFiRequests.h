#pragma once

#include <list>
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
    bool Equals(const RequestItem *a, const RequestItem *b) const;

  public:
    std::list<RequestItem>::iterator Add(RequestItem *new_request);
    void Remove(std::list<RequestItem>::const_iterator it);
    RequestItem Pop();
    void StationDone();
    void ScannerDone(const char *ssid);
    void AccessPointDone(const char *ssid);
};
