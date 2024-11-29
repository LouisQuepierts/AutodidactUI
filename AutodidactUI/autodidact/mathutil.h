#pragma once
#ifndef _AUTODIDACT_UI_MATHUTIL_H_
#define _AUTODIDACT_UI_MATHUTIL_H_

#include <type_traits>
#include <algorithm>

template<typename T>
concept arithmetic = std::is_arithmetic<T>::value;

template<arithmetic T, arithmetic VT = float>
constexpr T lerp(T from, T to, VT value) {
	return from + T(value * (to - from));
}

template<arithmetic T, arithmetic VT = float>
constexpr T smooth_lerp(T from, T to, VT value) {
	return from + T(value * value * (to - from));
}

template<arithmetic T>
constexpr T saturate(T value) {
	return value > T(1) ? T(1) : value < T(0) ? T(0) : value;
}

template<arithmetic T>
constexpr T clamp(T value, T top, T bottom) {
	return value > top ? top : value < bottom ? bottom : value;
}

#endif
