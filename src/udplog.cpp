#include "udplog.hpp"
#include <stdarg.h>

void udpprint(const char* str){
    WHBLogPrint(str);
}

inline void udplog(const char* str){
    char buffer[LOG_BUFFER_SIZE];
    snprintf(buffer, LOG_BUFFER_SIZE, "[%s]>Log: %s", __FILE__, str);
    udpprint(buffer);
}

inline void udplogf(const char* str, ...){
    char buffer[LOG_BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsnprintf(buffer, LOG_BUFFER_SIZE, str, args);
	udplog(buffer);
	va_end(args);
}

inline void udpwarn(const char* str){
    char buffer[LOG_BUFFER_SIZE];
    snprintf(buffer, LOG_BUFFER_SIZE, "[%s]>Warn: %s", __FILE__, str);
    udpprint(buffer);
}

inline void udpwarnf(const char* str, ...){
    char buffer[LOG_BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsnprintf(buffer, LOG_BUFFER_SIZE, str, args);
	udpwarn(buffer);
	va_end(args);
}