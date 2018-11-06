#pragma once  
#ifdef CREATEOPENCVDLL_EXPORTS  
#define CREATEOPENCVDLL_API __declspec(dllexport)   
#else  
#define CREATEOPENCVDLL_API __declspec(dllimport)   
#endif  

#include "createopencldll.h"  

#include "opencv2\opencv_modules.hpp"  
#include "opencv2\highgui\highgui.hpp"  
#include "opencv2\core\core.hpp"  

using namespace cv;

class CREATEOPENCVDLL_API CreateOpenCVDLL
{
public:
	CreateOpenCVDLL();
	~CreateOpenCVDLL();
	void showImage();

private:
	Mat m_image;
};
