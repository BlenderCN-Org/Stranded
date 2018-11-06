#include <windows.h>
//ǰ�ô�����ָʾ����
//����windef.h �������Ͷ���
//winnt.h ֧��Unicode�����Ͷ���
//winbase.h Kernel����
//winuser.h ʹ���߽��溯��
//wingdi.h ͼ��װ�ý��溯��


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//����DC�豸
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		rect;

	
	//�ж���Ϣ����
	switch(message)
	{
	case WM_CREATE:
		break;
	//���´�����Ϣ
	case WM_PAINT:
		//�õ��豸hDC
		hDC = BeginPaint(hwnd, &ps);

		TextOut(hDC, 200, 0, TEXT("Visual C++ ��Ϸ����"), strlen(TEXT("Visual C++ ��Ϸ����")));

		GetClientRect(hwnd, &rect);
		DrawText(hDC, szLine, -1, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

		EndPaint(hwnd, &ps);
		break;
	//�����رհ�ť�������ر���Ϣ
	case WM_CLOSE:
		if(IDYES == MessageBox(hwnd, TEXT("�Ƿ���Ľ���?"), TEXT("��Ϸ����"), MB_YESNO))
		{
			//����"ȷ��"��ť�����ٴ���
			DestroyWindow(hwnd);
		}
		break;
	//���ٴ�����Ϣ
	case WM_DESTROY:
		//�˳�����
		PostQuitMessage(0);
		break;
	default:
		//�����Windows�ڲ�Ĭ�ϵ���Ϣ������
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	static TCHAR szAppName[] = TEXT("Visual C++ Game");

	//�������ھ��
	HWND hwnd;

	MSG msg;

	//���һ��������, ע�ᴰ����, ��������, ��ʾ�����´���
	WNDCLASS wndclass;

	wndclass.cbClsExtra = 0;		//ָ�������ڴ�ռ�
	wndclass.cbWndExtra = 0;
	//ָ�����ڱ���ɫ
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	//���ù����ʽ
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//����ͼ����ʽ
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	//ָ������ʵ�����
	wndclass.hInstance = hInstance;
	//ָ�����ں�������������������
	wndclass.lpfnWndProc = WndProc;
	//����������
	wndclass.lpszClassName = szAppName;
	//�˵�
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	//ע�ᴰ����
	if(!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	//��������
	//---����---
	//��ע�ᴰ��������ơ����ڱ��⡢���ڷ�񡢴���λ�õĺ����ꡢ����λ�õ�������
	//���ڿ�ȡ����ڸ߶ȡ������ھ�������ڲ˵����������ʵ���������������
	hwnd = CreateWindow(szAppName, TEXT("Visual C++ ��Ϸ����"), WS_OVERLAPPEDWINDOW, \
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, \
						NULL, NULL, hInstance, NULL);

	//��ʾ����
	ShowWindow(hwnd, SW_SHOWNORMAL);
	//������ʾ
	UpdateWindow(hwnd);

	//������Ϣѭ��
	while(GetMessage(&msg, NULL, 0, 0))
	{
		//ת��������Ϣ
		TranslateMessage(&msg);
		//������Ϣ
		DispatchMessage(&msg);
	}

	return msg.wParam;
}