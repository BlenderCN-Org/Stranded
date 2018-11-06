#define CREATEOPENCLDLL_EXPORTS  

#include <iostream>  
#include <fstream>  
#include <string>  
#include <vector>  
#include <sstream>  

#include "createopencldll.h"  

using namespace std;

const int KArraySize = 1000;

CreateOpenCLDLL::CreateOpenCLDLL()
{
	m_context = 0;
	m_commandQueue = 0;
	m_program = 0;
	m_device = 0;
	m_kernel = 0;
	m_memObjects[0] = 0;
	m_memObjects[1] = 0;
	m_memObjects[2] = 0;
	m_errNum = 0;
}

CreateOpenCLDLL::~CreateOpenCLDLL()
{

}

//  ѡ��ƽ̨������������  
cl_context CreateOpenCLDLL::CreateContext()
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;

	//ѡ���һ�����õ�ƽ̨    
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}

	// ����һ��opencl�����ģ��ɹ���ʹ��GUP�����ģ�����ʹ��cpu    
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)firstPlatformId,
		0
	};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
		NULL, NULL, &errNum);
	if (errNum != CL_SUCCESS)
	{
		std::cout << "Could not create GPU context, trying CPU..." << std::endl;
		context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU,
			NULL, NULL, &errNum);
		if (errNum != CL_SUCCESS)
		{
			std::cerr << "Failed to create an OpenCL GPU or CPU context." << std::endl;
			return NULL;
		}
	}

	return context;
}

//ѡ���һ�����õ��豸������һ���������  
cl_command_queue CreateOpenCLDLL::CreateCommandQueue(cl_context context, cl_device_id *device)
{
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;

	//���clGetContextInfo����豸�������Ĵ�С    
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Failed call to clGetContextInfo(...,GL_CONTEXT_DEVICES,...)";
		return NULL;
	}

	if (deviceBufferSize <= 0)
	{
		std::cerr << "No devices available.";
		return NULL;
	}

	//Ϊ�豸�����������ڴ棬���clGetContextInfo������������������п��õ��豸    
	devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	if (errNum != CL_SUCCESS)
	{
		delete[] devices;
		std::cerr << "Failed to get device IDs";
		return NULL;
	}
	char    deviceName[512];
	char    deviceVendor[512];
	char    deviceVersion[512];
	errNum = clGetDeviceInfo(devices[0], CL_DEVICE_VENDOR, sizeof(deviceVendor),
		deviceVendor, NULL);
	errNum |= clGetDeviceInfo(devices[0], CL_DEVICE_NAME, sizeof(deviceName),
		deviceName, NULL);
	errNum |= clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, sizeof(deviceVersion),
		deviceVersion, NULL);

	printf("OpenCL Device Vendor = %s,  OpenCL Device Name = %s,  OpenCL Device Version = %s\n", deviceVendor, deviceName, deviceVersion);
	// ����������У�����ֻѡ���һ�����õ��豸����ʵ�ʵĳ�������ܻ�ʹ�����п��õ��豸�����OpenCL�豸��ѯѡ��������ߵ��豸    
	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (commandQueue == NULL)
	{
		delete[] devices;
		std::cerr << "Failed to create commandQueue for device 0";
		return NULL;
	}

	*device = devices[0];
	delete[] devices;
	return commandQueue;
}

//�Ӵ��̼����ں�Դ�ļ�������һ���������  
cl_program CreateOpenCLDLL::CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
	cl_int errNum;
	cl_program program;

	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open())
	{
		std::cerr << "Failed to open file for reading: " << fileName << std::endl;
		return NULL;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	//�����������    
	program = clCreateProgramWithSource(context, 1,
		(const char**)&srcStr,
		NULL, NULL);
	if (program == NULL)
	{
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}
	//�����ں�Դ����    
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		// ����ʧ�ܿ���ͨ��clGetProgramBuildInfo��ȡ��־    
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);

		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}

	return program;
}

//�����ڴ����   
bool CreateOpenCLDLL::CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float *b)
{
	//�����ڴ����    
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float)* KArraySize, a, NULL);
	memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float)* KArraySize, b, NULL);
	memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
		sizeof(float)* KArraySize, NULL, NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
	{
		std::cerr << "Error creating memory objects." << std::endl;
		return false;
	}

	return true;
}

//�����κδ���OpenCL����Դ   
void CreateOpenCLDLL::Cleanup(cl_context context, cl_command_queue commandQueue,
	cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
	for (int i = 0; i < 3; i++)
	{
		if (memObjects[i] != 0)
			clReleaseMemObject(memObjects[i]);
	}
	if (commandQueue != 0)
		clReleaseCommandQueue(commandQueue);

	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (context != 0)
		clReleaseContext(context);
}

void CreateOpenCLDLL::Calcaute()
{
	// ����opencl�����ĺ͵�һ������ƽ̨    
	m_context = CreateContext();
	if (m_context == NULL)
	{
		std::cerr << "Failed to create OpenCL context." << std::endl;
	}

	// �ڴ�����һ����������ѡ���һ�����õ��豸������һ���������    
	m_commandQueue = CreateCommandQueue(m_context, &m_device);
	if (m_commandQueue == NULL)
	{
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// ����һ��������� HelloWorld.cl kernel source    
	m_program = CreateProgram(m_context, m_device, "HelloWorld.cl");
	if (m_program == NULL)
	{
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// �����ں�    
	m_kernel = clCreateKernel(m_program, "hello_kernel", NULL);
	if (m_kernel == NULL)
	{
		std::cerr << "Failed to create kernel" << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// ����һ�������������ں��ڴ��еĶ������ȴ������������������洢���ں������洢������    
	float result[KArraySize];
	float a[KArraySize];
	float b[KArraySize];
	for (int i = 0; i < KArraySize; i++)
	{
		a[i] = (float)i;
		b[i] = (float)(i * 2);
	}

	if (!CreateMemObjects(m_context, m_memObjects, a, b))
	{
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// �����ں˲�����ִ���ں˲����ؽ��    
	m_errNum = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_memObjects[0]);
	m_errNum |= clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &m_memObjects[1]);
	m_errNum |= clSetKernelArg(m_kernel, 2, sizeof(cl_mem), &m_memObjects[2]);
	if (m_errNum != CL_SUCCESS)
	{
		std::cerr << "Error setting kernel arguments." << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	size_t globalWorkSize[1] = { KArraySize };//��֮��������Ĵ�С    
	size_t localWorkSize[1] = { 1 };  //��֮����1    

									  // �����������ʹ�����豸ִ�е��ں��Ŷ�    

	m_errNum = clEnqueueNDRangeKernel(m_commandQueue, m_kernel, 1, NULL,
		globalWorkSize, localWorkSize,
		0, NULL, NULL);
	if (m_errNum != CL_SUCCESS)
	{
		std::cerr << "Error queuing kernel for execution." << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	std::cout << "Executed program succesfully." << std::endl;
	// Read the output buffer back to the Host    


	m_errNum = clEnqueueReadBuffer(m_commandQueue, m_memObjects[2], CL_TRUE,
		0, KArraySize * sizeof(float), result,
		0, NULL, NULL);
	if (m_errNum != CL_SUCCESS)
	{
		std::cerr << "Error reading result buffer." << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}


	//������    
	for (int i = 0; i < KArraySize; i++)
	{
		std::cout << result[i] << " ";
	}
	std::cout << std::endl;
	std::cout << "Executed program succesfully." << std::endl;
	Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);

}
/* ������������ */
CreateOpenCLDLL* GetCreateOpenCLDLL(void)
{
	CreateOpenCLDLL* pCreateOpenCLDLL = new CreateOpenCLDLL();
	return pCreateOpenCLDLL;
}