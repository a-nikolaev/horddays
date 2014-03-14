
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
#ifndef _SIM_H
#define _SIM_H

#include "state.h"

#define FAILED_WALK -99
#define FAILED_ATTACK -98

int attack_attempt (struct state *s, int i, int j, int k, int ni, int nj);

/* returns dz if can walk to (ni,nj,k), it is -1, 0, or 1. Returns FAILED_WALK otherwise  */
int walk_attempt (struct grid *g, int ni, int nj, int k);

void hit(struct state *s, int i, int j, int k, enum hit_type);

void add_smell(struct state *s, int i, int j, int k, int amount);

void make_noise(struct state *s, int i, int j, int k, double amount);

void print_prio(struct prio *p);

void run_all_mobs_simplified (struct state *s);

void run_mobs (struct state *s);

#endif
