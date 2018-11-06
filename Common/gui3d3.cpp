

    

#include <fstream.h>   
#include "gui16.h"   
   
//****************************************************************************   
//CPointList implement!!!   
//****************************************************************************   
CPointList::CPointList(UINT s)   
{   topPos = 0;   
    if(s <= 300000)   
    {   size = s;   
        points = new POINT3D[size];   
        shouldConvert = new CHAR[size];   
    }   
    else   
    {   points = NULL;   
        shouldConvert = NULL;   
    }   
   
}   
   
CPointList::~CPointList()   
{   if(points != NULL)   
        delete[] points;   
    if(shouldConvert != NULL)   
        delete[] shouldConvert;   
}   
   
BOOL CPointList::operator!()   
{   if(points != NULL && shouldConvert != NULL)   
        return TRUE;   
    else   
        return FALSE;   
}   
   
BOOL CPointList::copy(CPointList* l)   
{   if(size != l->size)   
        return FALSE;   
    topPos = l->topPos;   
    memcpy( points, l->points, topPos*sizeof(POINT3D) );   
    memcpy( shouldConvert, l->shouldConvert, topPos*sizeof(CHAR) );   
    return TRUE;   
}   
   
int CPointList::addPoint(POINT3D* p)   
{   if(topPos < size)   
    {   points[topPos] = *p;   
        topPos++;   
        return topPos - 1;   
    }   
    else   
        return -1;   
}   
   
int CPointList::addPointAt(UINT pos,POINT3D* p)   
{   if(pos < size)   
    {   points[pos] = *p;   
        if(topPos <= pos)   
            topPos = pos + 1;   
        return pos;   
    }   
    else   
        return -1;   
}   
   
void CPointList::clearConvert()   
{   memset( shouldConvert, 0, sizeof(CHAR)*topPos );   
}   
   
void CPointList::convertPoints(CONVERT_MATRIX3D* m,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   POINT3D* p=points;   
        for(i=0;i<topPos;i++)   
            dotConvert(p++,m);   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
                dotConvert(points+i,m);   
    }   
}   
   
void CPointList::scalePoints(CONVERT_MATRIX3D* m,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   POINT3D* p=points;   
        for(i=0;i<topPos;i++)   
            dotScale(p++,m);   
    }   
    else   
    {   for(i=0;i<topPos;i++)   
            if(shouldConvert[i])   
                dotScale(points+i,m);   
    }   
}   
   
void CPointList::rotatePoints(CONVERT_MATRIX3D* m,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   POINT3D* p=points;   
        for(i=0;i<topPos;i++)   
            dotRotate(p++,m);   
    }   
    else   
    {   for(i=0;i<topPos;i++)   
            if(shouldConvert[i])   
                dotRotate(points+i,m);   
    }   
}   
   
void CPointList::movePoints(CONVERT_MATRIX3D* m,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   POINT3D* p=points;   
        for(i=0;i<topPos;i++)   
            dotMove(p++,m);   
    }   
    else   
    {   for(i=0;i<topPos;i++)   
            if(shouldConvert[i])   
                dotMove(points+i,m);   
    }   
}   
   
//******************************************************************************   
//CFaceList implement!!!   
//******************************************************************************   
CFaceList::CFaceList(UINT s)   
{   topPos = 0;   
    if(s <= 800000)   
    {   size = s;   
        faces = new TRIANGLE3D[size];   
        shouldConvert = new CHAR[size];   
    }   
    else   
    {   faces = NULL;   
        shouldConvert = NULL;   
    }   
}   
   
CFaceList::~CFaceList()   
{   if(faces != NULL)   
        delete[] faces;   
    if(shouldConvert != NULL)   
        delete[] shouldConvert;   
}   
   
BOOL CFaceList::operator!()   
{   if(faces != NULL && shouldConvert != NULL)   
        return TRUE;   
    else   
        return FALSE;   
}   
   
void CFaceList::clearConvert()   
{   memset( shouldConvert, 0, sizeof(CHAR)*topPos );   
}   
   
int CFaceList::addTriangle(TRIANGLE3D* p)   
{   if(topPos < size)   
    {   faces[topPos] = *p;   
        topPos++;   
        return topPos - 1;   
    }   
    else   
        return -1;   
}   
   
int CFaceList::addAsPolygon(POLYGON3D* p)   
{   UINT pos = topPos;   
    if(p->num < 3 || (pos > size - p->num))   
        return -1;   
    for(UINT i=0;i<=p->num - 3;i++)   
    {   faces[topPos].p1 = p->points[i];   
        faces[topPos].p2 = p->points[i+1];   
        faces[topPos].p3 = p->points[i+2];   
        topPos++;   
    }   
    return pos;   
}   
   
int CFaceList::addTriangleAt(UINT pos,TRIANGLE3D* p)   
{   if(pos < size)   
    {   faces[pos] = *p;   
        if(topPos <= pos)   
            topPos = pos + 1;   
        return pos;   
    }   
    else   
        return -1;   
}   
   
void CFaceList::updataParams(CPointList* ppl,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   TRIANGLE3D* p=faces;   
        for(i=0;i<topPos;i++)   
            fillTriangleParam(ppl,p++);   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
                fillTriangleParam(ppl,faces+i);   
    }   
}   
   
void CFaceList::judgeSide(CPointList* pl,POINT3D* pt)   
{   UINT i;   
    TRIANGLE3D* p=faces;   
    char* pc=pl->shouldConvert;   
    for(i=0;i<topPos;i++)   
    {   if((p->a*pt->x + p->b*pt->y + p->c*pt->z + p->d) >= 0)   
        {   shouldConvert[i] = -1;   
            pc[p->p1] = -1;   
            pc[p->p2] = -1;   
            pc[p->p3] = -1;   
        }   
        p++;   
    }   
}   
   
void CFaceList::calLightMirror(POINT3D* pot,CPointList* pt,UINT groundLight,UINT spotLight,CPLCFLAG f)   
{   VECTOR3D l,v,h,n;   
    float cos;   
    UINT i,s,lps;   
    float x1,y1,z1;   
//  float zero=0,spl=spotLight;   
    s = sizeof(TRIANGLE3D);   
    if(f == CONVERTALL)   
    {   TRIANGLE3D* p=faces;   
        for(i=0;i<topPos;i++)   
        {   x1 = pt->points[p->p1].x;   
            y1 = pt->points[p->p1].y;   
            z1 = pt->points[p->p1].z;   
            l.x = pot->x - x1;   
            l.y = pot->y - y1;   
            l.z = pot->z - z1;   
            vector3dToSTD(&l);   
            v.x = 20-x1;   
            v.y = 150-y1;   
            v.z = 750-z1;   
            vector3dToSTD(&v);   
            h.x = l.x + v.x;   
            h.y = l.y + v.y;   
            h.z = l.z + v.z;   
            vector3dToSTD(&h);   
            n.x = p->a;   
            n.y = p->b;   
            n.z = p->c;   
            vector3dToSTD(&n);   
    //      cos = (n.x*h.x + n.y*h.y + n.z*h.z)   
    //            /(float)sqrt(n.x*n.x + n.y*n.y + n.z*n.z)   
    //            /(float)sqrt(h.x*h.x + h.y*h.y + h.z*h.z);   
            cos = vectorDotMul(&n,&h);   
            for(int i=0;i<7;i++)   
                cos =cos*cos;   
            if(cos > 0)   
                p->light = groundLight + UINT(spotLight*cos);   
            else   
                p->light = groundLight;   
            p++;   
        }   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
            {   x1 = pt->points[faces[i].p1].x;   
                y1 = pt->points[faces[i].p1].y;   
                z1 = pt->points[faces[i].p1].z;   
                l.x = pot->x - x1;   
                l.y = pot->y - y1;   
                l.z = pot->z - z1;   
                vector3dToSTD(&l);   
                v.x = 20-x1;   
                v.y = 150-y1;   
                v.z = 750-z1;   
                vector3dToSTD(&v);   
                h.x = l.x + v.x;   
                h.y = l.y + v.y;   
                h.z = l.z + v.z;   
        //      vector3dToSTD(&h);   
                n.x = faces[i].a;   
                n.y = faces[i].b;   
                n.z = faces[i].c;   
        //      vector3dToSTD(&n);   
        //      cos = vectorDotMul(&n,&h);   
                cos = (n.x*h.x + n.y*h.y + n.z*h.z)   
                      /(float)sqrt((n.x*n.x + n.y*n.y + n.z*n.z)   
                      *(h.x*h.x + h.y*h.y + h.z*h.z));   
                cos = pow(cos,8);   
                if(cos > 0)   
                    faces[i].light = groundLight + UINT(spotLight*cos);   
                else   
                    faces[i].light = groundLight;   
            }   
    }   
}   
   
void CFaceList::calLightSpot(POINT3D* pot,CPointList* pt,UINT groundLight,UINT spotLight,CPLCFLAG f)   
{   VECTOR3D v;   
    UINT i,s,lps;   
    long cos1;   
    float cos,zero=0,spl=spotLight;   
    s = sizeof(TRIANGLE3D);   
    if(f == CONVERTALL)   
    {   TRIANGLE3D* p=faces;   
        for(i=0;i<topPos;i++)   
        {   v.x = pt->points[p->p1].x - pot->x;   
            v.y = pt->points[p->p1].y - pot->y;   
            v.z = pt->points[p->p1].z - pot->z;   
            vector3dToSTD(&v);   
            cos = -(v.x*p->a + v.y*p->b + v.z*p->c)/   
                (float)sqrt(p->a*p->a + p->b*p->b + p->c*p->c);   
            if(cos > 0)   
                p->light = groundLight + UINT(spotLight*cos);   
            else   
                p->light = groundLight;   
            p++;   
        }   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
            {   v.x = pt->points[faces[i].p1].x - pot->x;   
                v.y = pt->points[faces[i].p1].y - pot->y;   
                v.z = pt->points[faces[i].p1].z - pot->z;   
                vector3dToSTD(&v);   
                cos = -(v.x*faces[i].a + v.y*faces[i].b + v.z*faces[i].c)/   
                (float)sqrt(faces[i].a*faces[i].a + faces[i].b*faces[i].b + faces[i].c*faces[i].c);   
                if(cos > 0)   
                    faces[i].light = groundLight + UINT(spotLight*cos);   
                else   
                    faces[i].light = groundLight;   
            }   
    }   
/*  __asm   
    {   mov     eax,f   
        mov     edx,this   
        mov     ebx,[edx]this.faces   
        mov     ecx,[edx]this.topPos   
        cmp     eax,CONVERTALL   
        mov     lps,ecx   
        jne     short __else   
        test    ecx,ecx   
        mov     edx,v   
        jz      __exit   
__loopTop1:   
        fld     [edx]v.x   
        fmul    [ebx]TRIANGLE3D.a   
        fld     [edx]v.y   
        fmul    [ebx]TRIANGLE3D.b   
        fld     [edx]v.z   
        fmul    [ebx]TRIANGLE3D.c   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fld     [ebx]TRIANGLE3D.a   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.b   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.c   
        fmul    st(0),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        faddp   st(1),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fsqrt   
        fdivp   st(1),st(0)   
        fchs   
        fcom    zero   
        fnstsw  ax   
        test    ah,65   
        mov     eax,groundLight   
        jne     short __s01   
        fmul    spl   
        frndint   
        fistp   cos1   
        add     eax,cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
        jmp     short __loop1   
__s01:   
        fstp    cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
__loop1:   
        mov     eax,s   
        dec     ecx   
        lea     ebx,[ebx+eax]   
        jnz     short __loopTop1   
        jmp     short __exit   
__else:   
        test    ecx,ecx   
        mov     ecx,[edx]this.shouldConvert   
        mov     edx,v   
        jz      __exit   
__loopTop2:   
        mov     ax,WORD PTR [ecx]   
        inc     ecx   
        cmp     al,0   
        je      __loop2   
        fld     [edx]v.x   
        fmul    [ebx]TRIANGLE3D.a   
        fld     [edx]v.y   
        fmul    [ebx]TRIANGLE3D.b   
        fld     [edx]v.z   
        fmul    [ebx]TRIANGLE3D.c   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fld     [ebx]TRIANGLE3D.a   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.b   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.c   
        fmul    st(0),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        faddp   st(1),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fsqrt   
        fdivp   st(1),st(0)   
        fchs   
        fcom    zero   
        fnstsw  ax   
        test    ah,65   
        mov     eax,groundLight   
        jne     short __s02   
        fmul    spl   
        frndint   
        fistp   cos1   
        add     eax,cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
        jmp     short __loop2   
__s02:   
        fstp    st(0)   
        mov     [ebx]TRIANGLE3D.light,eax   
__loop2:   
        mov     eax,s   
        dec     lps   
        lea     ebx,[ebx+eax]   
        jnz     short __loopTop2   
__exit:   
    }*/   
}   
   
void CFaceList::calLight(VECTOR3D* v,UINT groundLight,UINT spotLight,CPLCFLAG f)   
{   UINT i,s,lps;   
    long cos1;   
    float cos,zero=0,spl=spotLight;   
    s = sizeof(TRIANGLE3D);   
    __asm   
    {   mov     eax,f   
        mov     edx,this   
        mov     ebx,[edx]this.faces   
        mov     ecx,[edx]this.topPos   
        cmp     eax,CONVERTALL   
        mov     lps,ecx   
        jne     short __else   
        test    ecx,ecx   
        mov     edx,v   
        jz      __exit   
__loopTop1:   
        fld     [edx]v.x   
        fmul    [ebx]TRIANGLE3D.a   
        fld     [edx]v.y   
        fmul    [ebx]TRIANGLE3D.b   
        fld     [edx]v.z   
        fmul    [ebx]TRIANGLE3D.c   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fld     [ebx]TRIANGLE3D.a   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.b   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.c   
        fmul    st(0),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        faddp   st(1),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fsqrt   
        fdivp   st(1),st(0)   
        fchs   
        fcom    zero   
        fnstsw  ax   
        test    ah,65   
        mov     eax,groundLight   
        jne     short __s01   
        fmul    spl   
        frndint   
        fistp   cos1   
        add     eax,cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
        jmp     short __loop1   
__s01:   
        fstp    cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
__loop1:   
        mov     eax,s   
        dec     ecx   
        lea     ebx,[ebx+eax]   
        jnz     short __loopTop1   
        jmp     short __exit   
__else:   
        test    ecx,ecx   
        mov     ecx,[edx]this.shouldConvert   
        mov     edx,v   
        jz      __exit   
__loopTop2:   
        mov     ax,WORD PTR [ecx]   
        inc     ecx   
        cmp     al,0   
        je      __loop2   
        fld     [edx]v.x   
        fmul    [ebx]TRIANGLE3D.a   
        fld     [edx]v.y   
        fmul    [ebx]TRIANGLE3D.b   
        fld     [edx]v.z   
        fmul    [ebx]TRIANGLE3D.c   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fld     [ebx]TRIANGLE3D.a   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.b   
        fmul    st(0),st(0)   
        fld     [ebx]TRIANGLE3D.c   
        fmul    st(0),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        faddp   st(1),st(0)   
        fxch    st(1)   
        faddp   st(2),st(0)   
        fsqrt   
        fdivp   st(1),st(0)   
        fchs   
        fcom    zero   
        fnstsw  ax   
        test    ah,65   
        mov     eax,groundLight   
        jne     short __s02   
        fmul    spl   
        frndint   
        fistp   cos1   
        add     eax,cos1   
        mov     [ebx]TRIANGLE3D.light,eax   
        jmp     short __loop2   
__s02:   
        fstp    st(0)   
        mov     [ebx]TRIANGLE3D.light,eax   
__loop2:   
        mov     eax,s   
        dec     lps   
        lea     ebx,[ebx+eax]   
        jnz     short __loopTop2   
__exit:   
    }   
/*  if(f == CONVERTALL)  
    {   TRIANGLE3D* p=faces;  
        for(i=0;i<topPos;i++)  
        {   cos = -(v->x*p->a + v->y*p->b + v->z*p->c)/  
                (float)sqrt(p->a*p->a + p->b*p->b + p->c*p->c);  
            if(cos > 0)  
                p->light = groundLight + UINT(spotLight*cos);  
            else  
                p->light = groundLight;  
            p++;  
        }  
    }  
    else  
    {   CHAR* cp=shouldConvert;  
        for(i=0;i<topPos;i++)  
            if(*cp++)  
            {   cos = -(v->x*faces[i].a + v->y*faces[i].b + v->z*faces[i].c)/  
                (float)sqrt(faces[i].a*faces[i].a + faces[i].b*faces[i].b + faces[i].c*faces[i].c);  
                if(cos > 0)  
                    faces[i].light = groundLight + UINT(spotLight*cos);  
                else  
                    faces[i].light = groundLight;  
            }  
    }*/   
}   
   
//******************************************************************************   
//CPolygonList implement!!!   
//******************************************************************************   
CPolygonList::CPolygonList(UINT s)   
{   topPos = 0;   
    if(s <= 800000)   
    {   size = s;   
        faces = new POLYGON3D[size];   
        shouldConvert = new CHAR[size];   
        for(UINT i=0;i<size;i++)   
            faces[i].points = NULL;   
    }   
    else   
    {   faces = NULL;   
        shouldConvert = NULL;   
    }   
}   
   
CPolygonList::~CPolygonList()   
{   if(faces != NULL)   
    {   for(UINT i=0;i<size;i++)   
            if(faces[i].points != NULL)   
                delete[] faces[i].points;   
        delete[] faces;   
    }   
    if(shouldConvert != NULL)   
        delete[] shouldConvert;   
   
}   
   
BOOL CPolygonList::operator!()   
{   if(faces != NULL && shouldConvert != NULL)   
        return TRUE;   
    else   
        return FALSE;   
}   
   
void CPolygonList::clearConvert()   
{   memset( shouldConvert, 0, sizeof(CHAR)*topPos );   
}   
   
int CPolygonList::addPolygon(POLYGON3D* p)   
{   if(topPos < size)   
    {   faces[topPos] = *p;   
        faces[topPos].points = new UINT[p->num];   
        for(UINT i=0;i<p->num;i++)   
            faces[topPos].points[i] = p->points[i];   
        topPos++;   
        return topPos - 1;   
    }   
    else   
        return -1;     
}   
   
int CPolygonList::addPolygonAt(UINT pos,POLYGON3D* p)   
{   if(pos < size)   
    {   faces[pos] = *p;   
        faces[topPos].points = new UINT[p->num];   
        for(UINT i=0;i<p->num;i++)   
            faces[topPos].points[i] = p->points[i];   
        if(topPos <= pos)   
            topPos = pos + 1;   
        return pos;   
    }   
    else   
        return -1;   
}   
   
void CPolygonList::updataParams(CPointList* ppl,CPLCFLAG f)   
{   UINT i;   
    if(f == CONVERTALL)   
    {   POLYGON3D* p=faces;   
        for(i=0;i<topPos;i++)   
            fillPolygonParam(ppl,p++);   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
                fillPolygonParam(ppl,faces+i);   
    }   
}   
   
void CPolygonList::judgeSide(CPointList* pl,POINT3D* pt)   
{   UINT i,l;   
    POLYGON3D* p=faces;   
    for(i=0;i<topPos;i++)   
    {   if((p->a*pt->x + p->b*pt->y + p->c*pt->z + p->d) >= 0)   
        {   shouldConvert[i] = -1;   
            for(l=0;l<p->num;l++)   
                pl->shouldConvert[p->points[i]] = -1;   
        }   
        p++;   
    }   
}   
   
void CPolygonList::calLight(VECTOR3D* v,UINT groundLight,UINT spotLight,CPLCFLAG f)   
{   UINT i;   
    float cos;   
    if(f == CONVERTALL)   
    {   POLYGON3D* p=faces;   
        for(i=0;i<topPos;i++)   
        {   cos = -(v->x*p->a + v->y*p->b + v->z*p->c)/   
                (float)sqrt(p->a*p->a + p->b*p->b + p->c*p->c);   
            if(cos > 0)   
                p->light = groundLight + UINT(spotLight*cos);   
            else   
                p->light = groundLight;   
            p++;   
        }   
    }   
    else   
    {   CHAR* cp=shouldConvert;   
        for(i=0;i<topPos;i++)   
            if(*cp++)   
            {   cos = -(v->x*faces[i].a + v->y*faces[i].b + v->z*faces[i].c)/   
                (float)sqrt(faces[i].a*faces[i].a + faces[i].b*faces[i].b + faces[i].c*faces[i].c);   
                if(cos > 0)   
                    faces[i].light = groundLight + UINT(spotLight*cos);   
                else   
                    faces[i].light = groundLight;   
            }   
    }   
}   
   
//******************************************************************************   
//CTextureList implement!!!   
//******************************************************************************   
CTextureList::CTextureList(UINT s)   
{   topPos = 0;   
    if(s <= 100000)   
    {   size = s;   
        textures = new CTexture*[size];   
        memset( textures, NULL, sizeof(CTexture*)*size );   
    }   
    else   
    {   textures = NULL;   
    }   
}   
   
CTextureList::~CTextureList()   
{   if(textures != NULL)   
    {   for(UINT i=0;i<size;i++)   
            if(textures[i] != NULL)   
                delete textures[i];   
        delete[] textures;   
    }   
}   
   
int CTextureList::addTexture(LPCSTR fileName)   
{   if(topPos < size)   
    {   if((textures[topPos] = new CTexture(fileName)) == NULL||textures[topPos]->bad())   
            return -1;   
        topPos++;   
        return topPos - 1;   
    }   
    else   
        return -1;     
}   
   
int CTextureList::addTextureAt(UINT pos,LPCSTR fileName)   
{   if(pos < size)   
    {   if((textures[pos] = new CTexture(fileName)) == NULL)   
            return -1;   
        if(topPos <= pos)   
            topPos = pos + 1;   
        return pos;   
    }   
    else   
        return -1;   
}   
   
int CTextureList::addLevelTexture(LPCSTR fileName)   
{   if(topPos < size)   
    {   if((textures[topPos] = new CTexture(fileName,TRUE)) == NULL||textures[topPos]->bad())   
            return -1;   
        topPos++;   
        return topPos - 1;   
    }   
    else   
        return -1;     
}   
   
int CTextureList::addLevelTextureAt(UINT pos,LPCSTR fileName)   
{   if(pos < size)   
    {   if((textures[pos] = new CTexture(fileName,TRUE)) == NULL)   
            return -1;   
        if(topPos <= pos)   
            topPos = pos + 1;   
        return pos;   
    }   
    else   
        return -1;   
}   
   
BOOL CTextureList::operator!()   
{   if(textures != NULL)   
        return TRUE;   
    else   
        return FALSE;   
}   
   
//******************************************************************************   
//CTexture implement!!!   
//******************************************************************************   
CTexture::CTexture(LPCSTR fileName)   
{   badf = FALSE;   
    level = FALSE;   
    BMPHEAD head;   
    unsigned long bufLen;   
    unsigned char* buf;   
    image = NULL;   
    ifstream ifs(fileName,ios::binary);   
    if(!ifs)   
    {   badf = TRUE;   
        return;   
    }   
    ifs.read((char*)&head,sizeof(BMPHEAD));   
    if(!ifs.good())   
    {   badf = TRUE;   
        return;   
    }   
    width = head.width;   
    height = head.height;   
    sectLenth = width*height;   
    if((image = new WORD[width*height]) == NULL)   
    {   badf = TRUE;   
        return;   
    }   
    bufLen = (width*3 + 3) & 0xfffffffc;   
    if((buf = new unsigned char[bufLen]) == NULL)   
    {   badf = TRUE;   
        return;   
    }   
    WORD* p = image;   
    for(UINT i=0;i<height;i++)   
    {   ifs.read(buf,bufLen);   
        if(ifs.bad())   
        {   badf = TRUE;   
            return;   
        }   
        for(UINT j=0;j<width;j++)   
        {   *p++ = (((WORD)buf[j*3]&0xfff8)>>3) |   
                   (((WORD)buf[j*3+1]&0xfffc)<<3)|   
                   (((WORD)buf[j*3+2]&0xfff8)<<8);   
        }   
    }   
    delete[] buf;   
}   
   
CTexture::CTexture(LPCSTR fileName,BOOL level)   
{   badf = FALSE;   
    level = TRUE;   
    BMPHEAD head;   
    unsigned long bufLen;   
    unsigned char* buf;   
    image = NULL;   
    ifstream ifs(fileName,ios::binary);   
    if(!ifs)   
    {   badf = TRUE;   
        return;   
    }   
    ifs.read((char*)&head,sizeof(BMPHEAD));   
    if(!ifs.good())   
    {   badf = TRUE;   
        return;   
    }   
    width = head.width;   
    height = head.height;   
    sectLenth = width*height;   
    if((image = new WORD[width*height*33]) == NULL)   
    {   badf = TRUE;   
        return;   
    }   
    bufLen = (width*3 + 3) & 0xfffffffc;   
    if((buf = new unsigned char[bufLen]) == NULL)   
    {   badf = TRUE;   
        return;   
    }   
    WORD* p = image,*p1;   
    WORD color,color1;   
    WORD f16,f8,f4,f2,f1,w16,w8,w4,w2,w1;   
    for(UINT i=0;i<height;i++)   
    {   ifs.read(buf,bufLen);   
        if(ifs.bad())   
        {   badf = TRUE;   
            return;   
        }   
        for(UINT j=0;j<width;j++)   
        {   color = (((WORD)buf[j*3]&0xfff8)>>3) |   
                   (((WORD)buf[j*3+1]&0xfffc)<<3)|   
                   (((WORD)buf[j*3+2]&0xfff8)<<8);   
            f16 = (color&0x8610)>>4;   
            f8 = (color&0xc718)>>3;   
            f4 = (color&0xe79c)>>2;   
            f2 = (color&0xf7de)>>1;      
            f1 = color;   
            color1 = 0xffff - color;   
            w16 = (color1&0x8610)>>4;   
            w8 = (color1&0xc718)>>3;   
            w4 = (color1&0xe79c)>>2;   
            w2 = (color1&0xf7de)>>1;     
            w1 = color1;   
            p1 = p;   
            *p1 = 0;   
            p1 += sectLenth,*p1 = f16;   
            p1 += sectLenth,*p1 = f8;   
            p1 += sectLenth,*p1 = f8+f16;   
            p1 += sectLenth,*p1 = f4;   
            p1 += sectLenth,*p1 = f4+f16;   
            p1 += sectLenth,*p1 = f4+f8;   
            p1 += sectLenth,*p1 = f4+f8+f16;   
            p1 += sectLenth,*p1 = f2;   
            p1 += sectLenth,*p1 = f2+f16;   
            p1 += sectLenth,*p1 = f2+f8;   
            p1 += sectLenth,*p1 = f2+f8+f16;   
            p1 += sectLenth,*p1 = f2+f4;   
            p1 += sectLenth,*p1 = f2+f4+f16;   
            p1 += sectLenth,*p1 = f2+f4+f8;   
            p1 += sectLenth,*p1 = f2+f4+f8+f16;   
            p1 += sectLenth,*p1 = f1;   
            p1 += sectLenth,*p1 = f1+w16;   
            p1 += sectLenth,*p1 = f1+w8;   
            p1 += sectLenth,*p1 = f1+w8+w16;   
            p1 += sectLenth,*p1 = f1+w4;   
            p1 += sectLenth,*p1 = f1+w4+w16;   
            p1 += sectLenth,*p1 = f1+w4+w8;   
            p1 += sectLenth,*p1 = f1+w4+w8+w16;   
            p1 += sectLenth,*p1 = f1+w2;   
            p1 += sectLenth,*p1 = f1+w2+w16;   
            p1 += sectLenth,*p1 = f1+w2+w8;   
            p1 += sectLenth,*p1 = f1+w2+w8+w16;   
            p1 += sectLenth,*p1 = f1+w2+w4;   
            p1 += sectLenth,*p1 = f1+w2+w4+w16;   
            p1 += sectLenth,*p1 = f1+w2+w4+w8;   
            p1 += sectLenth,*p1 = f1+w2+w4+w8+w16;   
            p1 += sectLenth,*p1 = f1+w1;   
            p++;   
        }   
    }   
    delete[] buf;   
}   
   
   
CTexture::~CTexture()   
{   if(image != NULL)   
    {   delete[] image;   
        image = NULL;   
    }   
}   
   
BOOL CTexture::bad()   
{   return badf;   
}  

