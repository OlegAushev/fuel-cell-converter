/**
 * @defgroup sysinfo Sysinfo
 *
 * @file
 * @ingroup sysinfo
 */


#pragma once


#include <stdint.h>
#include <stddef.h>


namespace sys {
/// @addtogroup sysinfo
/// @{


extern const char* DEVICE_NAME;
extern const char* DEVICE_NAME_SHORT;

extern const char* FIRMWARE_VERSION;
extern const uint32_t FIRMWARE_VERSION_NUM;

extern const char* BUILD_CONFIGURATION;
extern const char* BUILD_CONFIGURATION_SHORT;


/// @}
} // namespace sys


