
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
#include "common.h"
#include "grid.h"

int tl_walkable (enum tl tl) {
  switch (tl) {
    case tl_floor: return 1; break;
    case tl_exit: return 1; break;
    default: return 0;  
  }
}

int tl_lookthru (enum tl tl) {
  switch (tl) {
    case tl_wall: return 0; break;
    default: return 1;  
  }
}

int loc_lookthru (struct grid *g, int i, int j, int k) {
  struct loc * loc = &(g->loc[i][j][k]);
  return tl_lookthru( loc->tl ) && loc->smoke <= 0;
}

int zlevel(struct grid *g, int i, int j) {
  int k = 0; 
  while (k+1 < SZ && g->block[i][j][k]) {++k;}
  return k;
}

int is_within_ij (struct grid *g, int i, int j){
  return (i >= 0 && j >= 0 && i < g->width && j < g->height);
}

int is_within_ijk (struct grid *g, int i, int j, int k){
  return ( is_within_ij(g, i, j) && k>=0 && k<SZ );
}

int is_not_occupied (struct grid *g, int i, int j, int k){
  return (g->loc[i][j][k].id_mob == ID_NONE); 
}


/* Computing vision */
void blank_vision (struct grid *g, struct vision *v, int x, int y, int rad) {
  int i, j, k;
  int i0 = MAX(0,x-rad);
  int i1 = MIN(g->width,x+rad+1);
  int j0 = MAX(0,y-rad);
  int j1 = MIN(g->height,y+rad+1);

  for(i=i0; i<i1; ++i) {
    for(j=j0; j<j1; ++j) {
      for(k=0; k<SZ; ++k) {
        v->v[i][j][k] = 0;
      }
    }
  }
}

void blank_vision_seen (struct grid *g, struct vision *v, int x, int y, int rad) {
  int i, j, k;
  int i0 = MAX(0,x-rad);
  int i1 = MIN(g->width,x+rad+1);
  int j0 = MAX(0,y-rad);
  int j1 = MIN(g->height,y+rad+1);

  for(i=i0; i<i1; ++i) {
    for(j=j0; j<j1; ++j) {
      for(k=0; k<SZ; ++k) {
        v->seen[i][j][k] = 0;
      }
    }
  }
}

int trans_0_x (int x, int y) {return x;}
int trans_0_y (int x, int y) {return y;}

int trans_1_x (int x, int y) {return y;}
int trans_1_y (int x, int y) {return x;}

int trans_2_x (int x, int y) {return -y;}
int trans_2_y (int x, int y) {return x;}

int trans_3_x (int x, int y) {return -x;}
int trans_3_y (int x, int y) {return y;}

int trans_4_x (int x, int y) {return -x;}
int trans_4_y (int x, int y) {return -y;}

int trans_5_x (int x, int y) {return -y;}
int trans_5_y (int x, int y) {return -x;}

int trans_6_x (int x, int y) {return y;}
int trans_6_y (int x, int y) {return -x;}

int trans_7_x (int x, int y) {return x;}
int trans_7_y (int x, int y) {return -y;}

void vmark(struct vision *v, int i, int j, int k, int value) {
  v->v[i][j][k] = value;
  v->seen[i][j][k] = MAX(v->seen[i][j][k],value);
}

void vmark_safe(struct grid *g, struct vision *v, int i, int j, int k, int value) {
  if (is_within_ij(g,i,j) && k >=0 && k < SZ) {
    vmark(v, i, j, k, value);
  }
}

void vmark_xynb(struct grid *g, struct vision *v, int i, int j, int k, int kminus, int kplus, int value) {
  vmark_safe(g,v,i,j,k,value);

  int dk;
  for (dk = -kminus; dk <= kplus; ++dk) {
    vmark_safe(g,v,i,j,k+dk,value);
  }
}

int vcheck(struct grid *g, struct vision *v, int i, int j, int k) {
  return ( is_within_ij(g,i,j) && loc_lookthru(g,i,j,k) );
}

void vscan(struct grid *g, struct vision *v,
    
    int(*trans_x)(int, int),
    int(*trans_y)(int, int),

    int x0, int y0, int z0,

    int zminus, int zplus,

    int range,

    int xfl, int yfl, int xfh, int yfh, 
    int prev_x, int prev_yl, int prev_yh) {
  
    int x, y, dyl, dyh, yl, yh;
    int b_lower = 0;
    int lower = y0;
    int range_u = range-1; 

    int xx, yy;
    
    if(range_u >= 0){
      x = prev_x + 1;
      dyl = 0; 
        if ( 2 * ((yfl - y0)*(x-x0) + (y0-prev_yl)*(xfl-x0)) / (xfl-x0) >= 1 ) {dyl = 1;} 
      yl = prev_yl + dyl;
      dyh = 0;
        if ( 2 * ((yfh - y0)*(x-x0) + (y0-prev_yh)*(xfh-x0)) / (xfh-x0) >= 1 ) {dyh = 1;}
      yh = prev_yh + dyh;


      xx = x0 + trans_x(x-x0,yl-y0);
      yy = y0 + trans_y(x-x0,yl-y0);
      if (vcheck(g,v,xx,yy,z0)) {b_lower = 1; lower = yl;}
      for(y = yl; y <= yh; ++y) {
        
        xx = x0 + trans_x(x-x0,y-y0);
        yy = y0 + trans_y(x-x0,y-y0);
        if (vcheck(g,v,xx,yy,z0)) {
          vmark_xynb(g,v, xx, yy,z0, zminus, zplus, 1);

          if (b_lower) {
            if (y == yh) {
              vscan (g,v, trans_x, trans_y, x0,y0,z0, zminus, zplus, range_u, xfl,yfl, xfh,yfh, x, lower, y); 
            }
            else {
              ;
            }
          }
          else {
            xfl = x;
            yfl = y;
            b_lower = 1;
            lower = y;
          }
        }
        else {
          /* mark the wall */
          xx = x0 + trans_x(x-x0,y-y0);
          yy = y0 + trans_y(x-x0,y-y0);
          vmark_safe(g,v, xx,yy,z0, 1);
          if (b_lower) {
            vscan (g,v, trans_x, trans_y, x0,y0,z0, zminus, zplus, range_u, xfl,yfl, x,y-1, x, lower, y-1); 
            xfl = x;
            yfl = y+1;
            b_lower = 0;
          }
          else {
            xfl = x;
            yfl = y+1;
            b_lower = 0;
          }
        }

      } /* end of for(y ...) */

    } /* end of if(range_u ...) */
}

void comp_vision (struct grid *g, struct vision *v, int x, int y, int z, int zminus, int zplus, int rad) {
  /* mark current point */
  if (vcheck(g,v, x,y,z)) {
    vmark_xynb(g,v, x,y,z, zminus, zplus, 1);
  }

  int xl = x+1;
  int yl = y;
  
  int xh = x+1;
  int yh = y+1;

  vscan(g,v, &trans_0_x, &trans_0_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_1_x, &trans_1_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_2_x, &trans_2_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_3_x, &trans_3_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_4_x, &trans_4_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_5_x, &trans_5_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_6_x, &trans_6_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
  vscan(g,v, &trans_7_x, &trans_7_y, x,y,z, zminus, zplus, rad, xl,yl, xh,yh, x, y, y);
}


void zero_cons (struct cons *c) {
  int i, j, k;
  for (i=0; i<SX; ++i){
    for (j=0; j<SY; ++j){
      for (k=0; k<SZ; ++k){
        c->empty[i][j][k] = CONS_DISCON;
      }
    }
  }

  for(i=0; i<SX*SY; ++i) {
    c->conx[i] = 1;
    c->conx[i] = 1;
    c->conz[i] = 1;
  }

  c->con_num = 0;;
}


