#pragma once
#include "precompile.hpp"
#include "RenderObject.hpp"

class Bullet : public RenderObject {
public:
	Bullet(float damage, float maxDistance);

	Vector2f direction;
	float damage{ 0 };
	float distance{ 0 };
	float maxDistance{ 0 };
};