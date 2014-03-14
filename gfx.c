
#include "gfx.h"

Uint32 getpixel(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      return *p;
      break;

    case 2:
      return *(Uint16 *)p;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
      break;

    case 4:
      return *(Uint32 *)p;
      break;

    default:
      return 0;       /* shouldn't happen, but avoids warnings */
  }
}

int load_image(char *filename, SDL_Surface **image) {
  SDL_Surface *temp;
   
  temp = SDL_LoadBMP(filename);
  if (temp == NULL) {
    printf("Unable to load bitmap: %s\n", SDL_GetError());
      return 1;
  }

  SDL_LockSurface(temp);
  Uint32 colorkey = getpixel(temp, temp->w-1, temp->h-1);
  SDL_UnlockSurface(temp);
  colorkey = SDL_MapRGB(temp->format, 0, 255, 255);

  if (SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, colorkey) == -1) {
    fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());
  }
   
  *image = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  return 0;
}

void blit_arb(SDL_Surface *src_surf, SDL_Surface *dst_surf, 
    int si, int sj, int sw, int sh, int di, int dj, int dw, int dh) {

  static SDL_Rect src, dest;
   
  src.x = si;
  src.y = sj;
  src.w = sw;
  src.h = sh;
   
  dest.x = di;
  dest.y = dj;
  dest.w = dw;
  dest.h = dh;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}

void blit_subpic(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj) {
  static SDL_Rect src, dest;
   
  src.x = srci * TILE_WIDTH;
  src.y = srcj * TILE_HEIGHT;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT;
   
  dest.x = dsti * TILE_WIDTH  /* + dstj*(TILE_WIDTH/2) */;
  dest.y = dstj * TILE_HEIGHT;
  dest.w = TILE_WIDTH;
  dest.h = TILE_HEIGHT;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}

void blit_subpic_noise(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj, int variant) {
  static SDL_Rect src, dest;
   
  src.x = srci * TILE_WIDTH;
  src.y = srcj * TILE_HEIGHT;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT;
   
  int rangex = 1;
  int rangey = 1;
  int rndx = variant%(2*rangex+1) - rangex;
  int rndy = variant%(rangey+1);

  dest.x = dsti * TILE_WIDTH + /* dstj*(TILE_WIDTH/2) */ + rndx;
  dest.y = dstj * TILE_HEIGHT + rndy;
  dest.w = TILE_WIDTH;
  dest.h = TILE_HEIGHT;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}

/* double height tile */
void blit_subpic_2h(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj) {
  static SDL_Rect src, dest;
   
  src.x = srci * TILE_WIDTH;
  src.y = srcj * TILE_HEIGHT - TILE_HEIGHT;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT*2;
   
  dest.x = dsti * TILE_WIDTH + dstj*(TILE_WIDTH/2);
  dest.y = dstj * TILE_HEIGHT - TILE_HEIGHT;
  dest.w = TILE_WIDTH;
  dest.h = TILE_HEIGHT*2;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}



void output_char(SDL_Surface *typeface, SDL_Surface *screen, int c, int dstx, int dsty){
  static SDL_Rect src, dest;
  
  int z = c - TYPE_FIRST;
  int j = z / TYPE_LINE_LENGTH;
  int i = z % TYPE_LINE_LENGTH;

  src.x = i * TYPE_WIDTH;
  src.y = j * TYPE_HEIGHT;
  src.w = TYPE_WIDTH;
  src.h = TYPE_HEIGHT;
   
  dest.x = dstx;
  dest.y = dsty;
  dest.w = TYPE_WIDTH;
  dest.h = TYPE_HEIGHT;
   
  SDL_BlitSurface(typeface, &src, screen, &dest);
}

void output_string(SDL_Surface *typeface, SDL_Surface *screen, char *str, int dstx, int dsty){
  int i=0;
  while(str[i]!='\0') {
    output_char(typeface, screen, str[i], dstx+i*TYPE_WIDTH, dsty);
    i++;
  }
}

void output_string_alt(SDL_Surface *typeface, int player, SDL_Surface *screen, char *str, int dstx, int dsty){
  int i=0;
  while(str[i]!='\0') {
    output_char(typeface, screen, str[i] + TYPE_LINE_LENGTH*(3+player), dstx+i*TYPE_WIDTH, dsty);
    i++;
  }
}

