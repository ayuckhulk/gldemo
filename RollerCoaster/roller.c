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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef M_PI  /* some math header doesn't define it */
#define M_PI 3.14159265359f
#endif

#include "GLES/gl.h"

#include "point.h"
#include "pillar.h"
#include "roller.h"

#include "grass_texture.h"
#include "sky_texture.h"

#define FTOX(fl) ((GLfixed)((fl)*65536.0f))
#define XTOF(x) (((float)((int)x))/65536.0f)
#define XHIGH(x) ((x) >> 16)
#define XLOW(x) ((x) & 0x0000FFFF)
#define XMUL(x1,x2) ((int)(((XHIGH(x1) * XHIGH(x2)) << 16) + ((unsigned int)(XLOW(x1) * XLOW(x2)) >> 16) + (XHIGH(x1) * XLOW(x2)) + (XHIGH(x2) * XLOW(x1))))
#define XADD(x1,x2) ((int)((x1) + (x2)))

#define FTO11X(fl) ((int)((fl)*2048.0f))

static const int nbRailSide = 4;
static const int nbMiddleSide = 3;
static const int nbSideLink = 3;
static const float radiusCyl = 0.02f;
static const float radiusLien = 0.01f;
static const float lengthLien = 0.1f;
static const float linkDistance = 0.45f;

/* field of view angle in degree */
static const float aperture = 60.0f;
/* near clip plane coord */
static const float near_clip = 0.01f;
/* far clip plane coord */
static const float far_clip = 60.0f;
/* factor by which the frustum coord is multiplied for culling test */
static const float frustum_factor = 1.0f;
/* threshold distance were the rail are displayed with fewer segment */
static const float lod_threshold = 1.5f;
/* threshold distance were the display is really degraded */
static const float degrade_threshold = 3.0f;

void AllocPipe(struct pipe *p, int nbSegment, int nbSide);
void InitNormales(struct pipe* stp);
void InitCouleurs(struct pipe* stp, point *light_dir);
void InitCouleursLiens(struct connection *link, point *light_dir);


void myPrintf(const char* format, ...)
{
    FILE* f = fopen("g_log.txt", "a");
    if (f)
    {
        va_list args;
        va_start(args, format);
        vfprintf(f, format, args);
        va_end(args);
        fclose(f);
    }
}


int InitGL()
{
	glEnable(GL_DEPTH_TEST);
// TODO: depth function !!!
//	glDepthFunc(GL_LESS);
	/* This will clear the background color to "la couleur du ciel" */
	glClearColorx(FTOX(0.3f), FTOX(0.3f), FTOX(0.7f), FTOX(0.0f));
	/* Enables clearing of the depth buffer */
	glClearDepthx(FTOX(1.0f));
	/* Enables Gouraud shading */
	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_NORMALIZE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	glDisable(GL_DITHER);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
//	glPolygonMode(GL_FRONT,GL_FILL);
	glFrontFace(GL_CCW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return 1;
}


void ReSizeGLScene(struct engine *eng)
{
	float ratio, wd2;

	/* Prevent a divide by zero if the window is too small */
	if(eng->wndHeight == 0)
		eng->wndHeight=1;

	/* Reset the current viewport and perspective transformation */
	glViewport(0, 0, eng->wndWidth, eng->wndHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ratio = eng->wndWidth / (float)eng->wndHeight;
	wd2 = near_clip * (float)tan(0.5f * M_PI * aperture / 180.0f);

	glFrustumx(FTOX(-ratio*wd2),  /* left   */
	           FTOX(ratio*wd2),   /* right  */
	           FTOX(-wd2),        /* bottom */
	           FTOX(wd2),         /* top    */
	           FTOX(near_clip), FTOX(far_clip));

	eng->frustum_right = FTO11X(ratio*wd2);
	eng->frustum_top = FTO11X(wd2);

	glMatrixMode(GL_MODELVIEW);
}


void InitTexture(struct rollercoaster *rc)
{
	glGenTextures(1, &rc->grass_texture);
	glBindTexture(GL_TEXTURE_2D, rc->grass_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass_texture_width, grass_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass_texture_data);

	glGenTextures(1, &rc->sky_texture);
	glBindTexture(GL_TEXTURE_2D, rc->sky_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sky_texture_width, sky_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, sky_texture_data);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void matrix_mult(float *r, float* a, float *b)
{
	int i,j;
	for(i=0 ; i<16 ; i++) r[i]=0.0f;
	for(i=0 ; i<4 ; i++){
		for(j=0 ; j<4 ; j++){
			r[i+4*j] = a[i]*b[4*j] + a[i+4]*b[4*j+1] + a[i+8]*b[4*j+2] + a[i+12]*b[4*j+3];
		}
	}
}


void mult_vect(point *r, float *m, point *v)
{
	float w;
	w = m[3]*v->x + m[7]*v->y + m[11]*v->z + m[15];
	r->x = (m[0]*v->x + m[4]*v->y + m[8]*v->z + m[12]) / w;
	r->y = (m[1]*v->x + m[5]*v->y + m[9]*v->z + m[13]) / w;
	r->z = m[2]*v->x + m[6]*v->y + m[10]*v->z + m[14];
}


/* compute the length of a B�zier patch by descretizing it with 50 line segment */
float longueur_bezier(point *p1, point *p2, point *p3, point *p4)
{
	point a,b;
	float t,f;
	float d = 1.0f/50.0f;
	a = *p1;
	f = 0.0f;
	for(t=d ; t<=1.0f ; t+=d){
		mult(&b, (1-t)*(1-t)*(1-t), p1);
		add_mult(&b, 3*t*(1-t)*(1-t), p2);
		add_mult(&b, 3*t*t*(1-t), p3);
		add_mult(&b, t*t*t, p4);
		sub(&a,&b,&a);
		f += norme(&a);
		a = b;
	}
	return f;
}


/* longueur_seg is the wished length for each line segment of the curve */
void InitCurve(struct track_desc *trkd, struct rail *str, float longueur_seg)
{
	int i,j,k;
	point a,b,c;
	float t,d;
	float *pnbsegment;
	point *pcontrole;
	int nbLine;

	/* initialize pcontrole from trkd->control */
	pcontrole = (point*) malloc((3 * trkd->nbControlPoint + 1) * sizeof(point));
	for(i = 0 ; i < trkd->nbControlPoint ; i++)
	{
		pcontrole[3*i] = trkd->control[2*i];
		pcontrole[3*i+1] = trkd->control[2*i+1];
	}
	pcontrole[3 * trkd->nbControlPoint] = pcontrole[0];
	/* compute intermediate points */
	for(i = 0 ; i < 3 * trkd->nbControlPoint ; i += 3)
	{
		b = pcontrole[i+1];
		if(i>0) j = i - 3; else j = 3 * trkd->nbControlPoint - 3;
		if(norme(&b)<=0.1f)
		{
			sub(&b,&pcontrole[i+3],&pcontrole[j]);
		}
		normalize(&b);
		sub(&c,&pcontrole[i+3],&pcontrole[i]);
		if(prod_scal(&b,&c)<0.0f){ b.x = -b.x; b.y = -b.y; b.z = -b.z; }
		mult(&a,norme(&c)*M_PI/5.5f/1.41421f,&b);
		add(&pcontrole[i+1],&pcontrole[i],&a);
		b.x = -b.x; b.y = -b.y; b.z = -b.z;
		sub(&c,&pcontrole[i],&pcontrole[j]);
		mult(&a,norme(&c)*M_PI/5.5f/1.41421f,&b);
		if(i>0) k = i - 1; else k = 3 * trkd->nbControlPoint - 1;
		add(&pcontrole[k],&pcontrole[i],&a);
	}
	/* compute the number of segment for each B�zier patch */
	pnbsegment = (float*)malloc(trkd->nbControlPoint * sizeof(float));
	nbLine = 0;
	for(i = 0 ; i < trkd->nbControlPoint ; i++)
	{
		j = i * 3;


		t = longueur_bezier(&pcontrole[j],&pcontrole[j+1],&pcontrole[j+2],&pcontrole[j+3]);
		pnbsegment[i] = t / longueur_seg;
		if(!pnbsegment[i]) pnbsegment[i] = 1;
		nbLine += (int)pnbsegment[i] + 1;
	}
	/* descretize all patches */
	AllocPipe(&str->middle, nbLine, nbMiddleSide);
	str->middle.radius = radiusCyl;
	k = 0;
	for(i = 0 ; i < trkd->nbControlPoint ; i++)
	{
		a = pcontrole[3*i];
		str->middle.segment[k++] = a;
		d = 1.0f / (pnbsegment[i] + 1.0f);
		for(j=1,t=d ; j<pnbsegment[i] ; j++,t+=d)
		{
			mult(&b, (1-t)*(1-t)*(1-t), &pcontrole[3*i]);
			add_mult(&b, 3*t*(1-t)*(1-t), &pcontrole[3*i+1]);
			add_mult(&b, 3*t*t*(1-t), &pcontrole[3*i+2]);
			add_mult(&b, t*t*t, &pcontrole[3*i+3]);
			str->middle.segment[k] = b;
//			if(b.x * b.x + b.y * b.y > maxDist) maxDist = b.x * b.x + b.y * b.y;
			k++;
		}
	}
//	maxDist = (float)sqrt(maxDist);
	free(pnbsegment);
	free(pcontrole);
}


/* intersection du cylindre d'axe v avec le plan passant par p de normal n */
void calcCercle(point *p, point *z, point *v, int nbCote, float rayon, float angle, point *n, point *pmesh)
{
	point r;
	int i;
	for(i=0 ; i<nbCote ; i++){
		rotate(&r, v, (angle+(float)i)*2.0f*M_PI/(float)nbCote, z);
		normalize(&r);
		mult(&r,rayon,&r);
		add(&r,p,&r);
		intersection(&r,&r,v,p,n);
		pmesh[i] = r;
	}
}


void CalcPipeVert(struct pipe *tube, struct curve *track, float angle)
{
	point a,b,c;
	point v,p;
	int i;
	a = tube->segment[tube->nbSegment - 1];
	for(i = 0 ; i < tube->nbSegment ; i++){
		b = tube->segment[i];
		c = tube->segment[(i+1) % tube->nbSegment];
		sub(&v,&b,&a);
		normalize(&v);
		sub(&c,&c,&b);
		normalize(&c);
		add(&p,&v,&c);
		c = track->position[i];
		calcCercle(&b, &c, &v, tube->nbSide, tube->radius, angle + track->bend[i], &p, &tube->vert[i * tube->nbSide]);
		a = b;
	}
}


void moyenne(float *tab, int nbelem, int nbmoy_pred, int nbmoy_succ)
{
	float *buf;
	int i,j,k;
	float tot,f,m;
	buf = (float*)malloc(nbelem*sizeof(float));
	for(i=0 ; i<nbelem ; i++) buf[i] = tab[i];
	for(i=0 ; i<nbelem ; i++){
		k = i;
		tot = 0.0f;
		m = 0.0f;
		for(j=0 ; j<nbmoy_pred ; j++){
			f = (float)j / nbmoy_pred;
			f = 1.0f - f*f;
			m += f;
			tot += f * buf[k];
			k = (k-1+nbelem) % nbelem;
		}
		k = (i+1) % nbelem;
		for(j=1 ; j<nbmoy_succ ; j++){
			f = (float)j / nbmoy_succ;
			f = 1.0f - f*f;
			m += f;
			tot += f * buf[k];
			k = (k+1) % nbelem;
		}
		tab[i] = tot / m;
	}
	free(buf);
}


void AllocPipe(struct pipe *p, int nbSegment, int nbSide)
{
	point* buf;
	p->nbSegment = nbSegment;
	p->nbSide = nbSide;
	p->nbVert = nbSegment * nbSide;
	buf = malloc((p->nbSegment + 2 * p->nbVert) * sizeof(point));
	p->segment = buf;
	p->vert = &buf[p->nbSegment];
	p->color = &buf[p->nbSegment + p->nbVert];
}


void InitLines(struct rail* str, struct curve* stc, point* light_dir, float twistFactor)
{
	int i,j,k;
	float t;
	point a,b,c;
	point p,v;
	point *pline = str->middle.segment;
	int nbLine = str->middle.nbSegment;
	char* buf;
	int bufSize;

	stc->nbVertex = str->middle.nbSegment;
	/* allocate memory for the curve structure arrays */
	bufSize = stc->nbVertex * (2 * sizeof(float) + 3 * sizeof(point));
	buf = malloc(bufSize);
	stc->trajectory = (point*)buf;
	buf += stc->nbVertex * sizeof(point);
	stc->tangent = (point*)buf;
	buf += stc->nbVertex * sizeof(point);
	stc->position = (point*)buf;
	buf += stc->nbVertex * sizeof(point);
	stc->bend = (float*)buf;
	buf += stc->nbVertex * sizeof(float);
	stc->length = (float*)buf;
	/* initialisation de stc->bend et stc->length */
	k=0;
	for(i=0 ; i<stc->nbVertex ; i++)
	{
		sub(&p, &pline[i], &pline[(i-1+nbLine)%nbLine]);
		sub(&v, &pline[(i+1)%nbLine], &pline[i]);
		stc->length[i] = norme(&p);
		normalize(&p);
		normalize(&v);
		p.z = v.z = 0;
		prod_vect(&a, &v, &p);
		stc->bend[i] = twistFactor * (float)asin(a.z);
/*		stc->bend[i] = stc->bend[i] / 1.8f; */
	}
	moyenne(stc->bend,nbLine,7,5);
	/* initialisation de position */
	c.x = 0.0f; c.y = 0.0f; c.z = 1.0f;
	/************** at point j, curve's upward vector must be +z ************/
	j = 0;
	for(i = 0 ; i < stc->nbVertex ; i++)
	{
		a = pline[(j+i-1+nbLine)%nbLine];
		b = pline[(j+i)%nbLine];
		sub(&p,&b,&a);
		normalize(&p);
		t = -prod_scal(&p,&c)/* / prod_scal(&p,&p)*/;
		add_mult(&c,t,&p);
		normalize(&c);
		prod_vect(&a,&p,&c);
		if(a.z != 0.0f){
			a.z = 0.0f;
			prod_vect(&c,&a,&p);
			normalize(&c);
		}
		stc->position[(i+j)%nbLine] = c;
	}
	/* initialisation de stc->trajectory et stc->tangent */
	a = pline[nbLine-1];
	for(i = 0 ; i < stc->nbVertex ; i++)
	{
		b = pline[i];
		c = pline[(i+1)%nbLine];
		sub(&p, &b, &a);
		normalize(&p);
		sub(&c, &c, &b);
		normalize(&c);
		add(&v, &p, &c);
		stc->tangent[i] = v;
		normalize(&(stc->tangent[i]));
		add(&c, &(stc->position[i]), &b);
		intersection(&a, &c, &p, &b, &v);
		sub(&a, &a, &b);
		normalize(&a);
		stc->trajectory[i] = a;
		a = b;
	}
	/* compute middle rail vertices */
	CalcPipeVert(&str->middle, stc, 0.0f);
	/* initialize rail1 and rail2 */
	AllocPipe(&str->rail1, nbLine, nbRailSide);
	str->rail1.radius = radiusCyl;
	AllocPipe(&str->rail2, nbLine, nbRailSide);
	str->rail2.radius = radiusCyl;
	a = str->middle.segment[nbLine-1];
	for(i=0 ; i<nbLine ; i++)
	{
		b = str->middle.segment[i];
		c = str->middle.segment[(i+1) % str->middle.nbSegment];
		sub(&p,&b,&a);
		normalize(&p);
		sub(&v,&c,&b);
		normalize(&v);
		t = stc->bend[i];
		add(&p,&v,&p);
		normalize(&p);
		v = stc->trajectory[i];
		rotate(&c,&p,t+1.0f*M_PI/3.0f,&v);
		mult(&c,lengthLien,&c);
		add(&(str->rail1.segment[i]),&c,&b);
		rotate(&c,&p,t-1.0f*M_PI/3.0f,&v);
		mult(&c,lengthLien,&c);
		add(&(str->rail2.segment[i]),&c,&b);
		a = b;
	}
	CalcPipeVert(&str->rail1, stc, 0.0f);
	CalcPipeVert(&str->rail2, stc, 0.0f);
	InitNormales(&str->middle);
	InitNormales(&str->rail1);
	InitNormales(&str->rail2);
	InitCouleurs(&str->middle, light_dir);
	InitCouleurs(&str->rail1, light_dir);
	InitCouleurs(&str->rail2, light_dir);
}


void ConvertToFixed(struct rail *str)
{
	int i;
	GLfixed *f;

	for(i = 0 ; i < str->middle.nbVert ; i++)
	{
		f = (GLfixed*)&(str->middle.vert[i]);
		f[0] = FTOX(str->middle.vert[i].x);
		f[1] = FTOX(str->middle.vert[i].y);
		f[2] = FTOX(str->middle.vert[i].z);
	}
	for(i = 0 ; i < str->middle.nbSegment ; i++)
	{
		f = (GLfixed*)&(str->middle.segment[i]);
		f[0] = FTOX(str->middle.segment[i].x);
		f[1] = FTOX(str->middle.segment[i].y);
		f[2] = FTOX(str->middle.segment[i].z);
	}
	for(i = 0 ; i < str->rail1.nbVert ; i++)
	{
		f = (GLfixed*)&(str->rail1.vert[i]);
		f[0] = FTOX(str->rail1.vert[i].x);
		f[1] = FTOX(str->rail1.vert[i].y);
		f[2] = FTOX(str->rail1.vert[i].z);
		f = (GLfixed*)&(str->rail2.vert[i]);
		f[0] = FTOX(str->rail2.vert[i].x);
		f[1] = FTOX(str->rail2.vert[i].y);
		f[2] = FTOX(str->rail2.vert[i].z);
	}
	for(i = 0 ; i < str->rail1.nbSegment ; i++)
	{
		f = (GLfixed*)&(str->rail1.segment[i]);
		f[0] = FTOX(str->rail1.segment[i].x);
		f[1] = FTOX(str->rail1.segment[i].y);
		f[2] = FTOX(str->rail1.segment[i].z);
		f = (GLfixed*)&(str->rail2.segment[i]);
		f[0] = FTOX(str->rail2.segment[i].x);
		f[1] = FTOX(str->rail2.segment[i].y);
		f[2] = FTOX(str->rail2.segment[i].z);
	}
	for(i = 0 ; i < str->link.nbVert ; i++)
	{
		f = (GLfixed*)&(str->link.vert[i]);
		f[0] = FTOX(str->link.vert[i].x);
		f[1] = FTOX(str->link.vert[i].y);
		f[2] = FTOX(str->link.vert[i].z);
	}
	for(i = 0 ; i < str->link.nbSegment ; i++)
	{
		f = (GLfixed*)&(str->link.segment[i]);
		f[0] = FTOX(str->link.segment[i].x);
		f[1] = FTOX(str->link.segment[i].y);
		f[2] = FTOX(str->link.segment[i].z);
	}
}


/* f is the wished distance between two consecutive links */
void InitLiens(struct rail *str, struct curve *stc, float f, point *light_dir)
{
	point a,a1,a2,p,p1,p2,b,v,n;
	float longueur,l;
	int i,j;
	int ia,ib;
	float distLien;
	float t,t1,t2;
	int nbLiens;

	longueur = 0.0f;
	for(i = 0 ; i < str->middle.nbSegment ; i++)
	{
		longueur += stc->length[i];

	}
	nbLiens = (int)floor(longueur / f);
	str->link.nbLink = nbLiens;
	str->link.nbVert = str->link.nbLink * 3 * nbSideLink;
	str->link.nbSegment = str->link.nbLink * 3;
	str->link.segment = (point*)malloc(str->link.nbSegment * sizeof(point));
	str->link.vert = (point*)malloc(str->link.nbVert * sizeof(point));
	str->link.color = (point*)malloc(str->link.nbVert * sizeof(point));
	str->link.radius = radiusLien;
	distLien = longueur / (float)str->link.nbLink;
	l = 0.0f;
	ia = str->middle.nbSegment - 1;
	ib = 0;
	sub(&p, &(str->middle.segment[ib]), &(str->middle.segment[ia]));
	sub(&p1, &(str->rail1.segment[ib]), &(str->rail1.segment[ia]));
	sub(&p2, &(str->rail2.segment[ib]), &(str->rail2.segment[ia]));
	for(i=0 ; i<nbLiens ; i++)
	{
		while(l > stc->length[ib])
		{
			l -= stc->length[ib];
			ia = (ia + 1) % str->middle.nbSegment;
			ib = (ib + 1) % str->middle.nbSegment;
			sub(&p, &(str->middle.segment[ib]), &(str->middle.segment[ia]));
			sub(&p1, &(str->rail1.segment[ib]), &(str->rail1.segment[ia]));
			sub(&p2, &(str->rail2.segment[ib]), &(str->rail2.segment[ia]));
		}
		t = l / stc->length[ib];
		a = str->middle.segment[ia];
		add_mult(&a,t,&p);
		str->link.segment[i*3] = a;
		a1 = str->rail1.segment[ia];
		sub(&v,&a,&a1);
		t1 = prod_scal(&v,&p) / prod_scal(&p1,&p);
		add_mult(&a1,t1,&p1);
		str->link.segment[i*3+1] = a1;
		a2 = str->rail2.segment[ia];
		sub(&v,&a,&a2);
		t2 = prod_scal(&v,&p) / prod_scal(&p2,&p);
		add_mult(&a2,t2,&p2);
		str->link.segment[i*3+2] = a2;
		sub(&n,&a1,&a2);
		normalize(&n);
		sub(&v,&a1,&a);
		normalize(&v);
		prod_vect(&b,&v,&p);
		normalize(&b);
		calcCercle(&a, &b, &v, nbSideLink, radiusLien, 0.0f, &n, &(str->link.vert[i * 3 * nbSideLink]));       // lien avec le rail central
		calcCercle(&a1, &b, &v, nbSideLink, radiusLien, 0.0f, &n, &(str->link.vert[(i * 3 + 1) * nbSideLink]));
		sub(&v,&a,&a2);
		normalize(&v);
		prod_vect(&b,&v,&p);
		normalize(&b);
		calcCercle(&a2, &b, &v, nbSideLink, radiusLien, 0.0f, &n, &(str->link.vert[(i * 3 + 2) * nbSideLink]));
		l += distLien;
	}
	/* compute the normals and store them in the color array */
	for(i = 0 ; i < str->link.nbSegment ; i++)
	{
		for(j = 0 ; j < nbSideLink ; j++)
		{
			sub(&(str->link.color[i*nbSideLink+j]), &(str->link.vert[i*nbSideLink+j]), &(str->link.segment[i]));
			normalize(&(str->link.color[i*nbSideLink+j]));
		}
	}
	InitCouleursLiens(&str->link, light_dir);
}


void InitNormales(struct pipe* stp)
{
	int i;
	for(i = 0 ; i < stp->nbVert ; i++)
	{
		sub(&(stp->color[i]), &(stp->vert[i]), &(stp->segment[i / stp->nbSide]));
		normalize(&(stp->color[i]));
	}
}


void InitCouleurs(struct pipe* stp, point *light_dir)
{
	int i;
	float f;
	const float lightAmbient[] = {0.4f,0.0f,0.4f,1.0f};
	const float lightDiffuse[] = {1.1f,0.8f,1.1f,1.0f};
	GLubyte *byte;
	int red, green, blue;

	byte = (GLubyte*)stp->color;
	normalize(light_dir);
	for(i = 0 ; i < stp->nbVert ; i++)
	{
		f = prod_scal(light_dir, &(stp->color[i]));
		if(f <= 0.0f) f = 0.0f;
		stp->color[i].x = lightDiffuse[0] * f + lightAmbient[0];
		stp->color[i].y = lightDiffuse[1] * f + lightAmbient[1];
		stp->color[i].z = lightDiffuse[2] * f + lightAmbient[2];
		red = (int)(stp->color[i].x * 255.0f);
		green = (int)(stp->color[i].y * 255.0f);
		blue = (int)(stp->color[i].z * 255.0f);
		if(red < 0)
			byte[i * 4] = 0;
		else if(red > 255)
			byte[i * 4] = 255;
		else
			byte[i * 4] = (char)red;
		if(green < 0)
			byte[i * 4 + 1] = 0;
		else if(green > 255)
			byte[i * 4 + 1] = 255;
		else
			byte[i * 4 + 1] = (char)green;
		if(blue < 0)
			byte[i * 4 + 2] = 0;
		else if(blue > 255)
			byte[i * 4 + 2] = 255;
		else
			byte[i * 4 + 2] = (char)blue;
		byte[i * 4 + 3] = 255;
	}
}


void InitCouleursLiens(struct connection *link, point *light_dir)
{
	int i;
	float f;
	const float lightAmbient[] = {0.4f,0.0f,0.4f,1.0f};
	const float lightDiffuse[] = {1.1f,0.8f,1.1f,1.0f};
	GLubyte *byte;
	int red, green, blue;

	byte = (GLubyte*)link->color;
	normalize(light_dir);
	for(i = 0 ; i < link->nbVert ; i++)
	{
		f = prod_scal(light_dir, &(link->color[i]));
		if(f <= 0.0f) f = 0.0f;
		link->color[i].x = lightDiffuse[0] * f + lightAmbient[0];
		link->color[i].y = lightDiffuse[1] * f + lightAmbient[1];
		link->color[i].z = lightDiffuse[2] * f + lightAmbient[2];
		red = (int)(link->color[i].x * 255.0f);
		green = (int)(link->color[i].y * 255.0f);
		blue = (int)(link->color[i].z * 255.0f);
		if(red < 0)
			byte[i * 4] = 0;
		else if(red > 255)
			byte[i * 4] = 255;
		else
			byte[i * 4] = (char)red;
		if(green < 0)
			byte[i * 4 + 1] = 0;
		else if(green > 255)
			byte[i * 4 + 1] = 255;
		else
			byte[i * 4 + 1] = (char)green;
		if(blue < 0)
			byte[i * 4 + 2] = 0;
		else if(blue > 255)
			byte[i * 4 + 2] = 255;
		else
			byte[i * 4 + 2] = (char)blue;
		byte[i * 4 + 3] = 255;
	}
}

/*
void drawPipe0(struct pipe* p, GLfixed *viewer, GLfixed *forward)
{
	unsigned short elem[14];
	int i, i0, i1, j, k;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, p->vert);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, p->color);
	glColor4x(FTOX(0.8f),FTOX(0.3f),FTOX(0.8f),FTOX(1.0f));
	i0 = 0;
	for(i = 0 ; i < p->nbSegment ; i++)
	{
		i1 = (i + 1) % p->nbSegment;
		k = 0;
		for(j = 0 ; j <= p->nbSide ; j++)
		{
			elem[k++] = (short)(i1 * p->nbSide + j % p->nbSide);
			elem[k++] = (short)(i0 * p->nbSide + j % p->nbSide);
		}
		i0 = i1;
		glDrawElements(GL_TRIANGLE_STRIP, p->nbSide * 2 + 2, GL_UNSIGNED_SHORT, elem);
	}
}
*/

void drawPipe(struct pipe *p, struct engine *eng, GLfixed *viewer, GLfixed *forward, GLfixed *rightward, GLfixed *upward)
{
	unsigned short elem[14];
	int i, i0, i1, j, k;
	int skip1, skip2;
	GLfixed *x;
	GLfixed fw[3], rw[3], uw[3];
	GLfixed ve[3];
	int xproj, yproj, zproj;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, p->vert);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, p->color);
	fw[0] = forward[0] >> 5;
	fw[1] = forward[1] >> 5;
	fw[2] = forward[2] >> 5;
	rw[0] = rightward[0] >> 5;
	rw[1] = rightward[1] >> 5;
	rw[2] = rightward[2] >> 5;
	uw[0] = upward[0] >> 5;
	uw[1] = upward[1] >> 5;
	uw[2] = upward[2] >> 5;
	x = (GLfixed*)&(p->segment[0]);
	ve[0] = (x[0] - viewer[0]) >> 5;
	ve[1] = (x[1] - viewer[1]) >> 5;
	ve[2] = (x[2] - viewer[2]) >> 5;
	skip1 = 1;
	zproj = (ve[0] * fw[0] + ve[1] * fw[1] + ve[2] * fw[2]) >> 11;
	if( zproj >= FTO11X(near_clip) )
	{
		xproj = abs(ve[0] * rw[0] + ve[1] * rw[1] + ve[2] * rw[2]) >> 11;
		yproj = abs(ve[0] * uw[0] + ve[1] * uw[1] + ve[2] * uw[2]) >> 11;
		if( xproj * eng->frustum_right < zproj * FTO11X(near_clip * frustum_factor) && yproj * eng->frustum_top < zproj * FTO11X(near_clip * frustum_factor) )
			skip1 = 0;
	}
	i0 = 0;
	for(i = 0 ; i < p->nbSegment ; i++)
	{
		i1 = (i + 1) % p->nbSegment;
		x = (GLfixed*)&(p->segment[i1]);
		ve[0] = (x[0] - viewer[0]) >> 5;
		ve[1] = (x[1] - viewer[1]) >> 5;
		ve[2] = (x[2] - viewer[2]) >> 5;
		skip2 = 1;
		zproj = (ve[0] * fw[0] + ve[1] * fw[1] + ve[2] * fw[2]) >> 11;
		if( zproj >= FTO11X(near_clip) )
		{
			xproj = abs(ve[0] * rw[0] + ve[1] * rw[1] + ve[2] * rw[2]) >> 11;
			yproj = abs(ve[0] * uw[0] + ve[1] * uw[1] + ve[2] * uw[2]) >> 11;
			if( xproj * eng->frustum_right < zproj * FTO11X(near_clip * frustum_factor) && yproj * eng->frustum_top < zproj * FTO11X(near_clip * frustum_factor) )
				skip2 = 0;
		}
		if( skip1 && skip2 )
		{
			i0 = i1;
			continue;
		}
		k = 0;
		for(j = 0 ; j <= p->nbSide ; j++)
		{
			elem[k++] = (short)(i1 * p->nbSide + j % p->nbSide);
			elem[k++] = (short)(i0 * p->nbSide + j % p->nbSide);
			if(zproj > FTO11X(degrade_threshold)) j++;
		}
		i0 = i1;
		skip1 = skip2;
		glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_SHORT, elem);
		if( (zproj > FTO11X(lod_threshold)) && i < (p->nbSegment - 2) && (i % 2) == 0 ) i++;
	}
}


void drawConnection(struct connection *c, struct engine *eng, GLfixed *viewer, GLfixed *forward, GLfixed *rightward, GLfixed *upward)
{
	unsigned short elem[10];
	int i,j,k;
	int skip1,skip2,skip3;
	int far_away;
	GLfixed *x;
	GLfixed fw[3], rw[3], uw[3];
	GLfixed ve[3];
	int xproj, yproj, zproj;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, c->vert);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, c->color);
	fw[0] = forward[0] >> 5;
	fw[1] = forward[1] >> 5;
	fw[2] = forward[2] >> 5;
	rw[0] = rightward[0] >> 5;
	rw[1] = rightward[1] >> 5;
	rw[2] = rightward[2] >> 5;
	uw[0] = upward[0] >> 5;
	uw[1] = upward[1] >> 5;
	uw[2] = upward[2] >> 5;
	for(i = 0 ; i < c->nbLink ; i++)
	{
		skip1 = 1;
		x = (GLfixed*)&(c->segment[i * 3]);
		ve[0] = (x[0] - viewer[0]) >> 5;
		ve[1] = (x[1] - viewer[1]) >> 5;
		ve[2] = (x[2] - viewer[2]) >> 5;
		zproj = (ve[0] * fw[0] + ve[1] * fw[1] + ve[2] * fw[2]) >> 11;
		if( zproj >= FTO11X(near_clip) && zproj < FTO11X(degrade_threshold) )
		{
			xproj = abs(ve[0] * rw[0] + ve[1] * rw[1] + ve[2] * rw[2]) >> 11;
			yproj = abs(ve[0] * uw[0] + ve[1] * uw[1] + ve[2] * uw[2]) >> 11;
			if( xproj * eng->frustum_right < zproj * FTO11X(near_clip * frustum_factor) && yproj * eng->frustum_top < zproj * FTO11X(near_clip * frustum_factor) )
				skip1 = 0;
		}
		far_away = zproj > FTO11X(lod_threshold);
		skip2 = 1;
		x = (GLfixed*)&(c->segment[i * 3 + 1]);
		ve[0] = (x[0] - viewer[0]) >> 5;
		ve[1] = (x[1] - viewer[1]) >> 5;
		ve[2] = (x[2] - viewer[2]) >> 5;
		zproj = (ve[0] * fw[0] + ve[1] * fw[1] + ve[2] * fw[2]) >> 11;
		if( zproj >= FTO11X(near_clip) && zproj < FTO11X(degrade_threshold) )
		{
			xproj = abs(ve[0] * rw[0] + ve[1] * rw[1] + ve[2] * rw[2]) >> 11;
			yproj = abs(ve[0] * uw[0] + ve[1] * uw[1] + ve[2] * uw[2]) >> 11;
			if( xproj * eng->frustum_right < zproj * FTO11X(near_clip * frustum_factor) && yproj * eng->frustum_top < zproj * FTO11X(near_clip * frustum_factor) )
				skip2 = 0;
		}
		skip3 = 1;
		x = (GLfixed*)&(c->segment[i * 3 + 2]);
		ve[0] = (x[0] - viewer[0]) >> 5;
		ve[1] = (x[1] - viewer[1]) >> 5;
		ve[2] = (x[2] - viewer[2]) >> 5;
		zproj = (ve[0] * fw[0] + ve[1] * fw[1] + ve[2] * fw[2]) >> 11;
		if( zproj >= FTO11X(near_clip) && zproj < FTO11X(degrade_threshold) )
		{
			xproj = abs(ve[0] * rw[0] + ve[1] * rw[1] + ve[2] * rw[2]) >> 11;
			yproj = abs(ve[0] * uw[0] + ve[1] * uw[1] + ve[2] * uw[2]) >> 11;
			if( xproj * eng->frustum_right < zproj * FTO11X(near_clip * frustum_factor) && yproj * eng->frustum_top < zproj * FTO11X(near_clip * frustum_factor) )
				skip3 = 0;
		}

		if( !(skip1 && skip2) )
		{
			k = 0;
			for(j=0 ; j<nbSideLink+1 ; j++)
			{
				if(far_away && j==nbSideLink-1) j++;
				elem[k++] = (unsigned short)((i * 3 + 1) * nbSideLink + (j % nbSideLink));
				elem[k++] = (unsigned short)(i * 3 * nbSideLink + (j % nbSideLink));
			}
			glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_SHORT, elem);
		}
		if( !(skip1 && skip3) )
		{
			k = 0;
			for(j=0 ; j<nbSideLink+1 ; j++)
			{
				if(far_away && j==nbSideLink-1) j++;
				elem[k++] = (unsigned short)(i * 3 * nbSideLink + (j % nbSideLink));
				elem[k++] = (unsigned short)((i * 3 + 2) * nbSideLink + (j % nbSideLink));
			}
			glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_SHORT, elem);
		}
	}
}


#define GROUND_RADIUS 50.0f
#define GROUND_RADIUS_TEX 50.0f
#define SKY_HEIGHT 10.0f
void drawSkyAndGround(struct rollercoaster *rc)
{
	const int nbSides = 6;
	const GLfixed ground[29*3] = {
		FTOX(0), FTOX(0), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS * 0.5), FTOX(0.05 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS * -0.5), FTOX(0.05 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS * -1), FTOX(0), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS * -0.5), FTOX(0.05 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS * 0.5), FTOX(0.05 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS * 0.5), FTOX(0.1 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS * -0.5), FTOX(0.1 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS * -1), FTOX(0), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS * -0.5), FTOX(0.1 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS * 0.5), FTOX(0.1 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS * 0.5), FTOX(0.2 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS * -0.5), FTOX(0.2 * GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS * -1), FTOX(0), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS * -0.5), FTOX(0.2 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS * 0.5), FTOX(0.2 * GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(GROUND_RADIUS * 0.5), FTOX(GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * -0.5), FTOX(GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * -1), FTOX(0), FTOX(0),
		FTOX(GROUND_RADIUS * -0.5), FTOX(GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * 0.5), FTOX(GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(GROUND_RADIUS), FTOX(0), FTOX(0)
		};
	const GLfixed ground_tex[29*2] = {
		FTOX(0), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS_TEX * 0.5), FTOX(0.05 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.05 * GROUND_RADIUS_TEX * -0.5), FTOX(0.05 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.05 * GROUND_RADIUS_TEX * -1), FTOX(0),
		FTOX(0.05 * GROUND_RADIUS_TEX * -0.5), FTOX(0.05 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.05 * GROUND_RADIUS_TEX * 0.5), FTOX(0.05 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.05 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS_TEX * 0.5), FTOX(0.1 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.1 * GROUND_RADIUS_TEX * -0.5), FTOX(0.1 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.1 * GROUND_RADIUS_TEX * -1), FTOX(0),
		FTOX(0.1 * GROUND_RADIUS_TEX * -0.5), FTOX(0.1 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.1 * GROUND_RADIUS_TEX * 0.5), FTOX(0.1 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.1 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS_TEX * 0.5), FTOX(0.2 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.2 * GROUND_RADIUS_TEX * -0.5), FTOX(0.2 * GROUND_RADIUS_TEX * 0.866025),
		FTOX(0.2 * GROUND_RADIUS_TEX * -1), FTOX(0),
		FTOX(0.2 * GROUND_RADIUS_TEX * -0.5), FTOX(0.2 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.2 * GROUND_RADIUS_TEX * 0.5), FTOX(0.2 * GROUND_RADIUS_TEX * -0.866025),
		FTOX(0.2 * GROUND_RADIUS_TEX), FTOX(0),
		FTOX(GROUND_RADIUS_TEX), FTOX(0),
		FTOX(GROUND_RADIUS_TEX * 0.5), FTOX(GROUND_RADIUS_TEX * 0.866025),
		FTOX(GROUND_RADIUS_TEX * -0.5), FTOX(GROUND_RADIUS_TEX * 0.866025),
		FTOX(GROUND_RADIUS_TEX * -1), FTOX(0),
		FTOX(GROUND_RADIUS_TEX * -0.5), FTOX(GROUND_RADIUS_TEX * -0.866025),
		FTOX(GROUND_RADIUS_TEX * 0.5), FTOX(GROUND_RADIUS_TEX * -0.866025),
		FTOX(GROUND_RADIUS_TEX), FTOX(0)
		};
	const GLubyte ground_elem1[14] = {
		1, 8,
		2, 9,
		3, 10,
		4, 11,
		5, 12,
		6, 13,
		7, 14
		};
	const GLubyte ground_elem2[14] = {
		8, 15,
		9, 16,
		10, 17,
		11, 18,
		12, 19,
		13, 20,
		14, 21
		};
	const GLubyte ground_elem3[14] = {
		15, 22,
		16, 23,
		17, 24,
		18, 25,
		19, 26,
		20, 27,
		21, 28
		};
	const GLfixed sky[29*3] = {
		FTOX(GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(GROUND_RADIUS * 0.5), FTOX(GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * -0.5), FTOX(GROUND_RADIUS * 0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * -1), FTOX(0), FTOX(0),
		FTOX(GROUND_RADIUS * -0.5), FTOX(GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(GROUND_RADIUS * 0.5), FTOX(GROUND_RADIUS * -0.866025), FTOX(0),
		FTOX(0.923879 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.923879 * GROUND_RADIUS * 0.5), FTOX(0.923879 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.923879 * GROUND_RADIUS * -0.5), FTOX(0.923879 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.923879 * GROUND_RADIUS * -1), FTOX(0), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.923879 * GROUND_RADIUS * -0.5), FTOX(0.923879 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.923879 * GROUND_RADIUS * 0.5), FTOX(0.923879 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.707107 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.707107 * GROUND_RADIUS * 0.5), FTOX(0.707107 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.707107 * GROUND_RADIUS * -0.5), FTOX(0.707107 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.707107 * GROUND_RADIUS * -1), FTOX(0), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.707107 * GROUND_RADIUS * -0.5), FTOX(0.707107 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.707107 * GROUND_RADIUS * 0.5), FTOX(0.707107 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.382683 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0.382683 * GROUND_RADIUS * 0.5), FTOX(0.382683 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0.382683 * GROUND_RADIUS * -0.5), FTOX(0.382683 * GROUND_RADIUS * 0.866025), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0.382683 * GROUND_RADIUS * -1), FTOX(0), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0.382683 * GROUND_RADIUS * -0.5), FTOX(0.382683 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0.382683 * GROUND_RADIUS * 0.5), FTOX(0.382683 * GROUND_RADIUS * -0.866025), FTOX(GROUND_RADIUS * 0.923879),
		FTOX(0), FTOX(0), FTOX(GROUND_RADIUS),
		FTOX(GROUND_RADIUS), FTOX(0), FTOX(0),
		FTOX(0.923879 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.382683),
		FTOX(0.707107 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.707107),
		FTOX(0.382683 * GROUND_RADIUS), FTOX(0), FTOX(GROUND_RADIUS * 0.923879)
		};
	const GLfixed sky_tex[29*2] = {
		FTOX(0), FTOX(1.0),
		FTOX(1.0 / 6.0), FTOX(1.0),
		FTOX(2.0 / 6.0), FTOX(1.0),
		FTOX(3.0 / 6.0), FTOX(1.0),
		FTOX(4.0 / 6.0), FTOX(1.0),
		FTOX(5.0 / 6.0), FTOX(1.0),
		FTOX(0), FTOX(0.75),
		FTOX(1.0 / 6.0), FTOX(0.75),
		FTOX(2.0 / 6.0), FTOX(0.75),
		FTOX(3.0 / 6.0), FTOX(0.75),
		FTOX(4.0 / 6.0), FTOX(0.75),
		FTOX(5.0 / 6.0), FTOX(0.75),
		FTOX(0), FTOX(0.5),
		FTOX(1.0 / 6.0), FTOX(0.5),
		FTOX(2.0 / 6.0), FTOX(0.5),
		FTOX(3.0 / 6.0), FTOX(0.5),
		FTOX(4.0 / 6.0), FTOX(0.5),
		FTOX(5.0 / 6.0), FTOX(0.5),
		FTOX(0), FTOX(0.25),
		FTOX(1.0 / 6.0), FTOX(0.25),
		FTOX(2.0 / 6.0), FTOX(0.25),
		FTOX(3.0 / 6.0), FTOX(0.25),
		FTOX(4.0 / 6.0), FTOX(0.25),
		FTOX(5.0 / 6.0), FTOX(0.25),
		FTOX(0.5), FTOX(0),
		FTOX(1.0), FTOX(1.0),
		FTOX(1.0), FTOX(0.75),
		FTOX(1.0), FTOX(0.5),
		FTOX(1.0), FTOX(0.25)
		};
	const GLubyte sky_elem1[14] = {
		0, 6,
		1, 7,
		2, 8,
		3, 9,
		4, 10,
		5, 11,
		25, 26
	};
	const GLubyte sky_elem2[14] = {
		6, 12,
		7, 13,
		8, 14,
		9, 15,
		10, 16,
		11, 17,
		26, 27
	};
	const GLubyte sky_elem3[14] = {
		12, 18,
		13, 19,
		14, 20,
		15, 21,
		16, 22,
		17, 23,
		27, 28
	};
	const GLubyte sky_elem4[8] = {
		24,
		28,
		23,
		22,
		21,
		20,
		19,
		18
	};

	glDisable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glVertexPointer(3, GL_FIXED, 0, ground);
	glTexCoordPointer(2, GL_FIXED, 0, ground_tex);
	glBindTexture(GL_TEXTURE_2D, rc->grass_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, nbSides + 2);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, ground_elem1);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, ground_elem2);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, ground_elem3);
	glVertexPointer(3, GL_FIXED, 0, sky);
	glTexCoordPointer(2, GL_FIXED, 0, sky_tex);
	glBindTexture(GL_TEXTURE_2D, rc->sky_texture);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, sky_elem1);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, sky_elem2);
	glDrawElements(GL_TRIANGLE_STRIP, nbSides * 2 + 2, GL_UNSIGNED_BYTE, sky_elem3);
	glDrawElements(GL_TRIANGLE_FAN, nbSides + 2, GL_UNSIGNED_BYTE, sky_elem4);
	glDisable(GL_TEXTURE_2D);
	glShadeModel(rc->eng.shade_model);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_DEPTH_TEST);
}


void drawEverything(struct rollercoaster *rc, GLfixed *viewer, GLfixed *forward, GLfixed *rightward, GLfixed *upward)
{
	drawSkyAndGround(rc);
	drawPipe(&rc->str.middle, &rc->eng, viewer, forward, rightward, upward);
	drawPipe(&rc->str.rail1, &rc->eng, viewer, forward, rightward, upward);
	drawPipe(&rc->str.rail2, &rc->eng, viewer, forward, rightward, upward);
	drawConnection(&rc->str.link, &rc->eng, viewer, forward, rightward, upward);
	support_draw(&rc->sts);
}


void fade(float opacity)
{
	static const int rect[4*3] = {
		65536,  65536,  0,
		-65536, 65536,  0,
		65536,  -65536, 0,
		-65536, -65536, 0
		};
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4x(0, 0, 0, FTOX(opacity));
    glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, rect);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


GLvoid DrawGLScene0(unsigned long timeTick, struct rollercoaster *rc)
{
	unsigned long diftime;
	float fps;
	float angle;
	float dist; /* camera distance from origin */
	const float maxZ = 4.0f;
	/* maximum distance of the curve from the origin */
	const float maxDist = 7.0f;
	struct engine *eng = &rc->eng;
	GLfixed rightward[3];
	GLfixed forward[3];
	GLfixed viewer[3];
	GLfixed upward[3];
	GLfixed m[16];
	point fw, uw, rw;
	point eye;

	if(eng->isFirstCall == 1)
	{
		eng->isFirstCall = 2;
		eng->oldTimeFrame = timeTick;
		eng->beginTime = eng->oldTimeFrame;
		eng->nbFrame = 0;
	}
	eng->time = timeTick;

	angle = (float)(eng->time - eng->beginTime) / 100; /* angle in degree */
	angle = (angle + 180.0f) / 180.0f * M_PI;          /* angle in radian */
	/* compute camera distance from the origin */
	if(maxDist*tan(aperture*M_PI/360.0f)*2.0f < maxZ)
		dist = maxZ / (float)tan(aperture*M_PI/360.0f) / 2.0f;
	else
		dist = 0.5f + maxDist;

//	gluLookAt(dist*cos(angle),dist*sin(angle),maxZ,                /* eye position */
//				0.0f,0.0f,maxZ+1.0f-dist*tan(aperture*M_PI/360.0f),  /* target       */
//				0.0f,0.0f,1.0f);                                     /* up vector    */
	fw.x = - dist * (float)cos(angle);
	fw.y = - dist * (float)sin(angle);
	fw.z = (maxZ + 1.0f - dist * (float)tan(aperture * M_PI / 360.0f)) - maxZ;
	uw.x = - dist * (float)cos(angle);
	uw.y = - dist * (float)sin(angle);
	uw.z = - dist * dist / fw.z;
	normalize(&fw);
	normalize(&uw);
	prod_vect(&rw, &fw, &uw);
	forward[0] = FTOX(fw.x);
	forward[1] = FTOX(fw.y);
	forward[2] = FTOX(fw.z);
	rightward[0] = FTOX(rw.x);
	rightward[1] = FTOX(rw.y);
	rightward[2] = FTOX(rw.z);
	upward[0] = FTOX(uw.x);
	upward[1] = FTOX(uw.y);
	upward[2] = FTOX(uw.z);
	eye.x = dist * (float)cos(angle);
	eye.y = dist * (float)sin(angle);
	eye.z = maxZ;
	viewer[0] = FTOX(eye.x);
	viewer[1] = FTOX(eye.y);
	viewer[2] = FTOX(eye.z);

	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = FTOX(0.0f);
	m[15] = FTOX(1.0f);
	m[0] = rightward[0];  m[4] = rightward[1];  m[8] = rightward[2];
	m[1] = upward[0];     m[5] = upward[1];     m[9] = upward[2];
	m[2] = FTOX(-fw.x);   m[6] = FTOX(-fw.y);   m[10] = FTOX(-fw.z);
	glLoadIdentity();
	glLoadMatrixx(m);
	glTranslatex(FTOX(-eye.x), FTOX(-eye.y), FTOX(-eye.z));

	drawEverything(rc, viewer, forward, rightward, upward);
	/* fade in at the beginning */
	if(eng->time - eng->beginTime < 2000)
	{
		diftime = eng->time - eng->beginTime;
		fade(1.0f-(float)diftime / 2000.0f);
	}
	/* fade out at the end */
	if(eng->time - eng->beginTime > 34000)
	{
		diftime = eng->time - eng->beginTime - 34000;
		fade((float)diftime / 2000.0f);
	}

	if(eng->time - eng->oldTimeFrame > 200.0f)
	{
		fps=(float)eng->nbFrame * 1000.0f / (float)(eng->time - eng->oldTimeFrame);
		eng->oldTimeFrame = eng->time;
		eng->nbFrame = 0;
	}
	eng->nbFrame++;
	if(eng->time - eng->beginTime >= 36000)
	{
		eng->isFirstCall = 3;
	}
}


GLvoid DrawGLScene(unsigned long timeTick, struct rollercoaster *rc, float roll_angle, float pitch_angle)
{
	float fps;
	float t;
	point p;
	point position, eye;
	point ddv,ddv1,ddv2;
	GLfixed m[16];
	GLfixed *pline = (GLfixed*)rc->str.middle.segment;
	int nbLine = rc->stc.nbVertex;
	GLfixed viewer[3];
	GLfixed forward[3];
	GLfixed rightward[3];
	GLfixed upward[3];
	struct engine *eng = &rc->eng;
	struct curve *stc = &rc->stc;

	if(eng->isFirstCall)
	{
		eng->oldTimeFrame = timeTick;
		eng->index = rc->trkd.startSegment;
		eng->time = eng->beginTime = eng->oldTimeFrame;
		eng->nbFrame = 0;
	}
	else
	{
		eng->time = timeTick;
	}
	while(eng->abscisse > stc->length[(eng->index+1)%nbLine])
	{
		eng->abscisse -= stc->length[(eng->index+1)%nbLine];
		eng->index++;
		eng->index %= nbLine;
	}

	t = eng->abscisse / stc->length[(eng->index+1)%nbLine];
	position.x = XTOF(pline[eng->index * 3]);
	position.y = XTOF(pline[eng->index * 3 + 1]);
	position.z = XTOF(pline[eng->index * 3 + 2]);
//	sub(&p, &pline[(eng->index+1)%nbLine], &pline[eng->index]);
	p.x = XTOF(pline[((eng->index + 1) % nbLine) * 3] - pline[eng->index * 3]);
	p.y = XTOF(pline[((eng->index + 1) % nbLine) * 3 + 1] - pline[eng->index * 3 + 1]);
	p.z = XTOF(pline[((eng->index + 1) % nbLine) * 3 + 2] - pline[eng->index * 3 + 2]);
	add_mult(&position, t, &p);
	ddv1 = stc->trajectory[eng->index];
	sub(&p, &(stc->trajectory[(eng->index+1)%nbLine]), &(stc->trajectory[eng->index]));
	add_mult(&ddv1, t, &p);
	/******** ddv1 vector points upward ********/
	normalize(&ddv1);
	ddv = stc->tangent[eng->index];
	sub(&p, &(stc->tangent[(eng->index+1)%nbLine]), &(stc->tangent[eng->index]));
	add_mult(&ddv, t, &p);
	/******** ddv vector points forward ********/
	normalize(&ddv);
	p = ddv1;
	rotate(&ddv1, &ddv, stc->bend[eng->index] + t * (stc->bend[(eng->index+1)%nbLine] - stc->bend[eng->index]), &p);
	/******** ddv2 vector points rightward ********/
	prod_vect(&ddv2, &ddv, &ddv1);
	/* compute camera position */
	mult(&eye,0.2f,&ddv1);
	add(&eye,&eye,&position);

	/* rotate camera */
	if (roll_angle!=0.0f)
	{
		/* Rotate the front and side vectors around the up vector */
	    p = ddv; /* Front vector */
	    rotate(&ddv, &ddv1, roll_angle, &p);
	    p = ddv2; /* Side vector */
	    rotate(&ddv2, &ddv1, roll_angle, &p);
	}
	else if (pitch_angle!=0.0f)
	{
		/* Rotate the up and front vectors around the side vector. */
	    p = ddv; /* Front vector */
	    rotate (&ddv, &ddv2, pitch_angle, &p);
	    p = ddv1; /* Up vector */
	    rotate (&ddv1, &ddv2, pitch_angle, &p);
	}

	forward[0] = FTOX(ddv.x);
	forward[1] = FTOX(ddv.y);
	forward[2] = FTOX(ddv.z);
	rightward[0] = FTOX(ddv2.x);
	rightward[1] = FTOX(ddv2.y);
	rightward[2] = FTOX(ddv2.z);
	upward[0] = FTOX(ddv1.x);
	upward[1] = FTOX(ddv1.y);
	upward[2] = FTOX(ddv1.z);

	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = FTOX(0.0f);
	m[15] = FTOX(1.0f);
	m[0] = FTOX(ddv2.x);  m[4] = FTOX(ddv2.y);  m[8] = FTOX(ddv2.z);
	m[1] = FTOX(ddv1.x);  m[5] = FTOX(ddv1.y);  m[9] = FTOX(ddv1.z);
	m[2] = FTOX(-ddv.x);  m[6] = FTOX(-ddv.y);  m[10] = FTOX(-ddv.z);
	glLoadMatrixx(m);
	mult(&ddv1,0.2f,&ddv1);
	glTranslatex(FTOX(-position.x - ddv1.x), FTOX(-position.y - ddv1.y), FTOX(-position.z - ddv1.z));

	viewer[0] = FTOX(eye.x);
	viewer[1] = FTOX(eye.y);
	viewer[2] = FTOX(eye.z);
	drawEverything(rc, viewer, forward, rightward, upward);
	/* fade in at the beginning of the ride */
	if(eng->time - eng->beginTime < 1500)
	{
		fade(1.0f-(float)(eng->time - eng->beginTime) / 1500.0f);
	}

	if(eng->isLiftHill)
	{
		if(eng->oldz - position.z > 0.0f) eng->isLiftHill = 0;
		else eng->vitesse = 0.3f;
	}
	else
	{
		if(eng->index >= rc->trkd.brakeSegment || eng->isStopping)
		{
			eng->isStopping = 1;
			if(eng->vitesse > 0.0f)
			{
				eng->vitesse = eng->vitesse * eng->vitesse - 5.6f * (float)(eng->time - eng->oldTime) / 1000.0f;
				if(eng->vitesse <= 0.0f)
				{
					eng->vitesse = 0.0f;
					eng->stopTime = eng->time;
				}
				else eng->vitesse = (float)sqrt(eng->vitesse);
			}
			else
			{
				if(eng->time - eng->stopTime > 5000)
				{
					eng->isStopping = 0;
					eng->isLiftHill = 1;
				}
			}
		}
		else
		{
			eng->vitesse = eng->vitesse * eng->vitesse + 3.0f * (eng->oldz - position.z);
			if(eng->vitesse <= 0.0f) eng->vitesse = 0.0f;
			else eng->vitesse = (float)sqrt(eng->vitesse);
		}
	}
	glFlush();
	if(eng->isFirstCall)
	{
		eng->isFirstCall = 0;
		eng->oldTime = eng->time - 1;
	}
	eng->abscisse += eng->vitesse * (float)(eng->time - eng->oldTime) / 1000.0f;
	if(eng->time - eng->oldTimeFrame > 200.0f)
	{
		fps=(float)eng->nbFrame * 1000.0f / (float)(eng->time - eng->oldTimeFrame);
		eng->oldTimeFrame = eng->time;
		eng->nbFrame = 0;
	}
	eng->nbFrame++;
	eng->oldz = position.z;
	eng->oldTime = eng->time;
}


int InitializeRoller(struct rollercoaster *rc, char *filename)
{
	point light_dir={1.0f,-0.5f,2.0f};
	rc->eng.shade_model = GL_SMOOTH;
	InitGL();
	ReSizeGLScene(&rc->eng);

	InitCurve(&rc->trkd,&rc-> str, rc->trkd.averageSegmentLength);
	if(rc->trkd.startSegment < 0) rc->trkd.startSegment = rc->str.middle.nbSegment + rc->trkd.startSegment;
	if(rc->trkd.brakeSegment < 0) rc->trkd.brakeSegment = rc->str.middle.nbSegment + rc->trkd.brakeSegment;
	InitLines(&rc->str, &rc->stc, &light_dir, rc->trkd.twistFactor);
	InitLiens(&rc->str, &rc->stc, linkDistance, &light_dir);
	support_init(&rc->supd, &rc->sts, &rc->str.middle, rc->stc.tangent);
	ConvertToFixed(&rc->str);
	rc->eng.index = 0;
	rc->eng.abscisse = 0.0f;
	rc->eng.oldz = 0.0f;
	rc->eng.vitesse = 0.0f;
	rc->eng.isLiftHill = 1;
	rc->eng.isFirstCall = 1;
	rc->eng.isStopping = 0;
	rc->eng.time = 0;
	rc->eng.oldTime = 0;
	rc->eng.oldTimeFrame = 0;
	rc->eng.stopTime = 0;
	rc->eng.beginTime = 0;
	rc->eng.nbFrame = 0;
	InitTexture(rc);

	rc->eng.perspective_correction = 1;

	rc->eng.mipmapping = 0;

	rc->eng.update_parameter = 1;

	return 1;
}


int DrawRoller(unsigned long tickTime, struct rollercoaster *rc)
{
	glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);
	if(rc->eng.update_parameter)
	{
		if(rc->eng.perspective_correction)
		{
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		}
		else
		{
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		}
		glBindTexture(GL_TEXTURE_2D, rc->grass_texture);
		if(rc->eng.mipmapping)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		rc->eng.update_parameter = 0;
	}
	if((rc->eng.isFirstCall == 1 || rc->eng.isFirstCall == 2) && rc->eng.doIntro)
	{
		DrawGLScene0(tickTime, rc);
	}
	else
	{
		DrawGLScene(tickTime, rc, 0, 0);
	}
	return ROLLER_NOERROR;
}


void SetRollerShadeMode(struct rollercoaster *rc, int mode)
{
	if(mode == ROLLER_SHADE_SMOOTH)
	{
		rc->eng.shade_model = GL_SMOOTH;
	}
	if(mode == ROLLER_SHADE_FLAT)
	{
		rc->eng.shade_model = GL_FLAT;
	}
	glShadeModel(rc->eng.shade_model);
}


void SetRollerPerspectiveCorrection(struct rollercoaster *rc, int perspective)
{
	rc->eng.perspective_correction = perspective;
	rc->eng.update_parameter = 1;
}


void SetRollerMipmapping(struct rollercoaster *rc, int mipmapping)
{
	rc->eng.mipmapping = mipmapping;
	rc->eng.update_parameter = 1;
}


int ExitRoller(struct rollercoaster *rc)
{
	free(rc->stc.trajectory);
	rc->stc.trajectory = NULL;
	rc->stc.tangent = NULL;
	rc->stc.position = NULL;
	rc->stc.bend = NULL;
	rc->stc.length = NULL;
	free(rc->str.link.segment);
	rc->str.link.segment = NULL;
	free(rc->str.link.vert);
	rc->str.link.vert = NULL;
	free(rc->str.link.color);
	rc->str.link.color = NULL;
	free(rc->str.middle.segment);
	rc->str.middle.segment = NULL;
	rc->str.middle.vert = NULL;
	rc->str.middle.color = NULL;
	free(rc->str.rail1.segment);
	rc->str.rail1.segment = NULL;
	rc->str.rail1.vert = NULL;
	rc->str.rail1.color = NULL;
	free(rc->str.rail2.segment);
	rc->str.rail2.segment = NULL;
	rc->str.rail2.vert = NULL;
	rc->str.rail2.color = NULL;
	support_free(&rc->sts);
	return ROLLER_NOERROR;
}
