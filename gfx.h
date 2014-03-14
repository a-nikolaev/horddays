

#ifndef _GFX_H
#define _GFX_H

#include "SDL.h"

#define TILE_WIDTH 28
#define TILE_HEIGHT 28

#define TYPE_WIDTH 9
#define TYPE_HEIGHT 15
#define TYPE_LINE_LENGTH 32
#define TYPE_FIRST 33

Uint32 getpixel(SDL_Surface *surface, int x, int y);

/*
   load_image (filename, &surface_ptr)
    Load BMP image, returns 0 on success, or 1 if fails.
 */
int load_image(char *filename, SDL_Surface **image);

void blit_subpic(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj);

void blit_subpic_noise(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj, int rnd_variant);

void blit_subpic_2h(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj);

/* Text output functions */
void output_char(SDL_Surface *typeface, SDL_Surface *screen, int c, int dsti, int dstj);
void output_string(SDL_Surface *typeface, SDL_Surface *screen, char *str, int dsti, int dstj);

void output_string_alt(SDL_Surface *typeface, int player, SDL_Surface *screen, char *str, int dsti, int dstj);


#endif
