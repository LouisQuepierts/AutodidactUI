#include "autodidact/ui/dint.h"

namespace Autodidact {
	DINT::DINT(int base)
		: base(base)
		, process(nullptr)
		, real(base)
		, mode(DINT::Mode::LEFT) {
	}

	DINT::DINT(UpdateProcess process)
		: process(process)
		, base(0)
		, real(0)
		, mode(DINT::Mode::FUNCTION) {
	}

	DINT::DINT(int base, UpdateProcess process)
		: process(process)
		, base(base)
		, real(base)
		, mode(DINT::Mode::FUNCTION) {
	}

	DINT::DINT(int base, DINT::Mode mode)
		: process(nullptr)
		, base(base)
		, real(base)
		, mode(mode) {
	}

	DINT::operator int() {
		return real;
	}

	void UpdateDPOS(DPOS& dPos, int pSize) {
		switch (dPos.mode) {
		case DINT::Mode::LEFT:
			dPos.real = dPos.base;
			break;
		case DINT::Mode::RIGHT:
			dPos.real = pSize - dPos.base;
			break;
		case DINT::Mode::LMIDDLE:
			dPos.real = pSize / 2 - dPos.base;
			break;
		case DINT::Mode::RMIDDLE:
			dPos.real = dPos.base + pSize / 2;
			break;
		case DINT::Mode::FUNCTION:
			dPos.real = dPos.process(dPos, DINT(0), pSize);
			break;
		}
	}

	void UpdateDSIZE(DSIZE& dSize, DPOS& dPos, int pSize) {
		switch (dSize.mode) {
		case DINT::Mode::LEFT:
			dSize.real = dSize.base;
			break;
		case DINT::Mode::RIGHT:
			dSize.real = pSize - dPos.real - dSize.base;
			break;
		case DINT::Mode::LMIDDLE:
			dSize.real = pSize / 2 - dPos.real - dSize.base;
			break;
		case DINT::Mode::RMIDDLE:
			dSize.real = pSize / 2 - dPos.real + dSize.base;
			break;
		case DINT::Mode::FUNCTION:
			dSize.real = dSize.process(dSize, dPos, pSize);
			break;
		}

		dSize.real = (((dSize.real) > (0)) ? (dSize.real) : (0));
	}
}