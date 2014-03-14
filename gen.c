
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
#include "grid.h"
#include "gen.h"
#include "item.h"


void gen_box (struct grid *g, int w, int h) {

  g->width = w;
  g->height = h;
  
  int i, j, k;
  for (i=0; i<SX; ++i){
    for (j=0; j<SY; ++j){
      for (k=0; k<SZ; ++k){
        g->block[i][j][k] = 1;
      }
    }
  }


  for (i=1; i<w-1; ++i){
    for (j=1; j<h-1; ++j){
      for (k=1; k<SZ; ++k){
        g->block[i][j][k] = 0;
      }
    }
  }

}

void gen_level_random(struct grid *g) {

  int i,j,k;
  int w = g->width;
  int h = g->height;

  for (i=1; i<w-1; ++i){
    for (j=1; j<h-1; ++j){
      for (k=1; k<SZ; ++k){
        g->block[i][j][k] = 0;
      }

      if (urandom(30) == 0) {
        g->block[i][j][1] = 1;
        if (urandom(3) > 0) {
          g->block[i][j][2] = 1;
          if (urandom(7) == 0) {
            g->block[i][j][3] = 1;
            if (urandom(3) > 0) {
              g->block[i][j][4] = 1;
            }
          }
        }

      }
    }
  }

}

void add_ramp_h(int z[SX][SY], int ll, int rr, int bb, int tt){
 
  int i, j;

  int lim = SZ-2;

  for(i=ll; i<=rr; ++i) {
    for(j=bb; j<=tt; ++j) {
      
      if (i==ll || i==rr) {
        z[i][j] = MIN(z[i][j] + 1, lim);
      }
      else 
      {
        z[i][j] = MIN(z[i][j] + 2, lim);
        /* intermediate stairs */
        if (z[i][j]-z[i-1][j]>1) {
          z[i][j] = z[i][j]-1;
        }
        else if (z[i][j]-z[i-1][j]<-1) {
          z[i][j] = z[i][j]+1;
        }
      }
    }
  }
}

void add_ramp_v(int z[SX][SY], int ll, int rr, int bb, int tt){
 
  int i, j;

  int lim = SZ-2;

  for(j=bb; j<=tt; ++j) {
    for(i=ll; i<=rr; ++i) {
      
      if (j==bb || j==tt) {
        z[i][j] = MIN(z[i][j] + 1, lim);
      }
      else 
      {
        z[i][j] = MIN(z[i][j] + 2, lim);
        /* intermediate stairs */
        if (z[i][j]-z[i][j-1]>1) {
          z[i][j] = z[i][j]-1;
        }
        else if (z[i][j]-z[i][j-1]<-1) {
          z[i][j] = z[i][j]+1;
        }
      }
    }
  }
}

int is_flat(int z[SX][SY], int ll, int rr, int bb, int tt){
  int i,j; 
  for(j=bb; j<=tt; ++j) {
    for(i=ll; i<=rr; ++i) {
      if(z[i][j] != z[ll][bb]) {return 0;}
    }
  }

  return 1;
}

int no_stairs(int z[SX][SY], int ll, int rr, int bb, int tt){
  int i,j; 
  for(j=bb; j<=tt; ++j) {
    for(i=ll; i<=rr; ++i) {
      if((z[i][j]) % 2 == 1) {return 0;}
    }
  }

  return 1;
}

/* can build a house/fence */
int can_build_a_box(int z[SX][SY], int ll, int rr, int bb, int tt){
  return
    (is_flat(z,ll-1,ll-1,bb-1,tt+1) && is_flat(z,ll,rr+1,bb-1,tt+1)) ||
    (is_flat(z,ll-1,rr,bb-1,tt+1) && is_flat(z,rr+1,rr+1,bb-1,tt+1)) ||
    (is_flat(z,ll-1,rr+1,bb-1,bb-1) && is_flat(z,ll-1,rr+1,bb,tt+1)) ||
    (is_flat(z,ll-1,rr+1,bb-1,tt) && is_flat(z,ll-1,rr+1,tt+1,tt+1)) ;
}
void build_a_house(int z[SX][SY], int ll, int rr, int bb, int tt){
  int high = z[ll][bb] + 2 + 2*urandom(2);
  if (high > SZ-1) { high = SZ-1; }

  int i_door = (ll+rr)/2;
  int j_door = (bb+tt)/2;

  int i,j;

  for(j=bb; j<=tt; ++j) {
    for(i=ll; i<=rr; ++i) {
      if ((i-ll)*(i-rr)*(j-bb)*(j-tt) == 0 && abs(i-i_door)>1 && abs(j-j_door)>1) {
        z[i][j] = high;
      }
    }
  }
}

void build_an_obstacle(int z[SX][SY], int ll, int rr, int bb, int tt){
  int high = z[ll][bb] + 2 + 2*urandom(2);
  if (high > SZ-1) { high = SZ-1; }

  int i,j;

  for(j=bb; j<=tt; ++j) {
    for(i=ll; i<=rr; ++i) {
        z[i][j] = high;
    }
  }
}

/* main function */
void gen_level_ramps(struct grid *g){
  static int z[SX][SY];
  int i, j, k; 

  int w = g->width;
  int h = g->height;

  int i0 = 1;
  int i1 = w-1;

  int j0 = 1;
  int j1 = h-1;
  
  for(i=0; i<w; ++i){
    for(j=0; j<h; ++j){
      z[i][j] = SZ-1;
    }
  }

  for(i=i0; i<=i1; ++i){
    for(j=j0; j<=j1; ++j){
      z[i][j] = 0;
    }
  }

  /* add ramps */
  int n;
  int tt, bb, ll, rr;
  for(n = 0; n<w*h/4; ++n) {
  
    if (urandom(4) > 0) {
      /* ramps begin */

      int wmin = 1;
      int wmax = 4;
      int lmin = 3;
      int lmax = 20;

      int vertical = urandom(2);

      if (vertical) {
        /* vertical */
        bb = i0 + urandom(i1-i0+1 - lmax);
        ll = j0 + urandom(j1-j0+1 - wmax);
        
        tt = bb + lmin + urandom(lmax-lmin);
        rr = ll + wmin + urandom(wmax-wmin);
      }
      else {
        /* horizontal */
        bb = i0 + urandom(i1-i0+1 - wmax);
        ll = j0 + urandom(j1-j0+1 - lmax);
        
        tt = bb + wmin + urandom(wmax-wmin);
        rr = ll + lmin + urandom(lmax-lmin);
      }

      if ( no_stairs(z, ll-1, rr+1, bb-1, tt+1) ) {

        if(vertical) {

          if ( is_flat(z, ll-1, rr+1, bb-1, bb+1) && 
               is_flat(z, ll-1, rr+1, tt-1, tt+1) ) {
            add_ramp_v (z, ll, rr, bb, tt);
          }

        }
        else {
          if ( is_flat(z, ll-1, ll+1, bb-1, tt+1) && 
               is_flat(z, rr-1, rr+1, bb-1, tt+1) ) {
            add_ramp_h (z, ll, rr, bb, tt);
          }
        }
      }

      /* ramps end */
    }
    else {
      if (urandom(2)>0) {
        /* houses */
        int wmin = 4;
        int wmax = 15;
        int lmin = 4;
        int lmax = 15;

        bb = i0 + urandom(i1-i0+1 - lmax);
        ll = j0 + urandom(j1-j0+1 - wmax);
        
        tt = bb + lmin + urandom(lmax-lmin);
        rr = ll + wmin + urandom(wmax-wmin);

        if (can_build_a_box(z,ll,rr,bb,tt)) {
          build_a_house(z,ll,rr,bb,tt);
        }
      }
      else {
        /* obstables  */
        int wmin = 0;
        int wmax = 2;
        int lmin = 0;
        int lmax = 2;

        bb = i0 + urandom(i1-i0+1 - lmax);
        ll = j0 + urandom(j1-j0+1 - wmax);
        
        tt = bb + lmin + urandom(lmax-lmin);
        rr = ll + wmin + urandom(wmax-wmin);

        if (can_build_a_box(z,ll,rr,bb,tt)) {
          build_an_obstacle(z,ll,rr,bb,tt);
        }
      }
    }
  }

  /* fill the grid - final transformation */
  for(i=i0; i<=i1; ++i){
    for(j=j0; j<=j1; ++j){
      int lim = z[i][j];
      if (lim == SZ-3) {lim = SZ-4;}
      for(k=0; k<=lim; ++k) {
        g->block[i][j][k] = 1;
      }

    }
  }

}

/* Generating levels */
void cleanup_locs (struct grid *g) {
  int i, j, k;
  for (i=0; i<SX; ++i){
    for (j=0; j<SY; ++j){
      for (k=0; k<SZ; ++k){
        
        g->loc[i][j][k].tl = tl_wall;

        if (!g->block[i][j][k]) {
          if (g->block[i][j][k-1]) {
            g->loc[i][j][k].tl = tl_floor;
          }
          else {
            g->loc[i][j][k].tl = tl_midair;
          }
        }

        g->loc[i][j][k].id_mob = ID_NONE;
        g->loc[i][j][k].id_item = ID_NO_ITEM;
        
        g->loc[i][j][k].stains = 0;
        g->loc[i][j][k].smell = 0;
        g->loc[i][j][k].smoke = 0;

      }
    }
  }
}


void distribute_items (struct grid *g, double prob, int level) {
  int i, j, k;

  double adj_prob = prob * (1.0 + 0.1*(double)level);
  if (adj_prob > prob * 4.0) {adj_prob = prob * 4.0;}

  for (i=0; i<SX; ++i){
    for (j=0; j<SY; ++j){
    
      k = zlevel(g, i, j);
      
      if (g->loc[i][j][k].tl == tl_floor) {
        if (urandomf(1.0) < adj_prob){ 
          int v = urandom(ITEMS_NUM);
          if (v < level+1){
            g->loc[i][j][k].id_item = v;
          }
        }
      }
    }
  }
}




