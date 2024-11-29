#pragma once

#include "division.h"
#include "repaintstruct.h"

namespace Autodidact {
	class Page {
	public:
		AUIDLL Page();

		AUIDLL void OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint);

		AUIDLL void UpdateBuffer();

		AUIDLL inline Division& operator[](const size_t index);

	public:
		std::vector<std::unique_ptr<Division>> divisions;
	};

	class PageControl : public Component {
	public:
		AUIDLL PageControl(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Component& parent,
			UINT		size
		);

		AUIDLL PageControl(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Window& window,
			UINT		size
		);

		AUIDLL void Select(const UINT index);

		AUIDLL inline Page& operator[](const UINT index);

		AUIDLL Division& Divide(const DRECT& rc, UINT page, int hit = HTCLIENT);

		AUIDLL bool GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted) override;

		AUIDLL void OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) override;

		AUIDLL void GetUpdateList(OUT std::queue<Component*>& update) override;

		AUIDLL void GetUpdateList(Filter filter, OUT std::queue<Component*>& update) override;

		void OnTimer() override;

	protected:
		AUIDLL void OnPaint(Gdiplus::Graphics& graphics) override;

	public:
		DWORD	color = NULL;
	private:
		UINT	size;
		Page*	selected;

		std::unique_ptr<Page[]> pages;

		bool clearBg = false;
	};
}