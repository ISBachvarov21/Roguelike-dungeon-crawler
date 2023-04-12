#pragma once
#include "precompile.hpp"
#include "RenderObject.hpp"

class Enemy : public RenderObject {
protected:
	int hp{100};
	Vector2f pos;

public:
	void move(Vector2f offset) override;
	void move(Vector2f offset, float dt);
	int getHp();
	void takeDamage(int damage);
};