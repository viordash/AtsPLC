#pragma once

#include <list>
#include <stdlib.h>

enum RequestItemType { //
    wqi_Connect = 0,
    wqi_ScanSsid,
    wqi_GenerateSsid
};

typedef struct {
    RequestItemType type;
    union {
        struct {
            const char *ssid;
            bool status;
        } ScanSsid;
        struct {
            const char *ssid;
        } GenerateSsid;
    } Payload;
} RequestItem;

class WiFiRequests : public std::list<RequestItem> {
  protected:
    bool Equals(const RequestItem *a, const RequestItem *b) const;

  public:
    std::list<RequestItem>::iterator AddRequest(RequestItem *new_request);
    RequestItem PopRequest();
};
