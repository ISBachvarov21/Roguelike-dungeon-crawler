#include "game.hpp"
#include <algorithm>

float clamp(float n, float lower, float upper) {
	return max(lower, min(n, upper));
}

void Game::init() {
	this->window.create(VideoMode(1280, 720), "game");
	this->window.setFramerateLimit(144);

	this->squareT.loadFromFile("./assets/square.png");
	this->plrT.loadFromFile("./assets/plr.png");

	this->squareT.setSmooth(true);
	this->plrT.setSmooth(true);

	this->plr.init(this->plrT, Vector2f(640, 360));

	for (int i = 0; i < 100; i++) {
		Square* square = new Square;
		square->init(this->squareT, Vector2f(64 * i, 0));
		this->objects.push_back(square);
	}


	this->view.setCenter(Vector2f(0, 0));
	this->view.setSize(Vector2f(1280, 720));

	this->update();
}

void Game::update() {
	while (this->window.isOpen()) {
		while (this->window.pollEvent(this->ev)) {
			if (this->ev.type == sf::Event::Closed) {
				this->window.close();
			}
		}

		keyHandler();

		if (this->dashing) {
			for (int i = 0; i < 20; i++) {
				this->plr.move((velocity * this->dt) / 20.f);
				if (resolveCollisions()) {
					this->velocity *= 0.f;
					this->force *= 0.f;
					this->dashing = false;
					break;
				}
			}
			this->velocity *= (1.f - this->dt * 0.99f);

			if (abs(this->velocity.x) < 1300 && abs(this->velocity.y) < 1300) {
				this->velocity *= 0.f;
				this->force *= 0.f;
				this->dashing = false;
			}
		}

		this->view.setCenter(plr.getPosition());

		this->plr.setRotation(atan2(this->plr.getPosition().x - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).x, this->plr.getPosition().y - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).y) * 180 / 3.14159265 * -1);
		
		this->render();

		cout << "fps: " << 1000 / (this->dt * 1000) << endl;

		this->dt = this->clock.restart().asSeconds();
	}
}

void Game::render() {
	this->window.clear();

	this->window.setView(this->view);

	for (auto& i : this->objects) {
		i->draw(this->window);
	}


	this->plr.draw(this->window);

	this->window.display();

}

void Game::keyHandler() {
	Vector2f dir;

	if (Keyboard::isKeyPressed(Keyboard::A)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(-200, 0) / 5.f, this->dt);
			if (resolveCollisions()) {
				break;
			}
		}
		dir.x = -1;
	}
	else if (Keyboard::isKeyPressed(Keyboard::D)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(200, 0) / 5.f, this->dt);
			if (resolveCollisions()) {
				break;
			}
		}
		dir.x = 1;
	}
	else {
		dir.x = 0;
	}

	if (Keyboard::isKeyPressed(Keyboard::W)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(0, -200) / 5.f, this->dt);
			if (resolveCollisions()) {
				break;
			}
		}
		dir.y = -1;

	}
	else if (Keyboard::isKeyPressed(Keyboard::S)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(0, 200) / 5.f, this->dt);
			if (resolveCollisions()) {
				break;
			}
		}
		dir.y = 1;

	}
	else {
		dir.y = 0;
	}

	if (Keyboard::isKeyPressed(Keyboard::LShift)) {
		float timePassed = this->dashCD.getElapsedTime().asSeconds();

		if (timePassed > 1 && !this->dashing) {
			this->dashCD.restart();
			this->force = Vector2f(1500 * dir.x, 1500 * dir.y);
			this->velocity = force;
			this->dashing = true;
		}
	}
}

bool Game::resolveCollisions() {
	for (int i = 0; i < 10; i++) {
		for (auto& i : this->objects) {
			Vector2f pointOnRect;

			pointOnRect.x = clamp(this->plr.getPosition().x, i->getPosition().x - 32, i->getPosition().x + 32);
			pointOnRect.y = clamp(this->plr.getPosition().y, i->getPosition().y - 32, i->getPosition().y + 32);

			float length = sqrt((pointOnRect - this->plr.getPosition()).x * (pointOnRect - this->plr.getPosition()).x + (pointOnRect - this->plr.getPosition()).y * (pointOnRect - this->plr.getPosition()).y);

			if (length < 23.5) {
				this->plr.move(Vector2f((this->plr.getPosition() - pointOnRect).x / 23.5, (this->plr.getPosition() - pointOnRect).y / 23.5));
				return true;
			}
		}
	}
	return false;
}