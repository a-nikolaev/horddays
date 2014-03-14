
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
#include <math.h>
#include "common.h"
#include "gen.h"
#include "state.h"

/* Vector */
struct vec vec_sum (struct vec *v1, struct vec *v2) { struct vec vu = {v1->x + v2->x, v1->y+v2->y}; return vu; }
struct vec vec_scale (double c, struct vec *v) { struct vec vu = {c*v->x, c*v->y}; return vu; }
struct vec random_unit_vec() {double phi = urandomf(2.0*M_PI); struct vec v = {cos(phi), sin(phi)}; return v;}
double vec_norm2 (struct vec *v){return (v->x*v->x + v->y*v->y);}
double vec_norm (struct vec *v){return sqrt(v->x*v->x + v->y*v->y);}

int dir_of_vec(struct vec *v) {
  double x = v->x;
  double y = v->y;
  const double z = 0.9239;
  const double mz = -0.9239; 
  double len = vec_norm(v);
  
  int d = urandom(8);
  
  if (len > 0.0001) {
    x = x/len;
    y = y/len;
    {
    if (x > z) 
      d = 0; // E 
    else if (x < mz)
      d = 4; // W
    else if (y > z) 
      d = 2; // N 
    else if (y < mz)
      d = 6; // S
    else if (x > 0.0) {
      if (y > 0.0) 
        d = 1; // NE 
      else 
        d = 7; // SE
    }
    else {
      if (y > 0.0)
        d = 3; // NW 
      else 
        d = 5; // SW
    }
    }
  }

  return d;
}

/* Prio */
/* Quick sort. From largest to smallest */
void aux_sort_prio_swap (struct prio *prio, int i, int j) {
  int tid = prio->id[i];
  double tap = prio->ap[i];
  prio->id[i] = prio->id[j];
  prio->ap[i] = prio->ap[j];
  prio->id[j] = tid;
  prio->ap[j] = tap;
}

int aux_sort_prio_partition (struct prio *prio, int left, int right, int pivot) {
  double ap_piv = prio->ap[pivot];
  aux_sort_prio_swap (prio, pivot, right); 

  int stored = left;
  int i;
  for(i=left; i<right; ++i) {
    if (prio->ap[i] > ap_piv) { 
      aux_sort_prio_swap(prio, i, stored); 
      stored++;
    }
  }
  aux_sort_prio_swap(prio, right, stored); 
  return stored;
}

void aux_sort_prio (struct prio *prio, int left, int right) {
  if (left < right) {
    int i1 = left;
    int i2 = (left + right)/2;
    int i3 = right;
    double ap1 = prio->ap[i1];
    double ap2 = prio->ap[i2];
    double ap3 = prio->ap[i3];

    /* choose the median of left, middle, and right */
    int pivot = i2;
    if ((ap1-ap2) * (ap1-ap3) <= 0) { pivot = i1; }
    if ((ap3-ap2) * (ap3-ap2) <= 0) { pivot = i3; }

    int pivot2 = aux_sort_prio_partition(prio, left, right, pivot);
    
    aux_sort_prio(prio, left, pivot2-1);
    aux_sort_prio(prio, pivot2+1, right);
  }
}

void sort_prio (struct prio *prio) {
  aux_sort_prio(prio, 0, prio->num-1); 
}


void update_vision(struct state *s) {

  int x = s->pl.coord.x;
  int y = s->pl.coord.y;
  int z = s->pl.coord.z;

  blank_vision(&s->grid, &s->vision, x, y, VIS_RAD+2);

  comp_vision (&s->grid, &s->vision, x, y, z-1, 0, 0, VIS_RAD);
  comp_vision (&s->grid, &s->vision, x, y, z,   5, 1, VIS_RAD);
  comp_vision (&s->grid, &s->vision, x, y, z+1, 0, 0, VIS_RAD);
}

