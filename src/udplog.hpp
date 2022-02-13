#include "main.hpp"

#define LOG_ENABLED
#define LOG_BUFFER_SIZE 1024 * 5

void udpprint(const char* str);

void udplog(const char* str);
void udplogf(const char* str, ...);
void udpwarn(const char* str);
void udpwarnf(const char* str, ...);