
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
#include <stdlib.h> /* for abs function */
#include <math.h> 

#include "common.h"
#include "state.h"
#include "item.h"
#include "sim.h"

/* returns a negative number on failure to walk or attack, or spent AP otherwise */
double pl_walk (struct state *s, int dx, int dy, double *noise) {
  int i = s->pl.coord.x;
  int j = s->pl.coord.y;
  int k = s->pl.coord.z;

  int dz = walk_attempt(&s->grid, i+dx, j+dy, k);

  if (dz != FAILED_WALK) {

    s->grid.loc[i][j][k].id_mob = ID_NONE;

    s->pl.coord.x = i+dx;
    s->pl.coord.y = j+dy;
    s->pl.coord.z = k+dz;
    
    s->grid.loc[s->pl.coord.x][s->pl.coord.y][s->pl.coord.z].id_mob = ID_PLAYER;

    /* walking noise */
    *noise = NOISE_WALK;

    return (sqrt((double)(dx*dx + dy*dy + dz*dz)));
  }
  else {
    
    int dz = attack_attempt(s, i, j, k, i+dx, j+dy);
    if (dz != FAILED_ATTACK){
      hit(s, i+dx, j+dy, k+dz, hit_normal);

      /* attacking noise - nothing here it's done by hit function */
      *noise = 0.0;
      
      return 1.0;
    }
    else
      return -1.0;
  }

}

void move_target(struct state *s, int dx, int dy){
  int nx = s->thr.x + dx;
  int ny = s->thr.y + dy;

  if (is_within_ij(&s->grid, nx, ny)) {
    int nz = zlevel(&s->grid, nx, ny);
    if (s->vision.v[nx][ny][nz] && !s->grid.block[nx][ny][nz]) {
      s->thr.x = s->thr.x + dx;
      s->thr.y = s->thr.y + dy;
    }
  }
  
}

int process_input (struct state *s, char c) {

  if (!s->players_turn) {return 0;}

  double dap = -1.0;

  double noise = NOISE_STAND;

  if (s->pl.hp <= 0) {
    switch (c) {
      case 'q': return 1;
      default : return 0;
    }
  }


  /* normal */
  if (s->mode == mode_move) {
    switch (c) {
      case 'h': dap = pl_walk(s,-1, 0, &noise); break;
      case 'j': dap = pl_walk(s, 0, 1, &noise); break;
      case 'k': dap = pl_walk(s, 0,-1, &noise); break;
      case 'l': dap = pl_walk(s, 1, 0, &noise); break;
      case 'y': dap = pl_walk(s,-1,-1, &noise); break;
      case 'u': dap = pl_walk(s, 1,-1, &noise); break;
      case 'b': dap = pl_walk(s,-1, 1, &noise); break;
      case 'n': dap = pl_walk(s, 1, 1, &noise); break;

      case 'f':
               {
                int id_item = take_item(&s->pl); 
                if (id_item == ID_NO_ITEM){
                  dap = -1.0;
                }
                else {
                  switch (get_action(id_item)){
                    case ac_consume:
                      use_item(s, id_item);
                      dap = 1.0;
                      break;
                    case ac_throw:
                      if (id_item == IT_PISTOL) {s->status.effect[EF_PISTOL] = 8;}
                      s->mode = mode_throw;
                      s->thr.x = s->pl.coord.x;
                      s->thr.y = s->pl.coord.y;
                      s->thr.id_item = id_item;
                      break;
                  }
                }
               };
               break;
      
      case ' ': dap = 0.5; break;
      case 'q': return 1;
      default : dap = -1.0;
    }
  }
  else {
    switch(c) {
      case 'h': move_target(s, -1, 0); break;
      case 'j': move_target(s,  0, 1); break;
      case 'k': move_target(s,  0,-1); break;
      case 'l': move_target(s,  1, 0); break;
      case 'y': move_target(s, -1,-1); break;
      case 'u': move_target(s,  1,-1); break;
      case 'b': move_target(s, -1, 1); break;
      case 'n': move_target(s,  1, 1); break;
      case 'f':
        if(s->pl.coord.x != s->thr.x || s->pl.coord.y != s->thr.y){
          dap = throw_item(s, &s->thr);
          s->thr.id_item = ID_NO_ITEM;
          s->mode = mode_move;
          /* pistol effect - return to the throw mode */ 
          if (s->mode == mode_move && s->status.effect[EF_PISTOL]>0) {
            s->mode = mode_throw;
            //s->thr.x = s->pl.coord.x;
            //s->thr.y = s->pl.coord.y;
            s->thr.id_item = IT_PISTOL;
          }
        }
    }
  }

  if (dap>=0) {

    /* add smell */
    add_smell(s, s->pl.coord.x, s->pl.coord.y, s->pl.coord.z, (int) round (40.0 * dap));
    
    /* add noise */
    make_noise(s, s->pl.coord.x, s->pl.coord.y, s->pl.coord.z, noise);
    
    /* move camera */
    int dx = (s->camera.x - s->pl.coord.x);
    int dy = (s->camera.y - s->pl.coord.y);

    if ( dx*dx + dy*dy > 0) { 
      s->camera.x = s->pl.coord.x; 
      s->camera.y = s->pl.coord.y; 
    }

    int j;
    for (j=0; j<s->prio.num; ++j) {
      if (s->prio.id[j] != ID_PLAYER) {
        s->prio.ap[j] += dap;
      }
    }
 
    s->pl.marks = 0;

    s->players_turn = 0;
  }

  return 0;
}
