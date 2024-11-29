#pragma once
#ifndef _AUTODIDACT_UI_DINT_H_
#define _AUTODIDACT_UI_DINT_H_

#ifndef __EXPORT
#define AUIDLL __declspec(dllexport)
#else
#define AUIDLL __declspec(dllimport)
#endif

namespace Autodidact {
	struct AUIDLL DINT {
		typedef int VDPOS;

		typedef int (*UpdateProcess)(DINT&, VDPOS, int);

		int				base;
		int				real;
		UpdateProcess	process;

		enum Mode {
			LEFT = 0,
			RIGHT = 1,
			LMIDDLE = 2,
			RMIDDLE = 3,
			FUNCTION = 4,
			TOP = LEFT,
			BOTTOM = RIGHT,
			TMIDDLE = LMIDDLE,
			BMIDDLE = RMIDDLE
		} mode;

		DINT(int base);

		DINT(UpdateProcess process);

		DINT(int base, UpdateProcess process);

		DINT(int base, DINT::Mode mode);

		inline operator int();
	};

	AUIDLL typedef DINT DPOS;
	AUIDLL typedef DINT DSIZE;

	struct AUIDLL DRECT {
		DPOS x;
		DPOS y;
		DSIZE width;
		DSIZE height;
	};

	typedef bool CHANGED;

	struct AUIDLL SPOINT {
		int x;
		int y;
	};

	AUIDLL void UpdateDPOS(DPOS& dPos, int pSize);

	AUIDLL void UpdateDSIZE(DSIZE& dSize, DPOS& dPos, int pSize);
}

#endif // !_AUTODIDACT_UI_DINT_H_