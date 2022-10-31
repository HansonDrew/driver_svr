#pragma once
#include <stdint.h>
#include <windows.h>
#define TIMESPACE 1

uint64_t GetTimestampMs();
uint64_t GetTimestampUs();
int64_t nowInNs(void);
void delay_us(int us);
void sleep_micro_seconds(ULONG ulMicroSeconds);