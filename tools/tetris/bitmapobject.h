//BitMapObject.h
#ifndef __BITMAPOBJECT_H__
#define __BITMAPOBJECT_H__

#include <Windows.h>

// th bitmap object
class BitMapObject
{
public:
	BitMapObject();
	~BitMapObject();

	//loads bitmap from a file
	void Load(HDC hdcCompatible, LPCTSTR lpszFilename);
	//creates a blank bitmap
	void Create(HDC hdcCompatible, int width, int height);
	//destroys bitmap and dc
	void Destroy();

	//return width
	int GetWidth();
	//return height
	int GetHeight();
	
	//converts to HDC
	operator HDC();

private:
	//memory dc
	HDC hdcMemory;
	//new bitmap
	HBITMAP hbmNewBitMap;
	//old bitmap.
	HBITMAP hbmOldBitMap;
	//with & height as integers.
	int iWidth;
	int iHeight;
};

#endif