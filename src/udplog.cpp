#include "udplog.hpp"
#include <stdarg.h>

void udplog(const char* str){
    #ifdef LOG_ENABLED
    WHBLogPrint(str);
    #endif
}

void udplogf(const char* str, ...){
    #ifdef LOG_ENABLED
    char buffer[LOG_BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsnprintf(buffer, LOG_BUFFER_SIZE, str, args);
	udplog(buffer);
	va_end(args);
    #endif
}