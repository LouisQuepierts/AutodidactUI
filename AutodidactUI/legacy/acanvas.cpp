#include "autodidact/autodidact.h"
#include "autodidact/acanvas.h"
#include "autodidact/awnd.h"

namespace Autodidact {
	Canvas::Canvas(
		HINSTANCE			hInstance,
		ApplicationWindow&	window,
		int					y,
		int					width,
		int					height
	) :
		widgets(),
		hMenu(CreateMenu()),
		y(y),
		width(width),
		height(height - y),
		buffer(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) - y),
		bGraphics(&buffer),
		window(window)
	{
		const static LPCWSTR clazz = [](HINSTANCE hInstance) {
			LPCWSTR clazz = L"_clazz__canvas";

			WNDCLASSEX wnd;
			wnd.cbSize = sizeof(WNDCLASSEX);
			wnd.style = NULL;
			wnd.cbClsExtra = 0;
			wnd.cbWndExtra = 0;
			wnd.hInstance = hInstance;
			wnd.hIcon = NULL;
			wnd.hIconSm = NULL;
			wnd.hbrBackground = NULL;
			wnd.hCursor = NULL;
			wnd.lpfnWndProc = CanvasProcess;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = clazz;

			if (!RegisterClassEx(&wnd)) {
				MessageBox(NULL, L"Registering Widget Class occur an error", NULL, MB_OK);
			}

			return clazz;
		}(hInstance);

		hwnd = CreateWindow(
			clazz, NULL,
			WS_CHILD | WS_VISIBLE,
			0, y, this->width, this->height,
			window.GetHWND(), hMenu, hInstance,
			this
		);

		Gdiplus::SolidBrush brush(0xfff0f0f0);
		bGraphics.FillRectangle(&brush, 0, 0, buffer.GetWidth(), buffer.GetHeight());

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	Canvas::~Canvas() {
		DestroyMenu(hMenu);
		DestroyWindow(hwnd);
	}

	void Canvas::OnResize(int nWidth, int nHeight) {
		RECT rc;
		GetWindowRect(window.GetHWND(), &rc);

		int sWidth = GetSystemMetrics(SM_CXSCREEN);
		int sHeight = GetSystemMetrics(SM_CYSCREEN);

		int x = rc.left < 0 ? -rc.left : 0;
		int y = (rc.top < 0 ? -rc.top : 0) + this->y;

		sWidth	= min(sWidth, nWidth);
		sHeight = min(sHeight, nHeight) - this->y;

		CHANGED changed = width != sWidth || height != sHeight;
		
		width	= sWidth;
		height	= sHeight;

		if (changed) {
			Gdiplus::SolidBrush		brush(0xfff0f0f0);
			Gdiplus::Rect			rect;
			std::vector<Widget*>	repaint;

			for (const auto& widget : widgets) {
				if (widget->OnResize(width, height)) {
					rect.X = widget->x;
					rect.Y = widget->y;
					rect.Width = widget->width;
					rect.Height = widget->height;

					repaint.push_back(widget.get());
					bGraphics.FillRectangle(&brush, rect);
				}
			}

			for (const auto& widget : repaint) {
				PaintWidget(widget);
			}
		}

		SetWindowPos(hwnd, NULL, x, y, this->width, this->height, SWP_NOREDRAW);
	}

	void Canvas::AddWidget(Widget* pWidget) {
		if (pWidget) {
			widgets.push_back(std::unique_ptr<Widget>(pWidget));
			PaintWidget(pWidget);
		}
	}

	inline void Canvas::PaintWidget(Widget* pWidget) {
		Gdiplus::Matrix mat(1, 0, 0, 1, float(pWidget->x), float(pWidget->y));
		bGraphics.SetTransform(&mat);
		pWidget->OnDraw(bGraphics);
	}

	void Canvas::UpdateBuffer() {
		bGraphics.ResetTransform();

		{
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			bGraphics.FillRectangle(&brush, 0, 0, width, height);
		}

		for (const auto& widget : widgets) {
			PaintWidget(widget.get());
		}
	}

	LRESULT Canvas::CanvasProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		Canvas* canvas = reinterpret_cast<Canvas*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (msg) {
		case WM_ERASEBKGND: {
			return TRUE;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			Gdiplus::Graphics graphics(hdc);
			graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
			graphics.DrawImage(&canvas->buffer, { 0, 0, canvas->width, canvas->height }, 0, 0, canvas->width, canvas->height, Gdiplus::UnitPixel);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_SHOWWINDOW: {
			if (canvas) {
				canvas->UpdateBuffer();
			}

			break;
		}
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN: {
			if (canvas->hovered) {
				canvas->hovered->state = WidgetState::PRESSED;
				canvas->PaintWidget(canvas->hovered);
				//canvas->hovered->OnDraw(canvas->bGraphics, canvas->focus);

				RECT rc;
				canvas->hovered->GetRect(&rc);
				InvalidateRect(hwnd, &rc, FALSE);
			}
			else if (wparam != HTCLIENT) {
				SendMessage(canvas->window.GetHWND(), WM_NCLBUTTONDOWN, wparam, 0);
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP: {
			if (canvas->hovered) {
				// click
				canvas->PaintWidget(canvas->hovered);
				//canvas->hovered->OnDraw(canvas->bGraphics, canvas->focus);

				RECT rc;
				canvas->hovered->GetRect(&rc);
				InvalidateRect(hwnd, &rc, FALSE);
			}
			else if (wparam != HTCLIENT) {
				SendMessage(canvas->window.GetHWND(), WM_NCLBUTTONUP, wparam, 0);
			}
			break;
		}
		case WM_MOUSEMOVE: {
			if (!canvas->tracking) {
				canvas->tracking = TRUE;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}
			break;
		}
		case WM_MOUSELEAVE: {
			canvas->tracking = FALSE;
			if (canvas->hovered) {
				canvas->hovered->state = WidgetState::NORMAL;
				canvas->PaintWidget(canvas->hovered);
				//canvas->hovered->OnDraw(canvas->bGraphics, canvas->focus);

				RECT rc;
				canvas->hovered->GetRect(&rc);
				canvas->hovered = nullptr;
				InvalidateRect(hwnd, &rc, FALSE);
			}
			break;
		}
		case WM_NCHITTEST: {
			POINT pt = { LOWORD(lparam), HIWORD(lparam) };
			ScreenToClient(canvas->hwnd, &pt);

			int row = pt.y > canvas->height - 4 ? 2 : 1;
			int col = (pt.x < 4) ? 0 : (
				(pt.x > canvas->width - 4) ? 2 : 1);

			LRESULT ht = hitTests[row][col];

			if (ht && !IsZoomed(canvas->window.GetHWND())) {
				SendMessage(canvas->window.GetHWND(), WM_NCHITTEST, ht, lparam);
				return ht;
			}



			return HTCLIENT;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

	Widget::Widget(
		ApplicationWindow&	window,
		DPOS				x,
		DPOS				y,
		DSIZE				width,
		DSIZE				height
	) :
		window(window),
		x(x),
		y(y),
		width(width),
		height(height)
	{
	}

	void Widget::OnDraw(Gdiplus::Graphics& graphics) {
		Gdiplus::SolidBrush brush(0xffff0000);
		graphics.FillRectangle(&brush, 0, 0, width, height);
	}

	bool Widget::OnResize(int nWidth, int nHeight) {
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHeight);
		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHeight);
		return true;
	}

	bool Widget::HitTest(POINT pt) {
		pt.x -= x;
		pt.y -= y;
		return (pt.x > 0 && pt.y > 0 && pt.x <= width && pt.y <= height);
	}

	void Widget::GetRect(RECT* rc) {
		rc->left = x;
		rc->top = y;
		rc->right = x + width;
		rc->bottom = y + height;
	}
}
