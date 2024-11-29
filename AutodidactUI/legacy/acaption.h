#ifndef ACAPTION_H
#define ACAPTION_H

#include "auidll.h"

namespace Autodidact {
	class ApplicationWindow;

	class AUIDLL CaptionWidget {
	public:
		CaptionWidget(
			ApplicationWindow& window,
			DPOS				x,
			DPOS				y,
			DSIZE				width,
			DSIZE				height,
			LRESULT				htresult = HTCLIENT
		);

		virtual void OnDraw(Gdiplus::Graphics& graphics, bool focus);

		virtual void Execute();

		LRESULT Hittest(POINT mouse);

		void GetRect(RECT* rc);

		CHANGED OnResize(int width, int height);
	public:
		DPOS		x, y;
		DSIZE		width, height;
		LRESULT		htresult;

		bool		enable = true;
		WidgetState state = WidgetState::NORMAL;

	protected:
		ApplicationWindow& window;
	};

	class UiCaption {
	public:
		UiCaption(
			HINSTANCE			hInstance,
			ApplicationWindow& window,
			int					width,
			int					height
		);

		~UiCaption();

		void OnResize(int width, int height, bool maximize = false);

		void AddWidget(CaptionWidget* widget);

		void SetFocus(bool focus = true);

		void UpdateBuffer();

		void DrawTitle();

	private:
		inline void PaintWidget(CaptionWidget* widget);

		static LRESULT CALLBACK CaptionProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		HWND	hWidget;
		HMENU	hMenu;
		bool	tracking = FALSE;
		bool	focus = TRUE;
		LRESULT	lresult = HTNOWHERE;

		int		width, height;

		CaptionWidget* hovered = nullptr;
		ApplicationWindow& window;

		std::vector<std::unique_ptr<CaptionWidget>> widgets;
	};

	class CaptionButtonMinimize : public CaptionWidget {
	public:
		CaptionButtonMinimize(ApplicationWindow& window, DPOS x, DPOS y);

		void OnDraw(Gdiplus::Graphics& graphics, bool focus);

		void Execute();
	};

	class CaptionButtonMaximize : public CaptionWidget {
	public:
		CaptionButtonMaximize(ApplicationWindow& window, DPOS x, DPOS y);

		void OnDraw(Gdiplus::Graphics& graphics, bool focus);

		void Execute();
	};

	class CaptionButtonClose : public CaptionWidget {
	public:
		CaptionButtonClose(ApplicationWindow& window, DPOS x, DPOS y);

		void OnDraw(Gdiplus::Graphics& graphics, bool focus);

		void Execute();
	};
}

#endif // !ACAPTION_H
