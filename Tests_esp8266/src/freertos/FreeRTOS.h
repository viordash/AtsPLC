#pragma once
#include <stdint.h>
#include <unistd.h>

/* Type definitions. */
#define portCHAR int8_t
#define portFLOAT float
#define portDOUBLE double
#define portLONG int32_t
#define portSHORT int16_t
#define portSTACK_TYPE uint8_t
#define portBASE_TYPE int

typedef portSTACK_TYPE StackType_t;
typedef portBASE_TYPE BaseType_t;
typedef unsigned portBASE_TYPE UBaseType_t;

typedef uint32_t TickType_t;
#define portMAX_DELAY ((TickType_t)0xffffffffUL)

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )


#define BIT31   0x80000000
#define BIT30   0x40000000
#define BIT29   0x20000000
#define BIT28   0x10000000
#define BIT27   0x08000000
#define BIT26   0x04000000
#define BIT25   0x02000000
#define BIT24   0x01000000
#define BIT23   0x00800000
#define BIT22   0x00400000
#define BIT21   0x00200000
#define BIT20   0x00100000
#define BIT19   0x00080000
#define BIT18   0x00040000
#define BIT17   0x00020000
#define BIT16   0x00010000
#define BIT15   0x00008000
#define BIT14   0x00004000
#define BIT13   0x00002000
#define BIT12   0x00001000
#define BIT11   0x00000800
#define BIT10   0x00000400
#define BIT9    0x00000200
#define BIT8    0x00000100
#define BIT7    0x00000080
#define BIT6    0x00000040
#define BIT5    0x00000020
#define BIT4    0x00000010
#define BIT3    0x00000008
#define BIT2    0x00000004
#define BIT1    0x00000002
#define BIT0    0x00000001

/*-----------------------------------------------------------*/
#define portTickType TickType_t

#define CONFIG_FREERTOS_HZ 100
#define configUSE_TICKLESS_IDLE 	1
#define configCPU_CLOCK_HZ			( ( unsigned long ) 80000000 )	
#define configTICK_RATE_HZ			( ( portTickType ) CONFIG_FREERTOS_HZ )

/* Architecture specifics. */
#define portSTACK_GROWTH			( -1 )
#define portTICK_PERIOD_MS			( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT			4
/*-----------------------------------------------------------*/

void portYIELD_FROM_ISR(void);