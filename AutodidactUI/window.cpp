#include <iostream>
#include <time.h>
#include "autodidact/ui/window.h"
#include "autodidact/ui/collider.h"
#include "autodidact/ui/component.h"
#include "autodidact/ui/division.h"
#include "autodidact/ui/caption.h"
#include "extern.h"

#ifndef TIMER_PAINT
#define TIMER_PAINT 1
#endif // !TIMER_PAINT


namespace Autodidact {
	bool FilterFocus(Component* component) {
		return CheckResponse(component->response, RESPONSE_FOCUS);
	}

	Window::Window(
		HINSTANCE	hInstance,
		LPCWSTR		name
	) :
		Collider(0, 0, 800, 600),
		divisions(),
		updateQueue(),
		posStack(),
		hoverStack(),
		paint()
	{
		static LPCWSTR CLASS_NAME = [](HINSTANCE hInstance) {
			LPCWSTR name = L"_atd_ui_window_";
			WNDCLASSEX wnd;
			wnd.cbSize = sizeof(wnd);
			wnd.style = NULL;
			wnd.cbClsExtra = 0;
			wnd.cbWndExtra = 0;
			wnd.hInstance = hInstance;
			wnd.hIcon = NULL;
			wnd.hIconSm = NULL;
			wnd.hbrBackground = NULL;
			wnd.hCursor = NULL;
			wnd.lpfnWndProc = Process;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = name;

			if (!RegisterClassEx(&wnd)) {
				std::cout << "WNDCLASS ERROR:" << GetLastError() << '\n';
			}

			return name;
		}(hInstance);

		hmenu = CreateMenu();

		hwnd = CreateWindow(
			CLASS_NAME,
			name,
			WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | DS_CENTER,
			x, y, width, height,
			NULL, hmenu, hInstance, NULL);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		const int sWidth = GetSystemMetrics(SM_CXSCREEN);
		const int sHeight = GetSystemMetrics(SM_CYSCREEN);

		bufferStatic.Reset(hwnd, sWidth, sHeight, 0xfff0f0f0);
		bufferDynamic.Reset(hwnd, sWidth, sHeight, 0xff000000);

		posStack.push({ 0, 0 });

		::SetTimer(hwnd, TIMER_PAINT, 16, PaintTimerProcess);
	}

	Window::~Window() {
		bufferStatic.Release();
		::KillTimer(hwnd, TIMER_PAINT);

		DestroyMenu(hmenu);
		DestroyWindow(hwnd);
	}

	void Window::SetShow(bool show) {
		ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
	}

	void Window::SetupCaption(int height) {
		Division& dCaption = Divide(
			{ 0, 0, {0, DINT::RIGHT}, height }, 
			RESPONSE_MOUSE | RESPONSE_FOCUS | RESPONSE_RESIZE, 
			HTCAPTION
		);

		dCaption.AddComponent(new CaptionTitle(0, 0, dCaption));
		dCaption.AddComponent(new CaptionCloseButton({ 60, DINT::RIGHT }, 0, height, dCaption));
		dCaption.AddComponent(new CaptionMaximizeButton({ 120, DINT::RIGHT }, 0, height, dCaption));
		dCaption.AddComponent(new CaptionMinimizeButton({ 180, DINT::RIGHT }, 0, height, dCaption));
	}

	void Window::UpdateRect(const Gdiplus::Rect& rc) {
		updateQueue.push({ rc.X + posStack.top().x + offset, rc.Y + posStack.top().y + offset, rc.Width, rc.Height });
	}

	void Window::ForceUpdate() {
		updateMode = FULLSCREEN;
		InvalidateRect(hwnd, NULL, FALSE);
		//UpdateWindow(hwnd);
	}

	void Window::ForceUpdate(const Gdiplus::Rect& rc) {
		Gdiplus::Rect push(rc.X + posStack.top().x + offset, rc.Y + posStack.top().y + offset, rc.Width, rc.Height);
		updateQueue.push(push);
		updateMode = SINGLE;
		RECT rect{ push.X, push.Y, push.X + push.Width, push.Y + push.Height };
		InvalidateRect(hwnd, &rect, FALSE);
		//UpdateWindow(hwnd);
	}

	void Window::TryUpdate() {
		if (!updateQueue.empty()) {
			updateMode = FULLSCREEN;
			//UpdateWindow(hwnd);
			//UpdateWindow(hwnd);
		}
		
		Gdiplus::Rect rect;
		RECT rc;
		while (!updateQueue.empty()) {
			rect = updateQueue.front();
			rc = { rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height };
			InvalidateRect(hwnd, &rc, FALSE);
			updateQueue.pop();
		}
	}

	void Window::PushPos(const SPOINT& pos) {
		posStack.push(pos);
		Gdiplus::Matrix mat{ 1, 0, 0, 1, float(pos.x), float(pos.y) };
		bufferStatic.graphics->SetTransform(&mat);
	}

	void Window::TranslatePos(const SPOINT& pos) {
		SPOINT top = posStack.top();
		top.x += pos.x;
		top.y += pos.y;

		posStack.push(top);
		Gdiplus::Matrix mat{ 1, 0, 0, 1, float(top.x), float(top.y) };
		bufferStatic.graphics->SetTransform(&mat);
	}

	void Window::PopPos() {
		if (posStack.size() > 1) {
			posStack.pop();
			SPOINT top = posStack.top();
			Gdiplus::Matrix mat{ 1, 0, 0, 1, float(top.x), float(top.y) };
			bufferStatic.graphics->SetTransform(&mat);
		}
	}

	void Window::SetTimer(Component* component, UINT delay) {
		//printf("%p Creat Timer: ", component);
		if (component) {
			UINT_PTR value = ::SetTimer(hwnd, reinterpret_cast<UINT_PTR>(component), delay, ComponentTimerProcess);
			//printf("%llu Succeed!\n", value);
		}
	}

	void Window::KillTimer(Component* component) {
		if (component) {
			::KillTimer(hwnd, reinterpret_cast<UINT_PTR>(component));
		}
	}

	void Window::SetPaintTimer(Component* component, PAINTTIMERPROC process) {
		UINT_PTR ptr = reinterpret_cast<UINT_PTR>(component);

		paint[ptr] = process;
	}

	void Window::KillPaintTimer(Component* component) {
		UINT_PTR ptr = reinterpret_cast<UINT_PTR>(component);

		auto it = paint.find(ptr);
		if (it != paint.end()) {
			paint.erase(it);
		}
	}

	Division& Window::Divide(const DRECT& rc, RESPONSE response, int hit) {
		Division* division = new Division(rc.x, rc.y, rc.width, rc.height, *this, response, hit);
		divisions.push_back(std::unique_ptr<Division>(division));
		division->OnResize(width, height, NULL, NULL);
		division->UpdateBuffer();
		return *division;
	}

	SPOINT Window::CurrentPos() {
		return posStack.top();
	}

	Gdiplus::ARGB Window::GetBgColor() {
		return bgColor;
	}

	Gdiplus::Graphics* Window::GetGrpahics() {
		return bufferStatic.graphics.get();
	}

	bool Window::Focused() {
		return focused;
	}

	bool Window::Maximized() {
		return maximized;
	}

	void Window::SetFocus(Component* component) {
		if (focus == component)
			return;

		if (focus)
			focus->OnKillFocus();
		focus = component;
		focus->OnSetFocus();
	}

	void Window::KillFocus() {
		if (focus)
			focus->OnKillFocus();
		focus = nullptr;
	}

	void Window::TryKillFocus(Component* component) {
		if (focus == component) {
			focus->OnKillFocus();
			focus = nullptr;
		}
	}

	void Window::SetMinSize(int width, int height) {
		minWidth = width;
		minHeight = height;
	}

	void Window::SetMaxSize(int width, int height) {
		maxWidth = width;
		maxHeight = height;
	}

	void Window::SetSize(int width, int height) {
		SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE);
	}

	void Window::ClearHoverStack(Component* dest) {
		while (!hoverStack.empty() && hoverStack.top() != dest) {
			if (!hoverStack.top()->static_)
				hoverStack.top()->MouseLeave();
			hoverStack.pop();
		}
	}

	void Window::ClearPosStack() {
		while (!posStack.empty())
			posStack.pop();
		bufferStatic.graphics->ResetClip();
		bufferStatic.graphics->ResetTransform();
	}

	LRESULT Window::Process(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		switch (msg) {
		case WM_CREATE: {
			SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE);
			break;
		}
		case WM_ACTIVATE: {
			auto& colorTheme = pResources->colorTheme;
			COLORREF color = RGB(colorTheme.GetRed(), colorTheme.GetGreen(), colorTheme.GetBlue());
			DwmSetWindowAttribute(hwnd, 34, &color, sizeof(COLORREF));

			MARGINS m{ 0, 0, 0, 1 };
			DwmExtendFrameIntoClientArea(hwnd, &m);
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			HDC bdc = CreateCompatibleDC(hdc);
			HBITMAP bmp = CreateCompatibleBitmap(hdc, window->width, window->height);
			SelectObject(bdc, bmp);

			if (IsZoomed(hwnd)) {
				SetViewportOrgEx(hdc, window->offset, window->offset, NULL);
			}

			switch (window->updateMode) {
			case Window::FULLSCREEN: {
				window->bufferStatic.BitBlt(bdc, 0, 0, window->width, window->height, 0, 0, SRCCOPY);
				window->bufferDynamic.BitBlt(bdc, 0, 0, window->width, window->height, 0, 0, SRCPAINT);

				BitBlt(hdc, 0, 0, window->width, window->height, bdc, 0, 0, SRCCOPY);
				break;
			}
			case Window::SINGLE: {
				Gdiplus::Rect back = window->updateQueue.back();
				window->bufferStatic.BitBlt(bdc, back.X, back.Y, back.Width, back.Height, back.X, back.Y, SRCCOPY);
				window->bufferDynamic.BitBlt(bdc, back.X, back.Y, back.Width, back.Height, back.X, back.Y, SRCPAINT);
				BitBlt(hdc, 0, 0, window->width, window->height, bdc, 0, 0, SRCCOPY);
				window->updateQueue.pop();
				break;
			}
			case Window::BATCHED: {

			}
			default:
				break;
			}

			DeleteDC(bdc);
			DeleteObject(bmp);

			window->updateMode = INVALID;

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_SIZE: {
			if (window) {
				window->maximized = wparam == SIZE_MAXIMIZED;

				if (window->maximized) {
					window->offset = 9;
					window->width = LOWORD(lparam) - 18;
					window->height = HIWORD(lparam) - 18;
				}
				else
				{
					window->offset = 0;
					window->width = LOWORD(lparam);
					window->height = HIWORD(lparam);
				}

				//printf("Window Size[%d, %d]\n", window->width.real, window->height.real);

				std::queue<REPAINTSTRUCT>	erase;
				std::queue<Component*>		repaint;

				for (auto& div : window->divisions) {
					div->OnResize(window->width, window->height, window->bgColor, &erase);
				}

				Gdiplus::SolidBrush brush(window->bgColor);

				REPAINTSTRUCT* front;
				while (!erase.empty()) {
					front = &erase.front();
					brush.SetColor(front->eraseColor);
					window->bufferStatic.graphics->FillRectangle(&brush, front->eraseRect);

					repaint.push(front->component);
					erase.pop();
				}

				while (!repaint.empty()) {
					repaint.front()->UpdateBuffer();
					repaint.pop();
				}

				window->ForceUpdate();
			}
			break;
		}
		case WM_GETMINMAXINFO: {
			if (!window)
				break;
			MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lparam);
			info->ptMinTrackSize.x = window->minWidth;
			info->ptMinTrackSize.y = window->minHeight;
			info->ptMaxTrackSize.x = window->maxWidth;
			info->ptMaxTrackSize.y = window->maxHeight;
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_NCCALCSIZE: {
			if (wparam == TRUE) {
				SetWindowLong(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			}
			return FALSE;
		}
		case WM_NCLBUTTONDOWN: {
			switch (wparam) {
			case HTCAPTION:
			case HTTOP:
			case HTTOPLEFT:
			case HTTOPRIGHT:
			case HTLEFT:
			case HTRIGHT:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
			case HTBOTTOM:
				return DefWindowProc(hwnd, WM_NCLBUTTONDOWN, wparam, lparam);
				break;
			default:
				if (!window->hoverStack.empty()) {
					Component* top = window->hoverStack.top();
					if (CheckResponse(top->response, RESPONSE_CLICK)) {
						window->SetFocus(top);
						top->LButtonDown(wparam, { LOWORD(lparam), HIWORD(lparam) });
						window->TryUpdate();
						break;
					}
				}

				window->KillFocus();
			}

			break;
		}
		case WM_NCLBUTTONUP: {
			switch (wparam) {
			case HTCAPTION:
			case HTTOP:
			case HTTOPLEFT:
			case HTTOPRIGHT:
			case HTLEFT:
			case HTRIGHT:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
			case HTBOTTOM:
				return DefWindowProc(hwnd, WM_NCLBUTTONUP, wparam, lparam);
				break;
			case HTMINBUTTON:
				ShowWindow(hwnd, SW_MINIMIZE);
				break;
			case HTMAXBUTTON:
				if (IsZoomed(hwnd)) {
					ShowWindow(hwnd, SW_RESTORE);
				}
				else {
					ShowWindow(hwnd, SW_MAXIMIZE);
				}
				break;
			case HTCLOSE:
				SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				break;
			default:
				if (!window->hoverStack.empty()) {
					Component* top = window->hoverStack.top();
					if (CheckResponse(top->response, RESPONSE_CLICK)) {
						window->SetFocus(top);
						top->LButtonDown(wparam, { LOWORD(lparam), HIWORD(lparam) });
						window->TryUpdate();
						break;
					}
				}

				window->KillFocus();
			}

			break;
		}
		case WM_LBUTTONDOWN: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->LButtonDown(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			break;
		}
		case WM_LBUTTONUP: {
			if (!window->hoverStack.empty()) {
				 window->SetFocus(window->hoverStack.top());
				 window->focus->LButtonUp(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			else {
				window->KillFocus();
			}
			break;
		}
		case WM_MBUTTONDOWN: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->MButtonDown(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			break;
		}
		case WM_MBUTTONUP: {
			if (!window->hoverStack.empty()) {
				window->SetFocus(window->hoverStack.top());
				window->focus->MButtonUp(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			else {
				window->KillFocus();
			}
			break;
		}
		case WM_RBUTTONDOWN: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->RButtonDown(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			break;
		}
		case WM_RBUTTONUP: {
			if (!window->hoverStack.empty()) {
				window->SetFocus(window->hoverStack.top());
				window->focus->RButtonUp(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			else {
				window->KillFocus();
			}
			break;
		}
		case WM_MOUSEMOVE: {
			if (!window->tracking) {
				window->tracking = true;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->MouseMove(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			break;
		}
		case WM_MOUSEWHEEL: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->MouseWheelV(wparam);
			}
			break;
		}
		case WM_MOUSEHWHEEL: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->MouseWheelH(wparam);
			}
			break;
		}
		case WM_CHAR: {
			if (window->focus) {
				window->focus->CharInput(wchar_t(wparam));
			}
			break;
		}
		case WM_KEYDOWN: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->KeyDown(wparam, lparam);
			}
			break;
		}
		case WM_KEYUP: {
			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->KeyUp(wparam, lparam);
			}
			break;
		}
		case WM_NCMOUSEMOVE: {
			if (!window->tracking) {
				window->tracking = true;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}

			if (!window->hoverStack.empty()) {
				window->hoverStack.top()->MouseMove(wparam, { LOWORD(lparam), HIWORD(lparam) });
			}
			break;
		}
		case WM_MOUSELEAVE: {
			window->tracking = false;
			Component* top;
			while (window->hoverStack.size() && (top = window->hoverStack.top())->IsClient()) {
				if (!top->static_ && CheckResponse(top->response, RESPONSE_MOUSE)) {
					top->MouseLeave();
				}
				window->hoverStack.pop();
			}

			window->TryUpdate();
			break;
		}
		case WM_NCMOUSELEAVE: {
			window->tracking = false;
			Component* top;
			while (window->hoverStack.size() && !(top = window->hoverStack.top())->IsClient()) {
				if (!top->static_ && CheckResponse(top->response, RESPONSE_MOUSE)) {
					top->MouseLeave();
				}
				window->hoverStack.pop();
			}

			window->TryUpdate();
			break;
		}
		case WM_NCHITTEST: {
			POINT pt = { LOWORD(lparam), HIWORD(lparam) };
			ScreenToClient(hwnd, &pt);

			RECT rc;
			GetClientRect(hwnd, &rc);

			if (!window->maximized) {
				USHORT row = 1;
				USHORT col = 1;

				if (pt.y >= rc.top && pt.y < rc.top + 4) {
					row = 0;
				}
				else if (pt.y < rc.bottom && pt.y >= rc.bottom - 8) {
					row = 2;
				}

				if (pt.x >= rc.left && pt.x < rc.left + 8) {
					col = 0;
				}
				else if (pt.x < rc.right && pt.x >= rc.right - 8) {
					col = 2;
				}

				LRESULT hitTests[3][3] = {
				{ HTTOPLEFT,    HTCLIENT,    HTTOPRIGHT },
				{ HTLEFT,       HTCLIENT ,     HTRIGHT },
				{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
				};

				LRESULT hit = hitTests[row][col];

				if (hit != HTCLIENT) {
					while (!window->hoverStack.empty()) {
						Component* top = window->hoverStack.top();
						if (top->response & RESPONSE_MOUSE) {
							top->MouseLeave();
						}
						window->hoverStack.pop();
					}
					return hit;
				}
			}

			std::queue<Component*> hitted;
			Component* top;
			while (!window->hoverStack.empty()) {
				top = window->hoverStack.top();
				if (top->GetHittedComponents(pt, hitted)) {

					if (top == hitted.front()) {
						hitted.pop();
					}

					break;
				}

				if (top->enable && !top->static_ && CheckResponse(top->response, RESPONSE_MOUSE)) {
					top->MouseLeave();
				}
				window->hoverStack.pop();
			}

			if (window->hoverStack.empty()) {
				for (auto& division : window->divisions) {
					if (division->GetHittedComponents(pt, hitted)) {
						break;
					}
				}
			}

			Component* front;
			while (!hitted.empty()) {
				front = hitted.front();
				hitted.pop();

				if (CheckResponse(front->response, RESPONSE_MOUSE)) {
					front->MouseEnter();
				}

				window->hoverStack.push(front);
			}

			window->TryUpdate();

			if (!window->hoverStack.empty()) {
				return window->hoverStack.top()->GetHitResult();
			}
			else {
				return HTCLIENT;
			}
		}
		case WM_SETFOCUS: {
			if (window) {
				window->focused = true;

				auto& colorTheme = pResources->colorTheme;
				COLORREF color = RGB(colorTheme.GetRed(), colorTheme.GetGreen(), colorTheme.GetBlue());

				std::queue<Component*> update;
				for (auto& division : window->divisions) {
					division->GetUpdateList(FilterFocus, update);
				}

				while (!update.empty()) {
					update.front()->UpdateBuffer();
					update.pop();
				}

				window->TryUpdate();
			}
			break;
		}
		case WM_KILLFOCUS: {
			if (window) {
				window->focused = false;

				COLORREF color = 0x808080;
				DwmSetWindowAttribute(hwnd, 34, &color, sizeof(COLORREF));

				std::queue<Component*> update;
				for (auto& division : window->divisions) {
					division->GetUpdateList(FilterFocus, update);
				}

				while (!update.empty()) {
					update.front()->UpdateBuffer();
					update.pop();
				}

				window->TryUpdate();
			}
			break;
		}
		case WM_SETCURSOR:
			if (LOWORD(lparam) == HTCLIENT) {
				SetCursor(iCursor);
				return TRUE;
			}
			else {
				return DefWindowProc(hwnd, WM_SETCURSOR, wparam, lparam);
			}
		case WM_ERASEBKGND: {
			return TRUE;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

	void Window::ComponentTimerProcess(HWND, UINT, UINT_PTR ptr, DWORD) {
		reinterpret_cast<Component*>(ptr)->OnTimer();
	}

	void Window::PaintTimerProcess(HWND hwnd, UINT, UINT_PTR, DWORD) {
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		window->bufferDynamic.Clear();

		if (!window->paint.empty()) {
			Gdiplus::Graphics& graphics = *window->bufferDynamic.graphics;
			for (auto& pair : window->paint) {
				Component* componenet = reinterpret_cast<Component*>(pair.first);
				Gdiplus::Matrix mat(1, 0, 0, 1, float(componenet->globeX), float(componenet->globeY));
				graphics.SetTransform(&mat);
				(reinterpret_cast<Component*>(pair.first)->*pair.second)(graphics);
			}

			graphics.ResetTransform();
			window->TryUpdate();
		}
		else if (window->updateMode) {
			UpdateWindow(hwnd);
		}
	}
}