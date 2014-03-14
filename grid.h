
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
#ifndef _GRID_H
#define _GRID_H


#define SX 100
#define SY 101
#define SZ 8

#define BLOOD_STAINS_MAX 6
#define SMELL_PIC_MAX 7

/* unit/mob ID */
#define ID_PLAYER -1
#define ID_NONE -2
#define ID_NO_ITEM -2

struct coord { int x; int y; int z; };

enum tl {tl_floor, tl_wall, tl_midair, tl_exit};

int tl_walkable (enum tl tl);
int tl_lookthru (enum tl tl);

struct loc { 
  int id_mob;
  int id_item;
  enum tl tl;
  int stains;
  int smell;
  int smoke;
};

struct grid {
  struct loc loc [SX][SY][SZ];
  int block [SX][SY][SZ];
  int height;
  int width;
};


int zlevel(struct grid *g, int i, int j);
int is_within_ij (struct grid *g, int i, int j);
int is_within_ijk (struct grid *g, int i, int j, int k);

int is_not_occupied (struct grid *g, int i, int j, int k);

#define VIS_RAD 10/* you can see this many tiles in one direction */
struct vision {
  int v [SX][SY][SZ];
  int seen [SX][SY][SZ];
};

void blank_vision (struct grid *g, struct vision *v, int x, int y, int rad);
void blank_vision_seen (struct grid *g, struct vision *v, int x, int y, int rad);

void comp_vision (struct grid *g, struct vision *v, int x, int y, int z, int z_minus, int z_plus, int rad);


#define CONS_DISCON -1

/* cons */
struct cons {
  /* empty for this many steps */
  int empty [SX][SY][SZ];

  /* connected points */
  int conx [SX*SY];
  int cony [SX*SY];
  int conz [SX*SY];
  int con_num;
};

void zero_cons (struct cons *c);

#endif
