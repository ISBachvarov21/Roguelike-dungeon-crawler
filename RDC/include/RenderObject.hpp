#pragma once
#include "precompile.hpp"

class RenderObject {
protected:
	Sprite sprite;

public:
	virtual void init(Texture& texture, Vector2f position, char type);
	virtual void setPosition(Vector2f position);
	virtual void setOrigin(Vector2f origin);
	virtual void draw(RenderWindow& window);
	virtual void move(Vector2f offset);
	virtual Vector2f getPosition();
	
	void setTexture(Texture& texture);
	
	char type;
};