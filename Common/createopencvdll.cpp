#define CREATEOPENCVDLL_EXPORTS  

#include "createopencvdll.h"  

CreateOpenCVDLL::CreateOpenCVDLL()
{

}

CreateOpenCVDLL::~CreateOpenCVDLL()
{

}

void CreateOpenCVDLL::showImage()
{
	m_image = imread("1.jpg");
	namedWindow("image", CV_WINDOW_AUTOSIZE);
	imshow("image", m_image);
	cvWaitKey(0);
}