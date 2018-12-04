#ifndef __GLOABAL_H__
#define __GLOABAL_H__

#include <math.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_spi_flash.h"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_tls.h"

#include "HZK16.h"

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"
#include "u8g2.h"

#include "https_request.h"

#include <iconv.h>
#include "UTF8ToGB2312.h"

extern EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about one event 
 * - are we connected to the AP with an IP? */
extern const int WIFI_CONNECTED_BIT;

extern char GBstrbuf[66];

#define PI 3.141592654
extern u8g2_t u8g2;
#endif
