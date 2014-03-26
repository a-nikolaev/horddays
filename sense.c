
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
#include <math.h>

#include "state.h"
#include "common.h"

void sense_sound (struct state *s, int id, int i, int j, int k, double mag) {
  struct mob *m = &s->mobs.mb[id];

  int dx = i - m->coord.x;
  int dy = j - m->coord.y;
  int dz = k - m->coord.z;

  double len2 = dx*dx + dy*dy + dz*dz;

  if (m->senses.sound && len2>0) {
    double power = mag / len2;
    double len = sqrt(len2);
    
    dx = (int)round ((double)dx / len);
    dy = (int)round((double)dy / len);
    dz = (int)round((double)dz / len);

    m->vec.x += power * dx;
    m->vec.y += power * dy;
  }
}

void sense_smell (struct state *s, int id) {
  struct mob *m = &s->mobs.mb[id];
  if (!m->senses.smell) { return; }
  
  int i, j, k;
  int i0, j0, k0;

  i0 = s->mobs.mb[id].coord.x;
  j0 = s->mobs.mb[id].coord.y;
  k0 = s->mobs.mb[id].coord.z;

  int sumx, sumy;
  sumx = 0;
  sumy = 0;

  int rad = 2;

  int di, dj;

  for(i=i0-rad; i<=i0+rad; ++i) {
    for(j=j0-rad; j<=j0+rad; ++j) {
      for(k=k0-rad; k<=k0+rad; ++k) {
        if (is_within_ij(&s->grid, i, j) && k>=0 && k<SZ && !s->grid.block[i][j][k]) {
         
          di = i - i0;
          dj = j - j0;
          di = MAX(MIN(di, 1), -1);
          dj = MAX(MIN(dj, 1), -1);

          sumx += (i-i0) * s->grid.loc[i][j][k].smell;
          sumy += (j-j0) * s->grid.loc[i][j][k].smell;

        }
      }
    }
  }
    
  m->vec.x += SMELL_STRENGTH * (double)sumx;
  m->vec.y += SMELL_STRENGTH * (double)sumy;

}

int bresenham (struct grid *g, int z, int x0, int y0, int x1, int y1) {
  int dx = abs(x1-x0);
  int dy = abs(y1-y0); 
  int sx, sy;
  if (x0 < x1) { sx = 1; } else { sx = -1; }
  if (y0 < y1) { sy = 1; } else { sy = -1; }
  
  int err = dx - dy;

  int e2;

  while(1){
    
    /* plot(x0,y0) */
    if (!loc_lookthru(g,x0,y0,z)) {return 0;}

    if (x0 == x1 && y0 == y1) {break;}
    e2 = 2*err;
    if (e2 > -dy) { 
      err = err - dy;
      x0 = x0 + sx;
    }
    if (e2 < dx) { 
      err = err + dx;
      y0 = y0 + sy; 
    }
  }

  return 1;
}

void sense_sight (struct state *s, int id) {
  struct mob *mob = &s->mobs.mb[id];

  if (!mob->senses.sight) { return; }
  if (!s->pl.ready){ return; }

  int dz = s->pl.coord.z - mob->coord.z;

  if ( (dz == 0 || dz == 1 || dz == -1) && 
      (s->pl.coord.x != mob->coord.x || s->pl.coord.y != mob->coord.y ) )
  {
    int dx = s->pl.coord.x - mob->coord.x;
    int dy = s->pl.coord.y - mob->coord.y;
    if (abs(dx) <= MOB_VIS_RAD && abs(dy) <= MOB_VIS_RAD &&
        dx * dx + dy * dy <= MOB_VIS_RAD * MOB_VIS_RAD &&

        mob->vec.x * dx + mob->vec.y * dy > 0 &&

       (bresenham(&s->grid, s->pl.coord.z, mob->coord.x, mob->coord.y, s->pl.coord.x, s->pl.coord.y) || 
        bresenham(&s->grid, mob->coord.z, mob->coord.x, mob->coord.y, s->pl.coord.x, s->pl.coord.y) ) ) 
    {
      double scalprod = mob->vec.x * (double) dx + mob->vec.y * (double) dy;
      double len2 = (double)(dx*dx + dy*dy);
      double len = sqrt(len2);
      double mag = scalprod / len2;

      mob->vec.x += mag * ((double)dx)/len * SIGHT_STRENGTH;
      mob->vec.y += mag * ((double)dy)/len * SIGHT_STRENGTH;
    }
  }
}
