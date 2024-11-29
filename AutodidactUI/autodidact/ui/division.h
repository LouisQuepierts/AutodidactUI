#pragma once
#ifndef _AUTODIDACT_UI_DIVISION_H_
#define _AUTODIDACT_UI_DIVISION_H_
#include "architect.h"
#include "component.h"

namespace Autodidact {
	class Window;

	class Division final : public ListComponent {
	public:
		AUIDLL Division(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Window&		window,
			RESPONSE	response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int			hit			= HTCLIENT,
			HINSTANCE	hInstance	= nullptr
		);

		AUIDLL Division(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Component&	parent,
			RESPONSE	response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int			hit			= HTCLIENT,
			HINSTANCE	hInstance	= nullptr
		);

		AUIDLL ~Division();

		AUIDLL Division& Divide(const DRECT& rc, int hit = HTCLIENT);

	private:
		static LRESULT CALLBACK Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		HWND	hwnd = NULL;
		HMENU	hmenu = NULL;
	};
}

#endif // !_AUTODIDACT_UI_DIVISION_H_