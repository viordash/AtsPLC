#define PRIVILEGED_FUNCTION

#include "FreeRTOS.h"


#ifdef __cplusplus
extern "C" {
#endif

TickType_t xTaskGetTickCount( void ) PRIVILEGED_FUNCTION;
#ifdef __cplusplus
}
#endif