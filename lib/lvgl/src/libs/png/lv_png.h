/**
 * @file lv_png.h
 *
 */

#ifndef LV_PNG_H
#define LV_PNG_H

/*********************
 *      INCLUDES
 *********************/
#include "Particle.h" 
#include "../../lv_conf_internal.h"
#if LV_USE_PNG

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register the PNG decoder functions in LVGL
 */
void lv_png_init(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_PNG*/

#endif /*LV_PNG_H*/
