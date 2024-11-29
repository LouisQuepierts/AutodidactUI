#include "autodidact/autodidact.h"
#include "autodidact/acaption.h"
#include "autodidact/awnd.h"

#define USE_WINDOW_BUFFER

namespace Autodidact {
	CaptionWidget::CaptionWidget(ApplicationWindow& window, DPOS x, DPOS y, DSIZE width, DSIZE height, LRESULT htresult)
		: window(window), x(x), y(y), width(width), height(height), htresult(htresult) {
		RECT rc;
		GetClientRect(window.GetHWND(), &rc);
		UpdateDPOS(this->x, rc.right);
		UpdateDPOS(this->y, rc.bottom);
	}

	LRESULT CaptionWidget::Hittest(POINT pt) {
		pt.x -= x;
		pt.y -= y;
		if (pt.x > 0 && pt.y > 0 && pt.x <= width && pt.y <= height) {
			return htresult;
		}

		return HTNOWHERE;
	}

	void CaptionWidget::GetRect(RECT* rc) {
		rc->left = x;
		rc->top = y;
		rc->right = x + width;
		rc->bottom = y + height;
	}

	void CaptionWidget::OnDraw(Gdiplus::Graphics& graphics, bool focus) {
		unsigned long color = 0xfff0f0f0;

		switch (state) {
		case WidgetState::HOVERED:
			color = 0xfffefefe;
			break;
		case WidgetState::PRESSED:
			color = 0xfff5f5f5;
			break;
		case WidgetState::DISABLED:
			color = 0xffb0b0b0;
			break;
		}

		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, x, y, width, height);
	}

	void CaptionWidget::Execute() {
	}

	CHANGED CaptionWidget::OnResize(int width, int height) {
		UpdateDPOS(x, width);
		UpdateDPOS(y, height);
		UpdateDSIZE(this->width, x, width);
		UpdateDSIZE(this->height, y, height);

		return true;
	}

	UiCaption::UiCaption(HINSTANCE hInstance, ApplicationWindow& window, int width, int height)
		:
		widgets(),
		hMenu(CreateMenu()),
		width(width),
		height(height),
		window(window)
	{
		const static LPCWSTR clazz = [](HINSTANCE hInstance) {
			LPCWSTR clazz = L"_clazz_catpion";

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
			wnd.lpfnWndProc = CaptionProcess;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = clazz;

			if (!RegisterClassEx(&wnd)) {
				MessageBox(NULL, L"Registering Widget Class occur an error", NULL, MB_OK);
			}

			return clazz;
			}(hInstance);

			hWidget = CreateWindow(
				clazz, NULL,
				WS_CHILD,
				0, 0, width, height,
				window.GetHWND(), hMenu, hInstance,
				this
			);

			SetWindowLongPtr(hWidget, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

			DrawTitle();

			ShowWindow(hWidget, SW_SHOW);
	}

	UiCaption::~UiCaption() {
		DestroyWindow(hWidget);
		DestroyMenu(hMenu);
	}

	void UiCaption::OnResize(int width, int height, bool maximize) {
		CHANGED changed;

		int x = 0;
		int y = 0;

		if (maximize) {
			RECT rc;
			GetWindowRect(window.GetHWND(), &rc);

			this->width = GetSystemMetrics(SM_CXSCREEN);
			x = -rc.left;
			y = -rc.top;
			changed = true;
		}
		else {
			int sWidth = GetSystemMetrics(SM_CXSCREEN);

			int nWidth = (((sWidth) < (width)) ? (sWidth) : (width));
			changed = this->width != nWidth;
			this->width = nWidth;
		}

		if (changed) {
#ifdef USE_WINDOW_BUFFER
			Gdiplus::Graphics& bGraphics = *window.GetGraphics();
#endif // USE_WINDOW_BUFFER

			Gdiplus::SolidBrush brush(0xfff0f0f0);

			bGraphics.ResetTransform();
			bGraphics.FillRectangle(&brush, 200, 0, this->width, this->height);

			for (const auto& widget : widgets) {
				widget->OnResize(this->width, this->height);
				PaintWidget(widget.get());
			}

			bGraphics.ResetTransform();

			RECT rc{ 0, 0, this->width, this->height };
			SetWindowPos(hWidget, NULL, x, y, this->width, this->height, SWP_NOREDRAW);
#ifdef USE_WINDOW_BUFFER
			window.UpdateRect({ 0, 0, this->width, this->height });
#else
			InvalidateRect(hWidget, &rc, FALSE);
#endif
			
		}
	}

	void UiCaption::AddWidget(CaptionWidget* widget) {
		if (widget) {
			widgets.push_back(std::unique_ptr<CaptionWidget>(widget));
			PaintWidget(widget);
		}
	}

	void UiCaption::SetFocus(bool focus) {
		this->focus = focus;
		UpdateBuffer();
		window.ForceUpdate();
	}

	inline void UiCaption::PaintWidget(CaptionWidget* widget) {
#ifdef USE_WINDOW_BUFFER
		Gdiplus::Graphics& bGraphics = *window.GetGraphics();
#endif // USE_WINDOW_BUFFER

		Gdiplus::Matrix matrix(1, 0, 0, 1, float(widget->x), float(widget->y));
		bGraphics.SetTransform(&matrix);
		widget->OnDraw(bGraphics, focus);
	}

	void UiCaption::UpdateBuffer() {
#ifdef USE_WINDOW_BUFFER
		Gdiplus::Graphics& bGraphics = *window.GetGraphics();
#endif // USE_WINDOW_BUFFER
		bGraphics.ResetTransform();

		{
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			bGraphics.FillRectangle(&brush, 0, 0, width, height);
		}

		DrawTitle();

		for (const auto& widget : widgets) {
			PaintWidget(widget.get());
		}
	}

	void UiCaption::DrawTitle() {
#ifdef USE_WINDOW_BUFFER
		Gdiplus::Graphics& bGraphics = *window.GetGraphics();
#endif // USE_WINDOW_BUFFER
		Gdiplus::Font font(GetFontName(), 15, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(focus ? Gdiplus::Color(255, 0, 0, 0) : Gdiplus::Color(255, 189, 189, 189));

		bGraphics.ResetTransform();

		bGraphics.DrawImage(pResources->bIcon.get(), 20, 20, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
		bGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		bGraphics.DrawString(appName, -1, &font, Gdiplus::PointF(57, 20), &brush);

#ifdef USE_WINDOW_BUFFER
		window.UpdateRect({ 0, 0, this->width, this->height });
#endif
	}

	LRESULT UiCaption::CaptionProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		UiCaption* caption = reinterpret_cast<UiCaption*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (msg) {
		case WM_ERASEBKGND: {
			return TRUE;
		}
		case WM_SHOWWINDOW: {
			if (caption) {
				caption->UpdateBuffer();
			}

			break;
		}
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN: {
			if (caption->hovered) {
				caption->hovered->state = WidgetState::PRESSED;
				caption->PaintWidget(caption->hovered);
				//caption->hovered->OnDraw(caption->bGraphics, caption->focus);

				RECT rc;
				caption->hovered->GetRect(&rc);
				caption->window.UpdateRect({ rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top });
				caption->window.ForceUpdate();
				//InvalidateRect(hwnd, &rc, FALSE);
			}
			else {
				SendMessage(caption->window.GetHWND(), WM_NCLBUTTONDOWN, caption->lresult, 0);
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP: {
			if (caption->hovered) {
				caption->hovered->Execute();
				caption->PaintWidget(caption->hovered);
				//caption->hovered->OnDraw(caption->bGraphics, caption->focus);

				RECT rc;
				caption->hovered->GetRect(&rc);
				caption->window.UpdateRect({ rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top });
				caption->window.ForceUpdate();
			}
			else {
				SendMessage(caption->window.GetHWND(), WM_NCLBUTTONUP, caption->lresult, 0);
			}
			break;
		}
		case WM_MOUSEMOVE: {
			if (!caption->tracking) {
				caption->tracking = TRUE;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}
			break;
		}
		case WM_MOUSELEAVE: {
			caption->tracking = FALSE;
			if (caption->hovered) {
				caption->hovered->state = WidgetState::NORMAL;
				caption->PaintWidget(caption->hovered);
				//caption->hovered->OnDraw(caption->bGraphics, caption->focus);

				RECT rc;
				caption->hovered->GetRect(&rc);
				caption->hovered = nullptr;
				caption->window.UpdateRect({ rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top });
				caption->window.ForceUpdate();
			}
			break;
		}
		case WM_NCHITTEST: {
			POINT pt = { LOWORD(lparam), HIWORD(lparam) };
			ScreenToClient(hwnd, &pt);

			RECT rc;
			GetClientRect(hwnd, &rc);

			if (!PtInRect(&rc, pt)) {
				return HTNOWHERE;
			}

			int row = pt.y > 4;
			int col = (pt.x < 4) ? 0 : (
				(pt.x > caption->width - 4) ? 2 : 1
				);

			LRESULT ht = hitTests[row][col];
			if (ht && !IsZoomed(caption->window.GetHWND())) {
				caption->lresult = ht;
				SendMessage(caption->window.GetHWND(), WM_NCHITTEST, ht, lparam);
				return ht;
			}

			CaptionWidget* hovering = nullptr;
			// Check Widgets
			for (const auto& widget : caption->widgets) {
				if (!widget->enable) {
					continue;
				}

				ht = widget->Hittest(pt);

				if (ht) {
					hovering = widget.get();
					hovering->state = WidgetState::HOVERED;
					break;
				}
			}

			// Update Hover
			if (caption->hovered != hovering) {
				RECT rc;
				if (caption->hovered) {
					caption->hovered->state = WidgetState::NORMAL;
					caption->PaintWidget(caption->hovered);
					//caption->hovered->OnDraw(caption->bGraphics, caption->focus);
					caption->hovered->GetRect(&rc);
					//InvalidateRect(hwnd, &rc, NULL);
					caption->window.UpdateRect({ rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top });
				}
				caption->hovered = hovering;

				if (caption->hovered) {
					caption->PaintWidget(caption->hovered);
					//caption->hovered->OnDraw(caption->bGraphics, caption->focus);
					caption->hovered->GetRect(&rc);
					//InvalidateRect(hwnd, &rc, FALSE);
					caption->window.UpdateRect({ rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top });
				}

				caption->window.ForceUpdate();
			}

			if (!ht)
				ht = HTCAPTION;

			SendMessage(caption->window.GetHWND(), WM_NCHITTEST, ht, lparam);

			caption->lresult = ht;
			return HTCLIENT;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

	CaptionButtonMinimize::CaptionButtonMinimize(ApplicationWindow& window, DPOS x, DPOS y)
		: CaptionWidget(window, x, y, 60, 60, HTMINBUTTON) {
	}

	void CaptionButtonMinimize::OnDraw(Gdiplus::Graphics& graphics, bool focus) {
		Gdiplus::Color color;

		switch (state) {
		case WidgetState::NORMAL:
			color = 0xfff0f0f0;
			break;
		case WidgetState::HOVERED:
			color = 0xfffefefe;
			break;
		case WidgetState::PRESSED:
			color = 0xfff5f5f5;
			break;
		}
		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, 0, 0, 60, 60);

		if (!focus && state == WidgetState::NORMAL) {
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
				0.75f,	0.75f,	0.75f,	0.0f,	1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
			graphics.DrawImage(pResources->bMinimize.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		else {
			graphics.DrawImage(pResources->bMinimize.get(), 22, 22, 16, 16);
		}
	}

	void CaptionButtonMinimize::Execute() {
		ShowWindow(window.GetHWND(), SW_MINIMIZE);
	}

	CaptionButtonMaximize::CaptionButtonMaximize(ApplicationWindow& window, DPOS x, DPOS y)
		: CaptionWidget(window, x, y, 60, 60, HTMAXBUTTON) {
	}

	void CaptionButtonMaximize::OnDraw(Gdiplus::Graphics& graphics, bool focus) {
		Gdiplus::Color color;

		switch (state) {
		case WidgetState::NORMAL:
			color = 0xfff0f0f0;
			break;
		case WidgetState::HOVERED:
			color = 0xfffefefe;
			break;
		case WidgetState::PRESSED:
			color = 0xfff5f5f5;
			break;
		}

		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, 0, 0, 60, 60);

		if (!focus && state == WidgetState::NORMAL) {
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
				0.75f,	0.75f,	0.75f,	0.0f,	1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
			graphics.DrawImage(IsZoomed(window.GetHWND()) ? pResources->bRestore.get() : pResources->bMaximize.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		else {
			graphics.DrawImage(IsZoomed(window.GetHWND()) ? pResources->bRestore.get() : pResources->bMaximize.get(), 22, 22, 16, 16);
		}
	}

	void CaptionButtonMaximize::Execute() {
		ShowWindow(window.GetHWND(), IsZoomed(window.GetHWND()) ? SW_RESTORE : SW_MAXIMIZE);
	}

	CaptionButtonClose::CaptionButtonClose(ApplicationWindow& window, DPOS x, DPOS y)
		: CaptionWidget(window, x, y, 60, 60, HTCLOSE) {
	}

	void CaptionButtonClose::OnDraw(Gdiplus::Graphics& graphics, bool focus) {
		switch (state) {
		case WidgetState::NORMAL:
		{
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);

			if (focus) {
				graphics.DrawImage(pResources->bClose.get(), 22, 22, 16, 16);
			}
			else {
				Gdiplus::ImageAttributes	imageAttribute;

				Gdiplus::ColorMatrix matrix = {
					1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
					0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
					0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
					0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
					0.75f,	0.75f,	0.75f,	0.0f,	1.0f
				};

				imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
				graphics.DrawImage(pResources->bClose.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
			}
		}
		break;
		case WidgetState::HOVERED:
		{
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

			Gdiplus::SolidBrush brush(0xffe81123);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);
			graphics.DrawImage(pResources->bClose.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		break;
		case WidgetState::PRESSED:
		{
			Gdiplus::SolidBrush brush(0xfff1707a);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);
			graphics.DrawImage(pResources->bClose.get(), 22, 22, 16, 16);
		}
		break;
		}
	}

	void CaptionButtonClose::Execute() {
		SendMessage(window.GetHWND(), WM_CLOSE, 0, 0);
	}
}