
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
#ifndef _COMMON_H
#define _COMMON_H

#define MIN(x,y) ((x)<(y))?(x):(y)
#define MAX(x,y) ((x)<(y))?(y):(x)

// A random integer variate from Uniform[0,n-1]
int urandom (int v);

double urandomf (double v);

int sample_arr (int num, int v[], double rate[]);

#endif
