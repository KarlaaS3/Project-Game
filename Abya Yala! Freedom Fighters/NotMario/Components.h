#pragma once

#include "Common.h"
#include "Animation.h"
#include "Assets.h"
 
struct Component
{
	bool has{ false };
	Component() = default;
};


struct CAnimation : public Component
{
	Animation animation;
	bool repeat{ false };
	CAnimation() = default;

	CAnimation(const Animation& animation, bool r)
		:animation(animation), repeat(r) {}
};


struct CCollision : public Component
{
	float radius{ 0.f };
	
	CCollision() = default;
	CCollision(float r)
		: radius(r) {}
};


struct CInput : public Component
{
	bool up			{false};
	bool left		{false};
	bool right		{false};
	bool down		{false};
	bool shoot		{false};
	bool canShoot	{true};
	bool canJump	{true};

	CInput() = default;
};


struct CLifespan : public Component
{
	int total{ 0 };
	int remaining{ 0 };

	CLifespan() = default;
	CLifespan(int t) : total(t), remaining{ t } {}
	 
};


struct CBoundingBox : public Component
{
	Vec2 size{0.f, 0.f};
	Vec2 halfSize{ 0.f, 0.f };

	CBoundingBox() = default;
	CBoundingBox(const Vec2& s) : size(s), halfSize(0.5f * s) 
	{}
};


struct CTransform : public Component
{
	
	Vec2	pos			{ 0.f, 0.f };
	Vec2	prevPos		{ 0.f, 0.f };
	Vec2	scale		{ 1.f, 1.f };
	Vec2	vel			{ 0.f, 0.f };

	float   angVel{ 0 };
	float	angle{ 0.f };

	CTransform() = default;
	CTransform(const Vec2& p) : pos(p)  {}
	CTransform(const Vec2& p, const Vec2& v, const Vec2& sc, float a) 
		: pos(p), prevPos(p), vel(v), scale(sc), angle(a) {}

};


struct CScore : public Component
{
	int score{ 0 };

	CScore() = default;
	CScore(int s = 0) : score(s) {}
};


struct CGravity : public Component
{
	float g{ 0 };

	CGravity() = default;
	CGravity(float g ) : g(g) {}
};


struct CState : public Component
{
	enum State {
		isGrounded		= 1,
		isFacingLeft	= 1 << 1, 
		isRunning		= 1 << 2
	};
	unsigned int  state{ 0 };

	CState() = default;
	CState(unsigned int s ) : state(s) {}
	bool test(unsigned int x) { return (state & x); }
	void set(unsigned int x) { state |= x; }
	void unSet(unsigned int x) { state &= ~x; }

};