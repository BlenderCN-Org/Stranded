

    

#include <fstream.h>   
#include <stdlib.h>   
#include <stdio.h>   
#include "gui16.h"   
   
UINT groundLight;   
UINT spotLight;   
WORD lightLevel[33];   
VECTOR3D vLight;   
UINT errorLine = 0;   
   
inline void vector3dToSTD(VECTOR3D* v)   
{   float d;   
    d = (float)sqrt(v->x*v->x + v->y*v->y + v->z*v->z);   
    v->x /= d,v->y /= d,v->z /= d;    
}   
   
inline float getVectorMod(VECTOR3D* v)   
{   return (float)sqrt(v->x*v->x + v->y*v->y + v->z*v->z);   
}   
   
inline float vectorDotMul(VECTOR3D* v1,VECTOR3D* v2)   
{   return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;   
}   
   
#pragma optimize( "", off )   
inline void vectorForkMul(VECTOR3D* v1,VECTOR3D* v2,VECTOR3D* v)   
{/* VECTOR3D vv;  
    vv.x = v1->y*v2->z - v1->z*v2->y;  
    vv.y = v1->z*v2->x - v1->x*v2->z;  
    vv.z = v1->x*v2->y - v1->y*v2->x;  
    *v = vv;*/   
    __asm   
    {   mov     eax,v1   
        mov     ecx,v2   
        mov     edx,v   
        fld     [eax]v1.y   
        fmul    [ecx]v2.z   
        fld     [eax]v1.z   
        fmul    [ecx]v2.x   
        fld     [eax]v1.x   
        fmul    [ecx]v2.y   
        fld     [eax]v1.z   
        fmul    [ecx]v2.y   
        fld     [eax]v1.x   
        fmul    [ecx]v2.z   
        fld     [eax]v1.y   
        fmul    [ecx]v2.x   
        fxch    st(2)   
        fsubp   st(5),st(0)   
        fsubp   st(3),st(0)   
        fsubp   st(1),st(0)   
        fxch    st(2)   
        fstp    [edx]v.x   
        fstp    [edx]v.y   
        fstp    [edx]v.z   
    }   
}   
#pragma optimize( "", on )   
   
void matrixTranspose(CONVERT_MATRIX3D* s)   
{   CONVERT_MATRIX3D t;   
    t.e11=s->e11, t.e12=s->e21, t.e13=s->e31, t.e14=s->e41;   
    t.e21=s->e12, t.e22=s->e22, t.e23=s->e32, t.e24=s->e42;   
    t.e31=s->e13, t.e32=s->e23, t.e33=s->e33, t.e34=s->e43;   
    t.e41=s->e14, t.e42=s->e24, t.e43=s->e34, t.e44=s->e44;   
    *s = t;   
}   
   
void matrixMul(CONVERT_MATRIX3D* m1,CONVERT_MATRIX3D* m2)   
{   CONVERT_MATRIX3D t;   
    t.e11=m1->e11*m2->e11 + m1->e12*m2->e21 + m1->e13*m2->e31 + m1->e14*m2->e41;   
    t.e12=m1->e11*m2->e12 + m1->e12*m2->e22 + m1->e13*m2->e32 + m1->e14*m2->e42;   
    t.e13=m1->e11*m2->e13 + m1->e12*m2->e23 + m1->e13*m2->e33 + m1->e14*m2->e43;   
    t.e14=m1->e11*m2->e14 + m1->e12*m2->e24 + m1->e13*m2->e34 + m1->e14*m2->e44;   
    t.e21=m1->e21*m2->e11 + m1->e22*m2->e21 + m1->e23*m2->e31 + m1->e24*m2->e41;   
    t.e22=m1->e21*m2->e12 + m1->e22*m2->e22 + m1->e23*m2->e32 + m1->e24*m2->e42;   
    t.e23=m1->e21*m2->e13 + m1->e22*m2->e23 + m1->e23*m2->e33 + m1->e24*m2->e43;   
    t.e24=m1->e21*m2->e14 + m1->e22*m2->e24 + m1->e23*m2->e34 + m1->e24*m2->e44;   
    t.e31=m1->e31*m2->e11 + m1->e32*m2->e21 + m1->e33*m2->e31 + m1->e34*m2->e41;   
    t.e32=m1->e31*m2->e12 + m1->e32*m2->e22 + m1->e33*m2->e32 + m1->e34*m2->e42;   
    t.e33=m1->e31*m2->e13 + m1->e32*m2->e23 + m1->e33*m2->e33 + m1->e34*m2->e43;   
    t.e34=m1->e31*m2->e14 + m1->e32*m2->e24 + m1->e33*m2->e34 + m1->e34*m2->e44;   
    t.e41=m1->e41*m2->e11 + m1->e42*m2->e21 + m1->e43*m2->e31 + m1->e44*m2->e41;   
    t.e42=m1->e41*m2->e12 + m1->e42*m2->e22 + m1->e43*m2->e32 + m1->e44*m2->e42;   
    t.e43=m1->e41*m2->e13 + m1->e42*m2->e23 + m1->e43*m2->e33 + m1->e44*m2->e43;   
    t.e44=m1->e41*m2->e14 + m1->e42*m2->e24 + m1->e43*m2->e34 + m1->e44*m2->e44;   
    *m1 = t;   
}   
   
void MKMoveMatrix(CONVERT_MATRIX3D* m,VECTOR3D* v)   
{   m->e11=1, m->e12=0, m->e13=0, m->e14=v->x;   
    m->e21=0, m->e22=1, m->e23=0, m->e24=v->y;   
    m->e31=0, m->e32=0, m->e33=1, m->e34=v->z;   
    m->e41=0, m->e42=0, m->e43=0, m->e44=1;   
}   
   
void MKScaleMatrix(CONVERT_MATRIX3D* m,POINT3D* p,float rate)   
{   m->e11=rate, m->e12=0, m->e13=0, m->e14=(1 - rate)*p->x;   
    m->e21=0, m->e22=rate, m->e23=0, m->e24=(1 - rate)*p->y;   
    m->e31=0, m->e32=0, m->e33=rate, m->e34=(1 - rate)*p->z;   
    m->e41=0, m->e42=0, m->e43=0, m->e44=1;   
}   
   
void MKRotateMatrix(CONVERT_MATRIX3D* m,VECTOR3D* u1,float th)   
{   float f,a,b,c,ab,ac,bc,a2,b2,c2,sa,sb,sc;   
    VECTOR3D u = *u1;   
    float s = (float)cos(th/2);   
    vector3dToSTD(&u);   
    f = (float)sin(th/2);   
    a = u.x * f;   
    b = u.y * f;   
    c = u.z * f;   
    a2 = a * a * 2;   
    b2 = b * b * 2;   
    c2 = c * c * 2;   
    ab = a * b * 2;   
    ac = a * c * 2;   
    bc = b * c * 2;   
    sa = s * a * 2;   
    sb = s * b * 2;   
    sc = s * c * 2;   
    m->e11=1-b2-c2, m->e12=ab-sc, m->e13=ac+sb, m->e14=0;   
    m->e21=ab+sc, m->e22=1-a2-c2, m->e23=bc-sa, m->e24=0;   
    m->e31=ac-sb, m->e32=bc+sa, m->e33=1-a2-b2, m->e34=0;   
    m->e41=0, m->e42=0, m->e43=0, m->e44=1;   
}   
   
void MKRotateMatrix(CONVERT_MATRIX3D* m,POINT3D* p,VECTOR3D* u1,float th)   
{   CONVERT_MATRIX3D mt,mt1;   
    float x=p->x, y=p->y, z=p->z;   
    MKRotateMatrix(&mt,u1,th);   
    mt1.e11=mt.e11+x*mt.e41;   
    mt1.e12=mt.e12+x*mt.e42;   
    mt1.e13=mt.e13+x*mt.e43;   
    mt1.e14=mt.e14+x*mt.e44;   
    mt1.e21=mt.e21+y*mt.e41;   
    mt1.e22=mt.e22+y*mt.e42;   
    mt1.e23=mt.e23+y*mt.e43;   
    mt1.e24=mt.e24+y*mt.e44;   
    mt1.e31=mt.e31+z*mt.e41;   
    mt1.e32=mt.e32+z*mt.e42;   
    mt1.e33=mt.e33+z*mt.e43;   
    mt1.e34=mt.e34+z*mt.e44;   
    mt1.e41=mt.e41;   
    mt1.e42=mt.e42;   
    mt1.e43=mt.e43;   
    mt1.e44=mt.e44;   
    m->e11 = mt1.e11, m->e12 = mt1.e12, m->e13 = mt1.e13;   
    m->e14 = mt1.e11*(-x) - mt1.e12*y - mt1.e13*z + mt1.e14;   
    m->e21 = mt1.e21, m->e22 = mt1.e22, m->e23 = mt1.e23;   
    m->e24 = mt1.e21*(-x) - mt1.e22*y - mt1.e23*z + mt1.e24;   
    m->e31 = mt1.e31, m->e32 = mt1.e32, m->e33 = mt1.e33;   
    m->e34 = mt1.e31*(-x) - mt1.e32*y - mt1.e33*z + mt1.e34;   
    m->e41 = mt1.e41, m->e42 = mt1.e42, m->e43 = mt1.e43;   
    m->e44 = mt1.e41*(-x) - mt1.e42*y - mt1.e43*z + mt1.e44;   
}   
   
void MKRotateMatrix(CONVERT_MATRIX3D* m,CONVERT_MATRIX3D* m2,POINT3D* p)   
{   CONVERT_MATRIX3D mt;   
    float x=p->x, y=p->y, z=p->z;   
    mt.e11=m2->e11+x*m2->e41;   
    mt.e12=m2->e12+x*m2->e42;   
    mt.e13=m2->e13+x*m2->e43;   
    mt.e14=m2->e14+x*m2->e44;   
    mt.e21=m2->e21+y*m2->e41;   
    mt.e22=m2->e22+y*m2->e42;   
    mt.e23=m2->e23+y*m2->e43;   
    mt.e24=m2->e24+y*m2->e44;   
    mt.e31=m2->e31+z*m2->e41;   
    mt.e32=m2->e32+z*m2->e42;   
    mt.e33=m2->e33+z*m2->e43;   
    mt.e34=m2->e34+z*m2->e44;   
    mt.e41=m2->e41;   
    mt.e42=m2->e42;   
    mt.e43=m2->e43;   
    mt.e44=m2->e44;   
    m->e11 = mt.e11, m->e12 = mt.e12, m->e13 = mt.e13;   
    m->e14 = mt.e11*(-x) - mt.e12*y - mt.e13*z + mt.e14;   
    m->e21 = mt.e21, m->e22 = mt.e22, m->e23 = mt.e23;   
    m->e24 = mt.e21*(-x) - mt.e22*y - mt.e23*z + mt.e24;   
    m->e31 = mt.e31, m->e32 = mt.e32, m->e33 = mt.e33;   
    m->e34 = mt.e31*(-x) - mt.e32*y - mt.e33*z + mt.e34;   
    m->e41 = mt.e41, m->e42 = mt.e42, m->e43 = mt.e43;   
    m->e44 = mt.e41*(-x) - mt.e42*y - mt.e43*z + mt.e44;   
}   
/*  
void MKProjectionMatrix(CONVERT_MATRIX3D* m,float eyePoint)  
{   m->e11 = 1,m->e12 = 0,m->e13 = 0,m->e14 = 0;  
    m->e21 = 0,m->e22 = 1,m->e23 = 0,m->e24 = 0;  
    m->e31 = 0,m->e32 = 0,m->e33 = 0,m->e34 = 0;  
    m->e41 = 0,m->e42 = 0,m->e43 = -1/eyePoint,m->e44 = 1;  
}*/   
/*  
void MKProjectionMatrix(CONVERT_MATRIX3D* m,float eyePoint,float w,float h)  
{   m->e11 = 1,m->e12 = 0,m->e13 = -w/eyePoint,m->e14 = w;  
    m->e21 = 0,m->e22 = -1,m->e23 = -h/eyePoint,m->e24 = h;  
    m->e31 = 0,m->e32 = 0,m->e33 = 0,m->e34 = 0;  
    m->e41 = 0,m->e42 = 0,m->e43 = -1/eyePoint,m->e44 = 1;  
}*/   
   
void MKProjectionMatrix(CONVERT_MATRIX3D* m,float screenPoint,float w,float h)   
{   m->e11 = 1,m->e12 = 0,m->e13 = w/screenPoint,m->e14 = 0;   
    m->e21 = 0,m->e22 = -1,m->e23 = h/screenPoint,m->e24 =0;   
    m->e31 = 0,m->e32 = 0,m->e33 = 1,m->e34 = 0;   
    m->e41 = 0,m->e42 = 0,m->e43 = 1/screenPoint,m->e44 = 0;   
}   
   
void MKWCtoVcMatrix(CONVERT_MATRIX3D* m,VECTOR3D* vz,POINT3D* vp)   
{   VECTOR3D n = *vz,u,v;   
    VECTOR3D vy = {0,1,0};   
    CONVERT_MATRIX3D mt;   
    vector3dToSTD(&n);   
    vectorForkMul(&n,&vy,&u);   
    if(getVectorMod(&u) < 0.9)   
        u.x = 1,u.y = 0, u.z = 0;   
    vectorForkMul(&u,&n,&v);   
    mt.e11 = u.x,mt.e12 = u.y,mt.e13 = u.z,mt.e14 = 0;   
    mt.e21 = v.x,mt.e22 = v.y,mt.e23 = v.z,mt.e24 = 0;   
    mt.e31 = n.x,mt.e32 = n.y,mt.e33 = n.z,mt.e34 = 0;   
    mt.e41 = 0,mt.e42 = 0,mt.e43 = 0,mt.e44 = 1;   
    m->e11 = mt.e11, m->e12 = mt.e12, m->e13 = mt.e13;   
    m->e14 = mt.e11*(-vp->x) - mt.e12*vp->y - mt.e13*vp->z + mt.e14;   
    m->e21 = mt.e21, m->e22 = mt.e22, m->e23 = mt.e23;   
    m->e24 = mt.e21*(-vp->x) - mt.e22*vp->y - mt.e23*vp->z + mt.e24;   
    m->e31 = mt.e31, m->e32 = mt.e32, m->e33 = mt.e33;   
    m->e34 = mt.e31*(-vp->x) - mt.e32*vp->y - mt.e33*vp->z + mt.e34;   
    m->e41 = mt.e41, m->e42 = mt.e42, m->e43 = mt.e43;   
    m->e44 = mt.e41*(-vp->x) - mt.e42*vp->y - mt.e43*vp->z + mt.e44;   
}   
   
inline void dotConvert(POINT3D* dot,CONVERT_MATRIX3D* m)   
{/* float hd;  
    POINT3D p = *dot;  
    dot->x = m->e11*p.x + m->e12*p.y + m->e13*p.z + m->e14;  
    dot->y = m->e21*p.x + m->e22*p.y + m->e23*p.z + m->e24;  
    dot->z = m->e31*p.x + m->e32*p.y + m->e33*p.z + m->e34;  
    hd = m->e41*p.x + m->e42*p.y + m->e43*p.z + m->e44;  
    dot->x /= hd;  
    dot->y /= hd;  
//  dot->z /= hd;*/   
    __asm   
    {   mov     eax,m   
        mov     ecx,dot   
        fld     [ecx]dot.x   
        fmul    [eax]m.e11   
        fld     [ecx]dot.x   
        fmul    [eax]m.e21   
        fld     [ecx]dot.x   
        fmul    [eax]m.e31   
        fld     [ecx]dot.y   
        fmul    [eax]m.e12   
        fld     [ecx]dot.y   
        fmul    [eax]m.e22   
        fld     [ecx]dot.y   
        fmul    [eax]m.e32   
        fxch    st(2)   
        faddp   st(5),st(0)   
        faddp   st(3),st(0)   
        faddp   st(1),st(0)   
        fld     [ecx]dot.z   
        fmul    [eax]m.e13   
        fld     [ecx]dot.z   
        fmul    [eax]m.e23   
        fld     [ecx]dot.z   
        fmul    [eax]m.e33   
        fxch    st(2)   
        faddp   st(5),st(0)   
        faddp   st(3),st(0)   
        faddp   st(1),st(0)   
        fxch    st(2)   
        fadd    [eax]m.e14   
        fxch    st(1)   
        fadd    [eax]m.e24   
        fxch    st(2)   
        fadd    [eax]m.e34   
        fxch    st(1)   
        fld     [ecx]dot.x   
        fmul    [eax]m.e41   
        fld     [ecx]dot.y   
        fmul    [eax]m.e42   
        fld     [ecx]dot.z   
        fmul    [eax]m.e43   
        fxch    st(2)   
        fadd    [eax]m.e44   
        fxch    st(4)   
        fstp    [ecx]dot.z   
        faddp   st(1),st(0)   
        faddp   st(2),st(0)   
        fxch    st(1)   
        fdiv    st(1),st(0)   
        fdivp   st(2),st(0)   
        fstp    [ecx]dot.x   
        fstp    [ecx]dot.y   
    }   
}   
   
inline void dotScale(POINT3D* dot,CONVERT_MATRIX3D* m)   
{   POINT3D p = *dot;   
    dot->x = m->e11*p.x + m->e14;   
    dot->y = m->e22*p.y + m->e24;   
    dot->z = m->e33*p.z + m->e34;   
}   
   
inline void dotRotate(POINT3D* dot,CONVERT_MATRIX3D* m)   
{/* POINT3D p = *dot;  
    dot->x = m->e11*p.x + m->e12*p.y + m->e13*p.z + m->e14;  
    dot->y = m->e21*p.x + m->e22*p.y + m->e23*p.z + m->e24;  
    dot->z = m->e31*p.x + m->e32*p.y + m->e33*p.z + m->e34;*/   
    __asm   
    {   mov     eax,m   
        mov     ecx,dot   
        fld     [ecx]dot.x   
        fmul    [eax]m.e11   
        fld     [ecx]dot.x   
        fmul    [eax]m.e21   
        fld     [ecx]dot.x   
        fmul    [eax]m.e31   
        fld     [ecx]dot.y   
        fmul    [eax]m.e12   
        fld     [ecx]dot.y   
        fmul    [eax]m.e22   
        fld     [ecx]dot.y   
        fmul    [eax]m.e32   
        fxch    st(2)   
        faddp   st(5),st(0)   
        faddp   st(3),st(0)   
        faddp   st(1),st(0)   
        fld     [ecx]dot.z   
        fmul    [eax]m.e13   
        fld     [ecx]dot.z   
        fmul    [eax]m.e23   
        fld     [ecx]dot.z   
        fmul    [eax]m.e33   
        fxch    st(2)   
        faddp   st(5),st(0)   
        faddp   st(3),st(0)   
        faddp   st(1),st(0)   
        fxch    st(2)   
        fadd    [eax]m.e14   
        fxch    st(1)   
        fadd    [eax]m.e24   
        fxch    st(2)   
        fadd    [eax]m.e34   
        fxch    st(1)   
        fstp    [ecx]dot.x   
        fstp    [ecx]dot.z   
        fstp    [ecx]dot.y   
    }   
}   
   
inline void dotMove(POINT3D* dot,CONVERT_MATRIX3D* m)   
{   POINT3D p = *dot;   
    dot->x = p.x + m->e14;   
    dot->y = p.y + m->e24;   
    dot->z = p.z + m->e34;   
}   
   
inline void fillTriangleParam(CPointList* p,TRIANGLE3D* t)   
{   float x1,y1,z1;   
    POINT3D* pt;   
    VECTOR3D v1,v2;   
    pt = &p->points[t->p1];   
    x1 = pt->x, y1 = pt->y, z1 = pt->z;   
    pt = &p->points[t->p2];   
    v1.x = pt->x - x1, v1.y = pt->y - y1, v1.z = pt->z - z1;   
    pt = &p->points[t->p3];   
    v2.x = pt->x - x1, v2.y = pt->y - y1, v2.z = pt->z - z1;   
    vectorForkMul(&v1,&v2,&v1);   
    t->a = v1.x, t->b = v1.y, t->c = v1.z;   
    t->d = -(v1.x*x1+v1.y*y1+v1.z*z1);   
}   
   
inline void fillPolygonParam(CPointList* p,POLYGON3D* t)   
{   float x1,y1,z1;   
    POINT3D* pt;   
    VECTOR3D v1,v2;   
    pt = &p->points[t->points[0]];   
    x1 = pt->x, y1 = pt->y, z1 = pt->z;   
    pt = &p->points[t->points[1]];   
    v1.x = pt->x - x1, v1.y = pt->y - y1, v1.z = pt->z - z1;   
    pt = &p->points[t->points[2]];   
    v2.x = pt->x - x1, v2.y = pt->y - y1, v2.z = pt->z - z1;   
    vectorForkMul(&v1,&v2,&v1);   
    t->a = v1.x, t->b = v1.y, t->c = v1.z;   
    t->d = -(v1.x*x1+v1.y*y1+v1.z*z1);   
}   
   
inline void drawTriangle3d(DDSURFACEDESC* ddsd,CPointList* p,TRIANGLE3D* t,WORD color)   
{   int x1,y1,x2,y2,x3,y3;   
    POINT3D* pt;   
    pt = &p->points[t->p1];   
    x1 = (int)pt->x, y1 = (int)pt->y;   
    pt = &p->points[t->p2];   
    x2 = (int)pt->x, y2 = (int)pt->y;   
    pt = &p->points[t->p3];   
    x3 = (int)pt->x, y3 = (int)pt->y;    
    line(ddsd,x1,y1,x2,y2,color);   
    line(ddsd,x1,y1,x3,y3,color);   
    line(ddsd,x2,y2,x3,y3,color);   
}   
   
#define MAX_LINE_LENTH 40   
//****************************************************************************   
//read .dxf file!   
//****************************************************************************   
BOOL readDxfFile(LPCSTR fileName,CPointList* p,CFaceList* f)   
{   char buf[MAX_LINE_LENTH],*cc;   
    double x,y,z;   
    long p1,p2,p3;   
    long l;   
    UINT base = p->topPos;   
    BOOL swc = FALSE;   
    POINT3D point;   
    TRIANGLE3D triangle;   
    ifstream ifs(fileName);   
    if(!ifs)   
        return FALSE;   
    triangle.maped = FALSE;   
    while(!ifs.eof())   
    {   ifs.getline( buf,MAX_LINE_LENTH-1);   
        if(strcmp(buf,"VERTEX") == 0)   
        {   ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            l = strtol( buf, &cc, 10 );   
            if(l != 10)   
                return FALSE;   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            x = strtod( buf, &cc);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            y = strtod( buf, &cc);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            z = strtod( buf, &cc);   
            if(x == 0 && y == 0 && z == 0)   
            {   ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p1 = strtol( buf, &cc, 10);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p2 = strtol( buf, &cc, 10);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p3 = strtol( buf, &cc, 10);   
                if(p1 >= 0)   
                    triangle.p1 = (UINT)(p1-1+base);   
                else   
                    triangle.p1 = (UINT)(-p1-1+base);   
                if(p2 >= 0)   
                    triangle.p2 = (UINT)(p2-1+base);   
                else   
                    triangle.p2 = (UINT)(-p2-1+base);   
                if(p3 >= 0)   
                    triangle.p3 = (UINT)(p3-1+base);   
                else   
                    triangle.p3 = (UINT)(-p3-1+base);   
                f->addTriangle(&triangle);   
                swc = TRUE;   
            }   
            else   
            {   if(swc)   
                {   swc = FALSE;   
                    base = p->topPos;   
                }   
                point.x = (float)x;   
                point.y = (float)y;   
                point.z = (float)z;   
                p->addPoint(&point);   
            }   
        }   
        if(ifs.bad())   
            return FALSE;   
    }   
    return TRUE;   
}   
   
//****************************************************************************   
//read .asc file!   
//****************************************************************************   
float uuu[5000],vvv[5000];   
BOOL readAscFile(LPCSTR fileName,CPointList* p,CFaceList* f)   
{   char buf[200],*cc;   
    double x,y,z;   
    long p1,p2,p3;   
    long l;   
    char *stopStr,*pos;   
    UINT pid,fid,line = 0;   
    UINT base = p->topPos;   
    BOOL swc = FALSE;   
    POINT3D point;   
    TRIANGLE3D triangle;   
    ifstream ifs(fileName);   
    if(!ifs)   
        return FALSE;   
    triangle.maped = TRUE;   
    triangle.map = 4;   
    while(!ifs.eof())   
    {   ifs.getline( buf,200-1);   
        line++;   
        if((strncmp(buf,"Vertex",6) == 0) && buf[7] >= 48 && buf[7] <= 57)   
        {   pid = strtoul(buf+7, &stopStr, 10 );   
            if(*stopStr != ':')   
                return FALSE;   
            if((pos = strchr(buf,':')) == NULL)   
                return FALSE;   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            point.x = (float)strtod(pos+1,&stopStr);   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            point.y = (float)strtod(pos+1,&stopStr);   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            point.z = (float)strtod(pos+1,&stopStr);   
            if((pos = strchr(pos+1,':')) != NULL)   
            {   uuu[pid+base] = (float)strtod(pos+1,&stopStr);   
                if((pos = strchr(pos+1,':')) != NULL)   
                    vvv[pid+base] = (float)strtod(pos+1,&stopStr);   
                else   
                    vvv[pid+base] = (float)0;   
            }   
            else   
            {   uuu[pid+base] = (float)0;   
                vvv[pid+base] = (float)0;   
            }   
            if(swc)   
            {   swc = FALSE;   
                base = p->topPos;   
            }   
            p->addPoint(&point);   
        }   
        else if((strncmp(buf,"Face",4) == 0) && buf[5] >= 48 && buf[5] <= 57)   
        {   fid = strtoul(buf+5, &stopStr, 10 );   
            if(*stopStr != ':')   
                return FALSE;   
            if((pos = strchr(buf,':')) == NULL)   
                return FALSE;   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            triangle.p1 = strtol(pos+1,&stopStr,10) + base;   
            triangle.u1 = uuu[triangle.p1];   
            triangle.v1 = vvv[triangle.p1];   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            triangle.p2 = strtol(pos+1,&stopStr,10) + base;   
            triangle.u2 = uuu[triangle.p2];   
            triangle.v2 = vvv[triangle.p2];   
            if((pos = strchr(pos+1,':')) == NULL)   
                return FALSE;   
            triangle.p3 = strtol(pos+1,&stopStr,10) + base;   
            triangle.u3 = uuu[triangle.p3];   
            triangle.v3 = vvv[triangle.p3];   
            f->addTriangle(&triangle);   
            swc = TRUE;   
        }   
        if(ifs.bad())   
            return FALSE;   
    }   
    return TRUE;   
}   
   
/*   
BOOL readDxfFile2(LPCSTR fileName,CPointList* p,CPolygonList* f)   
{   char buf[MAX_LINE_LENTH],*cc;   
    double x,y,z;   
    long p1,p2,p3;   
    long l;   
    UINT pbuf1[MAX_BORDER_NUM],pbuf2[10],flag;   
    POLYGON3D plg1,plg2;   
    UINT base = p->topPos,line = 0;   
    BOOL swc = FALSE,b1 = FALSE, b2 = FALSE;   
    POINT3D point;   
    plg1.num = 0;   
    plg1.points = pbuf1;   
    plg2.num = 0;   
    plg2.points = pbuf2;   
    ifstream ifs(fileName);   
    if(!ifs)   
        return FALSE;   
    while(!ifs.eof())   
    {   ifs.getline( buf,MAX_LINE_LENTH-1);   
        line++;   
        if(strcmp(buf,"VERTEX") == 0)   
        {   ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            l = strtol( buf, &cc, 10 );   
            if(l != 10)   
                return FALSE;   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            x = strtod( buf, &cc);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            y = strtod( buf, &cc);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            ifs.getline( buf,MAX_LINE_LENTH-1);   
            z = strtod( buf, &cc);   
            line += 8;   
            if(x == 0 && y == 0 && z == 0)   
            {   ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p1 = strtol( buf, &cc, 10);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p2 = strtol( buf, &cc, 10);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                ifs.getline( buf,MAX_LINE_LENTH-1);   
                p3 = strtol( buf, &cc, 10);   
                line+=8;   
                flag = 0;   
                if(p1 > 0)   
                    flag |= 0x01;   
                if(p2 > 0)   
                    flag |= 0x02;   
                if(p3 > 0)   
                    flag |= 0x04;   
                switch (flag)   
                {   case 0x01:   
                        b1 = TRUE;   
                        break;   
                    case 0x04:   
                           
                        b1 = FALSE;   
                        break;   
                    case 0x03:   
                        if(b1)   
                        {   errorLine = line;   
                            return FALSE;   
                        }   
                        b1 = FALSE;   
                        break;   
                    case 0x05:   
                        b1 = FALSE;   
                        break;   
                    case 0x06:   
                        b1 = FALSE;   
                        break;   
                    case 0x07:   
                        b1 = FALSE;   
                        break;   
                    default:   
                        return FALSE;   
                }   
                swc = TRUE;   
            }   
            else   
            {   if(swc)   
                {   swc = FALSE;   
                    base = p->topPos;   
                }   
                point.x = (float)x;   
                point.y = (float)y;   
                point.z = (float)z;   
                p->addPoint(&point);   
            }   
        }   
        if(ifs.bad())   
            return FALSE;   
    }   
    return TRUE;   
}*/   

