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

#include <stdlib.h>
#include <time.h>
#include "SDL.h"

#ifdef WIN32
#undef main
#endif

#include "state.h"
#include "sim.h"
#include "init.h"
#include "input.h"
#include "gfx.h"
#include "output.h"
#include "path.h"

#define TIME_DELAY 10

void run(struct state *s,
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface){
  
  int finished = 0;
  SDL_Event event;
  int previous_time = SDL_GetTicks();

  int k = 0;
  while (!finished){

    int time = SDL_GetTicks();

    if (time - previous_time >= TIME_DELAY) {

      previous_time = previous_time + TIME_DELAY;
      k++;
      if (k>=1600) k=0;

      char c = '\0';
      while (!finished && SDL_PollEvent(&event)){
        switch (event.type){
          case SDL_KEYDOWN:
            c = '\0';
            switch (event.key.keysym.sym) {
              case SDLK_KP4:
              case SDLK_LEFT: c = 'h'; break;
              case SDLK_KP6:
              case SDLK_RIGHT: c = 'l'; break;
              case SDLK_KP8:
              case SDLK_UP: c = 'k'; break;
              case SDLK_KP2:
              case SDLK_DOWN: c = 'j'; break;
              case SDLK_KP7: c = 'y'; break;
              case SDLK_KP9: c = 'u'; break;
              case SDLK_KP1: c = 'b'; break;
              case SDLK_KP3: c = 'n'; break;
              
              case SDLK_RCTRL: ;
              case SDLK_LCTRL: 
                             c = 'f'; break;
              
              case SDLK_q: c = 'q'; break;
              case SDLK_KP5:;
              case SDLK_SPACE: c = ' '; break;
              default:
                if ( (event.key.keysym.unicode & 0xFF80) == 0 ) {
                  c = event.key.keysym.unicode & 0x7F;
                }
            }
            finished = process_input(s, c);
            if ('q' == c) {finished = 1;}
            break;
        }
      }

      if (!(s->players_turn)) {update_vision(s);}
      run_mobs(s);

      if (k % 5 == 0) {
        draw_all(s, tileset, typeface, screen);
        SDL_Flip(screen);
      }
    }
    else{
      SDL_Delay(TIME_DELAY);
    }
  }
}

int main(int argc, char *argv[]) {

  srand(time(NULL));

  /* Initialize the state */
  struct state *st = malloc(sizeof(*st));

  state_init(st);

  int screen_width = 20 * TILE_WIDTH + TYPE_WIDTH * 23;
  int screen_height = 20 * TILE_HEIGHT + TYPE_HEIGHT * 5;

  //screen_width = 960;
  //screen_height = 540;

  /* Init SDL */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_Surface *screen;
  screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(150, 50);

  /* Load Data */
  char **path;
  path = get_search_paths();
  
  /* Images */
  char *filename;
  
  SDL_Surface *tileset;
  filename = find_file (path, "images/tileset.bmp");
  if ( load_image(filename, &tileset) != 0 ) return 1;
  free(filename); 
  
  SDL_Surface *typeface;
  filename = find_file (path, "images/type.bmp");
  if ( load_image(filename, &typeface) != 0 ) return 1;
  free(filename);
  
  run(st, screen, tileset, typeface);

  /* Finalize */
  SDL_FreeSurface(tileset);
  SDL_FreeSurface(typeface);
  
  destroy_search_paths(path);
  return 0;
}
