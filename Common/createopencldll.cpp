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

//  选择平台并创建上下文  
cl_context CreateOpenCLDLL::CreateContext()
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;

	//选择第一个可用的平台    
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}

	// 创建一个opencl上下文，成功则使用GUP上下文，否则使用cpu    
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

//选择第一个可用的设备并创建一个命令队列  
cl_command_queue CreateOpenCLDLL::CreateCommandQueue(cl_context context, cl_device_id *device)
{
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;

	//这个clGetContextInfo获得设备缓冲区的大小    
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

	//为设备缓冲区分配内存，这个clGetContextInfo用来获得上下文中所有可用的设备    
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
	// 在这个例子中，我们只选择第一个可用的设备。在实际的程序，你可能会使用所有可用的设备或基于OpenCL设备查询选择性能最高的设备    
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

//从磁盘加载内核源文件并创建一个程序对象  
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
	//创建程序对象    
	program = clCreateProgramWithSource(context, 1,
		(const char**)&srcStr,
		NULL, NULL);
	if (program == NULL)
	{
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}
	//编译内核源代码    
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		// 编译失败可以通过clGetProgramBuildInfo获取日志    
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

//创建内存对象   
bool CreateOpenCLDLL::CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float *b)
{
	//创建内存对象    
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

//清理任何创建OpenCL的资源   
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
	// 创建opencl上下文和第一个可用平台    
	m_context = CreateContext();
	if (m_context == NULL)
	{
		std::cerr << "Failed to create OpenCL context." << std::endl;
	}

	// 在创建的一个上下文中选择第一个可用的设备并创建一个命令队列    
	m_commandQueue = CreateCommandQueue(m_context, &m_device);
	if (m_commandQueue == NULL)
	{
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// 创建一个程序对象 HelloWorld.cl kernel source    
	m_program = CreateProgram(m_context, m_device, "HelloWorld.cl");
	if (m_program == NULL)
	{
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// 创建内核    
	m_kernel = clCreateKernel(m_program, "hello_kernel", NULL);
	if (m_kernel == NULL)
	{
		std::cerr << "Failed to create kernel" << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	// 创建一个将用作参数内核内存中的对象。首先创建将被用来将参数存储到内核主机存储器阵列    
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

	// 设置内核参数、执行内核并读回结果    
	m_errNum = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_memObjects[0]);
	m_errNum |= clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &m_memObjects[1]);
	m_errNum |= clSetKernelArg(m_kernel, 2, sizeof(cl_mem), &m_memObjects[2]);
	if (m_errNum != CL_SUCCESS)
	{
		std::cerr << "Error setting kernel arguments." << std::endl;
		Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);
	}

	size_t globalWorkSize[1] = { KArraySize };//让之等于数组的大小    
	size_t localWorkSize[1] = { 1 };  //让之等于1    

									  // 利用命令队列使将在设备执行的内核排队    

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


	//输出结果    
	for (int i = 0; i < KArraySize; i++)
	{
		std::cout << result[i] << " ";
	}
	std::cout << std::endl;
	std::cout << "Executed program succesfully." << std::endl;
	Cleanup(m_context, m_commandQueue, m_program, m_kernel, m_memObjects);

}
/* 导出函数定义 */
CreateOpenCLDLL* GetCreateOpenCLDLL(void)
{
	CreateOpenCLDLL* pCreateOpenCLDLL = new CreateOpenCLDLL();
	return pCreateOpenCLDLL;
}