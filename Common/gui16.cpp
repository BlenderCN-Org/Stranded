www.pudn.com > gui3d.rar > gui16.cpp, change:1999-03-04,size:21958b

    

#include <fstream.h>   
#include "gui16.h"   
   
LPDIRECTDRAW            lpDD;           // DirectDraw object   
   
inline WORD RGBto16bit(DDPIXELFORMAT* pixFormat,unsigned char r,unsigned char g,unsigned char b)   
{   if(pixFormat->dwGBitMask == 0x07e0)   
        return (((WORD)r<<8)&0xf800)|(((WORD)g<<3)&0x07e0)|((WORD)b>>3);   
    else   
        return (((WORD)r<<7)&0x7c00)|(((WORD)g<<2)&0x03e0)|((WORD)b>>3);   
}   
   
inline void writePixel(DDSURFACEDESC* ddsd,int x,int y,WORD color)   
{   if(x<0||y<0||x>=ddsd->dwWidth||y>=ddsd->dwHeight)   
        return;   
    WORD* p = (WORD*)ddsd->lpSurface;   
    *(p + (ddsd->lPitch>>1) * y + x) = color;   
}   
   
inline void line(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color)   
{   int a,b,dx,dy,dxa,dyb,x,y,pk,twodxa,twodyb;   
    WORD* p = (WORD*)ddsd->lpSurface;   
    long pitch = ddsd->lPitch>>1;   
    long pitchb;   
    x = x1;   
    y = y1;;   
    dx = x2 - x1;   
    dy = y2 - y1;   
    p += pitch * y + x;   
    *p = color;   
    if(dx > 0)   
        a = 1;   
    else if(dx < 0)   
            a = -1;   
        else   
        {   if(dy > 0)   
                b = 1;   
            else if(dy < 0)   
                b = -1;   
            else   
                return;   
            pitchb = pitch * b;   
            while(y!=y2)   
            {   y += b;   
                p += pitchb;   
                *p = color;   
            }   
            return;   
        }   
    if(dy > 0)   
        b = 1;   
    else if(dy < 0)   
            b = -1;   
        else   
        {   while(x!=x2)   
            {   x+=a;   
                p += a;   
                *p = color;   
            }   
            return;   
        }   
    dxa = dx*a;   
    dyb = dy*b;   
    twodxa = 2*dxa;   
    twodyb = 2*dyb;   
    pitchb = pitch * b;   
    if(dxa >= dyb)   
    {   int twodyb_dxa = twodyb - twodxa;   
        pk = twodyb - dxa;   
        while(x != x2)   
        {   x+=a;   
            if(pk<0)   
            {   pk+=twodyb;   
                p += a;   
                *p = color;   
            }   
            else   
            {   pk+=twodyb_dxa;   
                p += pitchb + a;   
                *p = color;   
            }   
        }   
    }   
    else   
    {   int twodxa_dyb = twodxa - twodyb;   
        pk = twodxa - dyb;   
        while(y != y2)   
        {   y+=b;   
            if(pk<0)   
            {   pk+=twodxa;   
                p += pitchb;   
                *p = color;   
            }   
            else   
            {   pk+= twodxa_dyb;   
                p += pitchb + a;   
                *p = color;   
            }   
        }   
    }   
}   
   
void lineS(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color)   
{   int a,b,dx,dy,dxa,dyb,x,y,pk,twodxa,twodyb,ck,ckd1,ckd2;   
    WORD colors[9];   
    WORD* p = (WORD*)ddsd->lpSurface;   
    long pitch = ddsd->lPitch>>1;   
    long pitchb;   
    x = x1;   
    y = y1;;   
    dx = x2 - x1;   
    dy = y2 - y1;   
    p += pitch * y + x;   
    *p = color;   
    if(dx > 0)   
        a = 1;   
    else if(dx < 0)   
            a = -1;   
        else   
        {   if(dy > 0)   
                b = 1;   
            else if(dy < 0)   
                b = -1;   
            else   
                return;   
            pitchb = pitch * b;   
            while(y!=y2)   
            {   y+=b;   
                p += pitchb;   
                *p = color;   
            }   
            return;   
        }   
    if(dy > 0)   
        b = 1;   
    else if(dy < 0)   
            b = -1;   
        else   
        {   while(x!=x2)   
            {   x+=a;   
                p += a;   
                *p = color;   
            }   
            return;   
        }   
    WORD f8,f4,f2;   
    if(ddsd->ddpfPixelFormat.dwGBitMask == 0x07e0)   
    {   f8 = (color&0xc718)>>3;   
        f4 = (color&0xe79c)>>2;   
        f2 = (color&0xf7de)>>1;      
    }   
    else   
    {   f8 = (color&0x6318)>>3;   
        f4 = (color&0x739c)>>2;   
        f2 = (color&0x7bde)>>1;   
    }   
    colors[1] = f8;   
    colors[2] = f4;   
    colors[3] = f2;   
    colors[4] = f2;   
    colors[5] = f2+f8;   
    colors[6] = f2+f4;   
    colors[7] = f2+f4+f8;   
    colors[0] = f8;   
    colors[8] = colors[7];   
    dxa = dx*a;   
    dyb = dy*b;   
    int dx8 = dxa*8;   
    int dy8 = dyb*8;   
    twodxa = 2*dxa;   
    twodyb = 2*dyb;   
    pitchb = pitch * b;   
    if(dxa >= dyb)   
    {   ckd1 = -(dyb*8);   
        ckd2 = dxa*8 + ckd1;   
        int twodyb_dxa = twodyb - twodxa;   
        pk = twodyb - dxa;   
        ck = 4*dxa-4*pk;   
        while(x != x2)   
        {   x+=a;   
            if(pk<0)   
            {   pk+=twodyb;   
                p += a;   
                *p = colors[ck/dxa];   
                *(p+pitchb) = colors[8-ck/dxa];   
                ck += ckd1;   
                if(ck < 0)   
                    ck = 0;   
            }   
            else   
            {   pk+=twodyb_dxa;   
                p += a;   
                *p = colors[ck/dxa];   
                p += pitchb;   
                *p = colors[8-ck/dxa];   
                ck += ckd2;   
                if(ck > dx8)   
                    ck = dx8;   
            }   
        }   
    }   
    else   
    {   ckd1 = -(dxa*8);   
        ckd2 = dyb*8 + ckd1;   
        int twodxa_dyb = twodxa - twodyb;   
        pk = twodxa - dyb;   
        ck = 4*dyb-4*pk;   
        while(y != y2)   
        {   y+=b;   
            if(pk<0)   
            {   pk+=twodxa;   
                p += pitchb;   
                *p = colors[ck/dyb];   
                *(p+a) = colors[8 - ck/dyb];   
                ck += ckd1;   
                if(ck < 0)   
                    ck = 0;   
            }   
            else   
            {   pk+=twodxa_dyb;   
                p += pitchb;   
                *p = colors[ck/dyb];   
                p += a;   
                *p = colors[8 - ck/dyb];   
                ck += ckd2;   
                if(ck > dy8)   
                    ck = dy8;   
            }   
        }   
    }   
}   
   
inline void calColor(DDSURFACEDESC* ddsd,WORD* colors,WORD color)   
{   WORD f8,f4,f2;   
    if(ddsd->ddpfPixelFormat.dwGBitMask == 0x07e0)   
    {   f8 = (color&0xc718)>>3;   
        f4 = (color&0xe79c)>>2;   
        f2 = (color&0xf7de)>>1;   
    }   
    else   
    {   f8 = (color&0x6318)>>3;   
        f4 = (color&0x739c)>>2;   
        f2 = (color&0x7bde)>>1;   
    }   
    colors[1] = f2+f4+f8;//f8;   
    colors[2] = f2+f4;//f4;   
    colors[3] = f2;//f2;   
    colors[4] = f2;//f2;   
    colors[5] = f4+f8;//f2+f8;   
    colors[6] = f4;//f2+f4;   
    colors[7] = f8;//f2+f4+f8;   
    colors[0] = colors[1];//f8;   
    colors[8] = colors[7];   
}   
   
void lineS2(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color)   
{   int a,b,dx,dy,dxa,dyb,x,y,pk,twodxa,twodyb,ck,ckd1,ckd2;   
    WORD colors[9];   
    WORD colors1[9];   
    WORD fixColor = 0;   
    WORD* p = (WORD*)ddsd->lpSurface;   
    long pitch = ddsd->lPitch>>1;   
    long pitchb;   
    x = x1;   
    y = y1;;   
    dx = x2 - x1;   
    dy = y2 - y1;   
    p += pitch * y + x;   
    *p = color;   
    if(dx > 0)   
        a = 1;   
    else if(dx < 0)   
            a = -1;   
        else   
        {   if(dy > 0)   
                b = 1;   
            else if(dy < 0)   
                b = -1;   
            else   
                return;   
            pitchb = pitch * b;   
            while(y!=y2)   
            {   y+=b;   
                p += pitchb;   
                *p = color;   
            }   
            return;   
        }   
    if(dy > 0)   
        b = 1;   
    else if(dy < 0)   
            b = -1;   
        else   
        {   while(x!=x2)   
            {   x+=a;   
                p += a;   
                *p = color;   
            }   
            return;   
        }   
    WORD f8,f4,f2;   
    colors1[0] = 0;   
    colors1[1] = 0;   
    colors1[2] = 0;   
    colors1[3] = 0;   
    colors1[4] = 0;   
    colors1[5] = 0;   
    colors1[6] = 0;   
    colors1[7] = 0;   
    colors1[8] = 0;   
    if(ddsd->ddpfPixelFormat.dwGBitMask == 0x07e0)   
    {   f8 = (color&0xc718)>>3;   
        f4 = (color&0xe79c)>>2;   
        f2 = (color&0xf7de)>>1;   
    }   
    else   
    {   f8 = (color&0x6318)>>3;   
        f4 = (color&0x739c)>>2;   
        f2 = (color&0x7bde)>>1;   
    }   
    colors[1] = f8;   
    colors[2] = f4;   
    colors[3] = f2;   
    colors[4] = f2;   
    colors[5] = f2+f8;   
    colors[6] = f2+f4;   
    colors[7] = f2+f4+f8;   
    colors[0] = f8;   
    colors[8] = colors[7];   
    dxa = dx*a;   
    dyb = dy*b;   
    int dx8 = dxa*8;   
    int dy8 = dyb*8;   
    int idx1,idx2;   
    twodxa = 2*dxa;   
    twodyb = 2*dyb;   
    pitchb = pitch * b;   
    if(dxa >= dyb)   
    {   ckd1 = -(dyb*8);   
        ckd2 = dxa*8 + ckd1;   
        int twodyb_dxa = twodyb - twodxa;   
        pk = twodyb - dxa;   
        ck = 4*dxa-4*pk;   
        while(x != x2)   
        {   x+=a;   
            if(pk<0)   
            {   pk+=twodyb;   
                p += a;   
                idx1 = ck/dxa;   
                idx2 = 8 - idx1;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }      
                *p = colors[idx1] + colors1[idx1];   
                if(fixColor != *(p+pitchb));   
                {   fixColor = *(p+pitchb);   
                    calColor(ddsd,colors1,fixColor);   
                }      
                *(p+pitchb) = colors[idx2] + colors1[idx2];   
                ck += ckd1;   
                if(ck < 0)   
                    ck = 0;   
            }   
            else   
            {   pk+=twodyb_dxa;   
                p += a;   
                idx1 = ck/dxa;   
                idx2 = 8 - idx1;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }   
                *p = colors[idx1] + colors1[idx1];   
                p += pitchb;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }   
                *p = colors[idx2] + colors1[idx2];;   
                ck += ckd2;   
                if(ck > dx8)   
                    ck = dx8;   
            }   
        }   
    }   
    else   
    {   ckd1 = -(dxa*8);   
        ckd2 = dyb*8 + ckd1;   
        pk = twodxa - dyb;   
        int twodxa_dyb = twodxa - twodyb;   
        ck = 4*dyb-4*pk;   
        while(y != y2)   
        {   y+=b;   
            if(pk<0)   
            {   pk+=twodxa;   
                p += pitchb;   
                idx1 = ck/dyb;   
                idx2 = 8 - idx1;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }      
                *p = colors[idx1] + colors1[idx1];   
                if(fixColor != *(p+a));   
                {   fixColor = *(p+a);   
                    calColor(ddsd,colors1,fixColor);   
                }   
                *(p+a) = colors[idx2] + colors1[idx2];   
                ck += ckd1;   
                if(ck < 0)   
                    ck = 0;   
            }   
            else   
            {   pk+=twodxa_dyb;   
                p += pitchb;   
                idx1 = ck/dyb;   
                idx2 = 8 - idx1;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }   
                *p = colors[idx1] + colors1[idx1];   
                p += a;   
                if(fixColor != *p);   
                {   fixColor = *p;   
                    calColor(ddsd,colors1,fixColor);   
                }   
                *p = colors[idx2] + colors1[idx2];   
                ck += ckd2;   
                if(ck > dy8)   
                    ck = dy8;   
            }   
        }   
    }   
}   
   
void circle(DDSURFACEDESC* ddsd,int x,int y,int r,WORD color)   
{   int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;   
    xx = 0;   
    yy = r;   
    x1 = x , y1 = y + r;   
    x2 = x , y2 = y + r;   
    x3 = x , y3 = y - r;   
    x4 = x , y4 = y - r;   
    x5 = x + r , y5 = y;   
    x6 = x - r , y6 = y;   
    x7 = x + r , y7 = y;   
    x8 = x - r , y8 = y;   
    pk = 1 - r;   
    writePixel(ddsd,x1,y1,color);   
    writePixel(ddsd,x2,y2,color);   
    writePixel(ddsd,x3,y3,color);   
    writePixel(ddsd,x4,y4,color);   
    writePixel(ddsd,x5,y5,color);   
    writePixel(ddsd,x6,y6,color);   
    writePixel(ddsd,x7,y7,color);   
    writePixel(ddsd,x8,y8,color);   
    while(xx < yy)   
    {   xx++;   
        x1++,x2--,x3++,x4--;   
        y5++,y6++,y7--,y8--;   
        if(pk < 0)   
            pk+= 2*xx + 1;   
        else   
        {   yy--;   
            y1--,y2--,y3++,y4++;   
            x5--,x6++,x7--,x8++;   
            pk+= 2*(xx - yy) + 1;   
        }   
        writePixel(ddsd,x1,y1,color);   
        writePixel(ddsd,x2,y2,color);   
        writePixel(ddsd,x3,y3,color);   
        writePixel(ddsd,x4,y4,color);   
        writePixel(ddsd,x5,y5,color);   
        writePixel(ddsd,x6,y6,color);   
        writePixel(ddsd,x7,y7,color);   
        writePixel(ddsd,x8,y8,color);   
    }   
}   
   
void circleFill(DDSURFACEDESC* ddsd,int x,int y,int r,WORD color)   
{   int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;   
    int xx1,xx2,xx3,xx4,xx5,xx6,xx7,xx8;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    WORD* p=(WORD*)ddsd->lpSurface,*pt;   
    long pitch = ddsd->lPitch>>1;   
    xx = 0;   
    yy = r;   
    x1 = x , y1 = y + r;   
    x2 = x , y2 = y + r;   
    x3 = x , y3 = y - r;   
    x4 = x , y4 = y - r;   
    x5 = x + r , y5 = y;   
    x6 = x - r , y6 = y;   
    x7 = x + r , y7 = y;   
    x8 = x - r , y8 = y;   
    pk = 1 - r;   
    writePixel(ddsd,x1,y1,color);   
    writePixel(ddsd,x2,y2,color);   
    writePixel(ddsd,x3,y3,color);   
    writePixel(ddsd,x4,y4,color);   
    xx5 = x5;   
    xx6 = x6;   
    if(xx5 >= 0 && x6 <= width)   
    {   if(xx5 > width)   
            xx5=xx7= width;   
        if(xx6 < 0)   
            xx6=xx8= 0;   
        if(y5>=0&&y5<=height)   
        {   pt = p + pitch*y6 + xx6;   
            *pt = color;   
            while(xx6<xx5)   
            {   pt++;   
                *pt = color;   
                xx6++;   
            }   
        }   
    }   
    bool b1,b2;   
    while(xx < yy)   
    {   xx++;   
        x1++,x2--,x3++,x4--;   
        y5++,y6++,y7--,y8--;   
        b2 = true;   
        if(pk < 0)   
        {   b1 = false;   
            pk+= 2*xx + 1;   
            writePixel(ddsd,x1,y1,color);   
            writePixel(ddsd,x2,y2,color);   
            writePixel(ddsd,x3,y3,color);   
            writePixel(ddsd,x4,y4,color);   
        }   
        else   
        {   yy--;   
            b1 = true;   
            y1--,y2--,y3++,y4++;   
            x5--,x6++,x7--,x8++;   
            pk+= 2*(xx - yy) + 1;   
        }   
        xx1=x1,xx2=x2,xx3=x3,xx4=x4,xx5=x5,xx6=x6,xx7=x7,xx8=x8;   
        if(xx1 < 0)   
        {   xx1=xx3= 0;   
            b1 = false;   
        }   
        if(xx1 > width)   
            xx1=xx3= width;   
        if(xx2 < 0)   
            xx2=xx4= 0;   
        if(xx2 > width)   
        {   xx2=xx4= width;   
            b1 = false;   
        }   
        if(xx5 < 0)   
        {   xx5=xx7= 0;   
            b2 = false;   
        }   
        if(xx5 > width)   
            xx5=xx7= width;   
        if(xx6 < 0)   
            xx6=xx8= 0;   
        if(xx6 > width)   
        {   xx6=xx8= width;   
            b2 = false;   
        }   
        if(b1)   
        {   if(y1>=0&&y1<=height)   
            {   pt = p + pitch*y2 + xx2;   
                *pt = color;   
                while(xx2<xx1)   
                {   pt++;   
                    *pt = color;   
                    xx2++;   
                }   
            }   
            if(y3>=0&&y3<=height)   
            {   pt = p + pitch*y4 + xx4;   
                *pt = color;   
                while(xx4<xx3)   
                {   pt++;   
                    *pt = color;   
                    xx4++;   
                }   
            }   
        }   
        if(b2)   
        {   if(y5>=0&&y5<=height)   
            {   pt = p + pitch*y6 + xx6;   
                *pt = color;   
                while(xx6<xx5)   
                {   pt++;   
                    *pt = color;   
                    xx6++;   
                }   
            }   
            if(y7>=0&&y7<=height)   
            {   pt = p + pitch*y8 + xx8;   
                *pt = color;   
                while(xx8<xx7)   
                {   pt++;   
                    *pt = color;   
                    xx8++;   
                }   
            }   
        }   
    }   
}   
   
void ellipse(DDSURFACEDESC* ddsd,int xx1,int yy1,int xx2,int yy2,WORD color)   
{   int xx,yy,x1,x2,x3,x4,y1,y2,y3,y4,pk;   
    int xc,yc,rx,ry,rx2,ry2,tworx2,twory2,px,py;   
    xc = (xx1 + xx2) >> 1;   
    yc = (yy1 + yy2) >> 1;   
    if(xx1 > xx2)   
        rx = (xx1 - xx2) >> 1;   
    else   
        rx = (xx2 - xx1) >> 1;   
    if(yy1 > yy2)   
        ry = (yy1 - yy2) >> 1;   
    else   
        ry = (yy2 - yy1) >> 1;   
    rx2 = rx * rx;   
    ry2 = ry * ry;   
    tworx2 = rx2<<1;   
    twory2 = ry2<<1;   
    xx = 0;   
    yy = ry;   
    px = 0;   
    py = tworx2 * yy;   
    pk = ry2 - rx2*ry + rx2>>2;    
    x1 = xc , y1 = yc + ry;   
    x2 = xc , y2 = yc + ry;   
    x3 = xc , y3 = yc - ry;   
    x4 = xc , y4 = yc - ry;   
    writePixel(ddsd,x1,y1,color);   
    writePixel(ddsd,x2,y2,color);   
    writePixel(ddsd,x3,y3,color);   
    writePixel(ddsd,x4,y4,color);   
    while(px < py)   
    {   xx++;   
        px+= twory2;   
        x1++,x2--,x3++,x4--;   
        if(pk < 0)   
            pk+= ry2 + px;   
        else   
        {   yy--;   
            y1--,y2--,y3++,y4++;   
            py -= tworx2;   
            pk+= ry2 + px - py;   
        }   
        writePixel(ddsd,x1,y1,color);   
        writePixel(ddsd,x2,y2,color);   
        writePixel(ddsd,x3,y3,color);   
        writePixel(ddsd,x4,y4,color);   
    }   
    pk = (int)sqrt(((ry2*(xx*2+1)*(xx*2+1))>>2) + rx2*(yy-1)*(yy-1) - rx2*ry2);   
    while(yy>0)   
    {   yy--;   
        y1--,y2--,y3++,y4++;   
        py-=tworx2;   
        if(pk>0)   
        {   pk+=rx2 - py;   
        }   
        else   
        {   xx++;   
            x1++,x2--,x3++,x4--;   
            px+=twory2;   
            pk+=rx2-py+px;   
        }   
        writePixel(ddsd,x1,y1,color);   
        writePixel(ddsd,x2,y2,color);   
        writePixel(ddsd,x3,y3,color);   
        writePixel(ddsd,x4,y4,color);   
    }   
}   
   
void ellipseFill(DDSURFACEDESC* ddsd,int a1,int b1,int a2,int b2,WORD color)   
{   int xx,yy,x1,x2,x3,x4,y1,y2,y3,y4,pk;   
    int xc,yc,rx,ry,rx2,ry2,tworx2,twory2,px,py;   
    int xx1,xx2,xx3,xx4;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    long pitch = ddsd->lPitch>>1;   
    WORD* p=(WORD*)ddsd->lpSurface,*pt;   
    bool b;   
    xc = (a1 + a2) >> 1;   
    yc = (b1 + b2) >> 1;   
    if(a1 > a2)   
        rx = (a1 - a2) >> 1;   
    else   
        rx = (a2 - a1) >> 1;   
    if(b1 > b2)   
        ry = (b1 - b2) >> 1;   
    else   
        ry = (b2 - b1) >> 1;   
    rx2 = rx * rx;   
    ry2 = ry * ry;   
    tworx2 = rx2<<1;   
    twory2 = ry2<<1;   
    xx = 0;   
    yy = ry;   
    px = 0;   
    py = tworx2 * yy;   
    pk = ry2 - rx2*ry + rx2>>2;    
    x1 = xc , y1 = yc + ry;   
    x2 = xc , y2 = yc + ry;   
    x3 = xc , y3 = yc - ry;   
    x4 = xc , y4 = yc - ry;   
    writePixel(ddsd,x1,y1,color);   
    writePixel(ddsd,x3,y3,color);   
    while(px < py)   
    {   xx++;   
        px+= twory2;   
        x1++,x2--,x3++,x4--;   
        if(pk < 0)   
        {   b = false;   
            pk+= ry2 + px;   
            writePixel(ddsd,x1,y1,color);   
            writePixel(ddsd,x2,y2,color);   
            writePixel(ddsd,x3,y3,color);   
            writePixel(ddsd,x4,y4,color);   
        }   
        else   
        {   yy--;   
            b = true;   
            y1--,y2--,y3++,y4++;   
            py -= tworx2;   
            pk+= ry2 + px - py;   
        }   
        xx1=x1,xx2=x2,xx3=x3,xx4=x4;   
        if(xx1 < 0 || xx2 > width)   
            b = false;   
        if(xx1 > width)   
            xx1=xx3=width;   
        if(xx2 < 0)   
            xx2=xx4=0;   
        if(b)   
        {   if(y1>=0&&y1<=height)   
            {   pt = p + pitch*y2 + xx2;   
                *pt = color;   
                while(xx2<xx1)   
                {   pt++;   
                    *pt = color;   
                    xx2++;   
                }   
            }   
            if(y3>=0&&y3<=height)   
            {   pt = p + pitch*y4 + xx4;   
                *pt = color;   
                while(xx4<xx3)   
                {   pt++;   
                    *pt = color;   
                    xx4++;   
                }   
            }   
        }   
    }   
    pk = (int)sqrt(((ry2*(xx*2+1)*(xx*2+1))>>2) + rx2*(yy-1)*(yy-1) - rx2*ry2);   
    while(yy>0)   
    {   b = true;   
        yy--;   
        y1--,y2--,y3++,y4++;   
        py-=tworx2;   
        if(pk>0)   
        {   pk+=rx2 - py;   
        }   
        else   
        {   xx++;   
            x1++,x2--,x3++,x4--;   
            px+=twory2;   
            pk+=rx2-py+px;   
        }   
        xx1=x1,xx2=x2,xx3=x3,xx4=x4;   
        if(xx1 < 0 || xx2 > width)   
            b = false;   
        if(xx1 > width)   
            xx1=xx3=width;   
        if(xx2 < 0)   
            xx2=xx4=0;   
        if(b)   
        {   if(y1>=0&&y1<=height)   
            {   pt = p + pitch*y2 + xx2;   
                *pt = color;   
                while(xx2<xx1)   
                {   pt++;   
                    *pt = color;   
                    xx2++;   
                }   
            }   
            if(y3>=0&&y3<=height)   
            {   pt = p + pitch*y4 + xx4;   
                *pt = color;   
                while(xx4<xx3)   
                {   pt++;   
                    *pt = color;   
                    xx4++;   
                }   
            }   
        }   
    }   
}   
   
void drawRect(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color)   
{   int chg,xx1,xx2,yy1,yy2,x,y;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    long pitch = ddsd->lPitch>>1;   
    WORD* p=(WORD*)ddsd->lpSurface,*pt;   
    if(x1 > x2)   
    {   chg = x1;   
        x1 = x2;   
        x2 = chg;   
    }   
    if(y1 > y2)   
    {   chg = y1;   
        y1 = y2;   
        y2 = chg;   
    }   
    if(x1 > width || x2 < 0 || y1 > height || y2 < 0)   
        return;   
    xx1 = x1;xx2 = x2;yy1 = y1;yy2 = y2;   
    if(x1<0)   
        xx1 = 0;   
    if(x2>width)   
        xx2 = width;   
    if(y1<0)   
        yy1 = 0;   
    if(y2>height)   
        yy2 = height;   
    if(y1>=0)   
    {   pt = p + pitch*y1 + xx1;   
        *pt = color;   
        x = xx1;   
        while(x<xx2)   
        {   pt++;   
            *pt = color;   
            x++;   
        }   
    }   
    if(y2<=height)   
    {   pt = p + pitch*y2 + xx1;   
        *pt = color;   
        x = xx1;   
        while(x<xx2)   
        {   pt++;   
            *pt = color;   
            x++;   
        }   
    }   
    if(x1 >= 0)   
    {   pt = p + pitch*yy1 + x1;   
        *pt = color;   
        y = yy1;   
        while(y<yy2)   
        {   pt+=pitch;   
            *pt = color;   
            y++;   
        }   
    }   
    if(x2 <= width)   
    {   pt = p + pitch*yy1 + x2;   
        *pt = color;   
        y = yy1;   
        while(y<yy2)   
        {   pt+=pitch;   
            *pt = color;   
            y++;   
        }   
    }   
}   
   
void rectFill(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color)   
{   int chg,x;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    long pitch1,pitch = ddsd->lPitch>>1;   
    WORD* p=(WORD*)ddsd->lpSurface;   
    if(x1 > x2)   
    {   chg = x1;   
        x1 = x2;   
        x2 = chg;   
    }   
    if(y1 > y2)   
    {   chg = y1;   
        y1 = y2;   
        y2 = chg;   
    }   
    if(x1 > width || x2 < 0 || y1 > height || y2 < 0)   
        return;   
    if(x1<0)   
        x1 = 0;   
    if(x2>width)   
        x2 = width;   
    if(y1<0)   
        y1 = 0;   
    if(y2>height)   
        y2 = height;   
    p +=pitch*y1 + x1;   
    pitch1 = pitch-x2+x1;   
    while(y1<=y2)   
    {   *p=color;   
        x = x1;   
        while(x<x2)   
        {   p++;   
            x++;   
            *p=color;   
        }   
        y1++;   
        p+=pitch1;   
    }   
}   
   
void drawPolygon(DDSURFACEDESC* ddsd,POINT* points,int num,WORD color)   
{   int i;   
    for(i=0;i<num-1;i++)   
        line(ddsd,points[i].x,points[i].y,points[i+1].x,points[i+1].y,color);   
    line(ddsd,points[0].x,points[0].y,points[num-1].x,points[num-1].y,color);   
}   
   
#define INDEX_FORWORD(index)    index = (index + 1) % num;   
#define INDEX_BACKWORD(index)   index = (index - 1 + num) % num;   
void polygonFill(DDSURFACEDESC* ddsd,POINT* points,int num,WORD color)   
{   int lx1,ly1,lx2,ly2,rx1,ry1,rx2,ry2,ldx,ldy,rdx,rdy;   
    int minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,rxadd,lxsgn,rxsgn,lxsum,rxsum;   
    int i,x1,x2;   
    long width=ddsd->dwWidth-1,height=ddsd->dwHeight-1;   
    long pitch = ddsd->lPitch>>1;   
    WORD* p=(WORD*)ddsd->lpSurface,*pt;   
    if(num < 3)   
        return;   
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
    while(minY <= maxY)   
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
        x1 = lx1,x2 = rx1 - 1;   
        if(minY>=0&&minY<=height&&x1<=width&&x2>=0&&x2>=x1)   
        {   if(x1<0)   
                x1=0;   
            if(x2>width)   
                x2=width;   
            pt = p + x1;       
            *pt = color;   
            while(x1<x2)   
            {   pt++;   
                *pt = color;   
                x1++;   
            }   
        }   
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
   
bool loadBitmap(LPDIRECTDRAWSURFACE FAR * lpsurface,LPCSTR fileName)   
{   BMPHEAD head;   
    unsigned long bufLen,width,height;   
    unsigned char* buf;   
    DDSURFACEDESC       ddsd;   
    HRESULT             ddrval;   
    ifstream ifs(fileName,ios::binary);   
    if(!ifs)   
        return FALSE;   
    ifs.read((char*)&head,sizeof(BMPHEAD));   
    if(!ifs.good())   
        return FALSE;   
    width = head.width;   
    height = head.height;   
    ddsd.dwSize = sizeof(ddsd);   
    ddsd.dwFlags = DDSD_CAPS |    
                   DDSD_WIDTH|   
                   DDSD_HEIGHT;   
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY ;   
    ddsd.dwWidth = width;   
    ddsd.dwHeight = height;   
    ddrval = lpDD->CreateSurface( &ddsd, lpsurface, NULL );   
    if(ddrval != DD_OK)   
        return FALSE;   
    bufLen = (width*3 + 3) & 0xfffffffc;   
    if((buf = new unsigned char[bufLen]) == NULL)   
        return FALSE;   
    while ((ddrval=(*lpsurface)->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)   
                ;   
    WORD* p = (WORD*)ddsd.lpSurface + width*height - 1 - width;   
    for(UINT i=0;i<height;i++)   
    {   ifs.read(buf,bufLen);   
        if(ifs.bad())   
        {   (*lpsurface)->Unlock(NULL);   
            return FALSE;   
        }   
        for(UINT j=0;j<width;j++)   
        {   *p++ = (((WORD)buf[j*3]&0xfff8)>>3) |   
                   (((WORD)buf[j*3+1]&0xfffc)<<3)|   
                   (((WORD)buf[j*3+2]&0xfff8)<<8);   
        }   
        p -= width*2;   
    }   
    (*lpsurface)->Unlock(NULL);   
    delete[] buf;   
    return TRUE;   
}   

