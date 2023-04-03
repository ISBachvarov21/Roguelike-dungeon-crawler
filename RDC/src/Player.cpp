#include "Player.hpp"

void Player::move(Vector2f offset) {
	this->sprite.move(offset);
}

void Player::move(Vector2f offset, float dt) {
	this->sprite.move(offset * dt);
}

void Player::setRotation(float angle) {
	this->sprite.setRotation(angle);
}