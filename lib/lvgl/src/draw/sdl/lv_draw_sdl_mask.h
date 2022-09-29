/**
 * @file lv_draw_sdl_mask.h
 *
 */

#ifndef LV_DRAW_SDL_MASK_H
#define LV_DRAW_SDL_MASK_H

/*********************
 *      INCLUDES
 *********************/
#include "Particle.h"
#include "../../lv_conf_internal.h"

#include LV_DRAW_SDL_INCLUDE_PATH

#include "lv_draw_sdl.h"
#include "../../misc/lv_area.h"
#include "../../misc/lv_color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_opa_t * lv_draw_sdl_mask_dump_opa(const lv_area_t * coords, const int16_t * ids, int16_t ids_count);

SDL_Texture * lv_draw_sdl_mask_dump_texture(SDL_Renderer * renderer, const lv_area_t * coords, const int16_t * ids,
                                            int16_t ids_count);


/**********************
 *      MACROS
 **********************/

#endif /*LV_DRAW_SDL_MASK_H*/
