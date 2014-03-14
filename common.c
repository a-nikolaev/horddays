
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

int urandom (int v){
  return (rand() % v);
}

double urandomf (double v){
  return v*((double)rand() / (double)RAND_MAX);
}

int sample_arr (int num, int v[], double rate[]) {
  
  int i;
  double total = 0;
  for(i=0; i<num; ++i) {
    total += rate[i];
  }

  double x = urandomf(total);

  double sum = 0.0;
  i = 0;
  while(i < num && sum < x) {
    sum += rate[i];
    i++;
  }

  return (i-1);
}
