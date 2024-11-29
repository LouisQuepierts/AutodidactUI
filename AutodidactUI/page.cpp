#include "autodidact/ui/page.h"
#include "autodidact/ui/division.h"
#include "autodidact/ui/window.h"

namespace Autodidact {
	PageControl::PageControl(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Component& parent,
		UINT		size
	) :
		Component(x, y, width, height, parent, false),
		size(size),
		pages(new Page[size]()),
		selected(pages.get()) {
	}

	PageControl::PageControl(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Window& window,
		UINT		size
	) :
		Component(x, y, width, height, window, false),
		size(size),
		pages(new Page[size]),
		selected(pages.get()) {
	}

	void PageControl::Select(const UINT index) {
		if (size > index) {
			selected = &pages[index];
			selected->OnResize(width, height, color ? color : window.GetBgColor(), nullptr);
			window.ClearHoverStack(this);
			clearBg = true;
			UpdateBuffer();
		}
	}

	Page& PageControl::operator[](const UINT index) {
		return pages[index];
	}

	void PageControl::OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) {
		Gdiplus::Rect last;
		Collider::Resize(nWidth, nHeight, &last);

		if (selected && Changed(last)) {
			selected->OnResize(nWidth, nHeight, eraseColor, repaint);
		}
	}

	void PageControl::GetUpdateList(std::queue<Component*>& update) {
		for (auto& division : selected->divisions) {
			division->GetUpdateList(update);
		}
	}

	void PageControl::GetUpdateList(Filter filter, std::queue<Component*>& update) {
		for (auto& division : selected->divisions) {
			division->GetUpdateList(filter, update);
		}
	}

	void PageControl::OnTimer() {
		//window.ClearHoverStack(this);
		//window.ClearPosStack();
		//UpdateBuffer();
		window.ForceUpdate();
		window.KillTimer(this);
	}

	void PageControl::OnPaint(Gdiplus::Graphics& graphics) {
		if (clearBg) {
			Gdiplus::SolidBrush brush(color ? color : window.GetBgColor());
			graphics.FillRectangle(&brush, 0, 0, width, height);
			clearBg = false;
		}
		
		if (selected) {
			for (auto& division : selected->divisions) {
				division->UpdateBuffer();
			}
		}
	}

	Division& PageControl::Divide(const DRECT& rc, UINT page, int hit) {
		Division* division = new Division(rc.x, rc.y, rc.width, rc.height, *this, hit);
		auto& divisions = pages[page].divisions;
		divisions.push_back(std::unique_ptr<Division>(division));
		division->OnResize(width, height, NULL, NULL);
		division->UpdateBuffer();
		return *division;
	}

	bool PageControl::GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted) {
		if (!Collider::Collide(pt) || !selected) {
			return false;
		}

		for (auto& division : selected->divisions) {
			if (division->GetHittedComponents(pt, hitted))
				return true;
		}
		return false;
	}

	Page::Page() : divisions() {
	}

	void Page::OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) {
		for (auto& division : divisions) {
			division->OnResize(nWidth, nHeight, eraseColor, repaint);
		}
	}

	void Page::UpdateBuffer() {
		for (auto& division : divisions) {
			division->UpdateBuffer();
		}
	}

	inline Division& Page::operator[](const size_t index) {
		return *divisions[index].get();
	}
}