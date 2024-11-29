#include "autodidact/ui/window.h"
#include "autodidact/ui/component.h"
#include "extern.h"

namespace Autodidact {
	Component::Component(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Component&	parent,
		RESPONSE	response,
		int			hit
	) :
		Collider(x, y, width, height, &parent),
		window(parent.window),
		response(response),
		hittest(hit),
		parent(reinterpret_cast<UINT_PTR>(&parent)) {
		//printf("Component Parent: %p [%d, %d]\n", &parent, parent.globeX, parent.globeY);
	}

	Component::Component(
		DPOS		x,
		DPOS		y,
		DSIZE		width,
		DSIZE		height,
		Window&		window,
		RESPONSE	response,
		int			hit
	) :
		Collider(x, y, width, height, &window),
		window(window),
		response(response),
		hittest(hit),
		parent(reinterpret_cast<UINT_PTR>(&window)) {
	}

	Component::Component(
		const DRECT&	drect,
		Component&		parent,
		RESPONSE		response,
		int				hit
	) :
		Collider(drect, &parent),
		window(parent.window),
		response(response),
		hittest(hit),
		parent(reinterpret_cast<UINT_PTR>(&parent)) {
	}

	Component::Component(
		const DRECT&	drect,
		Window&			window,
		RESPONSE		response,
		int				hit
	) :
		Collider(drect, &window),
		window(window),
		response(response),
		hittest(hit),
		parent(reinterpret_cast<UINT_PTR>(&window)) {
	}

	Component::~Component() {
	}

	bool Component::GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted) {
		if (enable && !static_ && Collider::Collide(pt)) {
			hitted.push(this);
			return true;
		}
		else {
			return false;
		}
	}

	void Component::LButtonDown(WPARAM wparam, const POINT& pt) {
		state = Component::PRESSED;
		UpdateBuffer();
	}

	void Component::LButtonUp(WPARAM wparam, const POINT& pt) {
		state = Component::HOVERED;
		UpdateBuffer();
	}

	void Component::MButtonDown(WPARAM wparam, const POINT& pt) {
	}

	void Component::MButtonUp(WPARAM wparam, const POINT& pt) {
	}

	void Component::RButtonDown(WPARAM wparam, const POINT& pt) {
	}

	void Component::RButtonUp(WPARAM wparam, const POINT& pt) {
	}

	void Component::UpdateBuffer() {
		if (enable) {
			window.PushPos({ globeX, globeY });

			Gdiplus::Graphics& graphics = *window.GetGrpahics();

			OnPaint(graphics);

			window.UpdateRect({ 0, 0, width, height });
			window.PopPos();
		}
	}

	void Component::UpdateBufferChained() {
		if (enable) {
			UpdateBuffer();
			std::queue<Component*> update;
			GetUpdateList(update);

			while (!update.empty()) {
				update.front()->UpdateBuffer();
				update.pop();
			}
		}
	}

	void Component::GetUpdateList(std::queue<Component*>& update) {
	}

	void Component::GetUpdateList(Filter filter, OUT std::queue<Component*>& update) {
		if (enable && filter(this)) {
			update.push(this);
		}
	}

	void Component::OnTimer() {
	}

	void Component::OnAnimate(Gdiplus::Graphics& graphics) {
	}

	void Component::OnSetFocus() {
	}

	void Component::OnKillFocus() {
	}

	void Component::OnPaint(Gdiplus::Graphics& graphics) {
		Gdiplus::SolidBrush brush(0xffb8b8b8);
		graphics.FillRectangle(&brush, 3, 3, width - 6, height - 6);

		if (state == Component::HOVERED) {
			Gdiplus::Pen pen(0xffff0000, 2);
			graphics.DrawRectangle(&pen, 4, 4, width - 8, height - 8);
		}
	}

	void Component::MouseMove(WPARAM wparam, const POINT& pt) {

	}

	void Component::MouseEnter() {
		state = HOVERED;
		UpdateBuffer();
	}

	void Component::MouseLeave() {
		state = NORMAL;
		UpdateBuffer();
	}

	void Component::MouseWheelH(WPARAM wparam) {
	
	}

	void Component::MouseWheelV(WPARAM wparam) {

	}

	void Component::CharInput(wchar_t c) {

	}

	void Component::KeyDown(WPARAM wparam, LPARAM lparam) {

	}

	void Component::KeyUp(WPARAM wparam, LPARAM lparam) {

	}

	void Component::OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) {
		Gdiplus::Rect last;
		Collider::Resize(nWidth, nHeight, &last);

		if (repaint) {
			repaint->push({ this, last, eraseColor ? eraseColor : window.GetBgColor() });
		}
	}

	int Component::GetHitResult() {
		return hittest;
	}

	bool Component::IsClient() {
		return hittest == HTCLIENT;
	}

	Component::State Component::GetState() {
		return state;
	}

	void ListComponent::AddComponent(Component* component, bool update) {
		if (update) {
			component->OnResize(width, height, NULL, NULL);
			component->UpdateBuffer();
		}
		components.push_back(std::unique_ptr<Component>(component));
	}

	bool ListComponent::GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted) {
		if (enable && Collider::Collide(pt)) {
			hitted.push(this);
			for (auto iter = components.rbegin(); iter != components.rend(); iter++) {
				if ((*iter)->GetHittedComponents(pt, hitted))
					break;
			}
			return true;
		}

		return false;
	}

	void ListComponent::OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) {
		Gdiplus::Rect last;
		Collider::Resize(nWidth, nHeight, &last);

		if (Changed(last)) {
			for (auto& component : components) {
				component->OnResize(width, height, color ? color : eraseColor, repaint);
			}
		}
	}

	void ListComponent::GetUpdateList(std::queue<Component*>& update) {
		for (auto iter = components.rbegin(); iter != components.rend(); iter++) {
			if ((*iter)->enable) {
				update.push((*iter).get());
				(*iter)->GetUpdateList(update);
			}
		}
	}

	void ListComponent::GetUpdateList(Filter filter, std::queue<Component*>& update) {
		if (filter(this)) {
			for (auto iter = components.rbegin(); iter != components.rend(); iter++) {
				if ((*iter)->enable) {
					update.push((*iter).get());
					(*iter)->GetUpdateList(filter, update);
				}
			}
		}
	}

	void ListComponent::MouseEnter() {
	}

	void ListComponent::MouseLeave() {
	}

	void ListComponent::LButtonDown(WPARAM wparam, const POINT& pt) {
	}

	void ListComponent::LButtonUp(WPARAM wparam, const POINT& pt) {
	}

	Component& ListComponent::operator[](const UINT index) {
		return *components[index];
	}

	void ListComponent::OnPaint(Gdiplus::Graphics& graphics) {
		Gdiplus::Region clip;
		graphics.GetClip(&clip);
		graphics.SetClip(Gdiplus::Rect{ 0, 0, width, height });
		for (auto& component : components) {
			component->UpdateBuffer();
		}
		graphics.SetClip(&clip);
	}
	
	PaintComponent::PaintComponent(
		DPOS			x,
		DPOS			y,
		DSIZE			width,
		DSIZE			height,
		Component&		parent,
		PaintProcess	paint,
		RESPONSE		response,
		int				hit
	) : Component(x, y, width, height, parent, response, hit),
		paint(paint) {
	}

	void PaintComponent::OnPaint(Gdiplus::Graphics& graphics) {
		if (paint)
			paint(this, graphics);
	}
}