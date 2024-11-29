#include <iostream>

#include "autodidact/ui/division.h"
#include "autodidact/ui/window.h"

namespace Autodidact {
	Division::Division(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Window&		window,
		RESPONSE	response,
		int			hit,
		HINSTANCE	hInstance
	) :
		ListComponent(x, y, width, height, window, response, hit)
	{
		static LPCWSTR CLASS_NAME = []() {
			LPCWSTR name = L"class_rgn";
			WNDCLASSEX wnd;
			wnd.cbSize = sizeof(wnd);
			wnd.style = NULL;
			wnd.cbClsExtra = 0;
			wnd.cbWndExtra = 0;
			wnd.hInstance = GetModuleHandle(0);
			wnd.hIcon = NULL;
			wnd.hIconSm = NULL;
			wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
			wnd.hCursor = NULL;
			wnd.lpfnWndProc = Process;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = name;

			RegisterClassEx(&wnd);

			return name;
		}();

		if (hInstance) {
			hmenu = CreateMenu();

			hwnd = CreateWindow(
				CLASS_NAME, NULL,
				WS_CHILD,
				x, y, width, height,
				NULL, hmenu, hInstance, NULL);
		}

		//printf("Division %p [%d, %d]\n", this, globeX, globeY);
	}

	Division::Division(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Component&	parent,
		RESPONSE	response,
		int			hit,
		HINSTANCE	hInstance
	) :
		ListComponent(x, y, width, height, parent, response, hit) {

		static LPCWSTR CLASS_NAME = []() {
			LPCWSTR name = L"_atd_ui_div";
			WNDCLASSEX wnd;
			wnd.cbSize = sizeof(wnd);
			wnd.style = NULL;
			wnd.cbClsExtra = 0;
			wnd.cbWndExtra = 0;
			wnd.hInstance = GetModuleHandle(0);
			wnd.hIcon = NULL;
			wnd.hIconSm = NULL;
			wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
			wnd.hCursor = NULL;
			wnd.lpfnWndProc = Process;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = name;

			RegisterClassEx(&wnd);

			return name;
			}();

			if (hInstance) {
				hmenu = CreateMenu();

				hwnd = CreateWindow(
					CLASS_NAME, NULL,
					WS_CHILD,
					globeX, globeY, width, height,
					NULL, hmenu, hInstance, NULL);
			}
	}

	Division::~Division() {
		if (hwnd) {
			DestroyWindow(hwnd);
			DestroyMenu(hmenu);
		}
	}

	Division& Division::Divide(const DRECT& rc, int hit) {
		Division* division = new Division(rc.x, rc.y, rc.width, rc.height, *this, hit);
		components.push_back(std::unique_ptr<Division>(division));
		division->OnResize(width, height, NULL, NULL);
		division->UpdateBuffer();
		return *division;
	}

	LRESULT Division::Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}