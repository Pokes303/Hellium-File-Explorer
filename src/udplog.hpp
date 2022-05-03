#pragma once
#include "main.hpp"
#include <string.h>

#define LOG_ENABLED
#define LOG_BUFFER_SIZE 1024 * 5

#define __FILENAME__ strrchr(__FILE__, '/') + 1

#define LOG(STR, ARGS...) do{ udplogf("[%s:%d]LOG: " STR, __FILENAME__, __LINE__, ## ARGS); } while(false)
#define LOG_E(STR, ARGS...) do{ udplogf("[%s:%d]ERROR: " STR, __FILENAME__, __LINE__, ## ARGS); } while(false)

void udplog(const char* str);
void udplogf(const char* str, ...);