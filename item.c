
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
#include "common.h"
#include "state.h"
#include "item.h"
#include "sim.h"

struct item_list *item_list_add (struct item_list *ls, int elem) {
  struct item_list *nls = malloc(sizeof(*nls));
  nls->id_item = elem; 
  nls->tail = ls;
  return nls;
}

/* edit in place, return item id */
int item_list_take (struct item_list **pls) {
  if (*pls == NULL) {
    return ID_NO_ITEM;
  }
  else {
    int id = (*pls)->id_item;
    struct item_list *tmp = (*pls)->tail;
    free(*pls);
    *pls = tmp;
    return id;
  }
}

void add_item(struct player *pl, int id_item) {
  pl->items = item_list_add(pl->items, id_item);
}

int take_item(struct player *pl) {
  return item_list_take(&(pl->items));
}

enum item_action get_action(int id_item) {
  switch(id_item){
    case IT_ROCK:;
    case IT_GRENADE:;
    case IT_PISTOL:
      return ac_throw;
    default:
      return ac_consume;
  }
}

/* using items */
void use_item(struct state *s, int id_item){
  switch(id_item){
    case IT_MEDKIT: 
      if (s->pl.cond == bitten) {
        if (s->pl.hp < MAX_HP + 3){
          s->pl.hp = s->pl.hp+1;
        }
      }
      else {
        if (s->pl.hp < MAX_HP){
          s->pl.hp = s->pl.hp+1;
        }
      }
      break;
    case IT_ANTIDOTE:
      s->pl.cond = healthy;
      break;
    case IT_OLFACTOVISOR:
      s->status.effect[EF_OLFACTOVISOR] = 120;
      break;
    case IT_CANNIBAL:
      s->status.effect[EF_CANNIBAL] = 120;
      break;
    default:
      ;
  }
}

double throw_item(struct state *s, struct throwing *thr){
  double dap = 1.0;
  switch(thr->id_item){
    case IT_ROCK:
      {
        int z = zlevel(&s->grid, thr->x, thr->y);
        if (is_not_occupied(&s->grid, thr->x, thr->y, z)) {
          make_noise(s, thr->x, thr->y, z, NOISE_HIT);
        }
        else{
          hit(s, thr->x, thr->y, z, hit_normal);
        }
        /* put the stone there */
        if (urandom(3)>0){
          s->grid.loc[thr->x][thr->y][z].id_item = thr->id_item;
        }
      };
      break;
    case IT_PISTOL:
      {
        make_noise(s, s->pl.coord.x, s->pl.coord.y, s->pl.coord.z, NOISE_PISTOL);
        int z = zlevel(&s->grid, thr->x, thr->y);
        if (is_not_occupied(&s->grid, thr->x, thr->y, z)) {
          make_noise(s, thr->x, thr->y, z, NOISE_HIT);
        }
        else{
          hit(s, thr->x, thr->y, z, hit_normal);
        }
        s->status.effect[EF_PISTOL]--;
      };
      break;
    case IT_GRENADE:
      {
        int dx, dy;
        int rad = 2;
        
        int z = zlevel(&s->grid, thr->x, thr->y);
        make_noise(s, thr->x, thr->y, z, NOISE_GRENADE);
        
        for(dx = -rad; dx<=rad; ++dx){
          for(dy = -(rad-abs(dx)); dy<=rad-abs(dx); ++dy){
            if (is_within_ij(&s->grid, thr->x + dx, thr->y+dy)) {
              z = zlevel(&s->grid, thr->x+dx, thr->y+dy);

              if (!is_not_occupied(&s->grid, thr->x+dx, thr->y+dy, z)) {
                if (urandomf(1.0)<0.85) {
                  hit(s, thr->x+dx, thr->y+dy, z, hit_normal);
                }
              }
             
              /* add smoke */
              s->grid.loc[thr->x+dx][thr->y+dy][z].smoke = 1+urandom(2);
            }
          }
        }
      };
      break;
    default:
      ;
  };
  return dap;
}
