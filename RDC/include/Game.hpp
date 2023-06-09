#pragma once
#include "precompile.hpp"
#include "Square.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"
#include <vector>

class Game {
private:
	RenderWindow window;
	View view;
	Event ev;

	Player plr;

	Texture squareT;
	Texture plrT;
	Texture enemyT;
	Texture gunT;
	Texture bulletT;

	RectangleShape healthBar;
	RectangleShape point;
	RectangleShape pointL;
	RectangleShape pointR;

	Sprite gun;

	Vector2f force;
	Vector2f velocity;
	Vector2f cameraOffset{0,0};

	vector<RenderObject*> objects;
	vector<Square*> squares;
	vector<Enemy*> enemies;
	vector<Bullet*> bullets;

	atomic_bool finished;

	Clock clock;
	Clock dashCD;
	Clock iTime;

	FloatRect screen;

	float dt;
	float cameraDamping{ 6.f };
	int hp = 100;
	bool dashing{false};

public:
	Game() = default;
	~Game() = default;

	void render();
	void update();
	void init();
	void keyHandler();
	void spawnBullet();
	void animatePushBack();
	//void bulletHandler();
	bool resolveCollisions(RenderObject& obj, float radius);
	vector<int> resolveCollisionsEnemy(Bullet& bullet, float size);
};
