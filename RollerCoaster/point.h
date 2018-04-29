/*
 *	RollerCoaster2005
 *	Copyright (C) 2005 Plusplus (plusplus AT free.fr)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _POINT_HEADER
#define _POINT_HEADER

typedef struct{
	float x,y,z;
} point;

#ifdef __cplusplus
	extern "C" {
#endif

void mult(point*, float, point*);

void add_mult(point*, float, point*);

float norme2(point*);

float norme(point*);

void normalize(point*);

void sub(point*, point*, point*);

void add(point*, point*, point*);

float prod_scal(point*, point*);

void prod_vect(point*, point*, point*);

void rotate(point*, point*, float, point*);

int intersection(point*, point*, point*, point*, point*);

#ifdef __cplusplus
	}
#endif

#endif /* _POINT_HEADER */
