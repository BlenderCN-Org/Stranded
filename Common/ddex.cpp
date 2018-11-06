

    

/*==========================================================================  
 *  
 *  Copyright (C) 1995-1996 Microsoft Corporation. All Rights Reserved.  
 *  
 *  File:       ddex1.cpp  
 *  Content:    Direct Draw example program 1.  Creates a Direct Draw   
 *              object and then a primary surface with a back buffer.  
 *              Slowly flips between the primary surface and the back  
 *              buffer.  Press F12 to terminate the program.  
 *  
 ***************************************************************************/   
   
#define NAME "DDExample1"   
#define TITLE "Direct Draw Example 1"   
   
#define WIN32_LEAN_AND_MEAN   
#include <windows.h>   
#include <windowsx.h>   
#include <ddraw.h>   
#include <fstream.h>   
#include <string.h>   
#include <stdlib.h>   
#include <stdarg.h>   
#include <time.h>   
#include <stdio.h>   
#include "resource.h"   
#include "gui16.h"   
   
#define TIMER_ID        1   
#define TIMER_RATE      30   
   
LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface   
LPDIRECTDRAWSURFACE     lpDDSBack;      // DirectDraw back surface   
LPDIRECTDRAWSURFACE     lpDDSZBuffer;   
LPDIRECTDRAWSURFACE     lpDDSEndBmp;   
char*                   txt[80];   
int                     lines;   
BOOL                    bActive;        // is application active?   
BOOL                    bQuit = false;   
BOOL                    bClear = true;   
int                     phase = 0;   
HANDLE                  thandle;   
HWND                hwnd;   
   
CPointList p1(2000);   
CPointList p2(2000);   
CFaceList face(4000);   
CONVERT_MATRIX3D m1,m2,m4;   
POINT3D pot,pot1;   
   
CPointList p3(200);   
CPointList p4(200);   
CFaceList face1(400);   
CTextureList tex(10);   
CONVERT_MATRIX3D m5,m6,m7,m8;   
POINT3D pot2,pot3;   
   
DWORD WINAPI demo(LPVOID lpparm);   
void lineDemo1(DDSURFACEDESC* ddsd);   
void lineDemo2(DDSURFACEDESC* ddsd);   
void circleDemo1(DDSURFACEDESC* ddsd);   
void circleDemo2(DDSURFACEDESC* ddsd);   
void ellipseDemo1(DDSURFACEDESC* ddsd);   
void ellipseDemo2(DDSURFACEDESC* ddsd);   
void rectDemo1(DDSURFACEDESC* ddsd);   
void rectDemo2(DDSURFACEDESC* ddsd);   
void polygonDemo1(DDSURFACEDESC* ddsd);   
void polygonDemo2(DDSURFACEDESC* ddsd);   
void fsDemo1(DDSURFACEDESC* ddsd);   
BOOL demo3dInit();   
BOOL demo2dInit();   
void clearUp();   
   
time_t tt;   
long frames,rate,k;   
POINT3D potLight;   
   
/*  
 * finiObjects  
 *  
 * finished with all objects we use; release them  
 */   
   
static void finiObjects( void )   
{   
    if( lpDD != NULL )   
    {   
        if( lpDDSPrimary != NULL )   
        {   
            lpDDSPrimary->Release();   
            lpDDSPrimary = NULL;   
        }   
        if( lpDDSZBuffer != NULL )   
        {   
            lpDDSZBuffer->Release();   
            lpDDSZBuffer = NULL;   
        }   
        if( lpDDSEndBmp != NULL )   
        {   
            lpDDSEndBmp->Release();   
            lpDDSEndBmp = NULL;   
        }   
        lpDD->Release();   
        lpDD = NULL;   
    }   
    clearUp();   
} /* finiObjects */   
   
char szMsg[] = "计算机图形学算法演示 (F12 退出)";   
char szMsg1[] = "按↓键前进，按↑键后退";   
char szFrontMsg[][40] = {   "普通直线",   
                            "反走样直线",   
                            "画圆",   
                            "圆填充",   
                            "画椭圆",   
                            "椭圆填充",   
                            "画矩形",   
                            "矩形填充",   
                            "画多边形",   
                            "多边形填充",   
                            "分形图",   
                            "三维图形",   
                            "纹理贴图",   
                            "The End"   
                        };     
char szBackMsg[] = "计算机图形学算发演示 (F12 退出)";   
   
long FAR PASCAL WindowProc( HWND hWnd, UINT message,    
                            WPARAM wParam, LPARAM lParam )   
{   
   // PAINTSTRUCT ps;   
  //  RECT        rc;   
   // SIZE        size;   
   
    switch( message )   
    {   
    case WM_ACTIVATEAPP:   
        bActive = wParam;   
        break;   
   
    case WM_CREATE:   
        break;   
   
    case WM_SETCURSOR:   
        SetCursor(NULL);   
        return TRUE;   
    case WM_KEYDOWN:   
        switch( wParam )   
        {   
        case VK_ESCAPE:   
        case VK_F12:   
            bQuit = true;   
            while(WaitForSingleObject(thandle,200) != WAIT_OBJECT_0)   
                ;   
            PostMessage(hWnd, WM_CLOSE, 0, 0);   
            break;   
        case VK_DOWN:   
            bClear = true;   
            if(phase < 13)   
                phase++;   
            if(phase == 10)   
                bClear = false;   
            break;   
        case  VK_UP:   
            bClear = true;   
            if(phase > 0)   
                phase--;   
            if(phase == 10|| phase==13)   
                bClear = false;   
            break;   
        }   
        break;   
   
    case WM_PAINT:   
        break;   
   
    case WM_DESTROY:   
        finiObjects();   
        PostQuitMessage( 0 );   
        break;   
    }   
   
    return DefWindowProc(hWnd, message, wParam, lParam);   
   
} /* WindowProc */   
   
/*  
 * doInit - do work required for every instance of the application:  
 *                create the window, initialize data  
 */   
static BOOL doInit( HINSTANCE hInstance, int nCmdShow )   
{          
    WNDCLASS            wc;   
    DDSURFACEDESC       ddsd;   
    DDSCAPS             ddscaps;   
    HRESULT             ddrval;   
    HDC                 hdc;   
    char                buf[256];   
   
    /*  
     * set up and register window class  
     */   
    wc.style = CS_HREDRAW | CS_VREDRAW;   
    wc.lpfnWndProc = WindowProc;   
    wc.cbClsExtra = 0;   
    wc.cbWndExtra = 0;   
    wc.hInstance = hInstance;   
    wc.hIcon = LoadIcon( hInstance, IDI_APPLICATION );   
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );   
    wc.hbrBackground = NULL;   
    wc.lpszMenuName = NAME;   
    wc.lpszClassName = NAME;   
    RegisterClass( &wc );   
       
    /*  
     * create a window  
     */   
    hwnd = CreateWindowEx(   
        WS_EX_TOPMOST,   
        NAME,   
        TITLE,   
        WS_POPUP,   
        0, 0,   
        GetSystemMetrics( SM_CXSCREEN ),   
        GetSystemMetrics( SM_CYSCREEN ),   
        NULL,   
        NULL,   
        hInstance,   
        NULL );   
   
    if( !hwnd )   
    {   
        return FALSE;   
    }   
   
    ShowWindow( hwnd, nCmdShow );   
    UpdateWindow( hwnd );   
//  MessageBox( hwnd, "Hello!", "", MB_OK);    
   
    /*  
     * create the main DirectDraw object  
     */   
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );   
    if( ddrval == DD_OK )   
    {   
        // Get exclusive mode   
        ddrval = lpDD->SetCooperativeLevel( hwnd,   
                                DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );   
        if(ddrval == DD_OK )   
        {   
            ddrval = lpDD->SetDisplayMode( 640,480, 16 );   
            if( ddrval == DD_OK )   
            {   
                // Create the primary surface with 1 back buffer   
                ddsd.dwSize = sizeof( ddsd );   
                ddsd.dwFlags = DDSD_CAPS |    
                               DDSD_BACKBUFFERCOUNT;   
                ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |   
                                      DDSCAPS_FLIP |    
                                      DDSCAPS_COMPLEX;   
                ddsd.dwBackBufferCount = 1;   
                ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );   
                if( ddrval == DD_OK )   
                {   
                    // Get a pointer to the back buffer   
                    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;   
                    ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps,    
                                                          &lpDDSBack);   
                    if( ddrval == DD_OK )   
                    {   
                        LOGBRUSH brush;   
                        RECT rc;   
                        brush.lbStyle=BS_SOLID ;   
                        brush.lbColor=(COLORREF)0x00000000;   
                        HBRUSH hbrush=CreateBrushIndirect(&brush);   
                        GetClientRect(hwnd, &rc);   
                        // draw some text.   
                        if (lpDDSPrimary->GetDC(&hdc) == DD_OK)   
                        {   
                            FillRect(hdc,&rc,hbrush);   
                            lpDDSPrimary->ReleaseDC(hdc);   
                        }   
   
                        if (lpDDSBack->GetDC(&hdc) == DD_OK)   
                        {   
                            FillRect(hdc,&rc,hbrush);   
                            lpDDSBack->ReleaseDC(hdc);   
                        }   
                        DeleteObject(hbrush);   
                        //Create Z buffer.   
                        ddsd.dwFlags = DDSD_CAPS |    
                                       DDSD_WIDTH|   
                                       DDSD_HEIGHT|   
                                       DDSD_ZBUFFERBITDEPTH ;   
                        ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER|DDSCAPS_SYSTEMMEMORY ;   
                        ddsd.dwWidth = 640;   
                        ddsd.dwHeight = 480;   
                        ddsd.dwZBufferBitDepth = 16;   
                        ddrval = lpDD->CreateSurface( &ddsd, &lpDDSZBuffer, NULL );   
                        if(ddrval == DD_OK)   
                        {   lpDDSBack->AddAttachedSurface(lpDDSZBuffer);   
                            if (demo2dInit())   
                                if(demo3dInit())   
                                {   frames=0;   
                                    time(&tt);   
                                    DWORD tID;   
                                    if((thandle = CreateThread(NULL,0,demo,NULL,0,&tID)) != NULL)   
                                        return TRUE;   
                                }   
                        }   
                    }   
                }   
            }   
        }   
    }   
   
    wsprintf(buf, "Direct Draw Init Failed (%08lx)\n", ddrval );   
    MessageBox( hwnd, buf, "ERROR", MB_OK );   
    finiObjects();   
    DestroyWindow( hwnd );   
    return FALSE;   
} /* doInit */   
   
void clearUp()   
{   for(int i=0;i<lines;i++)   
        delete[] txt[i];   
}   
   
#define MAX_LINE_LENTH 80   
#define MAX_LINES 80   
BOOL demo2dInit()   
{   if(loadBitmap(&lpDDSEndBmp,"end.bmp") == FALSE)   
        return FALSE;   
    char buf[MAX_LINE_LENTH];   
    ifstream ifs("end.txt");   
    if(!ifs)   
        return FALSE;   
    lines = MAX_LINES;   
    for(int i=0;i<MAX_LINES;i++)   
    {   ifs.getline( buf,MAX_LINE_LENTH-1);   
        if((txt[i] = new char[strlen(buf)+1]) == NULL)   
        {   lines = i;   
            return FALSE;   
        }   
        strcpy(txt[i],buf);   
        if(ifs.eof())   
        {   lines = i+1;   
            break;   
        }   
    }   
    return TRUE;   
}   
   
BOOL demo3dInit()   
{   POINT3D potp;   
    TRIANGLE3D trgl;   
    potp.x = 0, potp.y = 0, potp.z = 0;   
    p3.addPoint(&potp);   
    potp.x = 500, potp.y = 0, potp.z = 0;   
    p3.addPoint(&potp);   
    potp.x = 500, potp.y = 500, potp.z = 0;   
    p3.addPoint(&potp);   
    potp.x = 0, potp.y = 500, potp.z = 0;   
    p3.addPoint(&potp);   
    potp.x = 0, potp.y = 0, potp.z = 500;   
    p3.addPoint(&potp);   
    potp.x = 500, potp.y = 0, potp.z = 500;   
    p3.addPoint(&potp);   
    potp.x = 500, potp.y = 500, potp.z = 500;   
    p3.addPoint(&potp);   
    potp.x = 0, potp.y = 500, potp.z = 500;   
    p3.addPoint(&potp);   
    potp.x = 200, potp.y = 200, potp.z = 200;   
    p3.addPoint(&potp);   
    potp.x = 300, potp.y = 200, potp.z = 200;   
    p3.addPoint(&potp);   
    potp.x = 300, potp.y = 300, potp.z = 200;   
    p3.addPoint(&potp);   
    potp.x = 200, potp.y = 300, potp.z = 200;   
    p3.addPoint(&potp);   
    potp.x = 200, potp.y = 200, potp.z = 300;   
    p3.addPoint(&potp);   
    potp.x = 300, potp.y = 200, potp.z = 300;   
    p3.addPoint(&potp);   
    potp.x = 300, potp.y = 300, potp.z = 300;   
    p3.addPoint(&potp);   
    potp.x = 200, potp.y = 300, potp.z = 300;   
    p3.addPoint(&potp);   
/*  trgl.p1=3 , trgl.p2=2 , trgl.p3=1 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=3 , trgl.p2=1 , trgl.p3=0 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=3 , trgl.p2=0 , trgl.p3=4 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=3 , trgl.p2=4 , trgl.p3=7 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=0 , trgl.p2=1 , trgl.p3=4 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=4 , trgl.p2=1 , trgl.p3=5 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=3 , trgl.p2=7 , trgl.p3=6 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=3 , trgl.p2=6 , trgl.p3=2 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=7 , trgl.p2=4 , trgl.p3=5 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=6 , trgl.p2=7 , trgl.p3=5 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=6 , trgl.p2=5 , trgl.p3=2 ;  
    face1.addTriangle(&trgl);  
    trgl.p1=2 , trgl.p2=5 , trgl.p3=1 ;  
    face1.addTriangle(&trgl);*/   
    trgl.p1=11 , trgl.p2=9 , trgl.p3=8 ,trgl.maped=TRUE,trgl.map=0;   
    trgl.u1=(float)0.98,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=11 , trgl.p2=10 , trgl.p3=9 ,trgl.maped=TRUE,trgl.map=0;   
    trgl.u1=(float)0.98,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.98,trgl.u3=(float)0.02,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=11 , trgl.p2=8 , trgl.p3=12 ,trgl.maped=TRUE,trgl.map=1;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=15 , trgl.p2=11 , trgl.p3=12 ,trgl.maped=TRUE,trgl.map=1;   
    trgl.u1=(float)0.98,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.98,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=8 , trgl.p2=13 , trgl.p3=12 ,trgl.maped=TRUE,trgl.map=2;   
    trgl.u1=(float)0.98,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=8 , trgl.p2=9 , trgl.p3=13 ,trgl.maped=TRUE,trgl.map=2;   
    trgl.u1=(float)0.98,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.98,trgl.u3=(float)0.02,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=11 , trgl.p2=15 , trgl.p3=14,trgl.maped=TRUE,trgl.map=2 ;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=11 , trgl.p2=14 , trgl.p3=10 ,trgl.maped=TRUE,trgl.map=2;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.98,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.98;   
    face1.addTriangle(&trgl);   
    trgl.p1=15 , trgl.p2=12 , trgl.p3=14 ,trgl.maped=TRUE,trgl.map=0;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.98;   
    face1.addTriangle(&trgl);   
    trgl.p1=12 , trgl.p2=13 , trgl.p3=14 ,trgl.maped=TRUE,trgl.map=0;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.02,trgl.u2=(float)0.98,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.98;   
    face1.addTriangle(&trgl);   
    trgl.p1=14 , trgl.p2=13 , trgl.p3=9 ,trgl.maped=TRUE,trgl.map=1;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.02,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.02;   
    face1.addTriangle(&trgl);   
    trgl.p1=14 , trgl.p2=9 , trgl.p3=10 ,trgl.maped=TRUE,trgl.map=1;   
    trgl.u1=(float)0.02,trgl.v1=(float)0.98,trgl.u2=(float)0.98,trgl.v2=(float)0.02,trgl.u3=(float)0.98,trgl.v3=(float)0.98;   
    face1.addTriangle(&trgl);   
    VECTOR3D v2={4,7,2};   
    VECTOR3D v8={4,2,7};   
    VECTOR3D v3={0,0,-1};   
    pot2.x = 250,pot2.y = 250,pot2.z = 820;   
    pot3.x = 250,pot3.y = 250,pot3.z = 250;   
    MKScaleMatrix(&m5,&pot3,2);   
    p3.scalePoints(&m5,CONVERTALL);   
    MKRotateMatrix(&m5,&pot3,&v2,3.1416/90);   
    MKRotateMatrix(&m8,&pot3,&v8,3.1416/90);   
//  p3.rotatePoints(&m5,CONVERTALL);   
    MKWCtoVcMatrix(&m6,&v3,&pot2);   
    MKProjectionMatrix(&m7,550,320,240);   
    matrixMul(&m7,&m6);   
    if(tex.addTexture("w1.bmp") == -1)   
        return FALSE;   
    if(tex.addTexture("w2.bmp") == -1)   
        return FALSE;   
    if(tex.addTexture("w3.bmp") == -1)   
        return FALSE;   
    if(tex.addLevelTexture("cp.bmp") == -1)   
        return FALSE;   
    if(tex.addLevelTexture("chopper.bmp") == -1)   
        return FALSE;   
//  if(tex.addTexture("chopper.bmp") == -1)   
//      return FALSE;   
   
    CONVERT_MATRIX3D m;   
    POINT3D pot2;   
    pot2.x = -20,pot2.y= 0, pot2.z=0;   
//  if(readDxfFile("chopper.dxf",&p1,&face) == FALSE)   
//      return FALSE;   
    if(readAscFile("chopper.asc",&p1,&face) == FALSE)   
        return FALSE;   
/*  TRIANGLE3D *tp;  
    tp = &face.faces[438];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.24,tp->v1=(float)0.5,tp->u2=(float)0.24,tp->v2=(float)0.9,tp->u3=(float)0.1,tp->v3=(float)0.9;  
    tp = &face.faces[439];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.24,tp->v1=(float)0.5,tp->u2=(float)0.1,tp->v2=(float)0.9,tp->u3=(float)0.1,tp->v3=(float)0.5;  
    tp = &face.faces[436];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.24,tp->v1=(float)0.1,tp->u2=(float)0.24,tp->v2=(float)0.5,tp->u3=(float)0.1,tp->v3=(float)0.5;  
    tp = &face.faces[437];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.24,tp->v1=(float)0.1,tp->u2=(float)0.1,tp->v2=(float)0.5,tp->u3=(float)0.1,tp->v3=(float)0.1;  
    tp = &face.faces[390];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.38,tp->v1=(float)0.5,tp->u2=(float)0.38,tp->v2=(float)0.9,tp->u3=(float)0.24,tp->v3=(float)0.9;  
    tp = &face.faces[391];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.38,tp->v1=(float)0.5,tp->u2=(float)0.24,tp->v2=(float)0.9,tp->u3=(float)0.24,tp->v3=(float)0.5;  
    tp = &face.faces[388];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.38,tp->v1=(float)0.1,tp->u2=(float)0.38,tp->v2=(float)0.5,tp->u3=(float)0.24,tp->v3=(float)0.5;  
    tp = &face.faces[389];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.38,tp->v1=(float)0.1,tp->u2=(float)0.24,tp->v2=(float)0.5,tp->u3=(float)0.24,tp->v3=(float)0.1;  
    tp = &face.faces[345];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.52,tp->v1=(float)0.5,tp->u2=(float)0.38,tp->v2=(float)0.9,tp->u3=(float)0.38,tp->v3=(float)0.5;  
    tp = &face.faces[344];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.52,tp->v1=(float)0.5,tp->u2=(float)0.52,tp->v2=(float)0.9,tp->u3=(float)0.38,tp->v3=(float)0.9;  
    tp = &face.faces[342];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.52,tp->v1=(float)0.1,tp->u2=(float)0.52,tp->v2=(float)0.5,tp->u3=(float)0.38,tp->v3=(float)0.5;  
    tp = &face.faces[343];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.52,tp->v1=(float)0.1,tp->u2=(float)0.38,tp->v2=(float)0.5,tp->u3=(float)0.38,tp->v3=(float)0.1;  
    tp = &face.faces[296];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.66,tp->v1=(float)0.5,tp->u2=(float)0.66,tp->v2=(float)0.9,tp->u3=(float)0.52,tp->v3=(float)0.9;  
    tp = &face.faces[297];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.66,tp->v1=(float)0.5,tp->u2=(float)0.52,tp->v2=(float)0.9,tp->u3=(float)0.52,tp->v3=(float)0.5;  
    tp = &face.faces[294];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.66,tp->v1=(float)0.1,tp->u2=(float)0.66,tp->v2=(float)0.5,tp->u3=(float)0.52,tp->v3=(float)0.5;  
    tp = &face.faces[295];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.66,tp->v1=(float)0.1,tp->u2=(float)0.52,tp->v2=(float)0.5,tp->u3=(float)0.52,tp->v3=(float)0.1;  
    tp = &face.faces[248];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.8,tp->v1=(float)0.5,tp->u2=(float)0.8,tp->v2=(float)0.9,tp->u3=(float)0.66,tp->v3=(float)0.9;  
    tp = &face.faces[249];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.8,tp->v1=(float)0.5,tp->u2=(float)0.66,tp->v2=(float)0.9,tp->u3=(float)0.66,tp->v3=(float)0.5;  
    tp = &face.faces[246];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.8,tp->v1=(float)0.1,tp->u2=(float)0.8,tp->v2=(float)0.5,tp->u3=(float)0.66,tp->v3=(float)0.5;  
    tp = &face.faces[247];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.8,tp->v1=(float)0.1,tp->u2=(float)0.66,tp->v2=(float)0.5,tp->u3=(float)0.66,tp->v3=(float)0.1;  
    tp = &face.faces[202];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.94,tp->v1=(float)0.5,tp->u2=(float)0.94,tp->v2=(float)0.9,tp->u3=(float)0.8,tp->v3=(float)0.9;  
    tp = &face.faces[203];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.94,tp->v1=(float)0.5,tp->u2=(float)0.8,tp->v2=(float)0.9,tp->u3=(float)0.8,tp->v3=(float)0.5;  
    tp = &face.faces[200];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.94,tp->v1=(float)0.1,tp->u2=(float)0.94,tp->v2=(float)0.5,tp->u3=(float)0.8,tp->v3=(float)0.5;  
    tp = &face.faces[201];  
    tp->maped=TRUE,tp->map=3,tp->u1=(float)0.94,tp->v1=(float)0.1,tp->u2=(float)0.8,tp->v2=(float)0.5,tp->u3=(float)0.8,tp->v3=(float)0.1;  
*/   
    MKScaleMatrix(&m,&pot2,3);   
    p1.scalePoints(&m,CONVERTALL);   
    groundLight = 12;   
    spotLight = 19;   
    WORD f16,f8,f4,f2,f1,w16,w8,w4,w2,w1;   
    WORD color = 0xe400;   
    WORD color1 = 0xffff - color;   
/*  f16 = (color&0x8610)>>4;  
    f8 = (color&0xc718)>>3;  
    f4 = (color&0xe79c)>>2;  
    f2 = (color&0xf7de)>>1;     
    f1 = color;  
    w16 = (color1&0x8610)>>4;  
    w8 = (color1&0xc718)>>3;  
    w4 = (color1&0xe79c)>>2;  
    w2 = (color1&0xf7de)>>1;    
    w1 = color1;*/   
    f1 = color;   
    color = (color&0xf7de)>>1;   
    f2 = color;   
    color = ((color+0x0821)&0xf7de)>>1;   
    f4 = color;   
    color = ((color+0x0821)&0xf7de)>>1;   
    f8 = color;   
    color = ((color+0x0821)&0xf7de)>>1;   
    f16 = color;   
    w1 = color1;   
    color1 = (color1&0xf7de)>>1;   
    w2 = color1;   
    color1 = ((color1+0x0821)&0xf7de)>>1;   
    w4 = color1;   
    color1 = ((color1+0x0821)&0xf7de)>>1;   
    w8 = color1;   
    color1 = ((color1+0x0821)&0xf7de)>>1;   
    w16 = color1;   
    lightLevel[0] = 0;   
    lightLevel[1] = f16;   
    lightLevel[2] = f8;   
    lightLevel[3] = f8+f16;   
    lightLevel[4] = f4;   
    lightLevel[5] = f4+f16;   
    lightLevel[6] = f4+f8;   
    lightLevel[7] = f4+f8+f16;   
    lightLevel[8] = f2;   
    lightLevel[9] = f2+f16;   
    lightLevel[10] = f2+f8;   
    lightLevel[11] = f2+f8+f16;   
    lightLevel[12] = f2+f4;   
    lightLevel[13] = f2+f4+f16;   
    lightLevel[14] = f2+f4+f8;   
    lightLevel[15] = f2+f4+f8+f16;   
    lightLevel[16] = f1;   
    lightLevel[17] = f1+w16;   
    lightLevel[18] = f1+w8;   
    lightLevel[19] = f1+w8+w16;   
    lightLevel[20] = f1+w4;   
    lightLevel[21] = f1+w4+w16;   
    lightLevel[22] = f1+w4+w8;   
    lightLevel[23] = f1+w4+w8+w16;   
    lightLevel[24] = f1+w2;   
    lightLevel[25] = f1+w2+w16;   
    lightLevel[26] = f1+w2+w8;   
    lightLevel[27] = f1+w2+w8+w16;   
    lightLevel[28] = f1+w2+w4;   
    lightLevel[29] = f1+w2+w4+w16;   
    lightLevel[30] = f1+w2+w4+w8;   
    lightLevel[31] = f1+w2+w4+w8+w16;   
    lightLevel[32] = f1+w1;   
    vLight.x = -1,vLight.y = -1, vLight.z = -1;   
    potLight.x = 400,potLight.y = 000,potLight.z = 600;   
    vector3dToSTD(&vLight);   
    VECTOR3D v={0,0,-1};   
    VECTOR3D v1={1,4,3};   
    pot.x = -20,pot.y = 150,pot.z = 750;    
    pot1.x = -20,pot1.y = 0,pot1.z = 0;   
    MKRotateMatrix(&m4,&pot1,&v1,3.1416/120);   
    MKWCtoVcMatrix(&m1,&v,&pot);   
    MKProjectionMatrix(&m2,550,320,240);   
    matrixMul(&m2,&m1);   
    return TRUE;   
}   
   
/*  
 * WinMain - initialization, message loop  
 */   
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,   
                        LPSTR lpCmdLine, int nCmdShow)   
{   
    MSG         msg;   
   
    lpCmdLine = lpCmdLine;   
    hPrevInstance = hPrevInstance;   
   
    if( !doInit( hInstance, nCmdShow ) )   
    {   
        return FALSE;   
    }   
   
    while (GetMessage(&msg, NULL, 0, 0))   
    {   
        TranslateMessage(&msg);   
        DispatchMessage(&msg);   
    }   
   
    return msg.wParam;   
   
} /* WinMain */   
   
unsigned mrand(unsigned range)   
{   long l = rand();   
    return l*range/0x7fff;   
}   
   
void lineDemo1(DDSURFACEDESC* ddsd)   
{   static int l=0,c=0;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
        rrr+=1;ggg-=2;bbb+=3;   
    for(int i=l;i<640;i+=12)   
    {   line(ddsd,i,0,639-i,479,color);   
    }   
    for(i=11-l;i<480;i+=12)   
    {   line(ddsd,0,i,639,479-i,color);   
    }   
    l++;   
    if(l == 11)   
        l = 0;   
}   
   
void lineDemo2(DDSURFACEDESC* ddsd)   
{   static int l=0;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
        rrr+=1;ggg-=2;bbb+=3;   
    for(int i=l;i<640;i+=12)   
    {   lineS(ddsd,i,0,639-i,479,color);   
    }   
    for(i=11-l;i<480;i+=12)   
    {   lineS(ddsd,0,i,639,479-i,color);   
    }   
    l++;   
    if(l == 11)   
        l = 0;   
}   
   
void circleDemo1(DDSURFACEDESC* ddsd)   
{   static int l = 0;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
        rrr+=1;ggg-=2;bbb+=3;   
    for(int i=l;i<400;i+=10)   
    {   circle(ddsd,320,240,i,color);   
    }                  
    l++;   
    if(l == 10)   
        l=0;   
}   
   
void circleDemo2(DDSURFACEDESC* ddsd)   
{   static int l = 100;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
        rrr+=1;ggg-=2;bbb+=3;   
    circleFill(ddsd,320,240,l,color);   
    l++;   
    if(l>350)   
        l=100;   
}   
   
void ellipseDemo1(DDSURFACEDESC* ddsd)   
{   static int l = 0;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
        rrr-=1;ggg-=2;bbb+=3;   
    for(int i=l;i<350;i+=20)   
    {   ellipse(ddsd,240-i,230-i,400+i,250+i,color);   
        ellipse(ddsd,310-i,180-i,330+i,300+i,color);   
    }                  
    l++;   
    if(l == 20)   
        l=0;   
}   
   
void ellipseDemo2(DDSURFACEDESC* ddsd)   
{   static int l = 0,i=4;   
    static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
    rrr+=1;ggg-=2;bbb+=3;   
    ellipseFill(ddsd,240-l,230-l,400+l,250+l,color);               
    l+=i;   
    if(l >= 250)   
        i=-4;   
    if(l <= -280)   
        i=4;   
}   
   
void rectDemo1(DDSURFACEDESC* ddsd)   
{   static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
    rrr+=2;ggg+=1;bbb+=2;   
    static int i[9][6]={{10,13,110,43,1,1},   
                        {45,61,95,111,-1,1},   
                        {230,30,300,200,1,-1},   
                        {320,350,540,420,-1,-1},   
                        {140,300,220,380,-1,1},   
                        {420,20,620,220,1,-1},   
                        {430,30,460,80,-1,-1},   
                        {540,180,600,200,-1,1},   
                        {430,100,470,140,1,1}   
                        };   
    for(int l=0;l<6;l++)   
    {   i[l][0]+=i[l][4];   
        i[l][2]+=i[l][4];   
        i[l][1]+=i[l][5];   
        i[l][3]+=i[l][5];   
        drawRect(ddsd,i[l][0],i[l][1],i[l][2],i[l][3],color);   
    }   
    for(l=6;l<9;l++)   
    {   i[l][0]+=i[l][4]+i[5][4];   
        i[l][2]+=i[l][4]+i[5][4];   
        i[l][1]+=i[l][5]+i[5][5];   
        i[l][3]+=i[l][5]+i[5][5];   
        drawRect(ddsd,i[l][0],i[l][1],i[l][2],i[l][3],color);   
    }   
    for(l = 0;l<6;l++)   
    {   if( i[l][0]== 0||i[l][0]==639||i[l][2]==0||i[l][2]==639)   
        {   i[l][4] = -i[l][4];   
            if(i[l][1] == 0||i[l][1]==479||i[l][3]==0||i[l][3]==479)   
                i[l][5]= -i[l][5];   
            continue;   
        }   
        if(i[l][1] == 0||i[l][1]==479||i[l][3]==0||i[l][3]==479)   
        {   i[l][5]= -i[l][5];   
            continue;   
        }   
        for(int j=0;j<6;j++)   
        {   if(l!=j)   
            {   if(i[l][1]<=i[j][3]&&i[l][3]>=i[j][1])   
                if(i[l][0]==i[j][2]||i[l][0]+i[l][4]==i[j][2]||i[l][2]==i[j][0]||i[l][2]+i[l][4]==i[j][0])   
                {   i[l][4]=-i[l][4];   
                    break;   
                }   
            }   
        }   
        for(j=0;j<6;j++)   
        {   if(l!=j)   
            {   if(i[l][0]<=i[j][2]&&i[l][2]>=i[j][0])   
                if(i[l][1]==i[j][3]||i[l][1]+i[l][5]==i[j][3]||i[l][3]==i[j][1]||i[l][3]+i[l][5]==i[j][1])   
                {   i[l][5]=-i[l][5];   
                    break;   
                }   
            }   
        }   
    }   
    for(l = 6;l<9;l++)   
    {   for(int j=5;j<9;j++)   
        {   if(l!=j)   
            {   if(i[l][1]<=i[j][3]&&i[l][3]>=i[j][1])   
                if(i[l][0]==i[j][2]||i[l][0]+i[l][4]==i[j][2]||i[l][2]==i[j][0]||i[l][2]+i[l][4]==i[j][0]||i[l][0]==i[j][0]||i[l][0]+i[l][4]==i[j][0]||i[l][2]==i[j][2]||i[l][2]+i[l][4]==i[j][2])   
                {   i[l][4]=-i[l][4];   
                    break;   
                }   
            }   
        }   
        for(j=5;j<9;j++)   
        {   if(l!=j)   
            {   if(i[l][0]<=i[j][2]&&i[l][2]>=i[j][0])   
                if(i[l][1]==i[j][3]||i[l][1]+i[l][5]==i[j][3]||i[l][3]==i[j][1]||i[l][3]+i[l][5]==i[j][1]||i[l][1]==i[j][1]||i[l][1]+i[l][5]==i[j][1]||i[l][3]==i[j][3]||i[l][3]+i[l][5]==i[j][3])   
                {   i[l][5]=-i[l][5];   
                    break;   
                }   
            }   
        }   
    }   
}   
   
void rectDemo2(DDSURFACEDESC* ddsd)   
{   static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
    rrr+=2;ggg+=1;bbb+=2;   
    static int i[9][6]={{10,13,110,43,1,1},   
                        {45,61,95,111,-1,1},   
                        {230,30,300,200,1,-1},   
                        {320,350,540,420,-1,-1},   
                        {140,300,220,380,-1,1},   
                        {420,20,620,220,1,-1},   
                        {430,30,460,80,-1,-1},   
                        {540,180,600,200,-1,1},   
                        {430,100,470,140,1,1}   
                        };   
    for(int l=0;l<6;l++)   
    {   i[l][0]+=i[l][4];   
        i[l][2]+=i[l][4];   
        i[l][1]+=i[l][5];   
        i[l][3]+=i[l][5];   
        rectFill(ddsd,i[l][0],i[l][1],i[l][2],i[l][3],color);   
        color+=0x39e7;   
    }   
    for(l = 0;l<6;l++)   
    {   if( i[l][0]== 0||i[l][0]==639||i[l][2]==0||i[l][2]==639)   
        {   i[l][4] = -i[l][4];   
            if(i[l][1] == 0||i[l][1]==479||i[l][3]==0||i[l][3]==479)   
                i[l][5]= -i[l][5];   
            continue;   
        }   
        if(i[l][1] == 0||i[l][1]==479||i[l][3]==0||i[l][3]==479)   
        {   i[l][5]= -i[l][5];   
            continue;   
        }   
        for(int j=0;j<6;j++)   
        {   if(l!=j)   
            {   if(i[l][1]<=i[j][3]&&i[l][3]>=i[j][1])   
                if(i[l][0]==i[j][2]||i[l][0]+i[l][4]==i[j][2]||i[l][2]==i[j][0]||i[l][2]+i[l][4]==i[j][0])   
                {   i[l][4]=-i[l][4];   
                    break;   
                }   
            }   
        }   
        for(j=0;j<6;j++)   
        {   if(l!=j)   
            {   if(i[l][0]<=i[j][2]&&i[l][2]>=i[j][0])   
                if(i[l][1]==i[j][3]||i[l][1]+i[l][5]==i[j][3]||i[l][3]==i[j][1]||i[l][3]+i[l][5]==i[j][1])   
                {   i[l][5]=-i[l][5];   
                    break;   
                }   
            }   
        }   
    }   
}   
   
void polygonDemo1(DDSURFACEDESC* ddsd)   
{   static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
    rrr+=2;ggg+=1;bbb+=2;   
    static POINT points[5]={    {340,80},   
                        {490,180},   
                        {400,300},   
                        {250,330},   
                        {120,200}   
                    };   
    static int range[5][6]={    {300,60,360,120,1,1},   
                        {440,110,600,210,-1,1},   
                        {380,270,460,380,-1,1},   
                        {200,280,320,440,-1,-1},   
                        {100,150,200,260,1,-1}   
                    };   
    for(int i=0;i<5;i++)   
    {   points[i].x += range[i][4];   
        points[i].y += range[i][5];   
        if(points[i].x <= range[i][0]||points[i].x >= range[i][2])   
            range[i][4] = -range[i][4];   
        if(points[i].y <= range[i][1]||points[i].y >= range[i][3])   
            range[i][5] = -range[i][5];   
    }   
    drawPolygon(ddsd,points,5,color);   
}   
   
void polygonDemo2(DDSURFACEDESC* ddsd)   
{   static unsigned char rrr=0,ggg=80,bbb=160;   
    WORD color=RGBto16bit(&ddsd->ddpfPixelFormat,rrr,ggg,bbb);   
    rrr+=2;ggg+=1;bbb+=2;   
    static POINT points[5]={    {320,100},   
                        {100,250},   
                        {210,440},   
                        {500,390},   
                        {500,150}   
                    };   
    static int range[5][6]={    {210,-50,400,120,1,1},   
                                {-50,130,210,380,-1,1},   
                                {150,410,280,540,1,-1},   
                                {460,360,560,460,-1,-1},   
                                {480,130,700,260,1,-1}   
                            };   
    for(int i=0;i<5;i++)   
    {   points[i].x += range[i][4];   
        points[i].y += range[i][5];   
        if(points[i].x <= range[i][0]||points[i].x >= range[i][2])   
            range[i][4] = -range[i][4];   
        if(points[i].y <= range[i][1]||points[i].y >= range[i][3])   
            range[i][5] = -range[i][5];   
    }   
    polygonFill(ddsd,points,5,color);   
}   
   
typedef struct { float x,y; }Complex;   
inline Complex complexSquare(Complex c)   
{   Complex cSq;   
    cSq.x = c.x*c.x - c.y*c.y;   
    cSq.y = 2*c.x*c.y;   
    return cSq;   
}   
   
inline int iterate(Complex zInit,int maxIter)   
{   Complex z=zInit;   
    int cnt = 0;   
    while((z.x*z.x+z.y*z.y<=4.0)&&(cnt < maxIter))   
    {   z = complexSquare(z);   
        z.x += zInit.x;   
        z.y += zInit.y;   
        cnt++;   
    }   
    return cnt;   
}   
   
void fsDemo1(DDSURFACEDESC* ddsd)   
{   int nx,ny,maxIter;   
    WORD color;   
    static int l=0;   
    float realMin,realMax,imagMin,imagMax;   
    static int c[6][2]={    {0xf81f,-0x0800},{0x001f,0x0040},{0x07ff,-0x0001},   
                    {0x07e0,0x0800},{0xffe0,-0x0040},{0xf800,0x0001}   
                };   
    nx = 640,ny = 480,maxIter = 186;   
    realMin = -0.78937,realMax=-0.78928,imagMin=0.16434,imagMax=0.16443;   
    //realMin = -0.790,realMax=-0.788,imagMin=0.161,imagMax=0.163;   
    float realInc = (realMax - realMin)/nx;   
    float imagInc = (imagMax - imagMin)/ny;   
    Complex z;   
    int x,y;   
    int cnt;   
    //for(x=l,z.x=realMin+realInc*l;x<nx;x+=201,z.x+=realInc*201)   
    x=l,z.x=realMin+realInc*l;   
        for(y=0,z.y=imagMin;y<ny;y++,z.y+=imagInc)   
        {   cnt = iterate(z,maxIter);   
            if(cnt == maxIter)   
                writePixel(ddsd,x,y,0x0000);   
            else   
            {   color = c[cnt/31][0] + c[cnt/31][1]*(cnt%31);   
                writePixel(ddsd,x,y,color);   
            }   
        }   
    l++;   
    if(l>=640)   
        l = 0;   
}   
   
void demo3d(DDSURFACEDESC* ddsd)   
{   DDSURFACEDESC       ddsdZBuf;   
    ddsdZBuf.dwSize = sizeof(ddsdZBuf);   
    HRESULT ddrval;   
    UINT i;   
    p1.rotatePoints(&m4,CONVERTALL);   
    face.updataParams(&p1,CONVERTALL);   
    p1.clearConvert();   
    face.clearConvert();   
    face.judgeSide(&p1,&pot);   
//  face.calLight(&vLight,groundLight,spotLight,CONVERTSIGNED);   
//  face.calLightSpot(&potLight,&p1,groundLight,spotLight-4,CONVERTSIGNED);   
    face.calLightMirror(&potLight,&p1,groundLight,spotLight,CONVERTSIGNED);   
    p2.copy(&p1);   
    p2.convertPoints(&m2,CONVERTSIGNED);   
    while ((ddrval=lpDDSZBuffer->Lock(NULL, &ddsdZBuf, 0, NULL)) == DDERR_WASSTILLDRAWING)   
                ;   
    if(ddrval == DDERR_SURFACELOST )   
    {   lpDDSZBuffer->Restore();   
        return;   
    }   
    for(i=0;i<face.topPos;i++)   
    {      
        if(face.shouldConvert[i])   
        {// fillTriangle3dWithZ(ddsd,&ddsdZBuf,&p2,&face.faces[i],lightLevel[face.faces[i].light]);   
            fillTriangle3dWithZLT(ddsd,&ddsdZBuf,&p2,&face.faces[i],face.faces[i].light);   
        //  drawTriangle3dWithT1(ddsd,&p2,&face.faces[i],0xffff);   
        }   
    }   
    lpDDSZBuffer->Unlock(NULL);   
}   
   
void demo3d2(DDSURFACEDESC* ddsd)   
{   DDSURFACEDESC       ddsdZBuf;   
    ddsdZBuf.dwSize = sizeof(ddsdZBuf);   
    HRESULT ddrval;   
    UINT i;   
    static UINT ii=0;   
    ii++;   
    if((ii/110)%2 == 0)   
        p3.rotatePoints(&m5,CONVERTALL);   
    else   
        p3.rotatePoints(&m8,CONVERTALL);   
    face1.updataParams(&p3,CONVERTALL);   
    p3.clearConvert();   
    face1.clearConvert();   
    face1.judgeSide(&p3,&pot2);   
    face1.calLight(&vLight,groundLight,spotLight,CONVERTSIGNED);   
    p4.copy(&p3);   
    p4.convertPoints(&m7,CONVERTSIGNED);   
//  while ((ddrval=lpDDSZBuffer->Lock(NULL, &ddsdZBuf, 0, NULL)) == DDERR_WASSTILLDRAWING)   
//              ;   
//  if(ddrval == DDERR_SURFACELOST )   
//  {   lpDDSZBuffer->Restore();   
//      return;   
//  }   
    //for(i=0;i<p2.topPos;i++)   
    for(i=0;i<face1.topPos;i++)   
    {   //drawTriangle3d(ddsd,&p2,&face.faces[i],0xffff);   
        if(face1.shouldConvert[i])   
        {   //fillTriangle3d(ddsd,&p2,&face.faces[i],lightLevel[face.faces[i].light]);   
        //  fillTriangle3dWithZT1(ddsd,&ddsdZBuf,&p4,&face1.faces[i],lightLevel[face1.faces[i].light]);   
            fillTriangle3dWithT1(ddsd,&p4,&face1.faces[i],lightLevel[face1.faces[i].light]);   
        //  fillTriangle3dWithT1(ddsd,&p4,&face1.faces[i],lightLevel[face1.faces[i].light]);   
        }   
    }   
//  lpDDSZBuffer->Unlock(NULL);   
}   
   
void demoEnd()   
{   static int startPosition=390, startLine = 0;   
    HDC         hdc;   
    RECT rcRect;   
    rcRect.left = 0;   
    rcRect.top = 120;   
    rcRect.right = 640;   
    rcRect.bottom = 390;   
    if (lpDDSBack->GetDC(&hdc) == DD_OK)   
    {   SetBkColor( hdc, RGB( 0, 0, 255 ) );   
        int oldBkMode = SetBkMode(hdc,TRANSPARENT);   
        int pos = startPosition;   
        for(int i=startLine;i<lines;i++)   
        {   if(pos > 390)   
                break;   
            SetTextColor( hdc, RGB( 255, 255, 0 ) );   
            //TextOut( hdc, 20,pos, txt[i], lstrlen(txt[i]) );   
             ExtTextOut(hdc,20,pos,ETO_CLIPPED,&rcRect,txt[i],lstrlen(txt[i]),0);   
            pos += 30;   
        }   
        SetBkMode(hdc,oldBkMode);   
        lpDDSBack->ReleaseDC(hdc);   
    }   
    if(startPosition > 90)   
        startPosition--;   
    else   
    {   startPosition += 30;   
        if(startLine < lines-1)   
            startLine++;   
        else   
        {   startPosition=390;   
            startLine = 0;   
        }   
    }   
}   
   
DWORD WINAPI demo(LPVOID lpparm)   
{   HDC         hdc;   
    HRESULT ddrval;   
    DDSURFACEDESC       ddsd;   
    ddsd.dwSize = sizeof(ddsd);   
    while(1){   
        if( bQuit )   
            return -1;   
            //ExitThread(-1);   
        if( bActive )   
        {      
            DDBLTFX ddbltfx;   
            ddbltfx.dwSize = sizeof(ddbltfx);   
            ddbltfx.dwFillColor = 0;   
            if(bClear){   
            lpDDSBack->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT, &ddbltfx);   
            if(phase == 11)   
            {   ddbltfx.dwFillDepth = MAX_ZDEPTH;   
                lpDDSZBuffer->Blt(NULL,NULL,NULL,DDBLT_DEPTHFILL|DDBLT_WAIT, &ddbltfx);   
            }   
            while ((ddrval = lpDDSBack->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)   
                ;   
            frames++;   
            if( ddrval == DD_OK )   
            {   switch (phase)   
                {   case 0:   
                        lineDemo1(&ddsd);   
                        break;   
                    case 1:   
                        lineDemo2(&ddsd);   
                        break;   
                    case 2:   
                        circleDemo1(&ddsd);   
                        break;   
                    case 3:   
                        circleDemo2(&ddsd);   
                        break;   
                    case 4:   
                        ellipseDemo1(&ddsd);   
                        break;   
                    case 5:   
                        ellipseDemo2(&ddsd);   
                        break;   
                    case 6:   
                        rectDemo1(&ddsd);   
                        break;   
                    case 7:   
                        rectDemo2(&ddsd);   
                        break;   
                    case 8:   
                        polygonDemo1(&ddsd);   
                        break;   
                    case 9:   
                        polygonDemo2(&ddsd);   
                        break;   
                    case 10:   
                        fsDemo1(&ddsd);   
                        break;   
                    case 11:   
                        demo3d(&ddsd);   
                        break;   
                    case 12:   
                        demo3d2(&ddsd);   
                        break;   
                    case 13:   
                    {   RECT rcRect;   
                        rcRect.left = 0;   
                        rcRect.top = 0;   
                        rcRect.right = 640;   
                        rcRect.bottom = 480;   
                        lpDDSBack->Unlock(NULL);   
                        lpDDSBack->Blt(&rcRect,lpDDSEndBmp,&rcRect, FALSE, NULL);   
                        //lpDDSBack->BltFast(0,0,lpDDSEndBmp,&rcRect,FALSE);   
                        demoEnd();   
                    }   
                        break;   
                    default:   
                        ;   
                }   
                lpDDSBack->Unlock(NULL);   
            }   
            if (lpDDSBack->GetDC(&hdc) == DD_OK)   
            {   
                SetBkColor( hdc, RGB( 0, 0, 255 ) );   
                SetTextColor( hdc, RGB( 255, 255, 0 ) );   
                TextOut( hdc, 10, 10, szFrontMsg[phase], lstrlen(szFrontMsg[phase]) );   
                TextOut( hdc, 330, 420, szMsg, lstrlen(szMsg) );   
                TextOut( hdc, 330, 450, szMsg1, lstrlen(szMsg1) );   
                if(time(NULL)>tt)   
                {   tt = time(NULL);   
                    rate = frames;   
                    frames=0;   
                }   
                char cc[40];   
                sprintf(cc,"屏幕刷新率为：%ld幅/秒",rate);   
                TextOut( hdc, 120, 10, cc, lstrlen(cc) );   
                lpDDSBack->ReleaseDC(hdc);   
            }   
            while( 1 )   
            {   
                ddrval = lpDDSPrimary->Flip( NULL, 0 );   
                if( ddrval == DD_OK )   
                {   
                    break;   
                }   
                if( ddrval == DDERR_SURFACELOST )   
                {   
                    ddrval = lpDDSPrimary->Restore();   
                    if( ddrval != DD_OK )   
                    {   
                        break;   
                    }   
                }   
                if( ddrval != DDERR_WASSTILLDRAWING )   
                {   
                    break;   
                }   
            }   
            }   
            else   
            {   while ((ddrval = lpDDSPrimary->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)   
                ;   
                frames++;   
                if( ddrval == DD_OK )   
                {   switch (phase)   
                    {   case 10:   
                            fsDemo1(&ddsd);   
                            break;   
                        default:   
                            ;   
                    }   
                    lpDDSPrimary->Unlock(NULL);   
                }   
                if (lpDDSPrimary->GetDC(&hdc) == DD_OK)   
                {   
                    SetBkColor( hdc, RGB( 0, 0, 255 ) );   
                    SetTextColor( hdc, RGB( 255, 255, 0 ) );   
                    TextOut( hdc, 10, 10, szFrontMsg[phase], lstrlen(szFrontMsg[phase]) );   
                    TextOut( hdc, 330, 420, szMsg, lstrlen(szMsg) );   
                    TextOut( hdc, 330, 450, szMsg1, lstrlen(szMsg1) );   
                    if(time(NULL)>tt)   
                    {   tt = time(NULL);   
                        rate = frames;   
                        frames=0;   
                    }   
                    char cc[40];   
                    sprintf(cc,"屏幕刷新率为：%ld幅/秒",rate);   
                    TextOut( hdc, 120, 10, cc, lstrlen(cc) );   
                    lpDDSPrimary->ReleaseDC(hdc);   
                }   
            }   
        }   
    }   
}   
   

