#include <Windows.h>
#include <comdef.h>
#include "Window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try {
		Window wnd(hInstance, nCmdShow, L"TauonGameEngine");
		while (wnd.ProcessMessage())
		{
			wnd.CalculateFps();
			wnd.DrawScene();
		}
	}
	catch (std::exception &exp)
	{
		MessageBoxA(NULL, exp.what(), NULL, NULL);
	}
	
	catch (_com_error &exp)
	{
		MessageBox(NULL, exp.ErrorMessage(), NULL, NULL);
	}
	return 0;
}