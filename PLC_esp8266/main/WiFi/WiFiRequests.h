#pragma once

#include <list>
#include <stdlib.h>

enum RequestItemType { //
    wqi_Station = 0,
    wqi_Scaner,
    wqi_AccessPoint
};

typedef struct {
    RequestItemType type;
    union {
        struct {
            const char *ssid;
            bool status;
        } Scaner;
        struct {
            const char *ssid;
        } AccessPoint;
    } Payload;
} RequestItem;

class WiFiRequests : public std::list<RequestItem> {
  protected:
    bool Equals(const RequestItem *a, const RequestItem *b) const;

  public:
    std::list<RequestItem>::iterator AddRequest(RequestItem *new_request);
    RequestItem PopRequest();
};
