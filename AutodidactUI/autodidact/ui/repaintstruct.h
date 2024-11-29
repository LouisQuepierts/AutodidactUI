#pragma once
#ifndef _AUTODIDACT_UI_REPAINTSTRUCT_H_
#define _AUTODIDACT_UI_REPAINTSTRUCT_H_
#include <gdiplus.h>

namespace Autodidact {
	class Component;

	struct REPAINTSTRUCT {
		Component* const	component;

		Gdiplus::Rect	eraseRect;
		Gdiplus::ARGB	eraseColor;
	};
}

#endif // !_AUTODIDACT_UI_REPAINTSTRUCT_H_