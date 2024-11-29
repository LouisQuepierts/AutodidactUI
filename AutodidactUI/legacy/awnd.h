#ifndef AWND_H
#define AWND_H

#include <stack>
#include "auidll.h"

namespace Autodidact {
	class UiCaption;
	class UiCanvas;
	class Canvas;
	class CaptionWidget;
	class Page;

	class ApplicationWindow {
	public:
		AUIDLL ApplicationWindow(HINSTANCE hInstance, int width, int height);

		AUIDLL ~ApplicationWindow();

		AUIDLL void Resize(int width, int height, bool update = false);

		AUIDLL void AddPage(Page *page) const;

		AUIDLL void SelectPage(size_t i) const;

		AUIDLL inline void SetShow(bool show = 1) const;

		AUIDLL inline void SetEnable(bool enable = 1) const;

		AUIDLL inline HWND GetHWND() const;

		AUIDLL inline UiCaption& GetCaption() const;

		AUIDLL inline UiCanvas& GetCanvas() const;

		AUIDLL inline Gdiplus::Graphics* GetGraphics() const;

		AUIDLL inline void UpdateRect(const Gdiplus::Rect& rc);

		AUIDLL inline void ForceUpdate();

		AUIDLL inline void ForceUpdate(const Gdiplus::Rect& rc);

		AUIDLL inline void PushPos(const POINT& pos);

		AUIDLL inline void TranslatePos(const POINT& pos);

		AUIDLL inline void PopPos();

		AUIDLL inline POINT CurrentPos();

	private:
		AUIDLL static LRESULT CALLBACK ApplicationProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		AUIDLL LRESULT Hittest(WPARAM, LPARAM);

		AUIDLL LRESULT OnPaint(WPARAM, LPARAM);

		AUIDLL LRESULT OnCaptionPressed(WPARAM, LPARAM);

		AUIDLL LRESULT OnCaptionReleased(WPARAM, LPARAM);

		AUIDLL LRESULT OnResize(WPARAM, LPARAM);

	private:
		std::unique_ptr<UiCaption>			pCaption;
		std::unique_ptr<UiCanvas>			pCanvas;
		std::unique_ptr<Gdiplus::Graphics>	bGrpahics;

		std::vector<Gdiplus::Rect>			updateRects;
		std::stack<POINT>					posStack;

		HWND		hwnd;
		HBITMAP		buffer;
		HMENU		hMenu;
		HINSTANCE	hInstance;

		int			width, height;
		bool		maximized = false;

		enum UpdateMode {
			FULLSCREEN,
			SINGLE_RECT,
			RECTS
		} updateMode;
	};
}

#endif // !AWND_H