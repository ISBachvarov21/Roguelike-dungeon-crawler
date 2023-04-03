#pragma once
#include "precompile.hpp"
#include "RenderObject.hpp"

class Player : public RenderObject {
public:
	void move(Vector2f offset, float dt);
	void move(Vector2f offset) override;
	void setRotation(float angle);
};
