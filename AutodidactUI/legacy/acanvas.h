#ifndef ACANVAS_H
#define ACANVAS_H

#include "auidll.h"

namespace Autodidact {
	class ApplicationWindow;

	class HWNDInstance {
	public:
		HWND	hwnd;
		HMENU	hMenu;
	};

	class Widget {
	public:
		AUIDLL Widget(
			ApplicationWindow&	window,
			DPOS				x,
			DPOS				y,
			DSIZE				width,
			DSIZE				height
		);

		AUIDLL virtual void OnDraw(Gdiplus::Graphics& graphics);

		AUIDLL virtual bool OnResize(int nWidth, int nHeight);

		AUIDLL bool HitTest(POINT pt);

		AUIDLL void GetRect(RECT* rc);

	public:
		DPOS	x, y;
		DSIZE	width, height;

		WidgetState state = WidgetState::NORMAL;

	protected:
		ApplicationWindow& window;
	};

	class Region : public Widget {
	public:
		void OnDraw(Gdiplus::Graphics& graphics) override;

		bool OnResize(int nWidth, int nHeight) override;

		void AddWidget(Widget* pWidget);

	private:
		std::vector<std::unique_ptr<Widget>> widgets;
	};

	class Canvas {
	public:
		Canvas(
			HINSTANCE			hInstance,
			ApplicationWindow&	window,
			int					y,
			int					width,
			int					height
		);

		~Canvas();

		void OnResize(int nWidth, int nHeight);

		AUIDLL void AddWidget(Widget* pWidget);

	private:
		inline void PaintWidget(Widget* pWidget);

		void UpdateBuffer();

		static LRESULT CALLBACK CanvasProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		HWND	hwnd;
		HMENU	hMenu;
		bool	tracking = FALSE;

		int		y;
		int		width, height;

		Gdiplus::Bitmap		buffer;
		Gdiplus::Graphics	bGraphics;

		ApplicationWindow&	window;

		Widget* hovered;

		std::vector<std::unique_ptr<Widget>> widgets;
	};
}

#endif // !ACANVAS_H
