# Read and display BMP from SD card with LVGL on ESP32-WROVER-KIT

To force using PSRAM set something like this (malloc using PSRAM for allocations above 100 bytes) in `idf.py menuconfig`:

```
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=100
```

Format your SD card to FAT32 and put there 16bit and/or 32bit BMP images with short name (FATFS constraint) and set those to `BMP_16BIT`, `BMP_32BIT` defines in `main/lvgl_test.c`.

_32 bit pictures however won't display correctly on ESP32-WROVER-KIT due to it being only able to show max 18 bit (RGB666) so padded 16 bit colors (`CONFIG_LV_COLOR_16_SWAP`) are used in LVGL (RGB565)._

Then set desired bit depth in `idf.py menuconfig` LVGL color settings and compile.