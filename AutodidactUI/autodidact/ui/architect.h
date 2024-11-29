#pragma once
#ifndef _AUTODIDACT_UI_ARCHITECT_H_
#define _AUTODIDACT_UI_ARCHITECT_H_

#include <Windows.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <vector>
#include <stack>
#include <queue>
#include <memory>

#include "dint.h"

#ifndef __EXPORT
#define AUIDLL __declspec(dllexport)
#else
#define AUIDLL __declspec(dllimport)
#endif

#ifndef NULLABLE
#define NULLABLE
#endif

#endif // !_AUTODIDACT_UI_ARCHITECT_H_
