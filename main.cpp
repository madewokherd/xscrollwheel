#include <Windows.h>

LRESULT CALLBACK main_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* For now, just make a UI-less window so the app can be easily closed */
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static ATOM register_main_window_class(HINSTANCE hInstance)
{
	WNDCLASS cls;

	cls.style = 0;
	cls.lpfnWndProc = main_window_proc;
	cls.cbClsExtra = 0;
	cls.cbWndExtra = 0;
	cls.hInstance = hInstance;
	cls.hIcon = NULL;
	cls.hCursor = NULL;
	cls.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	cls.lpszMenuName = NULL;
	cls.lpszClassName = TEXT("XScrollWheelMain");

	return RegisterClass(&cls);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ATOM main_class;
	HWND main_window;
	MSG msg;

	main_class = register_main_window_class(hInstance);

	main_window = CreateWindow((LPCTSTR)main_class, TEXT("XScrollWheel"), WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}