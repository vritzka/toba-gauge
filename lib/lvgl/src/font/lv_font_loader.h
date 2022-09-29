/**
 * @file lv_font_loader.h
 *
 */

#ifndef LV_FONT_LOADER_H
#define LV_FONT_LOADER_H

/*********************
 *      INCLUDES
 *********************/
#include "Particle.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_font_t * lv_font_load(const char * fontName);
void lv_font_free(lv_font_t * font);

/**********************
 *      MACROS
 **********************/

#endif /*LV_FONT_LOADER_H*/
