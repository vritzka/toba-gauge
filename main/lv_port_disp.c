/**
 * @file lv_port_disp.c
 *
 */


/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

lv_init();

/*********************
 *      DEFINES
 *********************/

#define LCD_PIXEL_CLOCK_HZ     (18 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_BK_LIGHT       4
#define PIN_NUM_HSYNC          46
#define PIN_NUM_VSYNC          3
#define PIN_NUM_DE             0
#define PIN_NUM_PCLK           9
#define PIN_NUM_DATA0          14 // B0
#define PIN_NUM_DATA1          13 // B1
#define PIN_NUM_DATA2          12 // B2
#define PIN_NUM_DATA3          11 // B3
#define PIN_NUM_DATA4          10 // B4
#define PIN_NUM_DATA5          39 // G0
#define PIN_NUM_DATA6          38 // G1
#define PIN_NUM_DATA7          45 // G2
#define PIN_NUM_DATA8          48 // G3
#define PIN_NUM_DATA9          47 // G4
#define PIN_NUM_DATA10         21 // G5
#define PIN_NUM_DATA11         1  // R0
#define PIN_NUM_DATA12         2  // R1
#define PIN_NUM_DATA13         42 // R2
#define PIN_NUM_DATA14         41 // R3
#define PIN_NUM_DATA15         40 // R4
#define PIN_NUM_DISP_EN        -1

// The pixel number in horizontal and vertical
#define LCD_H_RES              800
#define LCD_V_RES              480

#define LVGL_TICK_PERIOD_MS    2

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[LCD_H_RES * 10];                          /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, LCD_H_RES * 10);   /*Initialize the display buffer*/

    if(false) {
    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[LCD_H_RES * 10];                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[LCD_H_RES * 10];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, LCD_H_RES * 10);   /*Initialize the display buffer*/
    }
    if(false) {
    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    static lv_disp_draw_buf_t draw_buf_dsc_3;
    static lv_color_t buf_3_1[LCD_H_RES * LCD_V_RES];            /*A screen sized buffer*/
    static lv_color_t buf_3_2[LCD_H_RES * LCD_V_RES];            /*Another screen sized buffer*/
    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, LCD_V_RES * LV_VER_RES_MAX);   /*Initialize the display buffer*/
    }
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    ESP_LOGI(TAG, "Install RGB LCD panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .psram_trans_align = 64,
#if CONFIG_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = 10 * LCD_H_RES,
#endif
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = PIN_NUM_DISP_EN,
        .pclk_gpio_num = PIN_NUM_PCLK,
        .vsync_gpio_num = PIN_NUM_VSYNC,
        .hsync_gpio_num = PIN_NUM_HSYNC,
        .de_gpio_num = PIN_NUM_DE,
        .data_gpio_nums = {
            PIN_NUM_DATA0,
            PIN_NUM_DATA1,
            PIN_NUM_DATA2,
            PIN_NUM_DATA3,
            PIN_NUM_DATA4,
            PIN_NUM_DATA5,
            PIN_NUM_DATA6,
            PIN_NUM_DATA7,
            PIN_NUM_DATA8,
            PIN_NUM_DATA9,
            PIN_NUM_DATA10,
            PIN_NUM_DATA11,
            PIN_NUM_DATA12,
            PIN_NUM_DATA13,
            PIN_NUM_DATA14,
            PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = 30, //space to the right
            .hsync_front_porch = 210,
            .hsync_pulse_width = 15, //Space on top
            .vsync_back_porch = 13,
            .vsync_front_porch = 22,
            .vsync_pulse_width = 10,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
#if CONFIG_DOUBLE_FB
        .flags.double_fb = true,   // allocate double frame buffer
#endif // CONFIG_DOUBLE_FB
    };

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{

 esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
    xSemaphoreGive(sem_gui_ready);
    xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
#endif
    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}


/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
