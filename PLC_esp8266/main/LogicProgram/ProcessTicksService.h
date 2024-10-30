#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class ProcessTicksService {
  protected:
    /* data */
  public:
    ProcessTicksService(/* args */);
    ~ProcessTicksService();

    void Request(uint32_t delay_ms);
    uint32_t GetTicksToWait();
};
