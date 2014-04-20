#include <Windows.h>
#include "resource.h"

static HWND main_dialog;

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


INT_PTR CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		main_dialog = hwndDlg;
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	}

	return FALSE;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HHOOK mouse_hook;

	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, hInstance, 0);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc);

	UnhookWindowsHookEx(mouse_hook);

	return 0;
}