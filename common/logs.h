#pragma

#include <iostream>

#ifdef ENABLE_LOGGING
    #define LOG_INFO(msg) std::cout << "[INFO]: " << msg << std::endl;
    #define LOG_ERROR(msg) std::cerr << "[ERROR]: " << msg << std::endl;
#else
    #define LOG_INFO(msg)
    #define LOG_ERROR(msg)
#endif