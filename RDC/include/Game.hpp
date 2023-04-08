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
	RectangleShape point;

	Vector2f force;
	Vector2f velocity;
	Vector2f cameraOffset{0,0};

	vector<RenderObject*> objects;
	vector<Square*> squares;
	vector<Enemy*> enemies;

	Clock clock;
	Clock dashCD;
	Clock iTime;

	float dt;
	float cameraDamping{ 6.f };
	bool dashing{false};

public:
	Game() = default;
	~Game() = default;

	void render();
	void update();
	void init();
	void keyHandler();
	void animatePushBack(Vector2f enemyPos, Vector2f plrPos);
	void handleZoom();
	bool resolveCollisions();
};
