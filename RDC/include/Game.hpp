#pragma once
#include "precompile.hpp"
#include "Square.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
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

	RectangleShape healthBar;

	Vector2f force;
	Vector2f velocity;

	vector<RenderObject*> objects;
	vector<Square*> squares;
	vector<Enemy*> enemies;

	Clock clock;
	Clock dashCD;
	Clock iTime;

	float dt;
	bool dashing{false};

public:
	Game() = default;
	~Game() = default;

	void render();
	void update();
	void init();
	void keyHandler();
	void animatePushBack(Vector2f enemyPos, Vector2f plrPos);
	bool resolveCollisions();
};
