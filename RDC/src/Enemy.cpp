#include "Enemy.hpp"

void Enemy::move(Vector2f offset) {
	this->sprite.move(offset);
	this->pos = this->sprite.getPosition();
}

void Enemy::move(Vector2f offset, float dt) {
	this->sprite.move(offset * dt);
	this->pos = this->sprite.getPosition();
}

void Enemy::takeDamage(int damage) {
	this->hp -= damage;
}

int Enemy::getHp() {
	return this->hp;
}