#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 帧缓冲函数，窗口大小改变被调用
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main(int argc, char *argv[])
{
	// 初始化 GLFW
	glfwInit();
	// 配置 GLFW
	// 主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// 次版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 核心模式(Core-profile
	glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 创建窗口对象
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// 通知 GLFW 将窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);

	// 初始化GLAD. 加载系统相关opengl函数指针地址的函数
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 设置视口的维度 前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度（像素）
	glViewport(0, 0, 800, 600);

	// 注册 当窗口被第一次显示的时候也会被调用。对于视网膜(Retina)显示屏，width和height都会明显比原输入值更高一点。
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 渲染循环
	// 检查GLFW是否被要求退出
	while (!glfwWindowShouldClose(window))
	{
		// 设置清空屏幕所用的颜色
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// 清空屏幕的颜色缓冲. 缓冲位(Buffer Bit) GL_COLOR_BUFFER_BIT，GL_DEPTH_BUFFER_BIT和GL_STENCIL_BUFFER_BIT
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// 交换颜色缓冲
		glfwSwapBuffers(window);
		// 检查触发事件，更新窗口状态，调用回调
		glfwPollEvents();
	}

	// 释放/删除之前的分配的所有资源
	glfwTerminate();
	return 0;
}
