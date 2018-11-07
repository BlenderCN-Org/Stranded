#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// ֡���庯�������ڴ�С�ı䱻����
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
	// ��ʼ�� GLFW
	glfwInit();
	// ���� GLFW
	// ���汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// �ΰ汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// ����ģʽ(Core-profile
	glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// �������ڶ���
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// ֪ͨ GLFW �����ڵ�����������Ϊ��ǰ�̵߳���������
	glfwMakeContextCurrent(window);

	// ��ʼ��GLAD. ����ϵͳ���opengl����ָ���ַ�ĺ���
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// �����ӿڵ�ά�� ǰ�����������ƴ������½ǵ�λ�á��������͵��ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶ȣ����أ�
	glViewport(0, 0, 800, 600);

	// ע�� �����ڱ���һ����ʾ��ʱ��Ҳ�ᱻ���á���������Ĥ(Retina)��ʾ����width��height�������Ա�ԭ����ֵ����һ�㡣
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// ��Ⱦѭ��
	// ���GLFW�Ƿ�Ҫ���˳�
	while (!glfwWindowShouldClose(window))
	{
		// ���������Ļ���õ���ɫ
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// �����Ļ����ɫ����. ����λ(Buffer Bit) GL_COLOR_BUFFER_BIT��GL_DEPTH_BUFFER_BIT��GL_STENCIL_BUFFER_BIT
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// ������ɫ����
		glfwSwapBuffers(window);
		// ��鴥���¼������´���״̬�����ûص�
		glfwPollEvents();
	}

	// �ͷ�/ɾ��֮ǰ�ķ����������Դ
	glfwTerminate();
	return 0;
}
