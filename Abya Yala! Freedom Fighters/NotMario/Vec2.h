#pragma once

#include <iostream>

struct Vec2
{
	float x{ 0.f };
	float y{ 0.f };

	Vec2();
	Vec2(float x, float y);
	Vec2(int x, int y);
	Vec2(float bering); // unit vector in bering direction
	
	bool operator==(const Vec2& rhs) const;
	bool operator!=(const Vec2& rhs) const;


	Vec2&	operator+=(const Vec2& rhs);
	Vec2	operator+(const Vec2& rhs) const;
	Vec2&	operator-=(const Vec2& rhs);
	Vec2	operator-(const Vec2& rhs) const;

	Vec2	operator*(const float& rhs) const;
	 
	float	length() const;
	float	dist(const Vec2& rhs); 
	Vec2	normalize() const; 
	
	float   bearingTo(const Vec2& target) const;

	friend std::ostream& operator<<(std::ostream& os, const Vec2& other);
};

Vec2 operator*(float lhs,  Vec2 rhs);
std::ostream& operator<<(std::ostream& os, const Vec2& other);
