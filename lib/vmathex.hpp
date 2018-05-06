#pragma once

#include <donny/file_stream.hpp>

#include "vmath.hpp"

namespace vmath {

template <typename T>
static inline Tmat4<T> rotateX(T angle) { return rotate(angle, 1.f, 0.f, 0.f); }

template <typename T>
static inline Tmat4<T> rotateY(T angle) { return rotate(angle, 0.f, 1.f, 0.f); }

template <typename T>
static inline Tmat4<T> rotateZ(T angle) { return rotate(angle, 0.f, 0.f, 1.f); }

template <typename T>
static inline Tmat4<T> rotateXYZ(const vecN<T, 3> rotation) {
    return rotateX(rotation[0]) * rotateY(rotation[1]) * rotateZ(rotation[2]);
}

template <typename T, const int N>
static inline vecN<T, N> reflect(const vecN<T, N> incident, const vecN<T, N> normal)
{
    return incident - 2.0f * dot(normal, incident) * normal;
}

template<typename CharType, typename T, const int N>
inline donny::filesystem::file_stream<CharType>& operator<<(donny::filesystem::file_stream<CharType> &fs, vecN<T, N> v)
{
    for (int a = 0; a < N; ++a) fs << v[a] << ' ';
    return fs;
}

} // vmath
