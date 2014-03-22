
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
#ifndef _STATE_H
#define _STATE_H

#include "grid.h"

#define DEF_ZOMBIE_DELAY 1.5

#define SENSE_ME 1.0

/* */
#define NOISE_STAND SENSE_ME*1.0
#define NOISE_WALK SENSE_ME*200.0
#define NOISE_HIT 600.0
#define NOISE_GRENADE 6000.0
#define NOISE_PISTOL 2000.0
#define NOISE_CANNIBAL 10000.0

/* smell */
#define SMELL_STRENGTH SENSE_ME*100.0

/* sight */
#define MOB_VIS_RAD VIS_RAD
#define SIGHT_STRENGTH SENSE_ME*200.0

#define MAX_HP 5

#define EFFECTS_NUM 3
#define EF_PISTOL 0
#define EF_OLFACTOVISOR 1
#define EF_CANNIBAL 2

enum mob_class { aggressive, friendly };
enum condition { healthy, bitten, zombie };
enum hit_type { hit_normal, hit_bite };
enum mode { mode_move, mode_throw };

/* Player  */
struct item_list { int id_item; struct item_list *tail; };

struct player { 
  struct coord coord; 
  enum condition cond;
  int hp;
  int ready;
  struct item_list *items;

  int marks;
};

/* Mob */
struct senses {
  int sight;
  int sound;
  int smell;
};

struct vec {
  double x; double y;
};

struct vec vec_sum (struct vec *v1, struct vec *v2);
struct vec vec_scale (double c, struct vec *v);
struct vec random_unit_vec ();
double vec_norm2 (struct vec *v);
double vec_norm (struct vec *v);
int dir_of_vec(struct vec *v);

struct mob {
  int alive;
  struct coord coord; 
  struct senses senses; 
  struct vec vec;
  int dir;
  int hp;
  int portrait;
  enum mob_class cl;
  enum condition cond;
};

/* Prio */
#define MOB_NUM 1000

/* ap = action points
 * Mobs move until their ap <= 0.
 */
struct prio { int num; double ap[MOB_NUM]; int id[MOB_NUM]; };

void sort_prio (struct prio *prio);

/* Mobs */
struct mobs {  
  struct mob mb[MOB_NUM];
  int total_num;  
};

/* Throwing */
struct throwing {  
  int x;
  int y;
  int id_item;  
};

/* Status */
struct status {  
  int score;
  int level;
  int effect [EFFECTS_NUM];
};

/* State */
struct state {
  struct grid grid;
  struct cons cons;
  struct vision vision;
  struct player pl;
  int wind_dir;

  struct mobs mobs;
  struct prio prio;

  int players_turn;

  enum mode mode;
  struct throwing thr;

  struct status status;

  struct coord camera;
};

void update_vision(struct state *s);



#endif
