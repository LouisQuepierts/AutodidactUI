#include "autodidact/ui/window.h"
#include "autodidact/ui/caption.h"
#include "extern.h"

namespace Autodidact {
	CaptionTitle::CaptionTitle(
		DPOS		x,
		DPOS		y,
		Component&	parent
	) : Component(x, y, 300, parent.height.real, parent, RESPONSE_FOCUS, HTCAPTION) {
		printf("Caption Parent: %p [%d, %d]\n", reinterpret_cast<Collider*>(&parent), parent.globeX, parent.globeY);
	}

	void CaptionTitle::OnPaint(Gdiplus::Graphics& graphics) {
		{
			Gdiplus::SolidBrush brush(window.GetBgColor());
			graphics.FillRectangle(&brush, 0, 0, width, height);
		}

		Gdiplus::Font font(fontName, 15, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(window.Focused() ? 0xff000000 : 0xffc6c6c6);

		graphics.DrawImage(pResources->bIcon.get(), 20, 20, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		graphics.DrawString(appName, -1, &font, Gdiplus::PointF(57, 20), &brush);

		window.UpdateRect({ 0, 0, this->width, this->height });
	}

	CaptionMinimizeButton::CaptionMinimizeButton(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		Component&	parent
	) : Component(x, y, width, parent.height.real, parent, RESPONSE_INTERACTABLE | RESPONSE_FOCUS | RESPONSE_RESIZE, HTMINBUTTON) {
	}

	void CaptionMinimizeButton::OnPaint(Gdiplus::Graphics& graphics) {
		Gdiplus::Color color;

		switch (state) {
		case Component::NORMAL:
			color = 0xfff0f0f0;
			break;
		case Component::HOVERED:
			color = 0xfffefefe;
			break;
		case Component::PRESSED:
			color = 0xfff5f5f5;
			break;
		}
		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, 0, 0, 60, 60);

		if (!window.Focused() && state == Component::NORMAL) {
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
				0.75f,	0.75f,	0.75f,	0.0f,	1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
			graphics.DrawImage(pResources->bMinimize.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		else {
			graphics.DrawImage(pResources->bMinimize.get(), 22, 22, 16, 16);
		}
	}

	CaptionMaximizeButton::CaptionMaximizeButton(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		Component& parent
	) : Component(x, y, width, parent.height.real, parent, RESPONSE_INTERACTABLE | RESPONSE_FOCUS | RESPONSE_RESIZE, HTMAXBUTTON) {
	}
	
	void CaptionMaximizeButton::OnPaint(Gdiplus::Graphics& graphics) {

		Gdiplus::Color color;

		switch (state) {
		case Component::NORMAL:
			color = 0xfff0f0f0;
			break;
		case Component::HOVERED:
			color = 0xfffefefe;
			break;
		case Component::PRESSED:
			color = 0xfff5f5f5;
			break;
		}

		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, 0, 0, 60, 60);

		if (!window.Focused() && state == Component::NORMAL) {
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
				0.75f,	0.75f,	0.75f,	0.0f,	1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
			graphics.DrawImage(window.Maximized() ? pResources->bRestore.get() : pResources->bMaximize.get(), Gdiplus::Rect{22, 22, 16, 16}, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		else {
			graphics.DrawImage(window.Maximized() ? pResources->bRestore.get() : pResources->bMaximize.get(), 22, 22, 16, 16);
		}
	}

	CaptionCloseButton::CaptionCloseButton(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		Component& parent
	) : Component(x, y, width, parent.height.real, parent, RESPONSE_INTERACTABLE | RESPONSE_FOCUS | RESPONSE_RESIZE, HTCLOSE) {
	}

	void CaptionCloseButton::OnPaint(Gdiplus::Graphics& graphics) {
		switch (state) {
		case Component::NORMAL:
		{
			Gdiplus::SolidBrush brush(0xfff0f0f0);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);

			if (window.Focused()) {
				graphics.DrawImage(pResources->bClose.get(), 22, 22, 16, 16);
			}
			else {
				Gdiplus::ImageAttributes	imageAttribute;

				Gdiplus::ColorMatrix matrix = {
					1.0f,	0.0f,	0.0f,	0.0f,	0.0f,
					0.0f,	1.0f,	0.0f,	0.0f,	0.0f,
					0.0f,	0.0f,	1.0f,	0.0f,	0.0f,
					0.0f,	0.0f,	0.0f,	1.0f,	0.0f,
					0.75f,	0.75f,	0.75f,	0.0f,	1.0f
				};

				imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
				graphics.DrawImage(pResources->bClose.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
			}
		}
		break;
		case Component::HOVERED:
		{
			Gdiplus::ImageAttributes	imageAttribute;

			Gdiplus::ColorMatrix matrix = {
				1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f
			};

			imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

			Gdiplus::SolidBrush brush(0xffe81123);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);
			graphics.DrawImage(pResources->bClose.get(), Gdiplus::Rect{ 22, 22, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
		}
		break;
		case Component::PRESSED:
		{
			Gdiplus::SolidBrush brush(0xfff1707a);
			graphics.FillRectangle(&brush, 0, 0, 60, 60);
			graphics.DrawImage(pResources->bClose.get(), 22, 22, 16, 16);
		}
		break;
		}
	}
}