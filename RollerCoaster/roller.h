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

#ifndef _ROLLER_HEADER
#define _ROLLER_HEADER

#include "point.h"
#include "pillar.h"

#define ROLLER_NOERROR 0
#define ROLLER_GLWINDOWCREATION_ERROR 1
#define ROLLER_SHADE_SMOOTH 0
#define ROLLER_SHADE_FLAT 1

struct track_desc
{
	/* number of control points that describes the track */
	int nbControlPoint;
	/* coordinate of the control points */
	point *control;
	/* index of segment where the train starts */
	int startSegment;
	/* index of segment where the train starts braking */
	int brakeSegment;
	/* length of segments that the track is made of */
	float averageSegmentLength;
	/* amount of tilt when the track tilt in turns */
	float twistFactor;
};

struct engine
{
	/* display width */
	int wndWidth;
	/* display height */
	int wndHeight;
	/* show coaster before starting the ride */
	int doIntro;
	/* coordinate of the right frustum border as a 21.11 fixed point value */
	int frustum_right;
	/* coordinate of the top frustum border as a 21.11 fixed point value */
	int frustum_top;
	int shade_model;
	int perspective_correction;
	int mipmapping;
	int update_parameter;
	unsigned long begin_time;
	int index;
	float abscisse;
	float oldz;
	float vitesse;
	int isLiftHill;
	int isFirstCall;
	int isStopping;
	unsigned long time;
	unsigned long oldTime;
	unsigned long oldTimeFrame;
	unsigned long stopTime;
	unsigned long beginTime;
	int nbFrame;
};

struct curve
{
	/* number of vertex that make the curve */
	int nbVertex;
	/* trajectory of the curve at each vertex */
	point *trajectory;
	/* tangent to the curve */
	point *tangent;
	/* positions of the camera with respect to the track */
	point *position;
	/* tilt angle for each vertex */
	float *bend;
	/* lenght of each curve segment */
	float *length;
};

struct pipe
{
	int nbSegment;
	int nbSide;
	int nbVert;
	float radius;
	point *segment;
	point *vert;
	point *color;
};

struct connection
{
	int nbLink;
	int nbSegment;
	int nbVert;
	float radius;
	point *segment;
	point *vert;
	point *color;
};

struct rail
{
	struct pipe middle;
	struct pipe rail1;
	struct pipe rail2;
	struct connection link;
};

struct rollercoaster
{
	struct track_desc trkd;
	struct engine eng;
	struct rail str;
	struct curve stc;
	struct support_desc supd;
	struct support sts;
	unsigned int grass_texture;
	unsigned int sky_texture;
};

#ifdef __cplusplus
extern "C" {
#endif
/*
void ReSizeGLScene(struct engine *eng);
int InitializeRoller(struct track_desc *trkd, struct engine *eng, struct rail *str, struct curve *stc, char *filename);
int DrawRoller(unsigned long tickTime, struct track_desc *trkd, struct engine *eng, struct rail *str, struct curve *stc);
int ExitRoller(struct track_desc *trkd, struct engine *eng, struct rail *str, struct curve *stc);
void SetRollerShadeMode(struct engine *eng, int mode);
*/
int InitializeRoller(struct rollercoaster *rc, char *filename);
int DrawRoller(unsigned long tickTime, struct rollercoaster *rc);
int ExitRoller(struct rollercoaster *rc);
void SetRollerShadeMode(struct rollercoaster *rc, int mode);
void SetRollerPerspectiveCorrection(struct rollercoaster *rc, int perspective);
void SetRollerMipmapping(struct rollercoaster *rc, int mipmapping);


#ifdef __cplusplus
}
#endif

#endif /* _ROLLER_HEADER */
