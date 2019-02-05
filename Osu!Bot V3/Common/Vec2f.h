// Vec2f.h : Holds the definition of a 2D vector (a point in 2D space)

#pragma once

class vec2f {
public:
	// Member variables.
	float X, Y;

	// Constructors.
	vec2f(float x, float y) : X(x), Y(y) {}
	vec2f() : X(0.f), Y(0.f) {}

	
	// Member functions.
	float Length() const {
		return sqrtf(X * X + Y * Y);
	}

	vec2f MidPoint(const vec2f& vec) const {
		return vec2f((X + vec.X) / 2.f, (Y + vec.Y) / 2.f);
	}

	vec2f Add(const vec2f& vec) {
		X += vec.X;
		Y += vec.Y;
		return *this;
	}

	vec2f Add(float x, float y) {
		X += x;
		Y += y;
		return *this;
	}

	vec2f Sub(const vec2f& vec) {
		X -= vec.X;
		Y -= vec.Y;
		return *this;
	}

	vec2f Sub(float x, float y) {
		X -= x;
		Y -= y;
		return *this;
	}

	vec2f Sub(float n) {
		X -= n;
		Y -= n;
		return *this;
	}

	vec2f Mult(const vec2f& vec) {
		X *= vec.X;
		Y *= vec.Y;
		return *this;
	}

	vec2f Mult(float x, float y) {
		X *= x;
		Y *= y;
		return *this;
	}

	vec2f Mult(float n) {
		X *= n;
		Y *= n;
		return *this;
	}

	vec2f Dev(const vec2f& vec) {
		X /= vec.X;
		Y /= vec.Y;
		return *this;
	}

	vec2f Dev(float n) {
		X /= n;
		Y /= n;
		return *this;
	}

	vec2f Copy() const {
		return vec2f(X, Y);
	}

	vec2f Rotate(float angle) {
		float oX = X;
		float oY = Y;
		X = (oX * cosf(angle) - oY * sinf(angle));
		Y = (oX * sinf(angle) + oY * cosf(angle));
		return *this;
	}

	vec2f Nor() {
		float nX = -Y;
		float nY = X;
		X = nX;
		Y = nY;
		return *this;
	}

	vec2f Normalize() {
		return this->Dev(this->Length());
	}

	vec2f ConvertToWindowSpace(const float& stackOffset, const UINT& stackIndex, const DX::Size<FLOAT>& multiplier, const DX::Size<INT>& offset) {
		this->Sub(stackOffset * (FLOAT)stackIndex);
		this->Mult((FLOAT)multiplier.Width, (FLOAT)multiplier.Height);
		this->Add((FLOAT)offset.Width, (FLOAT)offset.Height);
		
		//X = (X - stackOffset * (FLOAT)stackIndex) * multiplier.Width + (FLOAT)offset.Width;
		//Y = (Y - stackOffset * (FLOAT)stackIndex) * multiplier.Height + (FLOAT)offset.Height;

		return *this;
	}
};

// Inline operators.
inline vec2f operator+ (vec2f lhs, const vec2f& rhs) {
	return lhs.Copy().Add(rhs);
}

inline vec2f operator- (vec2f lhs, const vec2f& rhs) {
	return lhs.Copy().Sub(rhs);
}

inline vec2f operator* (vec2f lhs, const vec2f& rhs) {
	return lhs.Copy().Mult(rhs);
}

inline vec2f operator* (vec2f lhs, float rhs) {
	return lhs.Copy().Mult(rhs);
}

inline vec2f operator* (float lhs, vec2f rhs) {
	// This is a odd case of rhs being the base.
	return rhs.Copy().Mult(lhs);
}

inline vec2f operator/ (vec2f& lhs, const vec2f& rhs) {
	return lhs.Copy().Dev(rhs);
}

inline vec2f operator/ (vec2f lhs, float rhs) {
	return lhs.Copy().Dev(rhs);
}

inline vec2f operator/ (float lhs, vec2f rhs) {
	// This is a odd case of rhs being the base.
	return rhs.Copy().Dev(lhs);
}

inline bool operator== (const vec2f& lhs, const vec2f& rhs) {
	return (lhs.X == rhs.X) && (lhs.Y == rhs.Y);
}

inline bool operator!= (const vec2f& lhs, const vec2f& rhs) {
	return !(lhs == rhs);
}