#include <windows.h>
//前置处理器指示命令
//包含windef.h 基本类型定义
//winnt.h 支持Unicode的类型定义
//winbase.h Kernel函数
//winuser.h 使用者界面函数
//wingdi.h 图形装置界面函数


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//定义DC设备
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		rect;

	
	//判断消息类型
	switch(message)
	{
	case WM_CREATE:
		break;
	//更新窗口消息
	case WM_PAINT:
		//得到设备hDC
		hDC = BeginPaint(hwnd, &ps);

		TextOut(hDC, 200, 0, TEXT("Visual C++ 游戏开发"), strlen(TEXT("Visual C++ 游戏开发")));

		GetClientRect(hwnd, &rect);
		DrawText(hDC, szLine, -1, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

		EndPaint(hwnd, &ps);
		break;
	//单击关闭按钮，产生关闭消息
	case WM_CLOSE:
		if(IDYES == MessageBox(hwnd, TEXT("是否真的结束?"), TEXT("游戏开发"), MB_YESNO))
		{
			//单击"确定"按钮，销毁窗口
			DestroyWindow(hwnd);
		}
		break;
	//销毁窗口消息
	case WM_DESTROY:
		//退出程序
		PostQuitMessage(0);
		break;
	default:
		//须调用Windows内部默认的消息处理函数
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	static TCHAR szAppName[] = TEXT("Visual C++ Game");

	//声明窗口句柄
	HWND hwnd;

	MSG msg;

	//设计一个窗口类, 注册窗口类, 创建窗口, 显示及更新窗口
	WNDCLASS wndclass;

	wndclass.cbClsExtra = 0;		//指定额外内存空间
	wndclass.cbWndExtra = 0;
	//指定窗口背景色
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	//设置光标样式
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//设置图标样式
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	//指定窗口实例句柄
	wndclass.hInstance = hInstance;
	//指定窗口函数，即窗口主处理函数
	wndclass.lpfnWndProc = WndProc;
	//窗口类名称
	wndclass.lpszClassName = szAppName;
	//菜单
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	//注册窗口类
	if(!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	//创建窗口
	//---参数---
	//已注册窗口类的名称、窗口标题、窗口风格、窗口位置的横坐标、窗口位置的纵坐标
	//窗口宽度、窗口高度、父窗口句柄、窗口菜单句柄、程序实例句柄、创建参数
	hwnd = CreateWindow(szAppName, TEXT("Visual C++ 游戏开发"), WS_OVERLAPPEDWINDOW, \
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, \
						NULL, NULL, hInstance, NULL);

	//显示窗口
	ShowWindow(hwnd, SW_SHOWNORMAL);
	//更新显示
	UpdateWindow(hwnd);

	//进入消息循环
	while(GetMessage(&msg, NULL, 0, 0))
	{
		//转换键盘消息
		TranslateMessage(&msg);
		//分派消息
		DispatchMessage(&msg);
	}

	return msg.wParam;
}