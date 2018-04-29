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

#ifndef _POTEAU_HEADER
#define _POTEAU_HEADER

#include "point.h"

struct support
{
	int nbPillar;
	int nbLink;
	int maxNbPillar;
	int maxNbLink;
	point *pillar;
	point *link;
	unsigned char *pillar_color;
	unsigned char *link_color;
	unsigned int metal_texture;
	unsigned int tree_texture;
};

struct support_desc
{
	int nbPillarCoord;
	int nbPillarAbsc;
	point *pillar_coord;
	int *pillar_absc;
};

#ifdef __cplusplus
extern "C" {
#endif

struct pipe;

int support_init(struct support_desc* supd, struct support* sts, struct pipe* stp, point* ptang);
void support_free(struct support* sts);
void support_draw(struct support* sts);
void drawtree();

#ifdef __cplusplus
}
#endif

#endif /* _POTEAU_HEADER */
