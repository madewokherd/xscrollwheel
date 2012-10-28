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

WORD get_key_flags(void)
{
	WORD result=0;
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) result |= MK_CONTROL;
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) result |= MK_LBUTTON;
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) result |= MK_RBUTTON;
	if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) result |= MK_MBUTTON;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) result |= MK_SHIFT;
	if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) result |= MK_XBUTTON1;
	if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) result |= MK_XBUTTON2;
	return result;
}

HWND find_child_window(HWND parent, LONG x, LONG y)
{
	RECT client = {0};
	HWND child;
	POINT pt;

	GetClientRect(parent, &client);

	if (x < client.left || y < client.top ||
		x >= client.right || y >= client.bottom)
		return parent;

	x -= client.left;
	y -= client.top;

	pt.x = x;
	pt.y = y;

	child = ChildWindowFromPoint(parent, pt);

	if (child == NULL || child == parent)
		return parent;

	return find_child_window(child, x, y);
}

LRESULT CALLBACK mouse_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_MOUSEWHEEL || wParam == WM_MOUSEHWHEEL)
	{
		MSLLHOOKSTRUCT *hs = (MSLLHOOKSTRUCT*)lParam;
		HWND target;
		GUITHREADINFO ti;

		target = WindowFromPoint(hs->pt);
		target = find_child_window(target, hs->pt.x, hs->pt.y);

		ti.cbSize = sizeof(ti);
		GetGUIThreadInfo(0, &ti);
		
		/* Do nothing if the right window is already focused, or the mouse is captured. */
		if (target != NULL && target != ti.hwndFocus && ti.hwndCapture == NULL)
		{
			PostMessage(target, wParam, MAKELONG(get_key_flags(), hs->mouseData >> 16),
				MAKELONG(hs->pt.x, hs->pt.y));
			return 1; /* Eat this message. */
		}
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ATOM main_class;
	HWND main_window;
	MSG msg;
	HHOOK mouse_hook;

	main_class = register_main_window_class(hInstance);

	main_window = CreateWindow((LPCTSTR)main_class, TEXT("XScrollWheel"), WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, hInstance, 0);
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(mouse_hook);

	return 0;
}