#pragma once
#ifndef _AUTODIDACT_UI_COLLIDER_H_
#define _AUTODIDACT_UI_COLLIDER_H_

#include "architect.h"

namespace Autodidact {
	class Window;

	class AUIDLL Collider : public DRECT {
	public:
		Collider(
			const DRECT& drect,
			Collider* pParent = nullptr
		);

		Collider(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Collider* pParent = nullptr
		);

		bool Collide(const POINT& pt);

		bool Changed(const Gdiplus::Rect& last);

	protected:
		void inline Resize(int nWidth, int nHeight, OUT Gdiplus::Rect* last);

	public:
		int globeX;
		int globeY;

	protected:
		Collider* pParent;
	};
}

#endif // !_AUTODIDACT_UI_COLLIDER_H_