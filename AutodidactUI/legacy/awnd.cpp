#include "autodidact/awnd.h"
#include "autodidact//awgt.h"
#include "autodidact/acanvas.h"
#include "autodidact/acaption.h"

#include <iostream>

namespace Autodidact {
	ApplicationWindow::ApplicationWindow(
		HINSTANCE hInstance, 
		int width, 
		int height
	) : 
		hMenu(CreateMenu()), 
		hInstance(hInstance), 
		width(width), 
		height(height),
		pCaption(),
		pCanvas(),
		bGrpahics(),
		updateRects(),
		posStack()
	{

		static short counter = 0;

		wsprintf(GetWCharBuffer(), TEXT("appwnd_%hx"), counter);

		counter++;

		WNDCLASSEX wnd;
		wnd.cbSize			= sizeof(WNDCLASSEX);
		wnd.style			= NULL;
		wnd.cbClsExtra		= 0;
		wnd.cbWndExtra		= 0;
		wnd.hInstance		= hInstance;
		wnd.hIcon			= iIcon;
		wnd.hIconSm			= NULL;
		wnd.hbrBackground	= NULL;
		wnd.hCursor			= NULL;
		wnd.lpfnWndProc		= ApplicationProc;
		wnd.lpszMenuName	= NULL;
		wnd.lpszClassName	= GetWCharBuffer();

		if (!RegisterClassEx(&wnd)) {
			MessageBox(NULL, L"RegisterClassEx Failed", GetWCharBuffer(), MB_ICONERROR);
			delete this;
			return;
		}

		const int sWidth	= GetSystemMetrics(SM_CXSCREEN);
		const int sHeight	= GetSystemMetrics(SM_CYSCREEN);

		posStack.push({ 0, 0 });

		hwnd = CreateWindowEx(
			0,
			GetWCharBuffer(), appName,
			WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | DS_CENTER,
			0, 0, width, height,
			0, hMenu, hInstance, 0
		);

		HDC hdc = GetDC(hwnd);
		buffer = CreateCompatibleBitmap(hdc, sWidth, sHeight);
		ReleaseDC(hwnd, hdc);

		hdc = CreateCompatibleDC(NULL);
		SelectObject(hdc, buffer);

		bGrpahics.reset(new Gdiplus::Graphics(hdc));

		Gdiplus::SolidBrush brush(0xfff0f0f0);
		bGrpahics->FillRectangle(&brush, 0, 0, sWidth, sHeight);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		pCaption.reset(new UiCaption(hInstance, *this, width, 60));
		pCanvas.reset(new UiCanvas(hInstance, *this, 60, width, height));

		pCaption->AddWidget(new CaptionButtonClose(*this, { 60, DINT::Mode::RIGHT }, 0));
		pCaption->AddWidget(new CaptionButtonMaximize(*this, { 120, DINT::Mode::RIGHT }, 0));
		pCaption->AddWidget(new CaptionButtonMinimize(*this, { 180, DINT::Mode::RIGHT }, 0));
	}

	ApplicationWindow::~ApplicationWindow() {
		DeleteObject(buffer);
		DestroyWindow(hwnd);
		DestroyMenu(hMenu);
	}

	void ApplicationWindow::Resize(int width, int height, bool update) {
		this->width = width;
		this->height = height;

		pCanvas->OnResize(width, height);
		pCaption->OnResize(width, height);

		if (update) {
			SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOREDRAW);
		}

		InvalidateRect(hwnd, NULL, FALSE);
	}

	void ApplicationWindow::AddPage(Page* page) const {
		pCanvas->AddPage(page);
	}

	void ApplicationWindow::SelectPage(size_t i) const {
		pCanvas->SelectPage(i);
	}

	inline void ApplicationWindow::UpdateRect(const Gdiplus::Rect& rc) {
		updateRects.push_back({rc.X + posStack.top().x, rc.Y + posStack.top().y, rc.Width, rc.Height});
	}

	inline void ApplicationWindow::ForceUpdate() {
		InvalidateRect(hwnd, NULL, FALSE);
	}

	inline void ApplicationWindow::ForceUpdate(const Gdiplus::Rect& rc) {
		updateRects.push_back({ rc.X + posStack.top().x, rc.Y + posStack.top().y, rc.Width, rc.Height });
		updateMode = SINGLE_RECT;
		Gdiplus::Rect& back = updateRects.back();
		if (maximized) {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			rect.left = back.X - rect.left;
			rect.top = back.Y - rect.top;
			rect.right = rect.left + back.Width;
			rect.bottom = rect.top + back.Height;
			InvalidateRect(hwnd, &rect, FALSE);
		}
		else {
			RECT rect{ back.X, back.Y, back.X + back.Width, back.Y + back.Height };
			InvalidateRect(hwnd, &rect, FALSE);
		}
	}

	inline void ApplicationWindow::PushPos(const POINT& pos) {
		posStack.push(pos);
		Gdiplus::Matrix mat{ 1, 0, 0, 1, float(pos.x), float(pos.y) };
		bGrpahics->SetTransform(&mat);
	}

	inline void ApplicationWindow::TranslatePos(const POINT& pos) {
		POINT top = posStack.top();
		top.x += pos.x;
		top.y += pos.y;
		
		posStack.push(top);
		Gdiplus::Matrix mat{ 1, 0, 0, 1, float(top.x), float(top.y) };
		bGrpahics->SetTransform(&mat);
	}

	inline void ApplicationWindow::PopPos() {
		if (posStack.size() > 1) {
			posStack.pop();
			POINT top = posStack.top();
			Gdiplus::Matrix mat{ 1, 0, 0, 1, float(top.x), float(top.y) };
			bGrpahics->SetTransform(&mat);
		}
	}

	inline POINT ApplicationWindow::CurrentPos() {
		return posStack.top();
	}

	LRESULT ApplicationWindow::ApplicationProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		ApplicationWindow* pt = reinterpret_cast<ApplicationWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		static bool timer = false;
		switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			HDC bdc = pt->bGrpahics->GetHDC();

			if (pt->maximized) {
				RECT rc;
				GetWindowRect(hwnd, &rc);
				SetViewportOrgEx(hdc, -rc.left, -rc.top, NULL);
			}

			switch (pt->updateMode) {
			case ApplicationWindow::FULLSCREEN: {
				BitBlt(hdc, 0, 0, pt->width, pt->height, bdc, 0, 0, SRCCOPY);
				pt->updateRects.clear();
				break;
			}
			case ApplicationWindow::SINGLE_RECT: {
				Gdiplus::Rect& rect = pt->updateRects.back();
				BitBlt(hdc, rect.X, rect.Y, rect.Width, rect.Height, bdc, rect.X, rect.Y, SRCCOPY);
				pt->updateRects.pop_back();
				break;
			}
			case ApplicationWindow::RECTS: {
				for (const auto& rect : pt->updateRects) {
					BitBlt(hdc, rect.X, rect.Y, rect.Width, rect.Height, bdc, rect.X, rect.Y, SRCCOPY);
				}
				pt->updateRects.clear();
				break;
			}
			}

			pt->updateMode = ApplicationWindow::FULLSCREEN;

			pt->bGrpahics->ReleaseHDC(bdc);
			EndPaint(hwnd, &ps);
			break;
		}
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
		case WM_NCCALCSIZE: {
			if (wparam == TRUE) {
				SetWindowLong(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			}
			return FALSE;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			if (pt)
				pt->OnResize(wparam, lparam);
			break;
		case WM_SETCURSOR:
			if (LOWORD(lparam) == HTCLIENT) {
				SetCursor(iCursor);
				return TRUE;
			}
			else {
				return DefWindowProc(hwnd, WM_SETCURSOR, wparam, lparam);
			}
		case WM_GETMINMAXINFO: {
			MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lparam);
			info->ptMinTrackSize.x = 600;
			info->ptMinTrackSize.y = 500;
			break;
		}
		case WM_NCHITTEST: {
			return pt->Hittest(wparam, lparam);
		}
		case WM_COMMAND: {
			WORD id = LOWORD(wparam);
			if (id == IDOK || id == IDCANCEL)
			{
				EndDialog(hwnd, id);
				return TRUE;
			}
			return FALSE;
		}
		case WM_SETFOCUS: {
			auto& colorTheme = pResources->colorTheme;
			COLORREF color = RGB(colorTheme.GetRed(), colorTheme.GetGreen(), colorTheme.GetBlue());
			DwmSetWindowAttribute(hwnd, 34, &color, sizeof(COLORREF));
			if (pt == nullptr) {
				break;
			}

			pt->pCaption->SetFocus();
			break;
		}
		case WM_KILLFOCUS: {
			COLORREF color = 0x808080;
			DwmSetWindowAttribute(hwnd, 34, &color, sizeof(COLORREF));

			if (pt == nullptr) {
				break;
			}

			pt->pCaption->SetFocus(false);
			break;
		}
		case WM_ERASEBKGND: {

			return TRUE;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		return 0;
	}

	LRESULT ApplicationWindow::OnPaint(WPARAM wparam, LPARAM lparam) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		RECT rc;
		GetClientRect(hwnd, &rc);

		Gdiplus::Graphics			graphics(hdc);

		//graphics.DrawImage(pBuffer.get(), {0, 0, width, height}, 0, 0, width, height, Gdiplus::UnitPixel);

		EndPaint(hwnd, &ps);
		return 0;
	}

	LRESULT ApplicationWindow::OnCaptionPressed(WPARAM wparam, LPARAM lparam) {
		switch (wparam) {
		case HTMINBUTTON:
		case HTMAXBUTTON:
		case HTCLOSE:
			break;
		default:
			DefWindowProc(hwnd, WM_NCLBUTTONDOWN, wparam, lparam);

		}
		return 0;
	}

	LRESULT ApplicationWindow::OnCaptionReleased(WPARAM wparam, LPARAM lparam) {
		switch (wparam) {
		case HTMINBUTTON:
			ShowWindow(hwnd, SW_MINIMIZE);
			break;
		case HTMAXBUTTON:
			ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
			break;
		case HTCLOSE:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		default:
			DefWindowProc(hwnd, WM_NCLBUTTONUP, wparam, lparam);

		}

		return 0;
	}

	LRESULT ApplicationWindow::OnResize(WPARAM wparam, LPARAM lparam) {
		width = LOWORD(lparam);
		height = HIWORD(lparam);

		maximized = wparam == SIZE_MAXIMIZED;

		pCanvas->OnResize(width, height, maximized);
		pCaption->OnResize(width, height, maximized);

		InvalidateRect(hwnd, NULL, FALSE);

		return 0;
	}

	LRESULT ApplicationWindow::Hittest(WPARAM wparam, LPARAM lparam) {
		if (wparam) {
			return wparam;
		}

		POINT pt = { LOWORD(lparam), HIWORD(lparam) };
		ScreenToClient(hwnd, &pt);

		RECT rc;
		GetClientRect(hwnd, &rc);

		USHORT row = 1;
		USHORT col = 1;

		if (pt.y >= rc.top && pt.y < rc.top + 4) {
			row = 0;
		}
		else if (pt.y < rc.bottom && pt.y >= rc.bottom - 4) {
			row = 2;
		}

		if (pt.x >= rc.left && pt.x < rc.left + 4) {
			col = 0;
		}
		else if (pt.x < rc.right && pt.x >= rc.right - 4) {
			col = 2;
		}

		return hitTests[row][col];
	}

	void ApplicationWindow::SetShow(bool show) const {
		ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
	}

	void ApplicationWindow::SetEnable(bool enable) const {
		EnableWindow(hwnd, enable);
	}

	inline HWND ApplicationWindow::GetHWND() const {
		return hwnd;
	}

	inline UiCaption& ApplicationWindow::GetCaption() const {
		return *pCaption;
	}

	inline UiCanvas& ApplicationWindow::GetCanvas() const {
		return *pCanvas;
	}

	inline Gdiplus::Graphics* ApplicationWindow::GetGraphics() const {
		return bGrpahics.get();
	}
}