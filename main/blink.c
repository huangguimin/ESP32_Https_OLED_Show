/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "global.h"
// CLK - GPIO14
#define PIN_CLK 14

// MOSI - GPIO 13
#define PIN_MOSI 13

// RESET - GPIO 26
#define PIN_RESET 26

// DC - GPIO 27
#define PIN_DC 27

// CS - GPIO 15
#define PIN_CS 15
static char tag[] = "test_SSD1306";

#define BLINK_GPIO CONFIG_BLINK_GPIO

/* The examples use WiFi configuration that you can set via 'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY
/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about one event 
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAG = "wifi station";
static int s_retry_num = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        printf("got ip:%s\r\n",
               ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            printf("retry to connect to the AP\r\n");
        }
        printf("connect to the AP fail\n");
        break;
    }
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS},
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    printf("wifi_init_sta finished.\r\n");
    printf("connect to ap SSID:%s password:%s\r\n",
           EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void u8g2_DrawStrGB(u8g2_t *u8g2_GB, unsigned short x, unsigned short y, const unsigned char *pGB)
{
    unsigned long offset;
    unsigned int i;

    for (i = 0; *(pGB + i) != '\0' && i < 64; i += 2)
    {
        offset = (94 * (unsigned int)(pGB[i] - 0xa0 - 1) + (pGB[i + 1] - 0xa0 - 1)) * 32;
        u8g2_DrawBitmap(u8g2_GB, x, y, 2, 16, &data[offset]);
        x = x + 16;
        if (x >= 128)
        {
            y += 16;
            x = 0;
        }
    }
}
u8g2_t u8g2; // a structure which will contain all the data for one display
float temper = 1, humidity = 2, pressure = 3;
char GBstrbuf[66] = {0xCE, 0xD2};

void task_SSD1306(void *ignore)
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.clk = PIN_CLK;
    u8g2_esp32_hal.mosi = PIN_MOSI;
    u8g2_esp32_hal.cs = PIN_CS;
    u8g2_esp32_hal.dc = PIN_DC;
    u8g2_esp32_hal.reset = PIN_RESET;

    ESP_LOGD(tag, "stat init!");

    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_128x64_noname_f(
        &u8g2,
        U8G2_R0,
        u8g2_esp32_spi_byte_cb,
        u8g2_esp32_gpio_and_delay_cb); // init u8g2 structure

    u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

    u8g2_SetPowerSave(&u8g2, 0); // wake up display
    u8g2_ClearBuffer(&u8g2);
    //u8g2_DrawBox(&u8g2, 10,20, 20, 30);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
    //u8g2_DrawStr(&u8g2, 0,15,"Hello World!");
    u8g2_SendBuffer(&u8g2);

    ESP_LOGD(tag, "All done!");

    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    char str[10];
    memset(GBstrbuf, '\0', 64);
    GBstrbuf[0] = 0xCE;
    GBstrbuf[1] = 0xD2;

    while (1)
    {

        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStrGB(&u8g2, 0, 0, (const unsigned char *)GBstrbuf);

        //Test3D4();
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    //vTaskDelete(NULL);
}

void task_LED(void *ignore)
{
    while (1)
    {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    //task_test_SSD1306(NULL);
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    // wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("ESP_WIFI_MODE_STA\r\n");
    wifi_init_sta();

    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
    xTaskCreate(&task_SSD1306, "task_SSD1306", 2048, NULL, 5, NULL);
    xTaskCreate(&task_LED, "task_LED", 1024, NULL, 5, NULL);
}
