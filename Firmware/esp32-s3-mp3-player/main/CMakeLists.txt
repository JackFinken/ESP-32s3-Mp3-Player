idf_component_register(SRCS "esp32-s3-mp3-player.c"
        INCLUDE_DIRS "."
        REQUIRES esp_lcd esp_driver_gpio esp_driver_spi)