
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
#include "sim.h"
#include "gen.h"

void construct_level (struct state *s, int w, int h) {

  gen_box(&s->grid,w,h);
  
  //gen_level_random(&s->grid);
  gen_level_ramps(&s->grid);

  cleanup_locs (&s->grid);
  
  distribute_items (&s->grid, 0.005, s->status.level);

  zero_cons (&s->cons);

}

void player_init (struct state *s) {
  s->pl.hp = MAX_HP;
  s->pl.cond = healthy;
  s->pl.items = NULL;
  s->pl.ready = 0;
  s->pl.coord.x = 1;
  s->pl.coord.y = 1;

  s->pl.marks = 0;
}

int spawn_one_mob (struct state *s, int i, int j, int k){
    
  if (s->grid.loc[i][j][k].id_mob != ID_NONE) { return 0; }

  /* location is fine */
  int prio_index = s->prio.num;
  int id = s->mobs.total_num;

  struct mob *m = &s->mobs.mb[id];

  /* setup the mob */
  m->coord.x = i;
  m->coord.y = j;
  m->coord.z = k;

  m->alive = 1;

  m->cl = aggressive;

  m->cond = zombie;

  m->senses.sound = 0;
  m->senses.smell = 0;
  m->senses.sight = 0;

  int outof = 3;
  switch (s->status.level) {
    case 1: outof = 1; break;
    case 2: outof = 2; break; 
    default: outof = 3;

  }
  switch (urandom(outof)) {
    case 0: m->senses.sound = 1; break;  /* oddy */
    case 1: m->senses.smell = 1; break;  /* sniffy */
    default: m->senses.sight = 1;        /* spotty */
  }

  m->hp = 1;

  m->portrait = urandom(3);

  m->vec = random_unit_vec();

  m->dir = dir_of_vec(&m->vec);

  s->grid.loc[i][j][k].id_mob = id;

  /* add the mob to prio */
  s->prio.ap[prio_index] = -0.01 + -2.0 * ((double) urandom(100)) / 100.0;
  s->prio.id[prio_index] = id;

  /* update total mob number */
  s->mobs.total_num = id+1;
  s->prio.num = prio_index+1;

  return 1;
}

int is_good_to_start (struct state *s, int x, int y, int z){
 
  if(z != 1) {return 0;} 

  int di, dj;

  int rad = 1;

  for(di=-rad; di<=rad; ++di){
    for(dj=-rad; dj<=rad; ++dj){
      if ( !tl_walkable (s->grid.loc[x+di][y+dj][z].tl) ) {
        return 0;
      }
    }
  }

  return 1;
}

void find_where_to_start (struct state *s, int *x, int *y, int *z){
  int found = 0;
  int w = s->grid.width;
  int h = s->grid.height;
  
  int i, j, k;

  int margin = MIN(w/7,h/7) + 1;

  while (!found){
    /* pick coordinates */
    i = margin + urandom(w - 2*margin);
    j = margin + urandom(h - 2*margin);
    
    k = zlevel(&s->grid, i, j);
  
    if (is_good_to_start(s,i,j,k)) {
      *x = i;
      *y = j;
      *z = k;
      return;
    }
  }
}

void spawn_mobs_connected(struct state *s, int num){
  int x0, y0, z0;
  find_where_to_start(s, &x0, &y0, &z0);
  
  s->cons.conx[0] = x0; 
  s->cons.cony[0] = y0;
  s->cons.conz[0] = z0;
  s->cons.con_num = 1;

  int x, y, z;
  int i;
      
  struct mob *mob;

  int attempts = 0;
  int postgeneration_runs = 0;
  int mobs_ready = 0;
  int success = 0;
  
  while ( attempts < 100 + mobs_ready*50 && (mobs_ready < num || postgeneration_runs < 200) ) {
    attempts++;

    if (mobs_ready < num) {
      i = urandom(s->cons.con_num);
      x = s->cons.conx[i];
      y = s->cons.cony[i];
      z = s->cons.conz[i];
    
      success = spawn_one_mob(s, x, y, z);
      if (success) {
        mobs_ready++;
      }
    }

    /* simulate */
    if (attempts % 20 == 0){
      run_all_mobs_simplified(s);
      if (mobs_ready >= num) {postgeneration_runs++;}
    
      /* increment "empty" that are connected */
      for(i=0; i<s->cons.con_num; ++i){
        x = s->cons.conx[i];
        y = s->cons.cony[i];
        z = s->cons.conz[i];
        
        s->cons.empty[x][y][z]++;
      }

      for(i=0; i<s->mobs.total_num; ++i) {
        mob = &s->mobs.mb[i];
        x = mob->coord.x;
        y = mob->coord.y;
        z = mob->coord.z;
        /* visits conx cony */
        if (s->cons.empty[x][y][z] == CONS_DISCON) {
          /* if it was disconnected */
          s->cons.empty[x][y][z] = 0;
          
          s->cons.conx[s->cons.con_num] = x;
          s->cons.cony[s->cons.con_num] = y;
          s->cons.conz[s->cons.con_num] = z;
          
          s->cons.con_num++;
        }
        else{
          /* if it was connected */
          s->cons.empty[x][y][z] = 0;
        }
      }
    }

  }
  //printf("Map generation took: attempts = %i, postgen runs = %i\n", attempts, postgeneration_runs);
}


void find_minmax_empty (struct state *s, int rad, int *x, int *y, int *z) {
  
  int i;
  int ib = 0;

  int xx, yy, zz;
  int xxb, yyb, zzb;
  xxb = s->cons.conx[ib];
  yyb = s->cons.cony[ib];
  zzb = s->cons.conz[ib];
  int scoreb = 100000;

  int dx, dy, dz;


  for(i=0; i<s->cons.con_num; ++i) {
    xx = s->cons.conx[i];
    yy = s->cons.cony[i];
    zz = s->cons.conz[i];
    if (s->cons.empty[xx][yy][zz] != CONS_DISCON &&
        s->grid.loc[xx][yy][zz].id_mob == ID_NONE
        ) {
   
      /* smallest in the neighborhood */
      int tmpb = s->cons.empty[xx][yy][zz];

      for(dx = -rad; dx <= rad; ++dx) {
        for(dy = -rad; dy <= rad; ++dy) {
          for(dz = -rad; dz <= rad; ++dz) {

            if (is_within_ijk(&s->grid, xx+dx, yy+dy, zz+dz)){
              if ( tmpb < s->cons.empty[xx+dx][yy+dy][zz+dz] ) {
                tmpb = s->cons.empty[xx+dx][yy+dy][zz+dz];
              }
            }

          }
        }
      }

      if ( tmpb > scoreb )
      {
        ib = i;
        xxb = xx;
        yyb = yy;
        zzb = zz;
      }

    }
  }
  *x = xxb;
  *y = yyb;
  *z = zzb;
}

void spawn_mobs (struct state *s, int num) {
  int id;
  s->mobs.total_num = num;

  int w = s->grid.width;
  int h = s->grid.height;

  for (id=0; id<num; ++id) {
    int i = w/4 + urandom(w/2);
    int j = h/4 + urandom(h/2);
    int k = zlevel(&s->grid, i, j);

    if (s->grid.loc[i][j][k].id_mob != ID_NONE) {continue;}

    struct mob *m = &s->mobs.mb[id];

    m->coord.x = i;
    m->coord.y = j;
    m->coord.z = k;

    m->alive = 1;

    m->cl = aggressive;

    m->cond = zombie;

    m->senses.sound = 0;
    m->senses.smell = 0;
    m->senses.sight = 0;
    switch (urandom(3)) {
      case 0: m->senses.sound = 1; break;
      case 1: m->senses.smell = 1; break;
      default: m->senses.sight = 1;
    }

    m->hp = 1;

    m->portrait = urandom(3);

    m->vec = random_unit_vec();

    m->dir = dir_of_vec(&m->vec);

    s->grid.loc[i][j][k].id_mob = id;

    /* add the mob to prio */
    int index = s->prio.num;
    s->prio.ap[index] = -0.01 + -2.0 * ((double) urandom(100)) / 100.0;
    s->prio.id[index] = id;
    s->prio.num++;
  }
}



void level_init (struct state *s) {
  int w = 20 + s->status.level * 5;
  int h = 20 + s->status.level * 5;
  
  if (w > 70) {w = 70;}
  if (h > 70) {h = 70;}

  zero_cons(&s->cons);
  construct_level (s, w, h);

  int i, j, k;
  /* Prepare Prio */
  for(i=0; i<MOB_NUM; ++i) { 
    s->prio.ap[i] = 0; s->prio.id[i] = ID_NONE; 

    s->mobs.mb[i].alive = 0;
  }
  s->prio.num = 0;
  s->mobs.total_num = 0;

  s->pl.ready = 0; /* it cannot be detected/seen */

  /* add mobs */
  int mobsnum = w*h*(3 + s->status.level) / (150);
  if (mobsnum > 400) {mobsnum = 400;}
  spawn_mobs_connected (s, mobsnum);
  
  /* Place exit */
  int exit_loc = urandom(s->cons.con_num);
  i = s->cons.conx[exit_loc];
  j = s->cons.cony[exit_loc];
  k = s->cons.conz[exit_loc];
  s->grid.loc[i][j][k].tl = tl_exit;

  /* Wind */
  s->wind_dir = urandom(8);

  /* Position the player */
  find_minmax_empty(s, 5, &i, &j, &k);

  s->pl.ready = 1;
  s->pl.coord.x = i;
  s->pl.coord.y = j;
  s->pl.coord.z = k;
  
  s->grid.loc[i][j][k].id_mob = ID_PLAYER;

  s->prio.id[s->prio.num] = ID_PLAYER;
  s->prio.ap[s->prio.num] = 0.0;
  s->prio.num++;

  /* set camera */
  s->camera.x = i;
  s->camera.y = j;
  s->camera.z = SZ-1;

  /* update vision */
  blank_vision(&s->grid, &s->vision, SX/2, SY/2, (SX+SY));
  blank_vision_seen(&s->grid, &s->vision, SX/2, SY/2, (SX+SY));

  update_vision(s);
  s->players_turn = 1;
}

void state_init (struct state *s) {
  player_init (s);
  s->mode = mode_move;
  s->thr.id_item = ID_NO_ITEM;
  s->thr.x = 1;
  s->thr.y = 1;

  s->status.score = 0;
  s->status.level = 1;
  int i;
  for(i=0; i<EFFECTS_NUM; ++i){
    s->status.effect[i] = 0;
  }

  level_init (s);
}
