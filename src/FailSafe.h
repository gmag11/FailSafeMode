/**
  * @file FailSafe.h
  * @version 0.1.0
  * @date 11/11/2020
  * @author German Martin
  * @brief Library to add a simple fail safe mode to any ESP32 or ESP8266 project
  */

#ifndef _BOOTCHECK_h
#define _BOOTCHECK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef ESP32
#undef USE_RTC
#define USE_SPIFFS 1 // Do not modify
#endif // ESP32

#ifdef ESP8266
#define USE_SPIFFS                  1  // Set to 1 to use File System.
                                       // Set to 0 to use RTC memory. Only for ESP8266. Does not work with power off.
#if !USE_SPIFFS
#define USE_RTC 1 // Do not modify
#endif // !USE_SPIFFS
#endif // ESP8266

#define FAIL_SAFE_DEBUG 0 // Set to 1 to enable log. If usinf ESP32 you need to set log level to WARN at least

#if FAIL_SAFE_DEBUG
#ifdef ESP8266
#define DEBUG_INTERFACE Serial
#ifdef DEBUG_INTERFACE
const char* extractFileName (const char* path);
#define DEBUG_LINE_PREFIX() DEBUG_INTERFACE.printf_P (PSTR("[%lu][%s:%d] %s() Heap: %lu | "),millis(),extractFileName(__FILE__),__LINE__,__FUNCTION__,(unsigned long)ESP.getFreeHeap())
#define fsDebug(text,...) DEBUG_LINE_PREFIX();DEBUG_INTERFACE.printf_P(PSTR(text),##__VA_ARGS__);DEBUG_INTERFACE.println()
#endif // DEBUG_INTERFACE
#else  // ESP32
#define DEFAULT_LOG_TAG "FailSafe"
#define fsDebug(format,...) ESP_LOGW (DEFAULT_LOG_TAG,"%d Heap: %6d. " format, millis(), ESP.getFreeHeap(), ##__VA_ARGS__)
#endif // ESP8266
#else
#define fsDebug(text,...)
#endif // FAIL_SAFE_DEBUG


#if defined ESP32 || defined USE_SPIFFS
constexpr auto FILENAME = "/failsafe.bin";
#endif

#define DEFAULT_BOOT_FLAG_TIMEOUT   10000        /**< Boot flag is reset this time after boot */
#define DEFAULT_OFFSET              0            /**< If RTC is used this is offset. Modify it if your project uses RTC memory too */
#define DEFAULT_MAX_BOOT_CYCLES     3            /**< Number of quick boot loops on which fail safe mode is started */
constexpr auto PASSWD =             "laplacian"; /**< Fail Safe mode WiFi AP password */

enum failSafeMode_t {
    OFF = 0,        /**< Normal mode */
    TRIGGERED = 1,  /**< Fail safe mode triggered, not started yet */
    RUNNING = 2     /**< Fail safe mode active */
};

struct bootFlag_t {
#if USE_RTC
    int32_t crc;        /**< If RTC is used this field is used to check data integrify */
#endif // USE_RTC
    int32_t bootCycles; /**< Booth cycle counter */
};

#ifdef ESP8266
/**
 * @brief Extract filename fom a path, to use it in debug messages
 * @param path Complete file path
 * @return file name without path
 */
const char* IRAM_ATTR extractFileName (const char* path);
#endif

class FailSafeClass
{
 protected:
     bootFlag_t bootFlag;               /**< Boot counter */
     uint32_t offset;                   /**< RTC memory offset */
     failSafeMode_t failSafe = OFF;     /**< Current fail safe mode status */
     int indicatorLed;                  /**< LED to show fail safe mode status. Normally off (= HIGH) */

     /**
      * @brief Save boot flag on persistent storage (RTC or SPIFFS)
      * @return Save result, false in case of error
      */
     bool saveFlag ();

     /**
      * @brief Load last boot counter from storage (RTC or SPIFFS)
      * @return Save result, false in case of error
      */
     bool loadFlag ();

     /**
      * @brief Internal setup method for fail safe mode
      */
     void failSafeModeSetup ();

     /**
      * @brief Internal loop method in fail safe mode
      */
     void failSafeModeLoop ();

     /**
      * @brief Reset boot counter to 0 and update storage
      */
     void resetFlag ();

 public:

     /**
      * @brief Checks if fail safe mode should be activated. This should be called at the beginning of main setup function
      * @param maxBootCycles Number of cycles after device enters in Fail Safe mode
      * @param memOffset RTC memory offset. This is not used if storage is SPIFFS
      * @param led LED to indicate Fail Safe mode status. Normally off (= HIGH)
      */
     void checkBoot (int maxBootCycles = DEFAULT_MAX_BOOT_CYCLES, int led = -1, uint32_t memOffset = DEFAULT_OFFSET);

     /**
      * @brief Triggers fail safe mode manually from user code. IT can be used if WiFi cannot connect, for instance
      */
     void startFailSafe () {
         failSafe = TRIGGERED;
     }

     /**
      * @brief Checks if fail safe mode is active to help controlling user code
      * @return Fail Safe mode status
      */
     failSafeMode_t isActive () {
         return failSafe;
     }

     /**
      * @brief Internal loop should be called on every main code loop
      * @param timeout Boot flag is reset this time after boot
      */
     void loop (uint32_t timeout = DEFAULT_BOOT_FLAG_TIMEOUT);

     /**
      * @brief Converts current status to a char string
      * @return Textual Fail Safe mode status
      */
     const char* toString ();
};

extern FailSafeClass FailSafe; /**< Static instance */

#endif

