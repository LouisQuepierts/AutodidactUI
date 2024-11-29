#include <iostream>
#include "autodidact/ui/button.h"
#include "autodidact/ui/window.h"
#include "autodidact/mathutil.h"
#include "extern.h"

namespace Autodidact {
	Button::Button(
		DPOS			x,
		DPOS			y,
		DSIZE			width,
		DSIZE			height,
		Component&		parent,
		ClickProcess	click
	) :
		Component(x, y, width, height, parent, true, HTCLIENT),
		click(click) {
	}

	void Button::LButtonUp(WPARAM wparam, const POINT& pt) {
		if (click) {
			click(this, wparam, pt);
		}

		state = Component::HOVERED;
		UpdateBuffer();
	}

	VerticalSelection::VerticalSelection(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Component&	parent,
		UINT		size
	) :
		ListComponent(x, y, width, height, parent, RESPONSE_INTERACTABLE | RESPONSE_RESIZE, HTCLIENT),
		size(size){
		
		int selectionWidth = width - 10;
		int begin = 0;
		for (UINT i = 0; i < size; i++) {
			Selection* select = new Selection(10, 0, selectionWidth, 30, *this);
			select->y.real += begin;
			select->UpdateBuffer();
			begin += 30;
			ListComponent::AddComponent(select, false);

			if (!i) {
				Select(select);
			}
		}
	}

	void VerticalSelection::OnResize(
		int							nWidth, 
		int							nHeight, 
		Gdiplus::ARGB				eraseColor, 
		std::queue<REPAINTSTRUCT>*	repaint
	) {
		Gdiplus::Rect last;
		Collider::Resize(nWidth, nHeight, &last);

		int beginForward = 0;
		int beginBackward = 0;

		if (Changed(last)) {
			for (auto& component : components) {
				Selection* selection = reinterpret_cast<Selection*>(component.get());

				if (selection->forward) {
					selection->OnResize(width, height, beginForward, color ? color : eraseColor, repaint);
					beginForward += selection->height + selection->y.base;
				}
				else {
					beginBackward += selection->height;
					selection->OnResize(width, height, beginBackward, color ? color : eraseColor, repaint);
					beginBackward += selection->y.base;
				}

				selection->UpdateBuffer();
			}

			MoveSelectBar(selected->y, selected->height);
		}
	}

	void VerticalSelection::OnAnimate(Gdiplus::Graphics& graphics) {
		animate++;
		//printf("Timer %d\n", animate);

		float rate = 1 - (animate / 20.0f);
		selectBar.pos = smooth_lerp<int>(selectBar.nextPos, selectBar.lastPos, rate);
		selectBar.size = smooth_lerp<int>(selectBar.nextSize, selectBar.lastSize, rate);

		OnPaint(graphics);
		window.UpdateRect({ globeX, globeY, 20, height });

		if (animate == 20) {
			//printf("Timer Stop\n");
			animate = 0;
			window.KillPaintTimer(this);
		}
	}

	VerticalSelection::Selection& VerticalSelection::operator[](size_t index) {
		if (index > size)
			return (VerticalSelection::Selection&)*components[0];
		else
			return (VerticalSelection::Selection&)*components[index];
	}

	void VerticalSelection::OnPaint(Gdiplus::Graphics& graphics) {
		HRGN hRgn;

		{
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			graphics.FillRectangle(&brush, 0, 0, 10, height + 200);
			hRgn = CreateRoundRectRgn(5, selectBar.pos, 10, selectBar.pos + selectBar.size, 2, 2);
		}

		Gdiplus::Region region(hRgn);
		Gdiplus::SolidBrush brush(pResources->colorTheme);

		graphics.FillRegion(&brush, &region);

		DeleteObject(hRgn);
	}

	void VerticalSelection::MoveSelectBar(int pos, int size) {
		if (selectBar.pos != pos || selectBar.size != size) {
			selectBar.lastPos = selectBar.pos;
			selectBar.lastSize = selectBar.size;
			selectBar.nextPos = pos;
			selectBar.nextSize = size;

			if (!animate) {
				//window.SetTimer(this, 15);
				window.SetPaintTimer(this, PAINTTIMERPROC(& VerticalSelection::OnAnimate));
			}
			else {
				animate = 0;
			}
		}
	}


	void VerticalSelection::UpdateSelections() {
		int beginForward = 0;
		int beginBackward = 0;

		for (auto& component : components) {
			Selection* selection = reinterpret_cast<Selection*>(component.get());

			if (selection->forward) {
				selection->OnResize(width, height, beginForward, NULL, NULL);
				beginForward += selection->height + selection->y.base;
			}
			else {
				beginBackward += selection->height;
				selection->OnResize(width, height, beginBackward, NULL, NULL);
				beginBackward += selection->y.base;
			}

			selection->UpdateBuffer();
		}

		MoveSelectBar(selected->y, selected->height);
	}

	bool VerticalSelection::Select(Selection* selection) {
		//printf("%p Window %p\n", this, &window);
		if (selected == selection) {
			return false;
		}
		else {
			selected = selection;
			MoveSelectBar(selected->y, selected->height);
			return true;
		}
	}

	VerticalSelection::Selection::Selection(
		DPOS				x,
		DPOS				y,
		DSIZE				width,
		DSIZE				height,
		VerticalSelection&	parent
	) :
		Component(x, y, width, height, parent, RESPONSE_INTERACTABLE | RESPONSE_RESIZE, HTCLIENT),
		icon() {
	}

	void VerticalSelection::Selection::LButtonDown(WPARAM wparam, const POINT& pt) {
		state = Component::PRESSED;

		VerticalSelection* parent = reinterpret_cast<VerticalSelection*>(this->parent);
		//printf("%p Parent: %p\n", this, parent);
		if (parent->Select(this)) {
			if (click) {
				click();
			}
		}
	}

	void VerticalSelection::Selection::OnResize(
		int							nWidth,
		int							nHeight,
		int							begin,
		Gdiplus::ARGB				eraseColor,
		std::queue<REPAINTSTRUCT>*	repaint
	) {
		Gdiplus::Rect last{ globeX, globeY, width, height };
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHeight);
		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHeight);

		if (forward) {
			y.real += begin;
		}
		else {
			y.real -= begin;
		}

		if (pParent) {
			globeX = pParent->globeX + x;
			globeY = pParent->globeY + y;
		}
		else {
			globeX = x;
			globeY = y;
		}

		if (repaint && Changed(last)) {
			repaint->push({ this, last, eraseColor ? eraseColor : window.GetBgColor() });
		}
	}

	void VerticalSelection::Selection::operator<<(const SelectionStruct& in) {
		forward = in.forward;
		y		= in.pos;
		height	= in.size;
		click	= in.click;
		paint	= in.paint;

		wcscpy_s(text, in.text);
		if (text[15]) {
			text[15] = 0;
		}

		icon.reset(in.icon);
	}

	void VerticalSelection::Selection::OnPaint(Gdiplus::Graphics& graphics) {
		if (paint) {
			paint(graphics, this);
		}
		else {
			Gdiplus::Font font(fontName, 15, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			int color = 0xffb8b8b8;
			switch (state) {
			case Component::HOVERED:
				color = 0xfffefefe;
				break;
			case Component::PRESSED:
				color = 0xfff5f5f5;
				break;
			}

			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, 0, 0, width, height);

			Gdiplus::SolidBrush brushText(0xff000000);
			graphics.DrawString(text, -1, &font, { 46, 15 }, &brushText);
		}
	}
}