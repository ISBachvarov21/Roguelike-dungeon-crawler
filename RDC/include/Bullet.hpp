#pragma once
#include "precompile.hpp"
#include "RenderObject.hpp"

class Bullet : public RenderObject {
public:
	Vector2f direction;
	float distance{ 0 };
};