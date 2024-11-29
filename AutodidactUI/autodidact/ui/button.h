#pragma once
#ifndef _AUTODIDACT_UI_BUTTON_H_
#define _AUTODIDACT_UI_BUTTON_H_

#include "component.h"

namespace Autodidact {
	class AUIDLL Button : public Component {
	public:
		typedef void (*ClickProcess)(Component*, WPARAM, const POINT&);

	public:
		Button(
			DPOS			x,
			DPOS			y,
			DSIZE			width,
			DSIZE			height,
			Component&		parent,
			ClickProcess	click
		);

		void LButtonUp(WPARAM wparam, const POINT& pt) override;

	protected:
		ClickProcess click;
	};

	class VerticalSelection : public ListComponent {
	public:
		class Selection;
		typedef void (*Click)();
		typedef void (*Paint)(Gdiplus::Graphics&, const Selection*);

		struct SelectionStruct {
			DPOS				pos;
			DSIZE				size;
			bool				forward;
			LPCWSTR				text;
			Gdiplus::Bitmap*	icon;
			Click				click;
			Paint				paint;
		};

	public:
		class Selection : public Component {
		public:
			Selection(
				DPOS				x,
				DPOS				y,
				DSIZE				width,
				DSIZE				height,
				VerticalSelection&	parent
			);

			void LButtonDown(WPARAM wparam, const POINT& pt) override;

			void OnResize(int nWidth, int nHeight, int begin, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint);

			AUIDLL void operator<<(const SelectionStruct&);
		protected:
			void OnPaint(Gdiplus::Graphics& graphics) override;

		public:
			Click click	= NULL;
			Paint paint = NULL;

			WCHAR text[16] = { 0 };

			std::unique_ptr<Gdiplus::Bitmap> icon;
			bool forward = true;
		};

	public:
		AUIDLL VerticalSelection(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Component&	parent,
			UINT		size
		);

		AUIDLL void UpdateSelections();

		bool Select(Selection* selection);

		void OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) override;

		void OnAnimate(Gdiplus::Graphics& graphics) override;

		AUIDLL Selection& operator[](size_t index);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;

	private:
		void MoveSelectBar(int pos, int size);

		const UINT size;

		Selection* selected = nullptr;

		UINT	animate = 0u;

		struct SelectBar {
			int pos			= 0;
			int size		= 30;

			int lastPos		= 0;
			int lastSize	= 30;

			int nextPos		= 0;
			int nextSize	= 30;
		} selectBar;
	};
}
#endif // !_AUTODIDACT_UI_BUTTON_H_
