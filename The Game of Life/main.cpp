#include <iostream>
#include "application.h"
using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	CUserWindow::CreateWindowsClass();
	g_UserWindow = CUserWindow("The Game of Life", 960, 600);
	g_UserWindow.Show();

	MSG msg;
	msg.message = WM_NULL;
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return static_cast<INT>(msg.wParam);
}