#include "RenderObject.hpp"

void RenderObject::init(Texture& texture, Vector2f position) {
	this->sprite.setTexture(texture);
	this->sprite.setOrigin(Vector2f((float)(texture.getSize().x) / 2, (float)(texture.getSize().y) / 2));
	this->sprite.setPosition(position);
}

void RenderObject::move(Vector2f offset) {
	this->sprite.move(offset);
}

void RenderObject::draw(RenderWindow& window) {
	window.draw(this->sprite);
}

Vector2f RenderObject::getPosition() {
	return this->sprite.getPosition();
}

void RenderObject::setTexture(Texture& texture) {
	this->sprite.setTexture(texture);
}