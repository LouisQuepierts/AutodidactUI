#pragma once
#ifndef _AUTODIDACT_UI_CAPTION_H_
#define _AUTODIDACT_UI_CAPTION_H_

#include "component.h"

namespace Autodidact {
	class CaptionTitle : public Component {
	public:
		CaptionTitle(
			DPOS		x,
			DPOS		y,
			Component&	parent
		);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;
	};

	class CaptionMinimizeButton : public Component {
	public:
		CaptionMinimizeButton(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			Component&	parent
		);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;
	};

	class CaptionMaximizeButton : public Component {
	public:
		CaptionMaximizeButton(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			Component& parent
		);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;
	};

	class CaptionCloseButton : public Component {
	public:
		CaptionCloseButton(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			Component& parent
		);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;
	};
}

#endif // !_AUTODIDACT_UI_CAPTION_H_
