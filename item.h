
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
#ifndef _ITEM_H
#define _ITEM_H

#include "state.h"

#define ITEMS_NUM 9

#define IT_MEDKIT 0
#define IT_ROCK 1
#define IT_ANTIDOTE 2
#define IT_GRENADE 3
#define IT_PISTOL 4
#define IT_OLFACTOVISOR 5
#define IT_CANNIBAL 6
#define IT_SHOTGUN 7
#define IT_SMOKEGR 8

char* item_name[ITEMS_NUM];

enum item_action {ac_throw, ac_consume};

void add_item(struct player *pl, int id_item);

int take_item(struct player *pl);

void use_item(struct state *s, int id_item);

enum item_action get_action(int id_item);
        
double throw_item(struct state *s, struct throwing *thr);

#endif
