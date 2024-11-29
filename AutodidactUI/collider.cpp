#include "autodidact/ui/architect.h"
#include "autodidact/ui/collider.h"
#include "autodidact/ui/window.h"

namespace Autodidact {
	Collider::Collider(
		const DRECT& drect,
		Collider* pParent
	) :
		DRECT(drect),
		pParent(pParent),
		globeX(pParent ? x + pParent->globeX : int(x)),
		globeY(pParent ? y + pParent->globeY : int(y)) {
	}

	Collider::Collider(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Collider*	pParent
	) :
		DRECT{ x, y, width, height },
		pParent(pParent),
		globeX(pParent ? x + pParent->globeX : int(x)),
		globeY(pParent ? y + pParent->globeY : int(y)) {
	}

	void Collider::Resize(int nWidth, int nHeight, OUT Gdiplus::Rect* last) {
		*last = { globeX, globeY, width, height };
		UpdateDPOS(x, nWidth);
		UpdateDPOS(y, nHeight);
		UpdateDSIZE(width, x, nWidth);
		UpdateDSIZE(height, y, nHeight);

		if (pParent) {
			globeX = pParent->globeX + x;
			globeY = pParent->globeY + y;
		}
		else {
			globeX = x;
			globeY = y;
		}
	}

	bool Collider::Collide(const POINT& pt) {
		return pt.x >= globeX && pt.x <= globeX + width && pt.y >= globeY && pt.y <= globeY + height;
	}

	bool Collider::Changed(const Gdiplus::Rect& last) {
		return globeX != last.X || globeY != last.Y || width != last.Width || height != last.Height;
	}
}