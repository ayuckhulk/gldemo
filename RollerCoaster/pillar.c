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

/* under windows, gl.h needs windows.h to be included */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI  /* some math header doesn't define it */
#define M_PI 3.14159265359f
#endif

#include "GLES/gl.h"
#include "roller.h"
#include "pillar.h"

#include "metal_texture.h"
#include "tree_texture.h"

#define FTOX(fl) ((GLfixed)((fl)*65536.0f))
/*
point *ptree;
int nbTree;

float metalLength = 2.0f;
float metalAngle = 0.0f;
point metalPosition = {-1.0f,-1.0f,0.5f};

point panneauPosition = {0.0f,0.0f,0.0f};
point panneauDirection;
*/
static const int nbCotePoteau = 4;
static const int nbCoteLien = 3;

static const float rayonPoteau = 0.02f;
static const float rayonLien = 0.015f;

static const float distPoteau = 0.4f;
static const float hauteurLien = 0.1f;

static const point light_dir = {0.43643578047f, -0.2182178902f, 0.87287156094f};
static const point lightAmbient = {0.4f, 0.4f, 0.4f};
static const point lightDiffuse = {1.0f, 0.9f, 1.0f};
static const point light = {1.0f, 0.5f, 1.0f};

//FILE *file;


void cylinder(point* a, point* b, point* v, int nbcote, float rayon, point *buf, point* bufn){
	int i,k;
	point p1,p2,p4;
	sub(&p1,a,b);
	normalize(&p1);
	p4 = *v;
	add_mult(&p4,prod_scal(v,&p1)/norme2(&p1),&p1);
	normalize(&p4);
	mult(&p4,rayon,&p4);
	k = 0;
	for(i=0 ; i<nbcote ; i++){
		rotate(&p2,&p1,(float)i*2.0f*M_PI/(float)nbcote,&p4);
		add(&buf[k++],a,&p2);
		add(&buf[k++],b,&p2);
	}
	k = 0;
	for(i=0 ; i<nbcote ; i++){
		sub(&p1,&buf[k],a);
		normalize(&p1);
		bufn[k++] = p1;
		sub(&p1,&buf[k],b);
		normalize(&p1);
		bufn[k++] = p1;
	}
}


void calc_color(int nb, unsigned char* color, point* normal)
{
	int i;
	float f;
	int red, green, blue;

	for(i=0 ; i<nb ; i++)
	{
		f = prod_scal((point*)&light_dir, &normal[i]);
		if(f<=0.0f) f = 0.0f;
		red = (int)(255.0f * (lightDiffuse.x * f + light.x * lightAmbient.x));
		green = (int)(255.0f * (lightDiffuse.y * f + light.y * lightAmbient.y));
		blue = (int)(255.0f * (lightDiffuse.z * f + light.z * lightAmbient.z));
		if(red < 0)
			color[4*i] = 0;
		else if(red > 255)
			color[4*i] = 255;
		else
			color[4*i] = (char)red;
		if(green < 0)
			color[4*i+1] = 0;
		else if(green > 255)
			color[4*i+1] = 255;
		else
			color[4*i+1] = (char)green;
		if(blue < 0)
			color[4*i+2] = 0;
		else if(blue > 255)
			color[4*i+2] = 255;
		else
			color[4*i+2] = (char)blue;
		color[4*i+3] = 255;
	}
}


// p is a point on the trajectory
// v is the tangent to the trajectory at that point
void create_pillar(point* p1, point* p2, point* v, point* buf, point* norm)
{
	point a,b;
	int i;

	a = *p1;
	a.z = 0.0f;
	b = *p2;
	b.z = 0.0f;
	if(v->z < 0.0f)
	{
		a.z = b.z = -v->z;
		v->z = 0.0f;
	}
	cylinder(p1,&a,v,nbCotePoteau,rayonPoteau,buf,norm);
	i = nbCotePoteau * 2;
	norm[i].x = 0.0f;
	norm[i].y = 0.0f;
	norm[i].z = 1.0f;
	buf[i] = *p1;
	buf[i++].z += rayonPoteau;
	cylinder(p2,&b,v,nbCotePoteau,rayonPoteau,&buf[i],&norm[i]);
	i += nbCotePoteau * 2;
	norm[i].x = 0.0f;
	norm[i].y = 0.0f;
	norm[i].z = 1.0f;
	buf[i] = *p2;
	buf[i++].z += rayonPoteau;
}


void create_link(point* p, point* p1, point* p2, point* buf, point* norm)
{
	point a;
	float z1, z2;

	z1 = p1->z;
	z2 = p2->z;
	p1->z = p->z - hauteurLien;
	p2->z = p->z - hauteurLien;
	a.x = 0.0f; a.y = 0.0f; a.z = 1.0f;
	cylinder(p,p1,&a,nbCoteLien,rayonLien,buf,norm);
	cylinder(p,p2,&a,nbCoteLien,rayonLien,&buf[nbCoteLien*2],&norm[nbCoteLien*2]);
	p1->z = z1;
	p2->z = z2;
}


void draw_pillar(struct support* support)
{
	unsigned short elem[10];
	int i, j, k;

	glVertexPointer(3, GL_FIXED, 0, support->pillar);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, support->pillar_color);
	for(i=0 ; i<support->nbPillar*2 ; i++)
	{
		k = 0;
		for(j=0 ; j<2*nbCotePoteau ; j++)
		{
			elem[k++] = (unsigned short)(j+i*(2*nbCotePoteau+1));
		}
		elem[k++] = (unsigned short)(i*(2*nbCotePoteau+1));
		elem[k++] = (unsigned short)(i*(2*nbCotePoteau+1)+1);
		glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_SHORT, elem);
		k = 0;
		elem[k++] = (unsigned short)((i+1)*(2*nbCotePoteau+1)-1);
		for(j=0 ; j<2*nbCotePoteau ; j+=2)
		{
			elem[k++] = (unsigned short)(j+i*(2*nbCotePoteau+1));
		}
		elem[k++] = (unsigned short)(i*(2*nbCotePoteau+1));
		glDrawElements(GL_TRIANGLE_FAN, k, GL_UNSIGNED_SHORT, elem);
	}
}


void draw_link(struct support* support)
{
	unsigned short elem[8];
	int i, j, k;

	glVertexPointer(3, GL_FIXED, 0, support->link);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, support->link_color);
	for(i=0 ; i<support->nbLink*2 ; i++)
	{
		k = 0;
		for(j=0 ; j<2*nbCoteLien ; j++)
		{
			elem[k++] = (unsigned short)(j+i*(2*nbCoteLien));
		}
		elem[k++] = (unsigned short)(i*(2*nbCoteLien));
		elem[k++] = (unsigned short)(i*(2*nbCoteLien)+1);
		glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_SHORT, elem);
	}
}


int is_linked(point* r, point* p, point* p1, point* p2, int i, point* pline, int nbLine){
	point v,n,z;
	float t;
	sub(&v,p1,p2);
	z.x = 0.0f; z.y = 0.0f; z.z = 1.0f;
	prod_vect(&n,&v,&z);
	sub(&v,&pline[(i+1)%nbLine],&pline[i]);
	if(!intersection(r,&pline[i],&v,p1,&n)) return 0;
	sub(&z,p1,&pline[i]);
	t = prod_scal(&z,&n) / prod_scal(&v,&n);
	if(t<0.0f || t>=1.0f) return 0;
	sub(&v,r,p);
	v.z = 0.0f;
	if(norme(&v)>0.3f) return 0;
	return 1;
}


void create_support(struct support* sts, point* p, point* v, struct pipe* stp)
{
	point a,b,r;
	point p1, p2;
	int i;
	int flag_last_i=0;
	float zmax;
	point *norm;

	a = *v;
	a.z = 0.0f;
	normalize(&a);
	if(a.x*a.x>0.0001f)
	{
		a.x = -a.y / a.x;
		a.y = 1.0f;
	}
	else
	{ //a.y non nul
		a.y = -a.x / a.y;
		a.x = 1.0f;
	}
	normalize(&a);
	mult(&a,distPoteau/2.0f,&a);
	b = *p;
	add(&p1,&b,&a);
	sub(&p2,&b,&a);
	zmax = -1.0f;
	for(i=0 ; i<stp->nbSegment ; i++)
	{
		if(is_linked(&r, p, &p1, &p2, i, stp->segment, stp->nbSegment))
		{
			if(!flag_last_i && ((p->z*p->z)<0.0001f || p->z > r.z) && r.z-hauteurLien>-v->z)
			{
				norm  = (point*)sts->link_color;
				create_link(&r, &p1, &p2, &sts->link[sts->nbLink*4*nbCoteLien], &norm[sts->nbLink*4*nbCoteLien]);
				sts->nbLink++;
				flag_last_i = 1;
				if(r.z > zmax) zmax = r.z;
			}
			else flag_last_i = 0;
		}
		else flag_last_i = 0;
	}
	if(zmax > -0.5f)
	{
		p1.z = p2.z = zmax;
/*		if(v->z > 0.001f)
		{
			panneauPosition = *p;
			panneauPosition.z = v->z;
			p1.z = p2.z = v->z; v->z = 0.0f;
			panneauDirection = *v;
		}
*/		norm  = (point*)sts->pillar_color;
		create_pillar(&p1, &p2, v, &sts->pillar[sts->nbPillar*(4*nbCotePoteau+2)], &norm[sts->nbPillar*(4*nbCotePoteau+2)]);
		sts->nbPillar++;
	}
}


void convert_to_fixed(struct support* sts)
{
	int i;
	GLfixed *x;

	for(i=0 ; i<sts->nbPillar*(4*nbCotePoteau+2) ; i++)
	{
		x = (GLfixed*)&sts->pillar[i];
		x[0] = FTOX(sts->pillar[i].x);
		x[1] = FTOX(sts->pillar[i].y);
		x[2] = FTOX(sts->pillar[i].z);
	}
	for(i=0 ; i<sts->nbLink*4*nbCoteLien ; i++)
	{
		x = (GLfixed*)&sts->link[i];
		x[0] = FTOX(sts->link[i].x);
		x[1] = FTOX(sts->link[i].y);
		x[2] = FTOX(sts->link[i].z);
	}
}


int init_texture(struct support* sts)
{
	glGenTextures(1, &sts->metal_texture);
	glBindTexture(GL_TEXTURE_2D, sts->metal_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, metal_texture_width, metal_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, metal_texture_data);

	glGenTextures(1, &sts->tree_texture);
	glBindTexture(GL_TEXTURE_2D, sts->tree_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tree_texture_width, tree_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tree_texture_data);

	return 1;
}


/****************** exported functions *******************/


int support_init(struct support_desc* supd, struct support* sts, struct pipe* stp, point* ptang)
{
	point p,v;
	int i;
	int taille_tab;
	point *tab;
	int taille_tab1;
	int *tab1;
	int nbPoteau;

	taille_tab = supd->nbPillarCoord;
	tab = supd->pillar_coord;
	taille_tab1 = supd->nbPillarAbsc;
	tab1 = supd->pillar_absc;
	nbPoteau = taille_tab + taille_tab1;
	sts->maxNbPillar = nbPoteau;
	sts->pillar = (point*)malloc(sts->maxNbPillar * (4 * nbCotePoteau + 2) * sizeof(point));
	sts->pillar_color = (unsigned char*)malloc(sts->maxNbPillar * (4 * nbCotePoteau + 2) * sizeof(point));
	/******** Here we assume that the maximum is 4 links for 1 pillar ********/
	sts->maxNbLink = sts->maxNbPillar * 4;
	sts->link = (point*)malloc(sts->maxNbLink * (4 * nbCoteLien) * sizeof(point));
	sts->link_color = (unsigned char*)malloc(sts->maxNbLink * (4 * nbCoteLien) * sizeof(point));
	sts->nbPillar = 0;
	sts->nbLink = 0;
	for(i=0 ; i<taille_tab ; i++)
	{
		p = tab[2*i];
		v = tab[2*i+1];
		create_support(sts, &p, &v, stp);
//		myPrintf("%2d coord %2d\n", i, sts->nbPillar);
	}
	for(i=0 ; i<taille_tab1 ; i++)
	{
		p = stp->segment[tab1[i]];
		v = ptang[tab1[i]];
		p.z = v.z = 0.0f;
		create_support(sts, &p, &v, stp);
//		myPrintf("%2d absc %2d\n", i, sts->nbPillar);
	}
	calc_color(sts->nbPillar*(4*nbCotePoteau+2), sts->pillar_color, (point*)sts->pillar_color);
	calc_color(sts->nbLink*4*nbCoteLien, sts->link_color, (point*)sts->link_color);
	convert_to_fixed(sts);
	return init_texture(sts);
}


void support_free(struct support* sts)
{
	free(sts->pillar);
	free(sts->pillar_color);
	free(sts->link);
	free(sts->link_color);
}


void drawtree(struct support* sts)
{
	const GLfixed tree_vert1[6*3] = {
		FTOX(-0.5f), FTOX(0.0f), FTOX(1.5f),
		FTOX(0.5f), FTOX(0.0f), FTOX(1.5f),
		FTOX(-0.5f), FTOX(0.0f), FTOX(0.0f),
		FTOX(0.5f), FTOX(0.0f), FTOX(0.0f),
		FTOX(-0.5f), FTOX(0.0f), FTOX(1.5f),
		FTOX(0.5f), FTOX(0.0f), FTOX(1.5f)
	};
	const GLfixed tree_text1[6*2] = {
		FTOX(1.0f), FTOX(0.0f),
		FTOX(0.0f), FTOX(0.0f),
		FTOX(1.0f), FTOX(1.0f),
		FTOX(0.0f), FTOX(1.0f),
		FTOX(1.0f), FTOX(0.0f),
		FTOX(0.0f), FTOX(0.0f),
	};
	const GLfixed tree_vert2[6*3] = {
		FTOX(0.0f), FTOX(-0.5f), FTOX(1.5f),
		FTOX(0.0f), FTOX(0.5f), FTOX(1.5f),
		FTOX(0.0f), FTOX(-0.5f), FTOX(0.0f),
		FTOX(0.0f), FTOX(0.5f), FTOX(0.0f),
		FTOX(0.0f), FTOX(-0.5f), FTOX(1.5f),
		FTOX(0.0f), FTOX(0.5f), FTOX(1.5f),
	};
	const GLfixed tree_text2[6*2] = {
		FTOX(1.0f), FTOX(0.0f),
		FTOX(0.0f), FTOX(0.0f),
		FTOX(1.0f), FTOX(1.0f),
		FTOX(0.0f), FTOX(1.0f),
		FTOX(1.0f), FTOX(0.0f),
		FTOX(0.0f), FTOX(0.0f),
	};
	int shade_mode;
	/* backup previous shade mode before setting flat mode */
	glGetIntegerv(GL_SHADE_MODEL, &shade_mode);
	glShadeModel(GL_FLAT);
	glEnable(GL_ALPHA_TEST);

	glAlphaFuncx(GL_NOTEQUAL,0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sts->tree_texture);

	glVertexPointer(3, GL_FIXED, 0, tree_vert1);
	glTexCoordPointer(2, GL_FIXED, 0, tree_text1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glVertexPointer(3, GL_FIXED, 0, tree_vert2);
	glTexCoordPointer(2, GL_FIXED, 0, tree_text2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

	/* restore previous shade mode */
	glShadeModel(shade_mode);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void drawmetal(struct support* sts)
{
	const GLfixed box1v[30*3] = {
		FTOX(5.341640), FTOX(-3.565248), FTOX(0),
		FTOX(5.341640), FTOX(-3.565248), FTOX(0.24),
		FTOX(5.118033), FTOX(-3.677052), FTOX(0.24),
		FTOX(5.118033), FTOX(-3.677052), FTOX(0.18),
		FTOX(4.670819), FTOX(-3.900658), FTOX(0.18),
		FTOX(4.670819), FTOX(-3.900658), FTOX(0),

		FTOX(4.894427), FTOX(-2.670821), FTOX(0.000000),
		FTOX(4.894427), FTOX(-2.670821), FTOX(0.240000),
		FTOX(4.670820), FTOX(-2.782624), FTOX(0.240000),
		FTOX(4.670820), FTOX(-2.782624), FTOX(0.180000),
		FTOX(4.223606), FTOX(-3.006231), FTOX(0.180000),
		FTOX(4.223606), FTOX(-3.006231), FTOX(0.000000),

		FTOX(4.447214), FTOX(-1.776394), FTOX(0.000000),
		FTOX(4.447214), FTOX(-1.776394), FTOX(0.240000),
		FTOX(4.223607), FTOX(-1.888197), FTOX(0.240000),
		FTOX(4.223607), FTOX(-1.888197), FTOX(0.180000),
		FTOX(3.776393), FTOX(-2.111803), FTOX(0.180000),
		FTOX(3.776393), FTOX(-2.111803), FTOX(0.000000),

		FTOX(4.000000), FTOX(-0.881966), FTOX(0.000000),
		FTOX(4.000000), FTOX(-0.881966), FTOX(0.240000),
		FTOX(3.776394), FTOX(-0.993769), FTOX(0.240000),
		FTOX(3.776394), FTOX(-0.993769), FTOX(0.180000),
		FTOX(3.329180), FTOX(-1.217376), FTOX(0.180000),
		FTOX(3.329180), FTOX(-1.217376), FTOX(0.000000),

		FTOX(3.552788), FTOX(0.012461), FTOX(0),
		FTOX(3.552788), FTOX(0.012461), FTOX(0.24),
		FTOX(3.329181), FTOX(-0.099342), FTOX(0.24),
		FTOX(3.329181), FTOX(-0.099342), FTOX(0.18),
		FTOX(2.881967), FTOX(-0.322948), FTOX(0.18),
		FTOX(2.881967), FTOX(-0.322948), FTOX(0)
	};
	const GLfixed box1t[30*2] = {
		FTOX(-10), FTOX(0),
		FTOX(-5.2), FTOX(0),
		FTOX(-0.2), FTOX(0),
		FTOX(-1.4), FTOX(0),
		FTOX(8.6), FTOX(0),
		FTOX(5), FTOX(0),
		FTOX(-10), FTOX(20),
		FTOX(-5.2), FTOX(20),
		FTOX(-0.2), FTOX(20),
		FTOX(-1.4), FTOX(20),
		FTOX(8.6), FTOX(20),
		FTOX(5), FTOX(20),
		FTOX(-10), FTOX(40),
		FTOX(-5.2), FTOX(40),
		FTOX(-0.2), FTOX(40),
		FTOX(-1.4), FTOX(40),
		FTOX(8.6), FTOX(40),
		FTOX(5), FTOX(40),
		FTOX(-10), FTOX(60),
		FTOX(-5.2), FTOX(60),
		FTOX(-0.2), FTOX(60),
		FTOX(-1.4), FTOX(60),
		FTOX(8.6), FTOX(60),
		FTOX(5), FTOX(60),
		FTOX(-10), FTOX(80),
		FTOX(-5.2), FTOX(80),
		FTOX(-0.2), FTOX(80),
		FTOX(-1.4), FTOX(80),
		FTOX(8.6), FTOX(80),
		FTOX(5), FTOX(80)
	};
	const GLubyte elem1[10] = { 1, 0, 7, 6, 13, 12, 19, 18, 25, 24 };
	const GLubyte elem2[10] = { 2, 1, 8, 7, 14, 13, 20, 19, 26, 25 };
	const GLubyte elem3[10] = { 3, 2, 9, 8, 15, 14, 21, 20, 27, 26 };
	const GLubyte elem4[10] = { 4, 3, 10, 9, 16, 15, 22, 21, 28, 27 };
	const GLubyte elem5[10] = { 5, 4, 11, 10, 17, 16, 23, 22, 29, 28 };
	const GLfixed box2v[6*3] = {
		FTOX(3.329181), FTOX(-0.099342), FTOX(0.18),
		FTOX(3.329181), FTOX(-0.099342), FTOX(0.24),
		FTOX(3.552788), FTOX(0.012461), FTOX(0.24),
		FTOX(3.552788), FTOX(0.012461), FTOX(0),
		FTOX(2.881967), FTOX(-0.322948), FTOX(0),
		FTOX(2.881967), FTOX(-0.322948), FTOX(0.18)
	};
	const GLfixed box2t[6*2] = {
		FTOX(-5), FTOX(3.6),
		FTOX(-5), FTOX(4.8),
		FTOX(-10), FTOX(4.8),
		FTOX(-10), FTOX(0),
		FTOX(5), FTOX(0),
		FTOX(5), FTOX(3.6)
	};
	const GLfixed box3v[6*3] = {
		FTOX(5.118033), FTOX(-3.677052), FTOX(0.18),
		FTOX(4.670819), FTOX(-3.900658), FTOX(0.18),
		FTOX(4.670819), FTOX(-3.900658), FTOX(0),
		FTOX(5.341640), FTOX(-3.565248), FTOX(0),
		FTOX(5.341640), FTOX(-3.565248), FTOX(0.24),
		FTOX(5.118033), FTOX(-3.677052), FTOX(0.24)
	};
	const GLfixed box3t[6*2] = {
		FTOX(-5), FTOX(3.6),
		FTOX(5), FTOX(3.6),
		FTOX(5), FTOX(0),
		FTOX(-10), FTOX(0),
		FTOX(-10), FTOX(4.8),
		FTOX(-5), FTOX(4.8)
	};
	int shade_mode;

	/* backup previous shade mode before setting flat mode */
	glGetIntegerv(GL_SHADE_MODEL, &shade_mode);
	glShadeModel(GL_FLAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sts->metal_texture);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glVertexPointer(3, GL_FIXED, 0, box1v);
	glTexCoordPointer(2, GL_FIXED, 0, box1t);
	glColor4x(FTOX(0.692770), FTOX(0.692770), FTOX(0.692770), FTOX(1));
	glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, elem1);
	glColor4x(FTOX(1.272871), FTOX(1.272871), FTOX(1.272871), FTOX(1));
	glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, elem2);
	glColor4x(FTOX(0.4), FTOX(0.4), FTOX(0.4), FTOX(1));
	glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, elem3);
	glColor4x(FTOX(1.272871), FTOX(1.272871), FTOX(1.272871), FTOX(1));
	glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, elem4);
	glColor4x(FTOX(0.4), FTOX(0.4), FTOX(0.4), FTOX(1));
	glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, elem5);

	glVertexPointer(3, GL_FIXED, 0, box2v);
	glTexCoordPointer(2, GL_FIXED, 0, box2t);
	glColor4x(FTOX(0.4), FTOX(0.4), FTOX(0.4), FTOX(1));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glVertexPointer(3, GL_FIXED, 0, box3v);
	glTexCoordPointer(2, GL_FIXED, 0, box3t);
	glColor4x(FTOX(0.790360), FTOX(0.790360), FTOX(0.790360), FTOX(1));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	/* restore previous texture mode */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	/* restore previous shade mode */
	glShadeModel(shade_mode);
	glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void support_draw(struct support* sts)
{
	const int nbTree = 22;
	const GLfixed tree[22*2] = {
		FTOX(1), FTOX(-1.5),
		FTOX(2), FTOX(-2),
		FTOX(-1), FTOX(1.5),
		FTOX(7), FTOX(-4),
		FTOX(6), FTOX(0),
		FTOX(5), FTOX(3),
		FTOX(0), FTOX(5),
		FTOX(-5), FTOX(2),
		FTOX(-1), FTOX(-6),
		FTOX(-4), FTOX(-4),
		FTOX(-3.62033995527138), FTOX(-11.1114785314507),
		FTOX(-9.05828726674162), FTOX(8.1074921020288),
		FTOX(11.4470238806508), FTOX(-5.72286690999838),
		FTOX(-1.70215267118739), FTOX(10.5368591640589),
		FTOX(-0.702056634237779), FTOX(-12.1065680822491),
		FTOX(-2.56702265186181), FTOX(-10.5695213503598),
		FTOX(9.00752213173868), FTOX(-7.2124520920009),
		FTOX(7.495790370539), FTOX(2.88025572058083),
		FTOX(2.47873164219668), FTOX(-8.7618395912889),
		FTOX(-8.72148787584757), FTOX(-10.4433590904693),
		FTOX(-8.96907330103783), FTOX(-7.4251822129953),
		FTOX(-10.4671651105466), FTOX(3.40865499019243),
	};
	int i;
	GLfixed x = 0, y = 0;

	glPushMatrix();
	for(i=0 ; i<nbTree ; i++)
	{
		glTranslatex(tree[i*2] - x, tree[i*2+1] - y, 0);
		x = tree[i*2];
		y = tree[i*2+1];
		drawtree(sts);
	}
	glPopMatrix();
	draw_pillar(sts);
	draw_link(sts);
	drawmetal(sts);
}
