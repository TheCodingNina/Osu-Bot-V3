// Size.h : Holds the definition of a template class Size
// that can hold the Width and Height of a object.
// Good for storing a 2D rectangular area.

#pragma once

namespace DX
{
	// A template of a class Size to hold a Width and Height of a rectangular 2D area.
	template<class _T> class Size {
	public:
		_T Width;
		_T Height;

		Size() : Width(0), Height(0) {};
		Size(_T Width, _T Height) : Width(Width), Height(Height) {};
	};

	// Operator overloads.
	template<class _T> inline bool operator== (const Size<_T>& lhs, const Size<_T>& rhs) {
		return (lhs.Width == rhs.Width && lhs.Height == rhs.Height);
	}

	template<class _T> inline bool operator!= (const Size<_T>& lhs, const Size<_T>& rhs) {
		return !(lhs == rhs);
	}

	template<class _T> inline Size<_T> operator+ (const Size<_T>& lhs, const Size<_T>& rhs) {
		return Size<_T>(lhs.Width + rhs.Width, lhs.Height + rhs.Height);
	}

	template<class _T> inline Size<_T> operator- (const Size<_T>& lhs, const Size<_T>& rhs) {
		return Size<_T>(lhs.Width - rhs.Width, lhs.Height - rhs.Height);
	}

	template<class _T> inline Size<_T> operator* (const Size<_T>& lhs, const Size<_T>& rhs) {
		return Size<_T>(lhs.Width * rhs.Width, lhs.Height * rhs.Height);
	}

	template<class _T> inline Size<_T> operator* (const Size<_T>& lhs, const _T& rhs) {
		return Size<_T>(lhs.Width * rhs, lhs.Height * rhs);
	}

	template<class _T> inline Size<_T> operator/ (const Size<_T>& lhs, const Size<_T>& rhs) {
		return Size<_T>(lhs.Width / rhs.Width, lhs.Height / rhs.Height);
	}
	
	template<class _T> inline Size<_T> operator/ (const Size<_T>& lhs, const _T& rhs) {
		return Size<_T>(lhs.Width / rhs, lhs.Height / rhs);
	}
}