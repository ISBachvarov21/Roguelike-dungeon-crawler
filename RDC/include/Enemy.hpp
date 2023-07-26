#pragma once
#include "precompile.hpp"
#include "RenderObject.hpp"

class Enemy : public RenderObject {
protected:
	float hp{100};
	Vector2f pos;

public:
	void move(Vector2f offset) override;
	void move(Vector2f offset, float dt);
	void takeDamage(float damage);
	float getHp();
};