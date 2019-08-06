#pragma once
#include <stdbool.h>
#include <stdint.h>
bool base_sleep(int milliseconds);
uint64_t base_clock64(void);
uint32_t base_clock32(void);