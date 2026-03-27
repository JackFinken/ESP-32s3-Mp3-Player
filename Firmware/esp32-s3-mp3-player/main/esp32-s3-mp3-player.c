#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "lvgl.h"

static const char *TAG = "MP3_UI";

#define LCD_HOST        SPI2_HOST
#define PIN_SCLK        14
#define PIN_MOSI        13
#define PIN_RST         12
#define PIN_DC          11
#define PIN_CS          10
#define PIN_BLK         9
#define LCD_H_RES       240
#define LCD_V_RES       280
#define LVGL_BUF_HEIGHT 40

static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_disp_drv_t disp_drv;

static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_flush_ready(&disp_drv);
    return false;
}

static void lcd_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}

static void create_mp3_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x121212), 0);

    lv_obj_t *header = lv_label_create(scr);
    lv_label_set_text(header, "NOW PLAYING");
    lv_obj_set_style_text_color(header, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_14, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *album_art = lv_obj_create(scr);
    lv_obj_set_size(album_art, 160, 160);
    lv_obj_align(album_art, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(album_art, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_width(album_art, 0, 0);
    lv_obj_set_style_radius(album_art, 8, 0);

    lv_obj_t *music_note = lv_label_create(album_art);
    lv_label_set_text(music_note, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(music_note, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(music_note, &lv_font_montserrat_14, 0);
    lv_obj_center(music_note);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Song Title");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 195);

    lv_obj_t *artist = lv_label_create(scr);
    lv_label_set_text(artist, "Artist Name");
    lv_obj_set_style_text_color(artist, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(artist, &lv_font_montserrat_14, 0);
    lv_obj_align(artist, LV_ALIGN_TOP_MID, 0, 215);

    lv_obj_t *progress = lv_bar_create(scr);
    lv_obj_set_size(progress, 200, 4);
    lv_obj_align(progress, LV_ALIGN_TOP_MID, 0, 233);
    lv_bar_set_value(progress, 35, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x1DB954), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progress, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(progress, 2, LV_PART_INDICATOR);

    lv_obj_t *time_elapsed = lv_label_create(scr);
    lv_label_set_text(time_elapsed, "1:10");
    lv_obj_set_style_text_color(time_elapsed, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(time_elapsed, &lv_font_montserrat_14, 0);
    lv_obj_align(time_elapsed, LV_ALIGN_TOP_LEFT, 20, 241);

    lv_obj_t *time_total = lv_label_create(scr);
    lv_label_set_text(time_total, "3:20");
    lv_obj_set_style_text_color(time_total, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(time_total, &lv_font_montserrat_14, 0);
    lv_obj_align(time_total, LV_ALIGN_TOP_RIGHT, -20, 241);

    lv_obj_t *vol_icon = lv_label_create(scr);
    lv_label_set_text(vol_icon, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_color(vol_icon, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(vol_icon, &lv_font_montserrat_14, 0);
    lv_obj_align(vol_icon, LV_ALIGN_TOP_LEFT, 20, 256);

    lv_obj_t *volume = lv_bar_create(scr);
    lv_obj_set_size(volume, 160, 4);
    lv_obj_align(volume, LV_ALIGN_TOP_LEFT, 40, 259);
    lv_bar_set_value(volume, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(volume, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(volume, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(volume, 2, LV_PART_INDICATOR);

    lv_obj_t *prev_btn = lv_label_create(scr);
    lv_label_set_text(prev_btn, LV_SYMBOL_PREV);
    lv_obj_set_style_text_color(prev_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(prev_btn, &lv_font_montserrat_14, 0);
    lv_obj_align(prev_btn, LV_ALIGN_TOP_LEFT, 35, 268);

    lv_obj_t *play_btn = lv_label_create(scr);
    lv_label_set_text(play_btn, LV_SYMBOL_PAUSE);
    lv_obj_set_style_text_color(play_btn, lv_color_hex(0x1DB954), 0);
    lv_obj_set_style_text_font(play_btn, &lv_font_montserrat_14, 0);
    lv_obj_align(play_btn, LV_ALIGN_TOP_MID, 0, 265);

    lv_obj_t *next_btn = lv_label_create(scr);
    lv_label_set_text(next_btn, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(next_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(next_btn, &lv_font_montserrat_14, 0);
    lv_obj_align(next_btn, LV_ALIGN_TOP_RIGHT, -35, 268);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing display");

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t),
    };
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_DC,
        .cs_gpio_num = PIN_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = lcd_flush_ready,
        .user_ctx = NULL,
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };

    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 20);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    gpio_set_direction(PIN_BLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_BLK, 1);

    lv_init();

    lv_color_t *buf1 = heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);

    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_H_RES * LVGL_BUF_HEIGHT);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lcd_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Creating UI");
    create_mp3_ui();

    while (1) {
        lv_tick_inc(10);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}