#ifndef CRC16_H_
#define CRC16_H_

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

uint8_t *get_crc16_reflected(uint8_t const *buffer, int len, uint8_t *crc);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // CRC16_H_
