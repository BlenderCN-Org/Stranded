#pragma once  
#ifndef _CREATEOPENCLDLL_H_  
#define _CREATEOPENCLDLL_H_  

#ifdef CREATEOPENCLDLL_EXPORTS  
#define CREATEOPENCLDLL_API __declspec(dllexport)   
#else  
#define CREATEOPENCLDLL_API __declspec(dllimport)   
#endif  

#ifdef __APPLE__  
#include <OpenCL/cl.h>  
#else  
#include <CL/cl.h>  
#endif  


class CREATEOPENCLDLL_API CreateOpenCLDLL
{
public:
	CreateOpenCLDLL();
	~CreateOpenCLDLL();
	//  选择平台并创建上下文    
	cl_context CreateContext();
	cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);
	cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);
	bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float *b);
	void Cleanup(cl_context context, cl_command_queue commandQueue,
		cl_program program, cl_kernel kernel, cl_mem memObjects[3]);
	void Calcaute();

private:
	cl_context m_context;
	cl_command_queue m_commandQueue;
	cl_program m_program;
	cl_device_id m_device;
	cl_kernel m_kernel;
	cl_mem m_memObjects[3];
	cl_int m_errNum;
};
/* 导出函数声明 */
extern "C"
{
	CREATEOPENCLDLL_API CreateOpenCLDLL* GetCreateOpenCLDLL(void);
	typedef CreateOpenCLDLL* (*PFNGetCreateOpenCLDLL)(void);
}

#endif