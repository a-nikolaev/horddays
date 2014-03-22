
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
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "state.h"
#include "item.h"
#include "sense.h"
#include "sim.h"
#include "init.h"

int can_walk_to (struct grid *g, int i, int j, int k){
  return (tl_walkable (g->loc[i][j][k].tl)) && is_not_occupied(g,i,j,k);
}

int walk_attempt (struct grid *g, int ni, int nj, int k){
  if (is_within_ij(g, ni, nj) && k >=0 && k < SZ){
    if (can_walk_to (g,ni,nj,k)) {
      return 0;
    }
    else {
      if (k+1 < SZ && can_walk_to (g,ni,nj,k+1)) {
        return 1;
      }
      else {
        if (k > 0 && can_walk_to (g,ni,nj,k-1)) {
          return -1;
        }
        else {
          return FAILED_WALK;
        }
      }
    }
  }
  else {
    return FAILED_WALK;
  }
}

int can_attack (struct state *s, int id, int i, int j, int k){
  if ( (tl_walkable (s->grid.loc[i][j][k].tl)) && !is_not_occupied(&s->grid,i,j,k) ) {

    int idtt = s->grid.loc[i][j][k].id_mob;
    
    if (id == ID_PLAYER) {
      if (idtt != ID_PLAYER && s->mobs.mb[idtt].cl == aggressive) return 1;
    }
    else {
      struct mob * m = &s->mobs.mb[id];
      if (idtt == ID_PLAYER) {
        if (m->cl == aggressive) {return 1;}
        return 0;
      }
      else {
        if (m->cl != s->mobs.mb[idtt].cl) {return 1;}
        return 0;
      }
    }
  }
  return 0;
}

int attack_attempt (struct state *s, int i, int j, int k, int ni, int nj){
  struct grid *g = &s->grid;
  if (g->loc[i][j][k].id_mob == ID_NONE) {return FAILED_ATTACK;}

  int id = g->loc[i][j][k].id_mob;

  if (is_within_ij(g, ni, nj) && k >=0 && k < SZ){
    if (can_attack (s,id,ni,nj,k)) {
      return 0;
    }
    else {
      if (k+1 < SZ && can_attack (s,id,ni,nj,k+1)) {
        return 1;
      }
      else {
        if (k > 0 && can_attack (s,id,ni,nj,k-1)) {
          return -1;
        }
        else {
          return FAILED_ATTACK;
        }
      }
    }
  }
  else {
    return FAILED_ATTACK;
  }
}

void discard_mob(struct state *s, int id) {
  struct mob * m = &s->mobs.mb[id];
  m->alive = 0;
  s->grid.loc[m->coord.x][m->coord.y][m->coord.z].id_mob = ID_NONE;
  
  /* remove from prio */
  int i=0;
  while(s->prio.id[i] != id && i < s->prio.num) { ++i; }
 
  if (i<s->prio.num) {

    int j;
    for (j=i+1; j < s->prio.num; ++j){
      s->prio.id[j-1] = s->prio.id[j]; 
      s->prio.ap[j-1] = s->prio.ap[j]; 
    }

    s->prio.num--;

  }
  
}

void add_blood_stains(struct state *s, int i, int j, int k, int amount) {
  s->grid.loc[i][j][k].stains = MIN(BLOOD_STAINS_MAX, s->grid.loc[i][j][k].stains + amount);
}

void check_and_discard (struct state *s, int id) {
}

void hit(struct state *s, int i, int j, int k, enum hit_type ht){
  int id = s->grid.loc[i][j][k].id_mob;

  if (id == ID_PLAYER) {
    s->pl.hp -= 1;
    switch (ht) {
      case hit_bite:
        if(urandom(3)==0) {
          s->pl.cond = bitten;
        }
        s->pl.marks++;
        break;
      case hit_normal:
        ;
        break;
    }
  } 
  else {
    struct mob *m = &s->mobs.mb[id];
    m->hp -= 1;
    switch (ht) {
      case hit_bite: 
        m->cond = bitten;
        break;
      case hit_normal:
        ;
        break;
    }

    if (m->hp <= 0) {
      s->status.score += 1;
      discard_mob(s, id);
      add_blood_stains(s, i, j, k, 2);
    }
  } 
    
  make_noise(s, i, j, k, NOISE_HIT);

}


void mob_walk (struct state *s, int id, int nx, int ny, int nz) {
  int x = s->mobs.mb[id].coord.x;
  int y = s->mobs.mb[id].coord.y;
  int z = s->mobs.mb[id].coord.z;
  
  /* walk */
  if(s->grid.loc[nx][ny][nz].id_mob == ID_NONE){
    s->grid.loc[x][y][z].id_mob = ID_NONE;
    
    s->mobs.mb[id].coord.x = nx;
    s->mobs.mb[id].coord.y = ny;
    s->mobs.mb[id].coord.z = nz;
    
    s->grid.loc[nx][ny][nz].id_mob = id;
  }
  else if(s->grid.loc[nx][ny][nz].id_mob == ID_PLAYER){
    hit(s, nx, ny, nz, hit_bite);
  }
}


/* Mobs hording - flocking */
void flocking (struct state *s, int id) {
  struct mob *mob = &s->mobs.mb[id];

  int di, dj, dk;
  int i, j, k;

  const int rad = 3;

  struct vec sum = {0.0, 0.0};
  struct vec sum_radvec = {0.0, 0.0};
  struct vec tv = {0.0, 0.0};
  int num = 0;
  double len = 0.0;

  for (di = - rad; di <= rad; ++di) {
    for (dj = - rad; dj <= rad; ++dj) {
      for (dk = - rad; dk <= rad; ++dk) {

        i = mob->coord.x + di;
        j = mob->coord.y + dj;
        k = mob->coord.z + dk;
          
        if (is_within_ijk(&s->grid, i, j, k)){
          int id_other = s->grid.loc[i][j][k].id_mob;
          if (id_other != ID_PLAYER && id_other != ID_NONE && id_other != id) {
            /* there is a mob there */
            
            struct mob *other = &s->mobs.mb[id_other];
            int similar = 
              mob->senses.smell * other->senses.smell +
              mob->senses.sound * other->senses.sound +
              mob->senses.sight * other->senses.sight;

            double factor = 0.5 + 1.0 * (double) similar;
            double factor_rv = 1.0*factor;

            len = vec_norm(&s->mobs.mb[id_other].vec);
            if (len > 1.0) {
              sum.x += factor * s->mobs.mb[id_other].vec.x / len;
              sum.y += factor * s->mobs.mb[id_other].vec.y / len;

              num++;

              double rv_len = sqrt((double)(di*di + dj*dj + dk*dk)); 
              sum_radvec.x += factor_rv * ((double)di / (rv_len*rv_len));
              sum_radvec.y += factor_rv * ((double)dj / (rv_len*rv_len));
            }
          }
        }
      }
    }
  }

  if (num>0) {
    /* 
       How much is added?

       Less that average vec of the neighbors
     */

    sum.x = sum.x + sum_radvec.x;
    sum.y = sum.y + sum_radvec.y;

    double magnitude = 0.15;
    tv.x = mob->vec.x + sum.x * magnitude / (1.0 + (double) num);
    tv.y = mob->vec.y + sum.y * magnitude / (1.0 + (double) num);

    mob->vec.x = tv.x;
    mob->vec.y = tv.y;
  }
  
  /* also, randomize a bit */
  struct vec r = random_unit_vec();
  mob->vec.x = mob->vec.x + 0.05 * r.x;
  mob->vec.y = mob->vec.y + 0.05 * r.y;
}

void limit_mob_vec(struct mob *mob) {
  /* limit vec */
  double limit = 10.0;
  double len = vec_norm(&mob->vec);
  if (len > limit) {
    mob->vec.x = mob->vec.x * limit / len;
    mob->vec.y = mob->vec.y * limit / len;
  }
}





/*   RUN one MOB   */

#define DIR_NUM 8
const int arrdx[DIR_NUM] = {1, 1, 0, -1, -1, -1,  0,  1};
const int arrdy[DIR_NUM] = {0, 1, 1,  1,  0, -1, -1, -1};
  
/* returns ap spent */
double run_one_mob_id (struct state *s, int id) {
  struct mob *mob = &s->mobs.mb[id];
  if (!mob->alive) { return 0.0; }

  /* array of feasible directions */
  int darr_dir[DIR_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int darr_dz[DIR_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
  double darr_ap[DIR_NUM] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double dsample_rate[DIR_NUM];
  
  int x = mob->coord.x;
  int y = mob->coord.y;
  int z = mob->coord.z;

  /* sense smell */
  sense_smell(s, id);
  /* sense sight */
  sense_sight(s, id);
  /* adjust vec */
  flocking(s, id);
  
  limit_mob_vec(mob);

  /* get dir from vec */
  mob->dir = dir_of_vec(&mob->vec);

  /* choose the direction to walk */
  int i, dz;
  int good_dirs = 0;
  for(i=0; i<DIR_NUM; ++i) {

    double rate = 0.01 + (double) arrdx[i] * mob->vec.x + (double) arrdy[i] * mob->vec.y;
    
    if (rate <= 0) { continue; }
    
    dz = walk_attempt(&s->grid, x+arrdx[i], y+arrdy[i], z);

    if (dz != FAILED_WALK) {
      darr_dir[good_dirs] = i;
      darr_dz[good_dirs] = dz;
      dsample_rate[good_dirs] = rate*rate;

      darr_ap[good_dirs] = DEF_ZOMBIE_DELAY * 
        sqrt((double) (
              arrdx[i]*arrdx[i] + 
              arrdy[i]*arrdy[i] + 
              dz*dz));

      good_dirs++;
    }
    else {
      dz = attack_attempt(s, x, y, z, x+arrdx[i], y+arrdy[i]);
      if (dz != FAILED_ATTACK) {
        darr_dir[good_dirs] = i;
        darr_dz[good_dirs] = dz;
        dsample_rate[good_dirs] = rate*rate;

        darr_ap[good_dirs] = DEF_ZOMBIE_DELAY;
        good_dirs++;
      }
    }
  }

  /* delay */
  double dt = DEF_ZOMBIE_DELAY;

  /* choose direction */
  if (good_dirs > 0) {
    i = sample_arr (good_dirs, darr_dir, dsample_rate);
    if (i>0) {
      int dir = darr_dir[i];
      mob_walk (s, id, x+arrdx[dir], y+arrdy[dir], z+darr_dz[i]);
      
      mob->vec.x = mob->vec.x + 0.2 * arrdx[dir];
      mob->vec.y = mob->vec.y + 0.2 * arrdy[dir];

      dt = darr_ap[i];
    }
    else {
      mob->vec.x = 0.75 * mob->vec.x;
      mob->vec.y = 0.75 * mob->vec.y;
    }
    dt = DEF_ZOMBIE_DELAY;
  }
  else{
    mob->vec.x = 0.75 * mob->vec.x;
    mob->vec.y = 0.75 * mob->vec.y;
  
    dt = DEF_ZOMBIE_DELAY /* * 0.5 * (1.0 + urandomf(1.0)) */;
  }

 
  /* Limit mob's vec */
  limit_mob_vec(mob);

  return dt;

}






void run_all_mobs_simplified (struct state *s){
  int id;
  for (id = 0; id < s->mobs.total_num; ++id) {
    run_one_mob_id(s, id);
  }
}

void print_prio(struct prio *p) {
  int i;
  for(i=0; i < p->num; ++i) {
    printf("[%i -> %g] ", p->id[i], p->ap[i]);
  }
  printf("\n");
}

void run_all_mobs (struct state *s){
  //printf("Unsorted:\n");
  //print_prio(&s->prio);

  sort_prio(&s->prio);
  
  //printf("Sorted:\n");
  //print_prio(&s->prio);

  int id;
  int i = 0;
  while ( s->prio.id[i] != ID_PLAYER && i < s->prio.num ) {
    
    id = s->prio.id[i];
    while (s->prio.ap[i] > 0.0) {
      double dap = run_one_mob_id(s, id);
      if (dap < DEF_ZOMBIE_DELAY) {dap = DEF_ZOMBIE_DELAY;}
      s->prio.ap[i] -= dap;
    }

    i++;
  }
}



void add_smell(struct state *s, int i, int j, int k, int amount) {
  s->grid.loc[i][j][k].smell += amount;
}

void make_noise(struct state *s, int i, int j, int k, double amount) {
  const double min_perc_val = 0.6;
  double r2 = (fabs(amount) / min_perc_val);

  int r = (int) ceil(sqrt(r2));

  int x, y, z;

  int x0 = MAX(i-r,0);
  int xf = MIN(i+r,s->grid.width-1);
  int y0 = MAX(j-r,0);
  int yf = MIN(j+r,s->grid.height-1);
  int z0 = MAX(k-r,0);
  int zf = MIN(k+r,SZ-1);

  for(x=x0; x<=xf; ++x){
    for(y=y0; y<=yf; ++y){
      for(z=z0; z<=zf; ++z){
        if ( (x-i)*(x-i) + (y-j)*(y-j) + (z-k)*(z-k) < r2 ) {

          int id = s->grid.loc[x][y][z].id_mob;
          if (id != ID_PLAYER && id != ID_NONE) {
            sense_sound (s, id, i, j, k, amount);
          }

        }
      }
    }
  }


}

void loc_changes(struct state *s, double wind_p, double blood_decay_p, double smell_decay_p, double smoke_decay_p) {
  int i, j, k;
  int i2, j2, k2;

  for (j=0; j<s->grid.height; ++j) {
    for (i=0; i<s->grid.width; ++i) {
      for (k = 0; k<SZ; ++k) {
        /* smell */
        if (s->grid.loc[i][j][k].smell > 0) {
         
          /* repeat this many times */
          int rep = (s->grid.loc[i][j][k].smell+2)/3;
          while (rep > 0) {
            rep--;
            s->grid.loc[i][j][k].smell--;
            
            if (urandomf(1.0) < smell_decay_p) {;}
            else {
              i2 = i + urandom(3) - 1;
              j2 = j + urandom(3) - 1;
              k2 = k + urandom(3) - 1;
              if (urandom(1.0) < wind_p) {
                i2 += arrdx[s->wind_dir];
                j2 += arrdy[s->wind_dir];
              }

              if (is_within_ij(&s->grid, i2, j2) && k2>=0 && k2<SZ && !s->grid.block[i2][j2][k2]) {
                s->grid.loc[i2][j2][k2].smell++;
              }
            } 
          }
        }
        
        /* blood */
        if (s->grid.loc[i][j][k].stains > 0) {
          if (urandomf(1.0) < blood_decay_p) {
            s->grid.loc[i][j][k].stains--;
          }
        }

        /* smoke */
        if (s->grid.loc[i][j][k].smoke > 0) {

          /* repeat this many times */
          int rep = (s->grid.loc[i][j][k].smoke+2)/3;
          while (rep > 0) {
            rep--;
            s->grid.loc[i][j][k].smoke--;
            
            if (urandomf(1.0) < smoke_decay_p) {;}
            else {
              i2 = i + urandom(3) - 1;
              j2 = j + urandom(3) - 1;
              k2 = k + urandom(3) - 1;
              if (urandom(1.0) < wind_p) {
                i2 += arrdx[s->wind_dir];
                j2 += arrdy[s->wind_dir];
              }

              if (is_within_ij(&s->grid, i2, j2) && k2>=0 && k2<SZ && !s->grid.block[i2][j2][k2]) {
                s->grid.loc[i2][j2][k2].smoke++;
              }
            } 
          }


        }

      }
    }
  }

}

void run_mobs (struct state *s) {
  if (s->players_turn) return;

  struct loc *loc = &(s->grid.loc[s->pl.coord.x][s->pl.coord.y][s->pl.coord.z]);
  /* pick up the item if there is any */
  if (loc->id_item != ID_NO_ITEM) {
    add_item(&s->pl, loc->id_item);
    loc->id_item = ID_NO_ITEM;
  }

  /* found the exit */
  if (loc->tl == tl_exit) {
    s->status.level++;
    s->status.score+=100;
    level_init(s);
    update_vision(s);
  }
  else
  {
    /* blood decay, smell propagation */
    loc_changes(s, 0.05, 0.1, 0.03, 0.1);

    /* run effects */
    if (s->pl.cond == bitten && s->pl.hp > 0) {
      if (urandomf(1.0) < 0.02) { s->pl.hp--; }
    }
    
    if (s->status.effect[EF_OLFACTOVISOR] > 0) {
      s->status.effect[EF_OLFACTOVISOR]--;
    }
    if (s->status.effect[EF_CANNIBAL] > 0) {
      s->status.effect[EF_CANNIBAL]--;
      make_noise(s, s->pl.coord.x, s->pl.coord.y, s->pl.coord.z, NOISE_CANNIBAL);
    }

    run_all_mobs(s);
    update_vision(s);
  }
  /* finished */
  s->players_turn = 1;
}

