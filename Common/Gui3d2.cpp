

    

#include <stdlib.h>   
#include <fstream.h>   
#include <float.h>   
#include "gui16.h"   
   
#define INDEX_FORWORD(index)    index = (index + 1) % num;   
#define INDEX_BACKWORD(index)   index = (index - 1 + num) % num;   
inline void fillTriangle3d(DDSURFACEDESC* ddsd,CPointList* ppl,TRIANGLE3D* t,WORD color)   
{   long lx1,ly1,lx2,ly2,rx1,ry1,rx2,ry2,ldx,ldy,rdx,rdy;   
    long minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,rxadd,lxsgn,rxsgn,lxsum,rxsum;   
    long i,x11,x22,num = 3;   
    POINT3D_INT1 points[3];   
    POINT3D* p3d;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    long pitch = ddsd->lPitch>>1;   
    WORD* p=(WORD*)ddsd->lpSurface,*pt;   
    p3d = &ppl->points[t->p1];   
    points[0].x = (long)p3d->x, points[0].y = (long)p3d->y, points[0].z = (long)p3d->z;   
    p3d = &ppl->points[t->p2];   
    points[1].x = (long)p3d->x, points[1].y = (long)p3d->y, points[1].z = (long)p3d->z;   
    p3d = &ppl->points[t->p3];   
    points[2].x = (long)p3d->x, points[2].y = (long)p3d->y, points[2].z = (long)p3d->z;   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    ldx = lx2 - lx1;   
    ldy = ly2 - ly1;   
    lxsgn = (ldx >= 0)?1:-1;   
    if(ldy>0)   
    {   lxadd = ldx/ldy;   
        lxinc = (ldx%ldy)*lxsgn;   
    }   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rdx = rx2 - rx1;   
    rdy = ry2 - ry1;   
    rxsgn = (rdx >= 0)?1:-1;   
    if(rdy>0)   
    {   rxadd = rdx/rdy;   
        rxinc = (rdx%rdy)*rxsgn;   
    }   
    lxsum = 0;   
    rxsum = 0;   
    minY++;   
    p += pitch*minY;   
    while(1)   
    {   lxsum+=lxinc;   
        lx1+=lxadd;   
        if(lxsum >= ldy)   
        {   lx1+=lxsgn;   
            lxsum -= ldy;   
        }   
        rxsum+=rxinc;   
        rx1+=rxadd;   
        if(rxsum >= rdy)   
        {   rx1+=rxsgn;   
            rxsum -= rdy;   
        }   
        x11 = lx1,x22 = rx1 - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
                x11=0;   
            if(x22>width)   
                x22=width;   
            pt = p + x11;      
            *pt = color;   
            while(x11<x22)   
            {   pt++;   
                *pt = color;   
                x11++;   
            }   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
            }while(ly2 == ly1);   
            ldx = lx2 - lx1;   
            ldy = ly2 - ly1;   
            lxsgn = (ldx >= 0)?1:-1;   
            if(ldy>0)   
            {   lxadd = ldx/ldy;   
                lxinc = (ldx%ldy)*lxsgn;   
            }   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
            }while(ry2 == ry1);   
            rdx = rx2 - rx1;   
            rdy = ry2 - ry1;   
            rxsgn = (rdx >= 0)?1:-1;   
            if(rdy>0);   
            {   rxadd = rdx/rdy;   
                rxinc = (rdx%rdy)*rxsgn;   
            }   
        }   
        minY++;   
        p += pitch;   
    }   
}   
/*  
typedef struct { long x1,z1,x2,z2,y;}HLINE3D;  
#define FLOAT_TO_FIXED(f)   ((long)(f*0x00010000))  
#define FIXED_TO_SHORT(f)   ((short)(f>>16))  
#define ROUND_FIXED_TO_SHORT(f) ((short)((f+FLOAT_TO_FIXED(0.5))>>16))  
inline void fillTriangle3dWithZ(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color)  
{   long lx1,ly1,lz1,lx2,ly2,lz2,rx1,ry1,rz1,rx2,ry2,rz2,ldx,ldy,ldz,rdx,rdy,rdz;  
    long minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,lzadd,rxadd,rzadd;  
    long lxsgn,rxsgn,lxsum,rxsum;  
    long i,x11,x22,num = 3,lineNum = 0;  
    POINT3D_INT1 points[3];  
    POINT3D* p3d;  
    HLINE3D* lines;  
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;  
    long pitch1 = ddsd->lPitch>>1;  
    long pitch2 = ddsdZBuf->lPitch>>1;  
    WORD* p1=(WORD*)ddsd->lpSurface;  
    short* p2=(short*)ddsdZBuf->lpSurface;  
    p3d = &ppl->points[t->p1];  
    points[0].x = (long)p3d->x, points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);  
    p3d = &ppl->points[t->p2];  
    points[1].x = (long)p3d->x, points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);  
    p3d = &ppl->points[t->p3];  
    points[2].x = (long)p3d->x, points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);  
    rindex = lindex = 0;  
    maxY=minY=points[0].y;  
    for(i=0;i<num;i++)  
    {   if(points[i].y<minY)  
            minY = points[lindex = i].y;  
        else if(points[i].y > maxY)  
                maxY = points[i].y;  
    }  
    if(minY == maxY)  
        return;  
    if((lines = new HLINE3D[maxY - minY]) == NULL)  
        return;  
    rindex = lindex;  
    while(points[lindex].y == minY)  
        //INDEX_BACKWORD(lindex);  
        INDEX_FORWORD(lindex);  
    INDEX_BACKWORD(lindex);  
    //INDEX_FORWORD(lindex);  
    while(points[rindex].y == minY)  
        //INDEX_FORWORD(rindex);  
        INDEX_BACKWORD(rindex);  
    //INDEX_BACKWORD(rindex);  
    INDEX_FORWORD(rindex);  
    lx1 = points[lindex].x;  
    ly1 = points[lindex].y;  
    lz1 = points[lindex].z;  
    INDEX_FORWORD(lindex);  
    //INDEX_BACKWORD(lindex);  
    lx2 = points[lindex].x;  
    ly2 = points[lindex].y;  
    lz2 = points[lindex].z;  
    ldx = lx2 - lx1;  
    ldy = ly2 - ly1;  
    ldz = lz2 - lz1;  
    lxsgn = (ldx >= 0)?1:-1;  
    lxadd = ldx/ldy;  
    lxinc = (ldx%ldy)*lxsgn;  
    lzadd = ldz/ldy;  
    rx1 = points[rindex].x;  
    ry1 = points[rindex].y;  
    rz1 = points[rindex].z;  
    INDEX_BACKWORD(rindex);  
    //INDEX_FORWORD(rindex);  
    rx2 = points[rindex].x;  
    ry2 = points[rindex].y;  
    rz2 = points[rindex].z;  
    rdx = rx2 - rx1;  
    rdy = ry2 - ry1;  
    rdz = rz2 - rz1;  
    rxsgn = (rdx >= 0)?1:-1;  
    rxadd = rdx/rdy;  
    rxinc = (rdx%rdy)*rxsgn;  
    rzadd = rdz/rdy;  
    lxsum = 0;  
    rxsum = 0;  
    minY++;  
    while(1)  
    {   lxsum+=lxinc;  
        lx1+=lxadd;  
        lz1+=lzadd;  
        if(lxsum >= ldy)  
        {   lx1+=lxsgn;  
            lxsum -= ldy;  
        }  
        rxsum+=rxinc;  
        rx1+=rxadd;  
        rz1+=rzadd;  
        if(rxsum >= rdy)  
        {   rx1+=rxsgn;  
            rxsum -= rdy;  
        }  
        x11 = lx1,x22 = rx1 - 1;  
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)  
        {   if(x11<0)  
                x11=0;  
            if(x22>width)  
                x22=width;  
            lines[lineNum].x1 = x11;  
            lines[lineNum].z1 = lz1;  
            lines[lineNum].x2 = x22;  
            lines[lineNum].z2 = rz1;  
            lines[lineNum++].y = minY;  
        }  
        if(minY >= maxY)  
            break;  
        if(minY == ly2)  
        {   do{   
                lx1 = points[lindex].x;  
                ly1 = points[lindex].y;  
                lz1 = points[lindex].z;  
                INDEX_FORWORD(lindex);  
                //INDEX_BACKWORD(lindex);  
                lx2 = points[lindex].x;  
                ly2 = points[lindex].y;  
                lz2 = points[lindex].z;  
            }while(ly2 == ly1);  
            ldx = lx2 - lx1;  
            ldy = ly2 - ly1;  
            ldz = lz2 - lz1;  
            lxsgn = (ldx >= 0)?1:-1;  
            lxadd = ldx/ldy;  
            lxinc = (ldx%ldy)*lxsgn;  
            lzadd = ldz/ldy;  
            lxsum = 0;  
        }  
        if(minY == ry2)  
        {   do{  
                rx1 = points[rindex].x;  
                ry1 = points[rindex].y;  
                rz1 = points[rindex].z;  
                INDEX_BACKWORD(rindex);  
                //INDEX_FORWORD(rindex);  
                rx2 = points[rindex].x;  
                ry2 = points[rindex].y;  
                rz2 = points[rindex].z;  
            }while(ry2 == ry1);  
            rdx = rx2 - rx1;  
            rdy = ry2 - ry1;  
            rdz = rz2 - rz1;  
            rxsgn = (rdx >= 0)?1:-1;  
            rxadd = rdx/rdy;  
            rxinc = (rdx%rdy)*rxsgn;  
            rzadd = rdz/rdy;  
            rxsum = 0;  
        }  
        minY++;  
    }  
    WORD* p11;  
    short* p22;  
    int dx,dz,x,z,y,x2,z2,zadd;  
    for(i = 0;i < lineNum;i++)  
    {   x = lines[i].x1;  
        z = lines[i].z1;  
        x2 = lines[i].x2;  
        z2 = lines[i].z2;  
        y = lines[i].y;  
        dx = x2 - x;  
        dz = z2 - z;  
        p22 = p2 + pitch2*y + x;  
        p11 = p1 + pitch1*y + x;  
        if(FIXED_TO_SHORT(z) < *p22)  
        {   *p11 = color;  
            *p22 = FIXED_TO_SHORT(z);  
        }  
        if(dx <= 0)  
        {   continue;  
        }  
        zadd = dz/dx;  
        while(x < x2)  
        {   x++;  
            p11++;  
            p22++;  
            z += zadd;  
            if(FIXED_TO_SHORT(z) < *p22)  
            {   *p11 = color;  
                *p22 = FIXED_TO_SHORT(z);  
            }  
        }  
    }  
    delete[] lines;  
}*/   
   
#define ROUND_FIXED_TO_SHORT(f) ((short)((f+FLOAT_TO_FIXED(0.5))>>16))   
inline void fillTriangle3dWithZ(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color)   
{   long lx1,ly1,lz1,ly2,rx1,ry1,rz1,ry2,ldx,ldy,ldz,rdx,rdy,rdz;   
    long minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,lzadd,rxadd,rzadd;   
    long lxsgn,rxsgn,lxsum,rxsum;   
    long i,x11,x22,num = 3,lineNum = 0;   
    POINT3D_INT1 points[3];   
    POINT3D* p3d;   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    long pitch2 = ddsdZBuf->lPitch>>1;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    short* p2=(short*)ddsdZBuf->lpSurface;   
    WORD* p11;   
    short* p22;   
    long z,zadd,lps;   
    p3d = &ppl->points[t->p1];   
    points[0].x = (long)p3d->x, points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);   
    p3d = &ppl->points[t->p2];   
    points[1].x = (long)p3d->x, points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);   
    p3d = &ppl->points[t->p3];   
    points[2].x = (long)p3d->x, points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = points[lindex].z;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    ly2 = points[lindex].y;   
    ldx = points[lindex].x - lx1;   
    ldy = ly2 - ly1;   
    ldz = points[lindex].z - lz1;   
    lxsgn = (ldx >= 0)?1:-1;   
    lxadd = ldx/ldy;   
    lxinc = (ldx%ldy)*lxsgn;   
    lzadd = ldz/ldy;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = points[rindex].z;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    ry2 = points[rindex].y;   
    rdx = points[rindex].x - rx1;   
    rdy = ry2 - ry1;   
    rdz = points[rindex].z - rz1;   
    rxsgn = (rdx >= 0)?1:-1;   
    rxadd = rdx/rdy;   
    rxinc = (rdx%rdy)*rxsgn;   
    rzadd = rdz/rdy;   
    lxsum = 0;   
    rxsum = 0;   
    minY++;   
    p2 += pitch2*minY;   
    p1 += pitch1*minY;   
    while(1)   
    {/* __asm  
        {   push    edi  
            push    esi  
            mov     eax,lxsum  
            mov     esi,lxinc  
            mov     ebx,lx1  
            mov     edi,lxadd  
            mov     ecx,lzadd  
            mov     edx,ldy  
            add     eax,esi  
            mov     esi,lxsgn  
            add     ebx,edi  
            add     lz1,ecx  
            cmp     eax,edx  
            jl      SHORT __rr  
            add     ebx,esi  
            sub     eax,edx  
__rr:  
            mov     lx1,ebx  
            mov     lxsum,eax  
            mov     eax,rxsum  
            mov     esi,rxinc  
            mov     ebx,rx1  
            mov     edi,rxadd  
            mov     ecx,rzadd  
            mov     edx,rdy  
            add     eax,esi  
            mov     esi,rxsgn  
            add     ebx,edi  
            add     rz1,ecx  
            cmp     eax,edx  
            jl      SHORT __clp  
            add     ebx,esi  
            sub     eax,edx  
__clp:        
            mov     esi,width  
            mov     rx1,ebx  
            mov     rxsum,eax  
            mov     ecx,minY  
            dec     ebx  
            mov     eax,lx1  
            cmp     ecx,0  
            jl      SHORT __goon  
            cmp     ecx,height  
            jg      SHORT __goon  
            cmp     eax,esi  
            jg      SHORT __goon  
            cmp     ebx,0  
            jl      SHORT __goon  
            cmp     ebx,eax  
            jl      SHORT __goon  
            cmp     eax,0  
            jge     SHORT __clp2  
            mov     eax,0  
__clp2:  
            cmp     ebx,esi  
            jle     SHORT __fill  
            mov     ebx,esi  
__fill:       
            mov     si,color  
            mov     ecx,lz1  
            mov     x11,eax  
            mov     x22,ebx  
            mov     ebx,p2  
            mov     z,ecx  
            mov     edx,p1  
            sar     ecx,16  
            lea     ebx,[ebx+eax*2]  
            lea     edx,[edx+eax*2]  
            mov     edi,eax  
            cmp     cx,WORD PTR[ebx]  
            jge     SHORT __fill2  
            mov     WORD PTR[edx],si  
            mov     WORD PTR[ebx],cx  
__fill2:  
            mov     ecx,x22  
            cmp     ecx,edi  
            jle     SHORT __goon  
            mov     eax,rz1  
            sub     eax,z  
            mov     esi,edx  
            sub     ecx,edi  
            cdq  
            idiv    ecx  
            mov     edx,esi  
            mov     lps,ecx  
            mov     edi,eax  
            mov     eax,z  
            add     eax,edi           
__looptop:  
            add     ebx,2  
            add     edx,2  
            mov     ecx,eax  
            sar     ecx,16  
            mov     si,WORD PTR [ebx]  
            add     eax,edi  
            cmp     cx,si  
            mov     si,color  
            jge     SHORT __loop  
            mov     WORD PTR[ebx],cx  
            mov     WORD PTR[edx],si  
__loop:  
            dec     lps  
            jnz     SHORT __looptop  
__goon:  
            pop     esi  
            pop     edi  
        }*/   
        lxsum+=lxinc;   
        lx1+=lxadd;   
        lz1+=lzadd;   
        if(lxsum >= ldy)   
        {   lx1+=lxsgn;   
            lxsum -= ldy;   
        }   
        rxsum+=rxinc;   
        rx1+=rxadd;   
        rz1+=rzadd;   
        if(rxsum >= rdy)   
        {   rx1+=rxsgn;   
            rxsum -= rdy;   
        }   
        x11 = lx1,x22 = rx1 - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
                x11=0;   
            if(x22>width)   
                x22=width;   
            z = lz1;   
            p22 = p2 + x11;   
            p11 = p1 + x11;   
            if(FIXED_TO_SHORT(z) < *p22)   
            {   *p11 = color;   
                *p22 = FIXED_TO_SHORT(z);   
            }   
            if(x22 > x11)   
            {   zadd = (rz1 - z)/(x22 - x11);   
                while(x11++ < x22)   
                {   z += zadd;   
                    p11++;   
                    p22++;   
                    if(FIXED_TO_SHORT(z) < *p22)   
                    {   *p11 = color;   
                        *p22 = FIXED_TO_SHORT(z);   
                    }   
                }   
            }   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = points[lindex].z;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                ly2 = points[lindex].y;   
            }while(ly2 == ly1);   
            ldx = points[lindex].x - lx1;   
            ldy = ly2 - ly1;   
            ldz = points[lindex].z - lz1;   
            lxsgn = (ldx >= 0)?1:-1;   
            lxadd = ldx/ldy;   
            lxinc = (ldx%ldy)*lxsgn;   
            lzadd = ldz/ldy;   
            lxsum = 0;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = points[rindex].z;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                ry2 = points[rindex].y;   
            }while(ry2 == ry1);   
            rdx = points[rindex].x - rx1;   
            rdy = ry2 - ry1;   
            rdz = points[rindex].z - rz1;   
            rxsgn = (rdx >= 0)?1:-1;   
            rxadd = rdx/rdy;   
            rxinc = (rdx%rdy)*rxsgn;   
            rzadd = rdz/rdy;   
            rxsum = 0;   
        }   
        minY++;   
        p2 += pitch2;   
        p1 += pitch1;   
    }   
}   
   
typedef struct { long x1,x2,y,z1,u1,v1,z2,u2,v2;}HLINE3D;   
typedef struct { long x1,x2,y;   
                 float z1,u1,v1,z2,u2,v2;}HLINE3D1;   
   
inline void fillTriangle3dWithZT(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color)   
{   long lx1,ly1,lz1,lx2,ly2,lz2,rx1,ry1,rz1,rx2,ry2,rz2,ldx,ldy,ldz,rdx,rdy,rdz;   
    long minY,maxY,lindex,rindex,lxadd,lzadd,rxadd,rzadd;   
    long luadd,ruadd,lvadd,rvadd;   
    long lu1,lv1,lu2,lv2,ru1,rv1,ru2,rv2,ldu,ldv,rdu,rdv;   
    long i,x11,x22,num = 3,lineNum = 0;   
    long mapWidth,mapHeight;   
    POINT3D_INT2 points[3];   
    POINT3D* p3d;   
    HLINE3D* lines;   
    if(t->maped == FALSE)   
    {   fillTriangle3dWithZ(ddsd,ddsdZBuf,ppl,t,color);   
        return;   
    }   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    long pitch2 = ddsdZBuf->lPitch>>1;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    short* p2=(short*)ddsdZBuf->lpSurface;   
    mapWidth = tex.textures[t->map]->width-1;   
    mapHeight = tex.textures[t->map]->height-1;   
    p3d = &ppl->points[t->p1];   
    points[0].x = FLOAT_TO_FIXED(p3d->x), points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);   
    points[0].u = FLOAT_TO_FIXED(t->u1*mapWidth), points[0].v = FLOAT_TO_FIXED(t->v1*mapHeight);   
    p3d = &ppl->points[t->p2];   
    points[1].x = FLOAT_TO_FIXED(p3d->x), points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);   
    points[1].u = FLOAT_TO_FIXED(t->u2*mapWidth), points[1].v = FLOAT_TO_FIXED(t->v2*mapHeight);   
    p3d = &ppl->points[t->p3];   
    points[2].x = FLOAT_TO_FIXED(p3d->x), points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);   
    points[2].u = FLOAT_TO_FIXED(t->u3*mapWidth), points[2].v = FLOAT_TO_FIXED(t->v3*mapHeight);   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    if((lines = new HLINE3D[maxY - minY]) == NULL)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = points[lindex].z;   
    lu1 = points[lindex].u;   
    lv1 = points[lindex].v;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    lz2 = points[lindex].z;   
    lu2 = points[lindex].u;   
    lv2 = points[lindex].v;   
    ldx = lx2 - lx1;   
    ldy = ly2 - ly1;   
    ldz = lz2 - lz1;   
    ldu = lu2 - lu1;   
    ldv = lv2 - lv1;   
    lxadd = ldx/ldy;   
    lzadd = ldz/ldy;   
    luadd = ldu/ldy;   
    lvadd = ldv/ldy;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = points[rindex].z;   
    ru1 = points[rindex].u;   
    rv1 = points[rindex].v;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rz2 = points[rindex].z;   
    ru2 = points[rindex].u;   
    rv2 = points[rindex].v;   
    rdx = rx2 - rx1;   
    rdy = ry2 - ry1;   
    rdz = rz2 - rz1;   
    rdu = ru2 - ru1;   
    rdv = rv2 - rv1;   
    rxadd = rdx/rdy;   
    rzadd = rdz/rdy;   
    ruadd = rdu/rdy;   
    rvadd = rdv/rdy;   
    minY++;   
    while(1)   
    {   lx1+=lxadd;   
        lz1+=lzadd;   
        lu1+=luadd;   
        lv1+=lvadd;   
        rx1+=rxadd;   
        rz1+=rzadd;   
        ru1+=ruadd;   
        rv1+=rvadd;   
        x11 = FIXED_TO_LONG(lx1),x22 = FIXED_TO_LONG(rx1) - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
                x11=0;   
            if(x22>width)   
                x22=width;   
            lines[lineNum].x1 = x11;   
            lines[lineNum].z1 = lz1;   
            lines[lineNum].u1 = lu1;   
            lines[lineNum].v1 = lv1;   
            lines[lineNum].x2 = x22;   
            lines[lineNum].z2 = rz1;   
            lines[lineNum].u2 = ru1;   
            lines[lineNum].v2 = rv1;   
            lines[lineNum++].y = minY;   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = points[lindex].z;   
                lu1 = points[lindex].u;   
                lv1 = points[lindex].v;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
                lz2 = points[lindex].z;   
                lu2 = points[lindex].u;   
                lv2 = points[lindex].v;   
            }while(ly2 == ly1);   
            ldx = lx2 - lx1;   
            ldy = ly2 - ly1;   
            ldz = lz2 - lz1;   
            ldu = lu2 - lu1;   
            ldv = lv2 - lv1;   
            lxadd = ldx/ldy;   
            lzadd = ldz/ldy;   
            luadd = ldu/ldy;   
            lvadd = ldv/ldy;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = points[rindex].z;   
                ru1 = points[rindex].u;   
                rv1 = points[rindex].v;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
                rz2 = points[rindex].z;   
                ru2 = points[rindex].u;   
                rv2 = points[rindex].v;   
            }while(ry2 == ry1);   
            rdx = rx2 - rx1;   
            rdy = ry2 - ry1;   
            rdz = rz2 - rz1;   
            rdu = ru2 - ru1;   
            rdv = rv2 - rv1;   
            rxadd = rdx/rdy;   
            rzadd = rdz/rdy;   
            ruadd = rdu/rdy;   
            rvadd = rdv/rdy;   
        }   
        minY++;   
    }   
    WORD* p11,*pm;   
    short* p22;   
    pm = tex.textures[t->map]->image;   
    UINT wh = tex.textures[t->map]->width;   
    int dx,dz,du,dv,x,z,u,v,y,x2,z2,u2,v2,zadd,uadd,vadd;   
    for(i = 0;i < lineNum;i++)   
    {   x = lines[i].x1;   
        z = lines[i].z1;   
        u = lines[i].u1;   
        v = lines[i].v1;   
        x2 = lines[i].x2;   
        z2 = lines[i].z2;   
        u2 = lines[i].u2;   
        v2 = lines[i].v2;   
        y = lines[i].y;   
        dx = x2 - x;   
        dz = z2 - z;   
        du = u2 - u;   
        dv = v2 - v;   
        p22 = p2 + pitch2*y + x;   
        p11 = p1 + pitch1*y + x;   
        if(FIXED_TO_SHORT(z) < *p22)   
        {   *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
            *p22 = FIXED_TO_SHORT(z);   
        }   
        if(dx <= 0)   
        {   continue;   
        }   
        zadd = dz/dx;   
        uadd = du/dx;   
        vadd = dv/dx;   
        while(x < x2)   
        {   x++;   
            p11++;   
            p22++;   
            z += zadd;   
            u += uadd;   
            v += vadd;   
            if(FIXED_TO_SHORT(z) < *p22)   
            {   *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
                *p22 = FIXED_TO_SHORT(z);   
            }   
        }   
    }    
    delete[] lines;   
}   
   
inline void fillTriangle3dWithZT1(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color)   
{   long lx1,ly1,lx2,ly2,rx1,ry1,rx2,ry2,ldx,ldy,rdx,rdy;   
    float lz1,lz2,rz1,rz2,ldz,rdz,lzadd,rzadd,luadd,lvadd,ruadd,rvadd;   
    long minY,maxY,lindex,rindex,lxadd,rxadd;   
    float lu1,lv1,lu2,lv2,ru1,rv1,ru2,rv2,ldu,ldv,rdu,rdv;   
    long i,x11,x22,num = 3,lineNum = 0;   
    POINT3D_INT2 points[3];   
    POINT3D* p3d;   
    HLINE3D1* lines;   
    if(t->maped == FALSE)   
    {   fillTriangle3dWithZ(ddsd,ddsdZBuf,ppl,t,color);   
        return;   
    }   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    long pitch2 = ddsdZBuf->lPitch>>1;   
    long mapWidth,mapHeight;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    short* p2=(short*)ddsdZBuf->lpSurface;   
    mapWidth = tex.textures[t->map]->width-1;   
    mapHeight = tex.textures[t->map]->height-1;   
    p3d = &ppl->points[t->p1];   
    points[0].x = FLOAT_TO_FIXED(p3d->x), points[0].y = (long)p3d->y, points[0].z = p3d->z;   
    points[0].u = t->u1*mapWidth, points[0].v = t->v1*mapHeight;   
    p3d = &ppl->points[t->p2];   
    points[1].x = FLOAT_TO_FIXED(p3d->x), points[1].y = (long)p3d->y, points[1].z = p3d->z;   
    points[1].u = t->u2*mapWidth, points[1].v = t->v2*mapHeight;   
    p3d = &ppl->points[t->p3];   
    points[2].x = FLOAT_TO_FIXED(p3d->x), points[2].y = (long)p3d->y, points[2].z = p3d->z;   
    points[2].u = t->u3*mapWidth, points[2].v = t->v3*mapHeight;   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    if((lines = new HLINE3D1[maxY - minY]) == NULL)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = 1.0/points[lindex].z;   
    lu1 = points[lindex].u*lz1;   
    lv1 = points[lindex].v*lz1;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    lz2 = 1.0/points[lindex].z;   
    lu2 = points[lindex].u*lz2;   
    lv2 = points[lindex].v*lz2;   
    ldx = lx2 - lx1;   
    ldy = ly2 - ly1;   
    ldz = lz2 - lz1;   
    ldu = lu2 - lu1;   
    ldv = lv2 - lv1;   
    lxadd = ldx/ldy;   
    lzadd = ldz/ldy;   
    luadd = ldu/ldy;   
    lvadd = ldv/ldy;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = 1.0/points[rindex].z;   
    ru1 = points[rindex].u*rz1;   
    rv1 = points[rindex].v*rz1;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rz2 = 1.0/points[rindex].z;   
    ru2 = points[rindex].u*rz2;   
    rv2 = points[rindex].v*rz2;   
    rdx = rx2 - rx1;   
    rdy = ry2 - ry1;   
    rdz = rz2 - rz1;   
    rdu = ru2 - ru1;   
    rdv = rv2 - rv1;   
    rxadd = rdx/rdy;   
    rzadd = rdz/rdy;   
    ruadd = rdu/rdy;   
    rvadd = rdv/rdy;   
    minY++;   
    while(1)   
    {   lx1+=lxadd;   
        lz1+=lzadd;   
        lu1+=luadd;   
        lv1+=lvadd;   
        rx1+=rxadd;   
        rz1+=rzadd;   
        ru1+=ruadd;   
        rv1+=rvadd;   
        x11 = FIXED_TO_LONG(lx1),x22 = FIXED_TO_LONG(rx1) - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   lines[lineNum].x1 = x11;   
            lines[lineNum].z1 = lz1;   
            lines[lineNum].u1 = lu1;   
            lines[lineNum].v1 = lv1;   
            lines[lineNum].x2 = x22;   
            lines[lineNum].z2 = rz1;   
            lines[lineNum].u2 = ru1;   
            lines[lineNum].v2 = rv1;   
            lines[lineNum++].y = minY;   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = 1.0/points[lindex].z;   
                lu1 = points[lindex].u*lz1;   
                lv1 = points[lindex].v*lz1;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
                lz2 = 1.0/points[lindex].z;   
                lu2 = points[lindex].u*lz2;   
                lv2 = points[lindex].v*lz2;   
            }while(ly2 == ly1);   
            ldx = lx2 - lx1;   
            ldy = ly2 - ly1;   
            ldz = lz2 - lz1;   
            ldu = lu2 - lu1;   
            ldv = lv2 - lv1;   
            lxadd = ldx/ldy;   
            lzadd = ldz/ldy;   
            luadd = ldu/ldy;   
            lvadd = ldv/ldy;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = 1.0/points[rindex].z;   
                ru1 = points[rindex].u*rz1;   
                rv1 = points[rindex].v*rz1;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
                rz2 = 1.0/points[rindex].z;   
                ru2 = points[rindex].u*rz2;   
                rv2 = points[rindex].v*rz2;   
            }while(ry2 == ry1);   
            rdx = rx2 - rx1;   
            rdy = ry2 - ry1;   
            rdz = rz2 - rz1;   
            rdu = ru2 - ru1;   
            rdv = rv2 - rv1;   
            rxadd = rdx/rdy;   
            rzadd = rdz/rdy;   
            ruadd = rdu/rdy;   
            rvadd = rdv/rdy;   
        }   
        minY++;   
    }   
    WORD* p11,*pm;   
    short* p22;   
    pm = tex.textures[t->map]->image;   
    UINT wh = tex.textures[t->map]->width;   
    long dx,x,y,x2,lps;   
//  long zz,uu,vv;   
//  float z,u,v;   
    float zadd,uadd,vadd;   
    float uf,ub,vf,vb,zf,zb;   
    long uf1,ub1,vf1,vb1,zf1,zb1,zadd1,uadd1,vadd1;   
    float zzz;   
    for(i = 0;i < lineNum;i++)   
    {   x = lines[i].x1;   
        zf = lines[i].z1;   
        zzz = ((float)0x00010000)/zf;   
        zf1 = long(zzz);   
        uf = lines[i].u1;   
        uf1 = long(uf*zzz);   
        vf = lines[i].v1;   
        vf1 = long(vf*zzz);   
        x2 = lines[i].x2;   
        y = lines[i].y;   
        if(x2<0 || x>width || y<0 || y>height)   
            continue;   
        dx = x2 - x;   
        zadd = (lines[i].z2 - zf)/dx;   
        uadd = (lines[i].u2 - uf)/dx;   
        vadd = (lines[i].v2 - vf)/dx;   
        long tp;   
        if(x<0)   
        {   tp = 0 - x;   
            zf += zadd*tp;   
            zzz = ((float)0x00010000)/zf;   
            zf1 = long(zzz);   
            uf += uadd*tp;   
            uf1 = long(uf*zzz);   
            vf += vadd*tp;   
            vf1 = long(vf*zzz);   
            x = 0;   
        }   
        if(x2 > width)   
        {   x2 = width;   
        }   
        dx = x2 - x;   
        p22 = p2 + pitch2*y + x;   
        p11 = p1 + pitch1*y + x;   
        if(FIXED_TO_SHORT(zf1) < *p22)   
        {   *p11 = *(pm+FIXED_TO_SHORT(vf1)*wh + FIXED_TO_SHORT(uf1));   
            *p22 = FIXED_TO_SHORT(zf1);   
        }   
        if(dx <= 0)   
        {   continue;   
        }   
        long step;   
        float one = (float)0x00010000;   
/*      while(x < x2)  
        {   if((x+16)<x2)  
                step = 16;  
            else  
                step = x2 - x;  
            zb = zf + zadd * step;  
            ub = uf + uadd * step;  
            vb = vf + vadd * step;  
            zf = zb,uf = ub, vf = vb;  
            zb1 = LONG_TO_FIXED(long(((float)1.0)/zb));  
            ub1 = LONG_TO_FIXED(long(ub/zb));  
            vb1 = LONG_TO_FIXED(long(vb/zb));  
            zadd1 = (zb1 - zf1)/step;  
            uadd1 = (ub1 - uf1)/step;  
            vadd1 = (vb1 - vf1)/step;  
            for(long j=0;j<step;j++)  
            {   p11++,p22++;  
                zf1 += zadd1;  
                uf1 += uadd1;  
                vf1 += vadd1;  
                if(FIXED_TO_SHORT(zf1) < *p22)  
                {   *p11 = *(pm+FIXED_TO_SHORT(vf1)*wh + FIXED_TO_SHORT(uf1));  
                    *p22 = FIXED_TO_SHORT(zf1);  
                }  
            }  
            x += 16;  
        }*/   
            __asm   
            {   push    esi   
                push    edi   
                mov     eax,x   
                mov     ebx,x2   
out_loop:   
                fld     zadd   
                sub     ebx,eax   
                mov     step,16   
                fld     uadd   
                cmp     ebx,16   
                jg      stepOK   
                mov     step,ebx   
stepOK:   
                fld     vadd   
                fild    step   
                fmul    st(3),st(0)   
                fmul    st(2),st(0)   
                fmulp   st(1),st(0)   
                fld     vf   
                fld     uf   
                fld     zf   
                faddp   st(5),st(0)   
                faddp   st(3),st(0)   
                faddp   st(1),st(0)   
                fxch    st(2)   
                fst     zf   
                fxch    st(1)   
                fst     uf   
                fxch    st(2)   
                fst     vf   
                fld     one   
                fdivrp  st(2),st(0)   
                fmul    st(0),st(1)   
                fxch    st(1)   
                fmul    st(2),st(0)   
                fistp   zb1   
                fistp   vb1   
                fistp   ub1   
                mov     eax,zb1   
                mov     ecx,vb1   
                mov     ebx,ub1   
                sub     eax,zf1   
                sub     ebx,uf1   
                mov     esi,step   
                cdq   
                sub     ecx,vf1   
                idiv    esi   
                xchg    eax,ebx   
                cdq   
                mov     zadd1,ebx   
                idiv    esi   
                xchg    eax,ecx   
                cdq   
                idiv    esi   
                mov     uadd1,ecx   
                mov     vadd1,eax   
                mov     ecx,esi   
                mov     ebx,p11   
                mov     esi,p22   
in_loop1:   
                mov     edx,zf1   
                mov     eax,vf1   
                mov     edi,uf1   
in_loop2:          
                add     edx,zadd1   
                add     eax,vadd1   
                add     edi,uadd1   
                add     ebx,2   
                add     esi,2   
                mov     zf1,edx   
                mov     vf1,eax   
                mov     uf1,edi   
                sar     edx,16   
                cmp     dx,WORD PTR [esi]   
                jge     short writeEnd   
                sar     eax,16   
                mov     WORD PTR [esi],dx   
                sar     edi,16   
                imul    wh   
                add     eax,edi   
                mov     edx,pm   
                mov     edi,uf1   
                lea     edx,[eax*2+edx]   
                mov     ax,WORD PTR[edx]   
                mov     edx,zf1   
                mov     WORD PTR[ebx],ax   
                mov     eax,vf1                
                loop    in_loop2   
                jmp     short loopEnd   
writeEnd:          
                loop    in_loop1   
loopEnd:   
                add     x,16   
                mov     p11,ebx   
                mov     p22,esi   
                mov     eax,x   
                mov     ebx,x2   
                cmp     eax,ebx   
                jl      out_loop   
                pop     edi   
                pop     esi   
            }   
    }   
    delete[] lines;   
}   
   
inline void fillTriangle3dWithT1(DDSURFACEDESC* ddsd,CPointList* ppl,TRIANGLE3D* t,WORD color)   
{   long lx1,ly1,lz1,lx2,ly2,lz2,rx1,ry1,rz1,rx2,ry2,rz2,ldx,ldy,ldz,rdx,rdy,rdz;   
    long minY,maxY,lindex,rindex,lxadd,lzadd,rxadd,rzadd,uvtp;   
    long luds,lua,lvds,lva,ruds,rua,rvds,rva,lza,lzds,rza,rzds;   
    long lu1,lv1,lu2,lv2,ru1,rv1,ru2,rv2,ldu,ldv,rdu,rdv;   
    long i,x11,x22,num = 3,lineNum = 0;   
    long mapWidth,mapHeight;   
    POINT3D_INT2 points[3];   
    POINT3D* p3d;   
    HLINE3D* lines;   
//  if(t->maped == FALSE)   
//  {   fillTriangle3d(ddsd,ppl,t,color);   
//      return;   
//  }   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    mapWidth = tex.textures[t->map]->width-1;   
    mapHeight = tex.textures[t->map]->height-1;   
    p3d = &ppl->points[t->p1];   
    points[0].x = FLOAT_TO_FIXED(p3d->x), points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);   
    points[0].u = FLOAT_TO_FIXED(t->u1*mapWidth), points[0].v = FLOAT_TO_FIXED(t->v1*mapHeight);   
    p3d = &ppl->points[t->p2];   
    points[1].x = FLOAT_TO_FIXED(p3d->x), points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);   
    points[1].u = FLOAT_TO_FIXED(t->u2*mapWidth), points[1].v = FLOAT_TO_FIXED(t->v2*mapHeight);   
    p3d = &ppl->points[t->p3];   
    points[2].x = FLOAT_TO_FIXED(p3d->x), points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);   
    points[2].u = FLOAT_TO_FIXED(t->u3*mapWidth), points[2].v = FLOAT_TO_FIXED(t->v3*mapHeight);   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    if((lines = new HLINE3D[maxY - minY]) == NULL)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = points[lindex].z;   
    lu1 = points[lindex].u;   
    lv1 = points[lindex].v;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    lz2 = points[lindex].z;   
    lu2 = points[lindex].u;   
    lv2 = points[lindex].v;   
    ldx = lx2 - lx1;   
    ldy = ly2 - ly1;   
    ldz = lz2 - lz1;   
    ldu = lu2 - lu1;   
    ldv = lv2 - lv1;   
    lxadd = ldx/ldy;   
//  lzadd = ldz/ldy;   
    uvtp = fixedDiv(ldz*2,lz2+lz1)/ldy;   
    lua = fixedMul(uvtp,ldu)/ldy;   
    lva = fixedMul(uvtp,ldv)/ldy;   
    lza = fixedMul(uvtp,ldz)/ldy;   
    uvtp = fixedDiv(lz1*2,lz1+lz2)/ldy;   
    luds = fixedMul(ldu,uvtp) - lua/2;   
    lvds = fixedMul(ldv,uvtp) - lva/2;   
    lzds = fixedMul(ldz,uvtp) - lza/2;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = points[rindex].z;   
    ru1 = points[rindex].u;   
    rv1 = points[rindex].v;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rz2 = points[rindex].z;   
    ru2 = points[rindex].u;   
    rv2 = points[rindex].v;   
    rdx = rx2 - rx1;   
    rdy = ry2 - ry1;   
    rdz = rz2 - rz1;   
    rdu = ru2 - ru1;   
    rdv = rv2 - rv1;   
    rxadd = rdx/rdy;   
//  rzadd = rdz/rdy;   
    uvtp = fixedDiv(rdz*2,rz2+rz1)/rdy;   
    rua = fixedMul(uvtp,rdu)/rdy;   
    rva = fixedMul(uvtp,rdv)/rdy;   
    rza = fixedMul(uvtp,rdz)/rdy;   
    uvtp = fixedDiv(rz1*2,rz1+rz2)/rdy;   
    ruds = fixedMul(rdu,uvtp) - rua/2;   
    rvds = fixedMul(rdv,uvtp) - rva/2;   
    rzds = fixedMul(rdz,uvtp) - rza/2;   
    minY++;   
    while(1)   
    {   lx1+=lxadd;   
//      lz1+=lzadd;   
        luds += lua;   
        lvds += lva;   
        lzds += lza;   
        lu1+=luds;   
        lv1+=lvds;   
        lz1+=lzds;   
        rx1+=rxadd;   
//      rz1+=rzadd;   
        ruds += rua;   
        rvds += rva;   
        rzds += rza;   
        ru1+=ruds;   
        rv1+=rvds;   
        rz1+=rzds;   
        x11 = FIXED_TO_LONG(lx1),x22 = FIXED_TO_LONG(rx1) - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
                x11=0;   
            if(x22>width)   
                x22=width;   
            lines[lineNum].x1 = x11;   
            lines[lineNum].z1 = lz1;   
            lines[lineNum].u1 = lu1;   
            lines[lineNum].v1 = lv1;   
            lines[lineNum].x2 = x22;   
            lines[lineNum].z2 = rz1;   
            lines[lineNum].u2 = ru1;   
            lines[lineNum].v2 = rv1;   
            lines[lineNum++].y = minY;   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = points[lindex].z;   
                lu1 = points[lindex].u;   
                lv1 = points[lindex].v;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
                lz2 = points[lindex].z;   
                lu2 = points[lindex].u;   
                lv2 = points[lindex].v;   
            }while(ly2 == ly1);   
            ldx = lx2 - lx1;   
            ldy = ly2 - ly1;   
            ldz = lz2 - lz1;   
            ldu = lu2 - lu1;   
            ldv = lv2 - lv1;   
            lxadd = ldx/ldy;   
//          lzadd = ldz/ldy;   
            uvtp = fixedDiv(ldz*2,lz2+lz1)/ldy;   
            lua = fixedMul(uvtp,ldu)/ldy;   
            lva = fixedMul(uvtp,ldv)/ldy;   
            lza = fixedMul(uvtp,ldz)/ldy;   
            uvtp = fixedDiv(lz1*2,lz1+lz2)/ldy;   
            luds = fixedMul(ldu,uvtp) - lua/2;   
            lvds = fixedMul(ldv,uvtp) - lva/2;   
            lzds = fixedMul(ldz,uvtp) - lza/2;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = points[rindex].z;   
                ru1 = points[rindex].u;   
                rv1 = points[rindex].v;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
                rz2 = points[rindex].z;   
                ru2 = points[rindex].u;   
                rv2 = points[rindex].v;   
            }while(ry2 == ry1);   
            rdx = rx2 - rx1;   
            rdy = ry2 - ry1;   
            rdz = rz2 - rz1;   
            rdu = ru2 - ru1;   
            rdv = rv2 - rv1;   
            rxadd = rdx/rdy;   
//          rzadd = rdz/rdy;   
            uvtp = fixedDiv(rdz*2,rz2+rz1)/rdy;   
            rua = fixedMul(uvtp,rdu)/rdy;   
            rva = fixedMul(uvtp,rdv)/rdy;   
            rza = fixedMul(uvtp,rdz)/rdy;   
            uvtp = fixedDiv(rz1*2,rz1+rz2)/rdy;   
            ruds = fixedMul(rdu,uvtp) - rua/2;   
            rvds = fixedMul(rdv,uvtp) - rva/2;   
            rzds = fixedMul(rdz,uvtp) - rza/2;   
        }   
        minY++;   
    }   
    WORD* p11,*pm;   
    short* p22;   
    pm = tex.textures[t->map]->image;   
    UINT wh = tex.textures[t->map]->width;   
    int dx,dz,du,dv,x,z,u,v,y,x2,z2,zadd,uds,ua,vds,va,uadd,vadd,lps;   
    for(i = 0;i < lineNum;i++)   
    {   x = lines[i].x1;   
        z = lines[i].z1;   
        u = lines[i].u1;   
        v = lines[i].v1;   
        x2 = lines[i].x2;   
        z2 = lines[i].z2;   
        y = lines[i].y;   
        dx = x2 - x;   
        p11 = p1 + pitch1*y + x;   
        *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
        if(dx <= 0)   
        {   continue;   
        }   
        dz = lines[i].z2 - z;   
        du = lines[i].u2 - u;   
        dv = lines[i].v2 - v;   
        zadd = dz/dx;   
        uvtp = fixedDiv(zadd*2,z2+z);   
        ua = fixedMul(uvtp,du)/dx;   
        va = fixedMul(uvtp,dv)/dx;   
        uvtp = fixedDiv(z*2,z+z2)/dx;   
        uds = fixedMul(du,uvtp) - ua/2;   
        vds = fixedMul(dv,uvtp) - va/2;   
//      uadd = du/dx;   
//      vadd = dv/dx;   
/*      while(x < x2)  
        {   x++;   
            p11++;  
            p22++;  
            uds += ua;  
            vds += va;  
            u+=uds;  
            v+=vds;  
//          u += uadd;  
//          v += vadd;  
            *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));  
        }*/   
        __asm   
        {   push    esi   
            push    edi   
            mov     eax,x2   
            mov     edx,x   
            mov     ebx,p11   
            sub     eax,edx   
            mov     ecx,eax   
            jz      short loopOut   
            mov     edi,ua   
            mov     esi,va   
inLoopTop:   
            mov     eax,uds   
            mov     edx,vds   
            add     ebx,2   
            add     eax,edi   
            mov     edi,v   
            add     edx,esi   
            mov     esi,u   
            mov     uds,eax   
            mov     vds,edx   
            add     edi,edx   
            add     esi,eax   
            mov     eax,wh   
            sub     edx,edx   
            mov     v,edi   
            mov     u,esi   
            sar     edi,16   
            sar     esi,16   
            imul    edi   
            mov     edx,pm   
            add     eax,esi   
            mov     esi,va   
            mov     edi,ua   
            lea     edx,[eax*2+edx]   
            dec     ecx   
            mov     ax,WORD PTR [edx]   
            mov     WORD PTR [ebx],ax   
            jnz     short inLoopTop   
loopOut:   
            pop     edi   
            pop     esi   
        }   
    }   
    delete[] lines;   
}   
   
#pragma warning( disable : 4035 )   
inline long fixedMul(long a,long b)   
{   __asm   
    {   mov     eax,a   
        mov     ebx,b   
        imul    ebx   
        shrd    eax,edx,16   
    }   
}   
   
   
   
inline long fixedDiv(long a,long b)   
{   __asm   
    {   mov     eax,a   
        cdq   
        shld    edx,eax,16   
        sal     eax,16   
        mov     ebx,b   
        idiv    ebx   
    }   
}   
   
inline void fillTriangle3dWithZLT(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,UINT ll)   
{   long lx1,ly1,lz1,lx2,ly2,lz2,rx1,ry1,rz1,rx2,ry2,rz2,ldy,rdy;   
    long minY,maxY,lindex,rindex,lxadd,lzadd,rxadd,rzadd;   
    long luadd,ruadd,lvadd,rvadd;   
    long lu1,lv1,lu2,lv2,ru1,rv1,ru2,rv2;   
    long i,x11,x22,num = 3,lineNum = 0;   
    long mapWidth,mapHeight;   
    POINT3D_INT2 points[3];   
    POINT3D* p3d;   
    HLINE3D* lines;   
    WORD color = lightLevel[ll];   
    if(t->maped == FALSE)   
    {   fillTriangle3dWithZ(ddsd,ddsdZBuf,ppl,t,color);   
        return;   
    }   
//  if(tex.textures[t->map]->level == FALSE)   
//      return;   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    long pitch2 = ddsdZBuf->lPitch>>1;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    short* p2=(short*)ddsdZBuf->lpSurface;   
    mapWidth = tex.textures[t->map]->width-1;   
    mapHeight = tex.textures[t->map]->height-1;   
    p3d = &ppl->points[t->p1];   
    points[0].x = FLOAT_TO_FIXED(p3d->x), points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);   
    points[0].u = FLOAT_TO_FIXED(t->u1*mapWidth), points[0].v = FLOAT_TO_FIXED(t->v1*mapHeight);   
    p3d = &ppl->points[t->p2];   
    points[1].x = FLOAT_TO_FIXED(p3d->x), points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);   
    points[1].u = FLOAT_TO_FIXED(t->u2*mapWidth), points[1].v = FLOAT_TO_FIXED(t->v2*mapHeight);   
    p3d = &ppl->points[t->p3];   
    points[2].x = FLOAT_TO_FIXED(p3d->x), points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);   
    points[2].u = FLOAT_TO_FIXED(t->u3*mapWidth), points[2].v = FLOAT_TO_FIXED(t->v3*mapHeight);   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    if((lines = new HLINE3D[maxY - minY]) == NULL)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = points[lindex].z;   
    lu1 = points[lindex].u;   
    lv1 = points[lindex].v;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    lz2 = points[lindex].z;   
    lu2 = points[lindex].u;   
    lv2 = points[lindex].v;   
    ldy = ly2 - ly1;   
    lxadd = (lx2 - lx1)/ldy;   
    lzadd = (lz2 - lz1)/ldy;   
    luadd = (lu2 - lu1)/ldy;   
    lvadd = (lv2 - lv1)/ldy;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = points[rindex].z;   
    ru1 = points[rindex].u;   
    rv1 = points[rindex].v;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rz2 = points[rindex].z;   
    ru2 = points[rindex].u;   
    rv2 = points[rindex].v;   
    rdy = ry2 - ry1;   
    rxadd = (rx2 - rx1)/rdy;   
    rzadd = (rz2 - rz1)/rdy;   
    ruadd = (ru2 - ru1)/rdy;   
    rvadd = (rv2 - rv1)/rdy;   
    minY++;   
    while(1)   
    {   lx1+=lxadd;   
        lz1+=lzadd;   
        lu1+=luadd;   
        lv1+=lvadd;   
        rx1+=rxadd;   
        rz1+=rzadd;   
        ru1+=ruadd;   
        rv1+=rvadd;   
        x11 = FIXED_TO_LONG(lx1),x22 = FIXED_TO_LONG(rx1)-1;   
        long u11,u22,v11,v22,z11,z22;   
        u11 = lu1,u22 = ru1,v11 = lv1,v22 = rv1,z11 = lz1,z22 = rz1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
            {   long dx=x22-x11;   
                u11 = u11-x11*(u22-u11)/dx;   
                v11 = v11-x11*(v22-v11)/dx;   
                z11 = z11-x11*(z22-z11)/dx;   
                x11=0;   
            }   
            else if(x22>width)   
            {   long dx=x22-x11;   
                long ddx=x22-width;   
                x22=width;   
                u22 = u22-ddx*(u22-u11)/dx;   
                v22 = v22-ddx*(v22-v11)/dx;   
                z22 = z22-ddx*(z22-z11)/dx;   
            }   
            lines[lineNum].x1 = x11;   
            lines[lineNum].z1 = z11;   
            lines[lineNum].u1 = u11;   
            lines[lineNum].v1 = v11;   
            lines[lineNum].x2 = x22;   
            lines[lineNum].z2 = z22;   
            lines[lineNum].u2 = u22;   
            lines[lineNum].v2 = v22;   
            lines[lineNum++].y = minY;   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = points[lindex].z;   
                lu1 = points[lindex].u;   
                lv1 = points[lindex].v;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
                lz2 = points[lindex].z;   
                lu2 = points[lindex].u;   
                lv2 = points[lindex].v;   
            }while(ly2 == ly1);   
            ldy = ly2 - ly1;   
            lxadd = (lx2 - lx1)/ldy;   
            lzadd = (lz2 - lz1)/ldy;   
            luadd = (lu2 - lu1)/ldy;   
            lvadd = (lv2 - lv1)/ldy;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = points[rindex].z;   
                ru1 = points[rindex].u;   
                rv1 = points[rindex].v;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
                rz2 = points[rindex].z;   
                ru2 = points[rindex].u;   
                rv2 = points[rindex].v;   
            }while(ry2 == ry1);   
            rdy = ry2 - ry1;   
            rxadd = (rx2 - rx1)/rdy;   
            rzadd = (rz2 - rz1)/rdy;   
            ruadd = (ru2 - ru1)/rdy;   
            rvadd = (rv2 - rv1)/rdy;   
        }   
        minY++;   
    }   
    WORD* p11,*pm;   
    short* p22;   
    pm = tex.textures[t->map]->image + tex.textures[t->map]->sectLenth*ll;   
    UINT wh = tex.textures[t->map]->width;   
    UINT ht = tex.textures[t->map]->height;   
    int dx,dz,du,dv,x,z,u,v,y,x2,z2,zadd,uadd,vadd,us,vs;   
    for(i = 0;i < lineNum;i++)   
    {   x = lines[i].x1;   
        z = lines[i].z1;   
        u = lines[i].u1;   
        v = lines[i].v1;   
        x2 = lines[i].x2;   
        z2 = lines[i].z2;   
        y = lines[i].y;   
        dx = x2 - x;   
        p22 = p2 + pitch2*y + x;   
        p11 = p1 + pitch1*y + x;   
        if(FIXED_TO_SHORT(z) < *p22)   
        {   *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
            *p22 = FIXED_TO_SHORT(z);   
        }   
        if(dx <= 0)   
        {   continue;   
        }   
        dz = z2 - z;   
        du = lines[i].u2 - u;   
        dv = lines[i].v2 - v;   
        zadd = dz/dx;   
        uadd = du/dx;   
        vadd = dv/dx;   
/*      while(x <= x2)  
        {   if(FIXED_TO_SHORT(z) < *p22)  
            {   us = FIXED_TO_SHORT(u);  
                vs = FIXED_TO_SHORT(v);  
                if(us>=wh)  
                    us = us%wh;  
                if(vs>=ht)  
                    vs = vs%ht;  
                *p11 = *(pm+vs*wh + us);  
                *p22 = FIXED_TO_SHORT(z<<1);  
            }  
            x++;  
            p11++;  
            p22++;  
            z += zadd;  
            u += uadd;  
            v += vadd;  
        }*/   
        __asm   
        {   push    esi   
            push    edi   
            mov     ecx,x2   
            mov     eax,x   
            mov     esi,p11   
            mov     edi,p22   
            sub     ecx,eax   
inLoop:   
            mov     eax,z   
            mov     ebx,u   
            mov     edx,v   
            add     esi,2   
            add     edi,2   
            add     eax,zadd   
            add     ebx,uadd   
            add     edx,vadd   
            mov     z,eax   
            mov     u,ebx   
            mov     v,edx   
            sar     eax,16   
            cmp     ax,WORD PTR[edi]   
            jge     writeEnd   
            mov     WORD PTR[edi],ax   
            mov     eax,ebx   
            mov     ebx,edx   
            sar     eax,16   
            mov     edx,0   
            sar     ebx,16   
            cmp     eax,wh   
            jl      checkV   
            div     wh   
            mov     eax,edx   
checkV:   
            xchg    eax,ebx   
            mov     edx,0   
            cmp     eax,ht   
            jl      writePixel111   
            div     ht   
            mov     eax,edx   
writePixel111:   
            mul     wh   
            add     eax,ebx   
            mov     edx,pm   
            lea     edx,[eax*2+edx]   
            mov     ax,WORD PTR[edx]   
            mov     WORD PTR[esi],ax               
writeEnd:   
            loop    inLoop   
loopEnd:   
            pop     edi   
            pop     esi   
        }   
    }    
    delete[] lines;   
}   
   
inline void fillTriangle3dWithZLT1(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,UINT ll)   
{   long lx1,ly1,lz1,lx2,ly2,lz2,rx1,ry1,rz1,rx2,ry2,rz2,ldx,ldy,ldz,rdx,rdy,rdz;   
    long minY,maxY,lindex,rindex,lxadd,lzadd,rxadd,rzadd,uvtp;   
    long luds,lua,lvds,lva,ruds,rua,rvds,rva,lza,lzds,rza,rzds;   
    long lu1,lv1,lu2,lv2,ru1,rv1,ru2,rv2,ldu,ldv,rdu,rdv;   
    long i,x11,x22,num = 3,lineNum = 0;   
    long mapWidth,mapHeight;   
    POINT3D_INT2 points[3];   
    POINT3D* p3d;   
    HLINE3D* lines;   
    WORD color = lightLevel[ll];   
    if(t->maped == FALSE)   
    {   fillTriangle3dWithZ(ddsd,ddsdZBuf,ppl,t,color);   
        return;   
    }   
//  if(tex.textures[t->map]->level == FALSE)   
//      return;   
    //ofstream ofs("c:\\tp.txt");   
    long width=ddsd->dwWidth-1,width1=ddsd->dwWidth,height=ddsd->dwHeight-1;   
    long pitch1 = ddsd->lPitch>>1;   
    long pitch2 = ddsdZBuf->lPitch>>1;   
    WORD* p1=(WORD*)ddsd->lpSurface;   
    short* p2=(short*)ddsdZBuf->lpSurface;   
    mapWidth = tex.textures[t->map]->width-1;   
    mapHeight = tex.textures[t->map]->height-1;   
    p3d = &ppl->points[t->p1];   
    points[0].x = FLOAT_TO_FIXED(p3d->x), points[0].y = (long)p3d->y, points[0].z = FLOAT_TO_FIXED(p3d->z);   
    points[0].u = FLOAT_TO_FIXED(t->u1*mapWidth), points[0].v = FLOAT_TO_FIXED(t->v1*mapHeight);   
    p3d = &ppl->points[t->p2];   
    points[1].x = FLOAT_TO_FIXED(p3d->x), points[1].y = (long)p3d->y, points[1].z = FLOAT_TO_FIXED(p3d->z);   
    points[1].u = FLOAT_TO_FIXED(t->u2*mapWidth), points[1].v = FLOAT_TO_FIXED(t->v2*mapHeight);   
    p3d = &ppl->points[t->p3];   
    points[2].x = FLOAT_TO_FIXED(p3d->x), points[2].y = (long)p3d->y, points[2].z = FLOAT_TO_FIXED(p3d->z);   
    points[2].u = FLOAT_TO_FIXED(t->u3*mapWidth), points[2].v = FLOAT_TO_FIXED(t->v3*mapHeight);   
    rindex = lindex = 0;   
    maxY=minY=points[0].y;   
    for(i=0;i<num;i++)   
    {   if(points[i].y<minY)   
            minY = points[lindex = i].y;   
        else if(points[i].y > maxY)   
                maxY = points[i].y;   
    }   
    if(minY == maxY)   
        return;   
    if((lines = new HLINE3D[maxY - minY]) == NULL)   
        return;   
    rindex = lindex;   
    while(points[lindex].y == minY)   
        //INDEX_BACKWORD(lindex);   
        INDEX_FORWORD(lindex);   
    INDEX_BACKWORD(lindex);   
    //INDEX_FORWORD(lindex);   
    while(points[rindex].y == minY)   
        //INDEX_FORWORD(rindex);   
        INDEX_BACKWORD(rindex);   
    //INDEX_BACKWORD(rindex);   
    INDEX_FORWORD(rindex);   
    lx1 = points[lindex].x;   
    ly1 = points[lindex].y;   
    lz1 = points[lindex].z;   
    lu1 = points[lindex].u;   
    lv1 = points[lindex].v;   
    INDEX_FORWORD(lindex);   
    //INDEX_BACKWORD(lindex);   
    lx2 = points[lindex].x;   
    ly2 = points[lindex].y;   
    lz2 = points[lindex].z;   
    lu2 = points[lindex].u;   
    lv2 = points[lindex].v;   
    ldx = lx2 - lx1;   
    ldy = ly2 - ly1;   
    ldz = lz2 - lz1;   
    ldu = lu2 - lu1;   
    ldv = lv2 - lv1;   
    lxadd = ldx/ldy;   
//  lzadd = ldz/ldy;   
    uvtp = fixedDiv((lz2-lz1)*2,lz2+lz1)/ldy;   
    lua = fixedMul(uvtp,ldu)/ldy;   
    lva = fixedMul(uvtp,ldv)/ldy;   
    lza = fixedMul(uvtp,ldz)/ldy;   
    uvtp = fixedDiv(lz1*2,lz1+lz2)/ldy;   
    luds = fixedMul(ldu,uvtp) - lua/2;   
    lvds = fixedMul(ldv,uvtp) - lva/2;   
    lzds = fixedMul(ldz,uvtp) - lza/2;   
    rx1 = points[rindex].x;   
    ry1 = points[rindex].y;   
    rz1 = points[rindex].z;   
    ru1 = points[rindex].u;   
    rv1 = points[rindex].v;   
    INDEX_BACKWORD(rindex);   
    //INDEX_FORWORD(rindex);   
    rx2 = points[rindex].x;   
    ry2 = points[rindex].y;   
    rz2 = points[rindex].z;   
    ru2 = points[rindex].u;   
    rv2 = points[rindex].v;   
    rdx = rx2 - rx1;   
    rdy = ry2 - ry1;   
    rdz = rz2 - rz1;   
    rdu = ru2 - ru1;   
    rdv = rv2 - rv1;   
    rxadd = rdx/rdy;   
//  rzadd = rdz/rdy;   
    uvtp = fixedDiv((rz2-rz1)*2,rz2+rz1)/rdy;   
    rua = fixedMul(uvtp,rdu)/rdy;   
    rva = fixedMul(uvtp,rdv)/rdy;   
    rza = fixedMul(uvtp,rdz)/rdy;   
    uvtp = fixedDiv(rz1*2,rz1+rz2)/rdy;   
    ruds = fixedMul(rdu,uvtp) - rua/2;   
    rvds = fixedMul(rdv,uvtp) - rva/2;   
    rzds = fixedMul(rdz,uvtp) - rza/2;   
    minY++;   
    while(1)   
    {   lx1+=lxadd;   
//      lz1+=lzadd;   
        luds += lua;   
        lvds += lva;   
        lzds += lza;   
        lu1+=luds;   
        lv1+=lvds;   
        lz1+=lzds;   
        rx1+=rxadd;   
//      rz1+=rzadd;   
        ruds += rua;   
        rvds += rva;   
        rzds += rza;   
        ru1+=ruds;   
        rv1+=rvds;   
        rz1+=rzds;   
        x11 = FIXED_TO_LONG(lx1),x22 = FIXED_TO_LONG(rx1) - 1;   
        if(minY>=0&&minY<=height&&x11<=width&&x22>=0&&x22>=x11)   
        {   if(x11<0)   
                x11=0;   
            if(x22>width)   
                x22=width;   
            lines[lineNum].x1 = x11;   
            lines[lineNum].z1 = lz1;   
            lines[lineNum].u1 = lu1;   
            lines[lineNum].v1 = lv1;   
            lines[lineNum].x2 = x22;   
            lines[lineNum].z2 = rz1;   
            lines[lineNum].u2 = ru1;   
            lines[lineNum].v2 = rv1;   
            lines[lineNum++].y = minY;   
        }   
        if(minY >= maxY)   
            break;   
        if(minY == ly2)   
        {   do{    
                lx1 = points[lindex].x;   
                ly1 = points[lindex].y;   
                lz1 = points[lindex].z;   
                lu1 = points[lindex].u;   
                lv1 = points[lindex].v;   
                INDEX_FORWORD(lindex);   
                //INDEX_BACKWORD(lindex);   
                lx2 = points[lindex].x;   
                ly2 = points[lindex].y;   
                lz2 = points[lindex].z;   
                lu2 = points[lindex].u;   
                lv2 = points[lindex].v;   
            }while(ly2 == ly1);   
            ldx = lx2 - lx1;   
            ldy = ly2 - ly1;   
            ldz = lz2 - lz1;   
            ldu = lu2 - lu1;   
            ldv = lv2 - lv1;   
            lxadd = ldx/ldy;   
//          lzadd = ldz/ldy;   
            uvtp = fixedDiv((lz2-lz1)*2,lz2+lz1)/ldy;   
            lua = fixedMul(uvtp,ldu)/ldy;   
            lva = fixedMul(uvtp,ldv)/ldy;   
            lza = fixedMul(uvtp,ldz)/ldy;   
            uvtp = fixedDiv(lz1*2,lz1+lz2)/ldy;   
            luds = fixedMul(ldu,uvtp) - lua/2;   
            lvds = fixedMul(ldv,uvtp) - lva/2;   
            lzds = fixedMul(ldz,uvtp) - lza/2;   
        }   
        if(minY == ry2)   
        {   do{   
                rx1 = points[rindex].x;   
                ry1 = points[rindex].y;   
                rz1 = points[rindex].z;   
                ru1 = points[rindex].u;   
                rv1 = points[rindex].v;   
                INDEX_BACKWORD(rindex);   
                //INDEX_FORWORD(rindex);   
                rx2 = points[rindex].x;   
                ry2 = points[rindex].y;   
                rz2 = points[rindex].z;   
                ru2 = points[rindex].u;   
                rv2 = points[rindex].v;   
            }while(ry2 == ry1);   
            rdx = rx2 - rx1;   
            rdy = ry2 - ry1;   
            rdz = rz2 - rz1;   
            rdu = ru2 - ru1;   
            rdv = rv2 - rv1;   
            rxadd = rdx/rdy;   
//          rzadd = rdz/rdy;   
            uvtp = fixedDiv((rz2-rz1)*2,rz2+rz1)/rdy;   
            rua = fixedMul(uvtp,rdu)/rdy;   
            rva = fixedMul(uvtp,rdv)/rdy;   
            rza = fixedMul(uvtp,rdz)/rdy;   
            uvtp = fixedDiv(rz1*2,rz1+rz2)/rdy;   
            ruds = fixedMul(rdu,uvtp) - rua/2;   
            rvds = fixedMul(rdv,uvtp) - rva/2;   
            rzds = fixedMul(rdz,uvtp) - rza/2;   
        }   
        minY++;   
    }   
    WORD* p11,*pm;   
    short* p22;   
    pm = tex.textures[t->map]->image + tex.textures[t->map]->sectLenth*ll;   
    UINT wh = tex.textures[t->map]->width;   
    int dx,dz,du,dv,x,z,u,v,y,x2,z2,u2,v2,zadd,uds,ua,vds,va,uadd,vadd;   
    for(i = 0;i < lineNum;i++)   
    {   x = lines[i].x1;   
        z = lines[i].z1;   
        u = lines[i].u1;   
        v = lines[i].v1;   
        x2 = lines[i].x2;   
        z2 = lines[i].z2;   
        u2 = lines[i].u2;   
        v2 = lines[i].v2;   
        y = lines[i].y;   
        dx = x2 - x;   
        dz = z2 - z;   
        du = u2 - u;   
        dv = v2 - v;   
        p22 = p2 + pitch2*y + x;   
        p11 = p1 + pitch1*y + x;   
        if(FIXED_TO_SHORT(z) < *p22)   
        {   *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
            *p22 = FIXED_TO_SHORT(z);   
        }   
        if(dx <= 0)   
        {   continue;   
        }   
        zadd = dz/dx;   
        uvtp = fixedDiv(zadd*2,z2+z);   
        ua = fixedMul(uvtp,du)/dx;   
        va = fixedMul(uvtp,dv)/dx;   
        uvtp = fixedDiv(z*2,z+z2)/dx;   
        uds = fixedMul(du,uvtp) - ua/2;   
        vds = fixedMul(dv,uvtp) - va/2;   
    //  uadd = du/dx;   
    //  vadd = dv/dx;   
        while(x < x2)   
        {   x++;   
            p11++;   
            p22++;   
            z += zadd;   
            uds += ua;   
            vds += va;   
            u+=uds;   
            v+=vds;   
    //      u += uadd;   
    //      v += vadd;   
            if(FIXED_TO_SHORT(z) < *p22)   
            {   *p11 = *(pm+FIXED_TO_SHORT(v)*wh + FIXED_TO_SHORT(u));   
                *p22 = FIXED_TO_SHORT(z);   
            }   
        }   
    }   
    delete[] lines;   
}   
