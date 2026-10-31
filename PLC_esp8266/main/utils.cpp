#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

uint8_t hexchar_to_dec(char hex) {
    return (hex & 0x40) ? (hex & 0x0F) + 9 : (hex & 0x0F);
}
