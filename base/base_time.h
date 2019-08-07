#pragma once
#include <stdbool.h>
#include <stdint.h>
bool base_sleep(int milliseconds);
int64_t base_clock64();
uint32_t base_clock32();
namespace zsy{
class Clock{
public:
    virtual uint32_t Now()const=0;
};
void SetGlobalClock(Clock *clock);
uint32_t GetMilliSeconds();
}
