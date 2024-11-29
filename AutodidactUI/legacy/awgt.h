#ifndef ACWGT_H
#define ACWGT_H

#include "auidll.h"

namespace Autodidact {
	class ApplicationWindow;

	struct AUIDLL POSSTRUCT {
		HWND hwnd;
		int x, y, width, height;
		UINT flag;
		RECT rect;
	};

	class UiCanvas;
	class CanvasWidget {
	public:
		typedef void (*ClickProcess)(CanvasWidget*);

		typedef void (*PaintProcess)(Gdiplus::Graphics& graphics, CanvasWidget* widget);

		AUIDLL CanvasWidget(
			HINSTANCE			hInstance,
			HWND				hParent,
			UiCanvas&			canvas, 
			DPOS				x, 
			DPOS				y, 
			DSIZE				width, 
			DSIZE				height,
			bool				dynamic = false
		);

		AUIDLL virtual ~CanvasWidget();

		AUIDLL virtual CHANGED OnResize(int width, int height, std::vector<POSSTRUCT>* updateList);

		AUIDLL void SetShow(bool show = true);

		AUIDLL void SetEnable(bool enable = true);

		AUIDLL void SetText(LPCWSTR str);

		AUIDLL void RePaint();

		AUIDLL void PaintBuffer();

		AUIDLL inline HWND GetHWND() const;

		AUIDLL WidgetState GetState() const;

		AUIDLL ApplicationWindow& GetWindow() const;

		AUIDLL virtual void OnPaint(Gdiplus::Graphics& graphics);

	public:
		DPOS	x, y;
		DSIZE	width, height;

	protected:
		AUIDLL static LRESULT CALLBACK WidgetProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		AUIDLL virtual void OnLButtonDown(POINT pt);

		AUIDLL virtual void OnLButtonUp(POINT pt);

		AUIDLL virtual void OnMouseMove(POINT pt);

		AUIDLL virtual void OnMouseLeave();

		AUIDLL inline void PreparePos();
	protected:
		HWND	hWidget;
		HWND	hParent;
		HMENU	hMenu;
		bool	tracking	= FALSE;
		LRESULT lresult		= HTNOWHERE;

		ClickProcess	click	= nullptr;
		PaintProcess	paint	= nullptr;

		WidgetState		state	= WidgetState::NORMAL;

		UiCanvas&		canvas;

		const bool		dynamic;
	};

	class Page {
	public:
		AUIDLL Page(HWND hParent);

		AUIDLL void OnResize(int width, int height, std::vector<POSSTRUCT>* updateList);

		AUIDLL void SetShow(bool show = true);

		AUIDLL void AddWidget(CanvasWidget* widget);

		AUIDLL void RemoveWidget(CanvasWidget* widget);

		AUIDLL HWND GetHWND();

	private:
		HWND hParent;

		std::vector<std::unique_ptr<CanvasWidget>> widgets;
	};

	class CanvasPageController : public CanvasWidget {
	public:
		AUIDLL CanvasPageController(
			HINSTANCE			hInstance,
			HWND				hParent,
			UiCanvas&			canvas,
			DPOS				x,
			DPOS				y,
			DSIZE				width,
			DSIZE				height
		);

		AUIDLL void Select(const size_t i);

		AUIDLL CHANGED OnResize(int width, int height, std::vector<POSSTRUCT>* updateList);

		AUIDLL void AddPage(Page* page);

		AUIDLL Page& GetSelected();

		AUIDLL Page& operator[](const size_t i) const;
	private:
		size_t			selected = 0;
		
		std::vector<std::unique_ptr<Page>> pages;
	};

	class UiCanvas {
	public:
		AUIDLL UiCanvas(HINSTANCE, ApplicationWindow&, int y, int width, int height);

		AUIDLL ~UiCanvas();

		AUIDLL void OnResize(int width, int height, bool maximize = false);

		AUIDLL void AddPage(Page* page);

		AUIDLL void SelectPage(const size_t index);

		AUIDLL void PreparePos();

		AUIDLL inline int GetY() const;

		AUIDLL inline HWND GetHWND() const;

		AUIDLL inline ApplicationWindow& GetWindow() const;

		AUIDLL inline Page& operator[](const size_t index) const;

	private:
		static LRESULT CALLBACK CanvasProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		HWND	hWidget;
		HMENU	hMenu;
		bool	tracking = FALSE;

		int		y;
		int		width, height;

		Page*			selected = nullptr;
		ApplicationWindow& 	window;

		std::vector<std::unique_ptr<Page>> pages;
	};

	class RadioButton : public CanvasWidget {
	public:
		typedef void (*Click)();

		struct SubButton;
		typedef void (*Paint)(Gdiplus::Graphics&, const SubButton*, int size, bool focus);

		struct SubButtonRaw {
			DPOS	pos = NULL;
			DSIZE	size = NULL;
			Click	click = NULL;
			Paint	paint = NULL;
			LPCWSTR	text = NULL;

			HINSTANCE hInst = NULL;
			LPCWSTR	icon = NULL;
		};

		struct SubButton {
			DPOS	pos = 0;
			DSIZE	size = 0;
			Click	click = nullptr;
			Paint	paint = nullptr;

			WCHAR	text[16] = { 0 };
			std::unique_ptr<Gdiplus::Image> icon = std::unique_ptr<Gdiplus::Image>();

			int			begin = 0;
			WidgetState state = WidgetState::NORMAL;

			SubButton();

			AUIDLL void operator=(SubButton&);

			AUIDLL void operator=(const SubButtonRaw&);
		};

	public:
		AUIDLL RadioButton(
			HINSTANCE			hInstance,
			HWND				hParent,
			UiCanvas& canvas,
			DPOS				x,
			DPOS				y,
			DSIZE				width,
			DSIZE				height,
			UINT				size,
			UINT				defsel = 0,
			UINT				bottom = 0xffffffff,
			bool				vertical = true
		);

		AUIDLL CHANGED OnResize(int width, int height, std::vector<POSSTRUCT>* updateList) override;

		AUIDLL void OnPaint(Gdiplus::Graphics& graphics) override;

		AUIDLL SubButton& operator[](const UINT index) const;

	protected:
		void OnLButtonDown(POINT pt) override;

		void OnLButtonUp(POINT pt) override;

		void OnMouseMove(POINT pt) override;

		void OnMouseLeave() override;

	private:
		void SelectBarTo(int pos, int size);

		inline void DrawButton(Gdiplus::Graphics& graphics, SubButton* button);

		inline void DrawSelectBar(Gdiplus::Graphics& graphics);

		static void CALLBACK MoveSelectBar(HWND, UINT, UINT_PTR, DWORD);
	private:
		const UINT	size;
		const UINT	bottom;
		const bool	vertical;

		SubButton*	selected = nullptr;
		SubButton*	hovered = nullptr;

		UINT		animate = 0u;

		int			selectPos	= 0;
		int			selectSize	= 0;

		int			lastSelectPos = 0;
		int			lastSelectSize = 0;

		int			selectToPos = 0;
		int			selectToSize = 0;

		std::unique_ptr<SubButton[]>	buttons;
	};

	class PaintWidget : public CanvasWidget {
	public:
		AUIDLL PaintWidget(
			HINSTANCE			hInstance,
			HWND				hParent,
			UiCanvas&			canvas,
			DPOS				x,
			DPOS				y,
			DSIZE				width,
			DSIZE				height,
			PaintProcess		paint
		);
	};
}

#endif // !ACWGT_H
