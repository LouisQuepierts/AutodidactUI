#include <algorithm>
#include <cmath>

#include "autodidact/autodidact.h"
#include "autodidact/awnd.h"
#include "autodidact/awgt.h"

namespace Autodidact {
	DRAWITEMSTRUCT ds;

	CanvasWidget::CanvasWidget(
		HINSTANCE			hInstance, 
		HWND				hParent,
		UiCanvas&			canvas, 
		DPOS				x, 
		DPOS				y, 
		DSIZE				width, 
		DSIZE				height,
		bool				dynamic
	)	: 
		x(x), 
		y(y), 
		width(width), 
		height(height), 
		canvas(canvas),
		hMenu(CreateMenu()), 
		hParent(hParent),
		dynamic(dynamic) {
		const static LPCWSTR clazz = [](HINSTANCE hInstance) {
			LPCWSTR clazz = L"_clazz_canvas_widget";

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
			wnd.lpfnWndProc = WidgetProcess;
			wnd.lpszMenuName = NULL;
			wnd.lpszClassName = clazz;

			if (!RegisterClassEx(&wnd)) {
				MessageBox(NULL, L"Registering Widget Class occur an error", NULL, MB_OK);
			}

			return clazz;
		} (hInstance);


		RECT rc;
		GetClientRect(hParent, &rc);
		OnResize(rc.left, rc.right, nullptr);

		hWidget = CreateWindow(
			clazz, NULL,
			WS_CHILD | WS_VISIBLE,
			this->x, this->y, this->width, this->height,
			hParent, hMenu, hInstance,
			this
		);

		SetWindowLongPtr(hWidget, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	CanvasWidget::~CanvasWidget() {
		DestroyWindow(hWidget);
		DestroyMenu(hMenu);
	}

	CHANGED CanvasWidget::OnResize(int nWidth, int nHeight, std::vector<POSSTRUCT>* updateList) {
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHeight);
		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHeight);

		PaintBuffer();

		if (updateList) {
			updateList->push_back({ hWidget, x, y, width, height, SWP_NOREDRAW });
		}

		return true;
	}

	void CanvasWidget::SetShow(bool show) {
		ShowWindow(hWidget, show ? SW_SHOW : SW_HIDE);
	}

	void CanvasWidget::SetEnable(bool enable) {
		EnableWindow(hWidget, enable);
	}

	void CanvasWidget::SetText(LPCWSTR str) {
		SetWindowText(hWidget, str);
	}

	HWND CanvasWidget::GetHWND() const {
		return hWidget;
	}

	void CanvasWidget::RePaint() {
		PreparePos();
		auto& window = GetWindow();
		window.UpdateRect({ x, y, width, height });
		window.PopPos();
		window.ForceUpdate();
	}

	ApplicationWindow& CanvasWidget::GetWindow() const {
		return canvas.GetWindow();
	}

	WidgetState CanvasWidget::GetState() const {
		return state;
	}

	void CanvasWidget::PaintBuffer() {
		auto& window = GetWindow();
		Gdiplus::Graphics& graphics = *window.GetGraphics();
		PreparePos();
		this->OnPaint(graphics);
		window.UpdateRect({ x, y, width, height });
		window.PopPos();
	}

	LRESULT CanvasWidget::WidgetProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		CanvasWidget* widget = reinterpret_cast<CanvasWidget*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch(msg) {
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN: {
			if (widget->lresult != HTCLIENT) {
				SendMessage(widget->GetWindow().GetHWND(), WM_NCLBUTTONDOWN, widget->lresult, 0);
			}
			else {
				widget->state = WidgetState::PRESSED;

				widget->OnLButtonDown({ LOWORD(lparam), HIWORD(lparam) });

				if (widget->dynamic) {
					widget->PaintBuffer();
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP: {
			if (widget->lresult != HTCLIENT) {
				SendMessage(widget->GetWindow().GetHWND(), WM_NCLBUTTONUP, widget->lresult, 0);
			}
			else {
				widget->state = WidgetState::HOVERED;

				widget->OnLButtonUp({ LOWORD(lparam), HIWORD(lparam) });

				if (widget->dynamic) {
					widget->PaintBuffer();
				}
			}
			break;
		}
		case WM_MOUSEMOVE: {
			if (!widget->dynamic) {
				break;
			}

			widget->OnMouseMove({ LOWORD(lparam), HIWORD(lparam) });

			if (!widget->tracking) {
				widget->tracking = TRUE;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}

			if (widget->state == WidgetState::NORMAL) {
				widget->state = WidgetState::HOVERED;
				widget->PaintBuffer();
			}
			break;
		}
		case WM_MOUSELEAVE: {
			if (!widget->dynamic) {
				break;
			}

			widget->OnMouseLeave();

			widget->tracking	= FALSE;
			widget->state		= WidgetState::NORMAL;
			widget->PaintBuffer();
			break;
		}
		case WM_NCHITTEST: {
			auto& window = widget->GetWindow();
			POINT pt = { LOWORD(lparam), HIWORD(lparam) };

			RECT rc;
			GetWindowRect(hwnd, &rc);

			if (!PtInRect(&rc, pt)) {
				return HTNOWHERE;
			}

			GetWindowRect(window.GetHWND(), &rc);

			widget->lresult = HTCLIENT;

			int row = (pt.y > rc.bottom - 4) ? 2 : 1;
			int col = (pt.x > rc.right - 4) ? 2 : !(pt.x < rc.left + 4);

			LRESULT ht = hitTests[row][col];

			if (ht && !IsZoomed(window.GetHWND())) {
				widget->lresult = ht;
				SendMessage(window.GetHWND(), WM_NCHITTEST, ht, lparam);
			}

			return widget->lresult;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		return 0;
	}

	void CanvasWidget::OnLButtonDown(POINT pt) {}

	void CanvasWidget::OnLButtonUp(POINT pt) {}

	void CanvasWidget::OnMouseMove(POINT pt) {}

	void CanvasWidget::OnMouseLeave() {}

	void CanvasWidget::PreparePos() {
		RECT rc;
		GetWindowRect(hWidget, &rc);
		POINT pt{ rc.left, rc.top };
		ScreenToClient(hParent, &pt);
		pt.y += canvas.GetY();
		GetWindow().PushPos({pt.x, pt.y});
	}

	void CanvasWidget::OnPaint(Gdiplus::Graphics& graphics) {
		if (paint != nullptr) {
			paint(graphics, this);
		}
	}

	UiCanvas::UiCanvas(
		HINSTANCE hInstance, 
		ApplicationWindow& window, 
		int y, 
		int width, 
		int height
	) : 
		pages(), 
		hMenu(CreateMenu()), 
		y(y), 
		width(width), 
		height(height - y),
		window(window) {
		const static LPCWSTR clazz = [](HINSTANCE hInstance) {
			LPCWSTR clazz = L"_clazz_canvas";

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

		hWidget = CreateWindow(
			clazz, NULL,
			WS_CHILD | WS_VISIBLE,
			0, y, this->width, this->height,
			window.GetHWND(), hMenu, hInstance,
			this
		);

		SetWindowLongPtr(hWidget, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	}

	UiCanvas::~UiCanvas() {
		DestroyWindow(hWidget);
		DestroyMenu(hMenu);
	}

	void UiCanvas::OnResize(int width, int height, bool maximize) {
		int x = 0;
		int y = this->y;

		if (maximize) {
			RECT rc;
			GetWindowRect(window.GetHWND(), &rc);

			x = -rc.left;
			y -= rc.top;

			this->width		= GetSystemMetrics(SM_CXSCREEN);
			this->height	= GetSystemMetrics(SM_CYSCREEN) - this->y - this->y;
		}
		else {
			this->width = width;
			this->height = height - this->y;
		}

		std::vector<POSSTRUCT> updateList;
		for (const auto& page : pages) {
			page->OnResize(this->width, this->height, &updateList);
		}

		HDWP hdwp = BeginDeferWindowPos(int(updateList.size()));

		for (const auto& ps : updateList) {
			hdwp = DeferWindowPos(hdwp, ps.hwnd, NULL, ps.x, ps.y, ps.width, ps.height, ps.flag);

			if (ps.rect.right) {
				//InvalidateRect(ps.hwnd, &ps.rect, FALSE);
			}
		}

		EndDeferWindowPos(hdwp);

		SetWindowPos(hWidget, NULL, x, y, this->width, this->height, SWP_NOREDRAW);
	}

	void UiCanvas::AddPage(Page* page) {
		if (pages.size() == 0) {
			page->SetShow();
		}
		pages.push_back(std::unique_ptr<Page>(page));
	}
	
	void UiCanvas::SelectPage(const size_t index) {
		if (pages.size() > index) {
			if (selected != nullptr) {
				selected->SetShow(false);
			}

			selected = pages[index].get();
			selected->SetShow(true);
		}
	}

	void UiCanvas::PreparePos() {
		window.PushPos({ 0, y });
	}

	inline int UiCanvas::GetY() const {
		return y;
	}

	inline HWND UiCanvas::GetHWND() const {
		return hWidget;
	}

	inline ApplicationWindow& UiCanvas::GetWindow() const {
		return window;
	}

	Page& UiCanvas::operator[](const size_t index) const {
		return *(pages[index]);
	}

	LRESULT UiCanvas::CanvasProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		UiCanvas* canvas = reinterpret_cast<UiCanvas*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (msg) {
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN: {
			if (wparam != HTCLIENT) {
				SendMessage(canvas->window.GetHWND(), WM_NCLBUTTONDOWN, wparam, 0);
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP: {
			if (wparam != HTCLIENT) {
				SendMessage(canvas->window.GetHWND(), WM_NCLBUTTONUP, wparam, 0);
			}
			break;
		}
		case WM_MOUSEMOVE: {
			if (!canvas->tracking) {
				canvas->tracking = TRUE;
				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = canvas->hWidget;
				TrackMouseEvent(&tme);
			}
			break;
		}
		case WM_MOUSELEAVE: {
			canvas->tracking = FALSE;
			break;
		}
		case WM_NCHITTEST: {
			POINT pt = { LOWORD(lparam), HIWORD(lparam) };

			ScreenToClient(canvas->hWidget, &pt);

			int row = pt.y > canvas->height - 4 ? 2 : 1;
			int col = (pt.x < 4) ? 0 : (
				(pt.x > canvas->width - 4) ? 2 : 1);

			LRESULT ht = hitTests[row][col];

			if (ht && !IsZoomed(canvas->window.GetHWND())) {
				SendMessage(canvas->window.GetHWND(), WM_NCHITTEST, ht, lparam);
			}
			else {
				ht = HTCLIENT;
			}

			return ht;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		return 0;
	}
	
	CanvasPageController::CanvasPageController(
		HINSTANCE			hInstance,
		HWND				hParent,
		UiCanvas&			canvas,
		DPOS				x,
		DPOS				y,
		DSIZE				width,
		DSIZE				height
	) :
		CanvasWidget(hInstance, hParent, canvas, x, y, width, height),
		pages()
	{
	}

	Page& CanvasPageController::operator[](size_t size) const {
		return *(pages[size]);
	}

	CHANGED CanvasPageController::OnResize(int nWidth, int nHheight, std::vector<POSSTRUCT>* updateList) {
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHheight);

		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHheight);

		if (pages.size() > 0) {
			pages[selected]->OnResize(width, height, updateList);
		}

		if (updateList) {
			updateList->push_back({ hWidget, x, y, width, height });
		}
		
		return true;
	}

	Page& CanvasPageController::GetSelected() {
		return *(pages[selected]);
	}

	void CanvasPageController::AddPage(Page* page) {
		if (pages.size() == 0) {
			page->SetShow();
		}
		pages.push_back(std::unique_ptr<Page>(page));
	}

	void CanvasPageController::Select(const size_t i) {
		if (selected != i && pages.size() > i) {
			pages[selected]->SetShow(false);
			
			selected = i;
			//pages[selected]->OnResize(width, height);
			pages[selected]->SetShow();
		}
	}

	Page::Page(HWND hParent) : hParent(hParent) {

	}

	void Page::OnResize(int width, int height, std::vector<POSSTRUCT>* updateList) {
		for (const auto& widget : widgets) {
			widget->OnResize(width, height, updateList);
		}
	}

	void Page::SetShow(bool show) {
		int flag = show ? SW_SHOW : SW_HIDE;
		for (const auto& widget : widgets) {
			ShowWindow(widget->GetHWND(), flag);
		}
	}

	void Page::AddWidget(CanvasWidget* widget) {
		if (widget) {
			SetParent(widget->GetHWND(), hParent);
			widgets.push_back(std::unique_ptr<CanvasWidget>(widget));
		}
	}
	
	void Page::RemoveWidget(CanvasWidget* widget) {
		for (auto i = widgets.begin(); i != widgets.end(); i++) {
			if (i->get() == widget) {
				widgets.erase(i);
				break;
			}
		}
	}

	HWND Page::GetHWND() {
		return hParent;
	}

	RadioButton::RadioButton(
		HINSTANCE			hInstance,
		HWND				hParent,
		UiCanvas&			canvas,
		DPOS				x,
		DPOS				y,
		DSIZE				width,
		DSIZE				height,
		UINT				size,
		UINT				defsel,
		UINT				bottom,
		bool				vertical
	) :
		CanvasWidget(
			hInstance,
			hParent, 
			canvas,
			x,
			y,
			width,
			height,
			true
		),
		size(size),
		bottom(bottom),
		vertical(vertical),
		buttons(new SubButton[size]()) {
		
		if (defsel < size) {
			selected = &buttons[defsel];
			selectPos = 10;
			selectSize = 10;
		}
	}

	CHANGED RadioButton::OnResize(int nWidth, int nHeight, std::vector<POSSTRUCT>* updateList) {
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHeight);
		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHeight);

		auto	ptr = buttons.get();
		int		uSize = vertical ? height : width;
		int		forward = 0;
		int		reverse = height;

		bool	inverse = false;

		for (UINT i = 0; i < size; i++) {
			UpdateDPOS(ptr->pos, uSize);
			UpdateDSIZE(ptr->size, ptr->pos, uSize);

			if (i < bottom) {
				forward += ptr->pos;
				ptr->begin = forward;
				forward += ptr->size;
			}
			else {
				reverse -= ptr->size + ptr->pos;
				ptr->begin = reverse;
			}
			++ptr;
		}

		PaintBuffer();

		if (updateList) {
			POSSTRUCT pos = { hWidget, x, y, width, height };
			GetClientRect(hWidget, &pos.rect);
			updateList->push_back(pos);
		}

		SelectBarTo(selected->begin, selected->size);

		return true;
	}

	void RadioButton::OnPaint(Gdiplus::Graphics& graphics) {
		Gdiplus::SolidBrush brush(0xfff0f0f0);
		graphics.FillRectangle(&brush, 10, 0, width - 10, height);

		auto ptr	= buttons.get();

		for (UINT i = 0; i < size; i++) {
			DrawButton(graphics, ptr);
			++ptr;
		}
	}

	RadioButton::SubButton& RadioButton::operator[](const UINT index) const {
		return buttons[index];
	}

	void RadioButton::OnLButtonDown(POINT pt) {
		if (hovered) {
			if (selected == hovered) {
				return;
			}

			auto& window = GetWindow();

			Gdiplus::Graphics& graphics = *window.GetGraphics();
			PreparePos();

			hovered->state = WidgetState::PRESSED;
			DrawButton(graphics, hovered);

			window.UpdateRect({ x, y, width, height });

			window.PopPos();
			window.ForceUpdate();
		}
	}

	void RadioButton::OnLButtonUp(POINT pt) {
		if (hovered) {
			if (selected == hovered) {
				return;
			}

			auto& window = GetWindow();

			Gdiplus::Graphics& graphics = *window.GetGraphics();
			PreparePos();

			hovered->state = WidgetState::HOVERED;
			DrawButton(graphics, hovered);

			if (hovered->click) {
				hovered->click();
			}

			selected = hovered;

			window.ForceUpdate({ x, y, width, height });
			window.PopPos();

			SelectBarTo(hovered->begin, hovered->size);
		}
	}

	void RadioButton::OnMouseMove(POINT pt) {
		int comparator = int(vertical ? pt.y : pt.x);

		if (hovered) {
			if (comparator > hovered->begin && comparator < hovered->begin + hovered->size) {
				return;
			}
		}

		SubButton* last = hovered;
		hovered = nullptr;

		for (UINT i = 0; i < size; i++) {
			if (comparator > buttons[i].begin && comparator < buttons[i].begin + buttons[i].size) {

				hovered = &buttons[i];
				hovered->state = WidgetState::HOVERED;
				break;
			}
		}

		if (last != hovered) {
			auto& window = GetWindow();
			Gdiplus::Graphics& graphics = *window.GetGraphics();
			PreparePos();

			if (last) {
				last->state = WidgetState::NORMAL;
				DrawButton(graphics, last);
			}

			if (hovered) {
				DrawButton(graphics, hovered);
			}

			if (!animate) 
				DrawSelectBar(graphics);

			window.ForceUpdate({ x, y, width, height });
			window.PopPos();
		}
	}

	void RadioButton::OnMouseLeave() {
		if (hovered) {
			auto& window = GetWindow();
			Gdiplus::Graphics& graphics = *window.GetGraphics();
			PreparePos();

			hovered->state = WidgetState::NORMAL;
			DrawButton(graphics, hovered);

			//RECT rc = vertical ? RECT{ 0, hovered->begin, width, hovered->begin + hovered->size } : RECT{ hovered->begin, 0, hovered->begin + hovered->size, height };
			hovered = nullptr;

			if (!animate)
				DrawSelectBar(graphics);

			window.ForceUpdate({ x, y, width, height });
			window.PopPos();
		}
	}

	void RadioButton::SelectBarTo(int pos, int size) {
		pos += 5;
		size -= 5;
		if (selectPos != pos || selectSize != size) {
			selectToPos = pos;
			selectToSize = size;

			lastSelectPos = selectPos;
			lastSelectSize = selectSize;

			if (!animate) {
				SetTimer(hWidget, 1, 15, MoveSelectBar);
			}
			else {
				animate = 0;
			}
		}
	}

	inline void RadioButton::DrawButton(Gdiplus::Graphics& graphics, SubButton* button) {
		int size;

		auto& window = GetWindow();

		if (vertical) {
			size = width - 10;
			window.TranslatePos({ 10, button->begin });
		}
		else {
			size = height - 10;
			window.TranslatePos({ button->begin, 0 });
		}

		if (button->paint) {
			button->paint(graphics, button, size, button == selected);
		}

		window.PopPos();
	}

	inline void RadioButton::DrawSelectBar(Gdiplus::Graphics& graphics) {
		HRGN hRgn;
		
		if (vertical) {
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			graphics.FillRectangle(&brush, 0, 0, 10, height + 200);
			hRgn = CreateRoundRectRgn(5, selectPos, 10, selectPos + selectSize, 2, 2);
		}
		else {
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			graphics.FillRectangle(&brush, 0, height - 10, width + 200, 10);
			hRgn = CreateRoundRectRgn(selectPos, height - 9, selectPos + selectSize, height - 5, 2, 2);
		}

		if (hovered) {
			HRGN hRgn = CreateRoundRectRgn(5, hovered->begin + 5, 10, hovered->begin + hovered->size - 5, 2, 2);
			Gdiplus::SolidBrush brush(0xffd1d1d1);
			Gdiplus::Region region(hRgn);
			graphics.FillRegion(&brush, &region);

			DeleteObject(hRgn);
		}

		Gdiplus::Region region(hRgn);
		Gdiplus::SolidBrush brush(pResources->colorTheme);

		graphics.FillRegion(&brush, &region);

		DeleteObject(hRgn);
	}

	int smooth_lerp(int from, int to, float x) {
		x = 1 - x;
		return to + int(x * x * (from - to));
	}

	void RadioButton::MoveSelectBar(HWND hwnd, UINT, UINT_PTR, DWORD) {
		RadioButton* btn = reinterpret_cast<RadioButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		btn->animate++;

		float rate = btn->animate / 20.0f;
		btn->selectPos = smooth_lerp(btn->lastSelectPos, btn->selectToPos, rate);
		btn->selectSize = smooth_lerp(btn->lastSelectSize, btn->selectToSize, rate);

		auto& window = btn->GetWindow();

		btn->PreparePos();
		btn->DrawSelectBar(*window.GetGraphics());
		window.ForceUpdate({ 0, 0, 10, btn->height });
		window.PopPos();

		if (btn->animate == 20) {
			btn->animate = 0;
			KillTimer(hwnd, 1);
			return;
		}
	}

	RadioButton::SubButton::SubButton() {}

	void RadioButton::SubButton::operator=(const SubButtonRaw& raw) {
		pos = raw.pos;
		size = raw.size;
		click = raw.click;
		paint = raw.paint;

		if (raw.text) {
			wcscpy_s(text, raw.text);
			if (text[15]) {
				text[15] = 0;
			}
		}

		if (raw.hInst && raw.icon) {
			icon.reset(LoadPNGFromResource(raw.hInst, raw.icon));
		}
	}

	void RadioButton::SubButton::operator=(SubButton& btn) {
		pos		= btn.pos;
		size	= btn.size;
		click	= btn.click;
		paint	= btn.paint;

		wcscpy_s(text, btn.text);
		icon = std::move(btn.icon);
	}

	PaintWidget::PaintWidget(
		HINSTANCE			hInstance,
		HWND				hParent,
		UiCanvas& canvas,
		DPOS				x,
		DPOS				y,
		DSIZE				width,
		DSIZE				height,
		PaintProcess		paint
	) :
		CanvasWidget(
			hInstance,
			hParent,
			canvas,
			x,
			y,
			width,
			height)
	{
		this->paint = paint;
	}
}