/******************************************************************************

  Horddays - Zombie apocalypse roguelike game.
  Copyright (C) 2014 Alexey Nikolaev.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
******************************************************************************/
#include "grid.h"
#include "state.h"
#include "gfx.h"

#include "output.h"

#define POSX(s,i) ((i) + CENTER_X - (s)->camera.x)
#define POSY(s,j) ((j) + CENTER_Y - (s)->camera.y)

const char* noises[] = {
  "noise", "growl", "snarl", "moan", "groan", 
  "gurgle", "drawl", "hum", "rumble", "thud", 
  "yar", "bellow", "grumble", "grunt", "mutter", 
  "bang", "boom", "clang", "rattle", "cham", 
  "champ", "chomp", "crunch", "grump"  };

void draw_hp (struct state *s, int x, int y, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *screen) {
  int hp = s->pl.hp;
  int i;
  int dpic = 0;
  if(s->pl.cond == bitten) { dpic = 2; }
  for(i=0; i<hp; ++i) {
    blit_subpic (tileset, screen, 0 + (i+1)%2 + dpic , 12, x+i/2, y);
  }
}

void draw_all (struct state *s, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *screen) {
  static const int shadow[3][3] = {{4,0,7},{1,10,3},{5,2,6}};
  
  /* Clear screen */
  SDL_Rect rect = {0, 0, screen->w, screen->h};
  SDL_FillRect(screen, &rect, (SDL_MapRGB(screen->format, 20, 20, 20)));

  /* Draw */
  int i=0,j=0,k=0;

  int di, dj;

  int b_see_something;

  for (j=0; j<s->grid.height; ++j) {
    for (i=0; i<s->grid.width; ++i) {

      b_see_something = 0;
      for (k = 0; k<SZ; ++k) {
        if (s->vision.v[i][j][k] && 
            !tl_lookthru (s->grid.loc[i][j][k].tl)) {
          b_see_something = 1;
        }
      }

      k = zlevel(&s->grid, i, j);
      if (k < SZ-1 || 1) {
        if (s->vision.v[i][j][k]) {
          blit_subpic (tileset, screen, k, 0, POSX(s,i), POSY(s,j));

          /* blood */
          if (s->grid.loc[i][j][k].stains > 0) {
            blit_subpic (tileset, screen, BLOOD_STAINS_MAX - s->grid.loc[i][j][k].stains, 10, POSX(s,i), POSY(s,j));
          }

          /* shadows */
          if (i>0 && i<s->grid.width-1 && j>0 && j<s->grid.height-1 && k < SZ-1) {
            for(di=-1; di<=1; ++di){
              for(dj = -1; dj<=1; ++dj) {
                if ( s->grid.block[i+di][j+dj][k] ) { 
                  if ( s->grid.block[i+di][j+dj][k+1] ) {
                    blit_subpic (tileset, screen, 4 + shadow[di+1][dj+1], 1, POSX(s,i), POSY(s,j));
                  }
                  else {
                    blit_subpic (tileset, screen, 4 + shadow[di+1][dj+1], 2, POSX(s,i), POSY(s,j));
                  }
                }
              }
            }
          }
          
          /* exit */
          if (s->grid.loc[i][j][k].tl == tl_exit) {
            blit_subpic (tileset, screen, 3, 1, POSX(s,i), POSY(s,j));
          }
          
          /* item */
          if (s->grid.loc[i][j][k].id_item != ID_NO_ITEM) {
            blit_subpic (tileset, screen, s->grid.loc[i][j][k].id_item, 14, POSX(s,i), POSY(s,j));
          }

          /* draw the mob, if there is any */
          int id = s->grid.loc[i][j][k].id_mob;
          if (id != ID_NONE && id != ID_PLAYER) {
            struct mob * mob = &s->mobs.mb[id];
            int row = 4;
            if (mob->senses.smell) {row = 4;}
            if (mob->senses.sound) {row = 6;}
            if (mob->senses.sight) {row = 8;}
            blit_subpic (tileset, screen, mob->dir, row, POSX(s,i), POSY(s,j));
            blit_subpic (tileset, screen, mob->portrait, row+1, POSX(s,i), POSY(s,j));
          }

          /* draw smell */
          if (s->status.effect[EF_OLFACTOVISOR]>0) {
            if (s->grid.loc[i][j][k].smell > 0) {
              int smell = s->grid.loc[i][j][k].smell;
              int grain = 2;
              int z = (smell + grain - 1) / grain;
              int pic = (z > SMELL_PIC_MAX) ? (SMELL_PIC_MAX) : z;
              blit_subpic (tileset, screen, SMELL_PIC_MAX - pic, 11, POSX(s,i), POSY(s,j));
            }
          }

          /* smoke */
          if (s->grid.loc[i][j][k].smoke > 0) {
            blit_subpic (tileset, screen, 7, 10, POSX(s,i), POSY(s,j));
          }

          /* crosshair */
          if(s->mode == mode_throw) {
            blit_subpic (tileset, screen, 4, 12, POSX(s,s->thr.x), POSY(s,s->thr.y));
          }
        }
        else {
          if (s->vision.seen[i][j][k]) {        
            blit_subpic (tileset, screen, k, 0, POSX(s,i), POSY(s,j));
            blit_subpic (tileset, screen, 2, 1, POSX(s,i), POSY(s,j));
          }
          if (b_see_something) {
            if (k < SZ-1) {
              blit_subpic (tileset, screen, 0, 1, POSX(s,i), POSY(s,j)); }
            else {
              blit_subpic (tileset, screen, 1, 1, POSX(s,i), POSY(s,j)); }
          }
        }
      }
    }
  }
        
  /* Player */

  blit_subpic (tileset, screen, 3, 2, POSX(s, s->pl.coord.x), POSY(s, s->pl.coord.y));

  /* marks */
  for (i=0; i<7 && i<s->pl.marks; ++i) {
    blit_subpic (tileset, screen, i, 13, POSX(s, s->pl.coord.x), POSY(s, s->pl.coord.y));
  }

  draw_hp(s, CENTER_X*2+2, 1, tileset, typeface, screen);


  /* Items */
  {
    int x = CENTER_X*2 + 7;
    int y = CENTER_Y*2 + 1;
    int items_to_show = 7;
    int items_num = 0;
    struct item_list *ls = s->pl.items;
    while ( ls != NULL && items_num < items_to_show ) {items_num++; ls = ls->tail;}
    
    ls = s->pl.items;
   
    int i;
    for (i = items_num; i>0; --i){
      blit_subpic (tileset, screen, 3, 15, x-i, y-1);
      blit_subpic (tileset, screen, 3, 16, x-i, y);
      blit_subpic (tileset, screen, 3, 17, x-i, y+1);
    }

    /* frame */
    if(items_num>0){
      blit_subpic (tileset, screen, 3+1, 15, x, y-1);
      blit_subpic (tileset, screen, 3+1, 16, x, y);
      blit_subpic (tileset, screen, 3+1, 17, x, y+1);
      
      blit_subpic (tileset, screen, 3-1, 15, x-items_num-1, y-1);
      blit_subpic (tileset, screen, 3-1, 16, x-items_num-1, y);
      blit_subpic (tileset, screen, 3-1, 17, x-items_num-1, y+1);
      
      blit_subpic (tileset, screen, 1, 16, x-items_num, y);
    }

    /* items */
    i = items_num;
    while ( i > 0 ) {
      
      if(i == 1 && ls->tail != NULL) {
        blit_subpic (tileset, screen, 1, 15, x-i, y);
      }
      else {
        blit_subpic (tileset, screen, ls->id_item, 14, x-i, y);
      }
      
      ls = ls->tail;
      i--;
    }
  }
  
  /* Text */

  int screen_y = 20 * TILE_HEIGHT;
  
  output_string(typeface, screen, "[Ctrl]/[f] use item", 
      TILE_WIDTH + 25*TYPE_WIDTH, screen_y + 3*TYPE_HEIGHT);

  output_string(typeface, screen, "[Space]/[Num5] wait", 
      TILE_WIDTH + 0*TYPE_WIDTH, screen_y + 3*TYPE_HEIGHT);
 
  char str_score[25];
  char str_level[25];
  sprintf(str_score, "Score: %i", s->status.score);
  sprintf(str_level, "Day: %i", s->status.level);

  int status_text_x = (CENTER_X*2 + 1 + 1) * TILE_WIDTH + 0*TYPE_WIDTH;
  int status_text_y =  3 * TILE_HEIGHT;

  
  /* Wind info */
  output_string(typeface, screen, "Wind:", 
      status_text_x, status_text_y + 6 * TYPE_HEIGHT);
  blit_subpic (tileset, screen, s->wind_dir, 3, CENTER_X*2+4, 6);

  
  /* Score */ 
  output_string(typeface, screen, str_score, 
      status_text_x, status_text_y + 2*TYPE_HEIGHT);
  output_string(typeface, screen, str_level, 
      status_text_x, status_text_y + 4*TYPE_HEIGHT);

  if(s->status.effect[EF_PISTOL]>0){
    char str[25];
    output_string(typeface, screen, "Shooting", 
        status_text_x, status_text_y + 8*TYPE_HEIGHT);
    sprintf(str, "Rounds: %i", s->status.effect[EF_PISTOL]);
    output_string(typeface, screen, str, 
        status_text_x, status_text_y + 9*TYPE_HEIGHT);
  }
  if(s->status.effect[EF_OLFACTOVISOR]>0){
    char str[25];
    sprintf(str, "Olfactovisor on");
    output_string(typeface, screen, str, 
        status_text_x, status_text_y + 11*TYPE_HEIGHT);
  }
  if(s->status.effect[EF_CANNIBAL]>0){
    output_string(typeface, screen, "(: Playing  :)", 
        status_text_x, status_text_y + 13*TYPE_HEIGHT);
    output_string(typeface, screen, "(: Cannibal :)", 
        status_text_x, status_text_y + 14*TYPE_HEIGHT);
    output_string(typeface, screen, "(:  Corpse  :)", 
        status_text_x, status_text_y + 15*TYPE_HEIGHT);
  }
}
