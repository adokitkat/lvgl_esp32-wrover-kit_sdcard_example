#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "bsp/esp-bsp.h"
#include "lvgl.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#include "driver/sdmmc_host.h"
#endif

static const char *TAG = "LVGL_test";

#define MOUNT_POINT "/sdcard"

// ESP32-WROVER-KIT
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13

#define SD_CATD_SPI_HOST 2 // VSPI_HOST / SPI3

#define BMP_16BIT "/esp16.bmp"
#define BMP_32BIT "/esp24.bmp"

extern void example_lvgl_demo_ui(lv_obj_t *scr);

void mount_sd(void) {
    esp_err_t ret;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = 2; 
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SD_CATD_SPI_HOST);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

static lv_obj_t *pic;

void example_lvgl_bmp(lv_obj_t *scr) {
    pic = lv_img_create(scr);
#ifdef CONFIG_LV_COLOR_DEPTH_16
    // #ifndef CONFIG_LV_COLOR_16_SWAP // Color fix for ESP32-WROVER-KIT
    // #define CONFIG_LV_COLOR_16_SWAP
    // #endif
    lv_img_set_src(pic, "A:"MOUNT_POINT BMP_16BIT);
#elif CONFIG_LV_COLOR_DEPTH_32
    lv_img_set_src(pic, "A:"MOUNT_POINT BMP_32BIT);
#else
    #error "Unsupported LVGL color depth"
#endif
    lv_obj_center(pic);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Hello");

    bsp_display_start();

    ESP_LOGI(TAG, "Display LVGL picture");
    bsp_display_lock(0);
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    
    mount_sd();

    example_lvgl_bmp(scr);

    bsp_display_unlock();
    bsp_display_backlight_on();
}
