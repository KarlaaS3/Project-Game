#include "Vec2.h"

#include <cmath>


namespace {

	// fp comparisons by Donald Knuth from TAOCP
	const static float EPS{ 0.0000001f };

	bool approximatelyEqual(float a, float b, float epsilon= EPS)
	{
		return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool essentiallyEqual(float a, float b, float epsilon = EPS)
	{
		return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool definitelyGreaterThan(float a, float b, float epsilon = EPS)
	{
		return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool definitelyLessThan(float a, float b, float epsilon = EPS)
	{
		return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}
}

// trig helper 
const float PI = 3.14159265f;

inline float radToDeg(float rad) {
	return rad * 180.f / PI;
}

inline float degToRad(float deg) {
	return deg * PI / 180.f;
}


Vec2::Vec2()
{}

Vec2::Vec2(float x, float y) : x(x), y(y)
{}


Vec2::Vec2(int x, int y) : x(float(x)), y(float(y))
{}

Vec2::Vec2(float bering)  : x(std::cos(degToRad(bering))), y(std::sin(degToRad(bering)))
{}

bool Vec2::operator==(const Vec2& rhs) const 
{ 
	//return (x == rhs.x && y == rhs.y);
	return (essentiallyEqual(x, rhs.x) && essentiallyEqual(y, rhs.y));
}

bool Vec2::operator!=(const Vec2& rhs) const
{
	return !(*this == rhs);
}


Vec2 Vec2::operator*(const float& rhs) const
{
	return Vec2(x * rhs, y * rhs);
}


Vec2 Vec2::operator+(const Vec2& rhs) const
{
	return Vec2(*this) += rhs;
}


Vec2& Vec2::operator+=(const Vec2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}


Vec2& Vec2::operator-=(const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}


Vec2 Vec2::operator-(const Vec2& rhs) const
{
	return Vec2(*this) += rhs;
}


float Vec2::length() const
{
	return sqrt(x*x + y*y);
}


float Vec2::dist(const Vec2& rhs)
{ 
	return sqrt((rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y));
}


Vec2 Vec2::normalize() const
{
	static const float epsilon{ 0.00001f };

	if (length() > epsilon)
		return Vec2(*this) * (1.f / length());
	else
		return Vec2(0.f, 0.f);
}


float   Vec2::bearingTo(const Vec2& target) const
{
	return radToDeg(std::atan2(target.y - y, target.x - x));
}


Vec2 operator*(float lhs,  Vec2 rhs)
{
	return rhs * lhs;
}


std::ostream& operator<<(std::ostream& os, const Vec2& other) {
	os << "(" << other.x << ", " << other.y << ")";
	return os;
}
