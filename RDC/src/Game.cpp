#include "game.hpp"
#include <algorithm>
#include <atomic>
#include <thread>

#define CAMERADAMPING 3.f
#define PI 3.14159265359
constexpr auto spawn = true;
constexpr auto unlockFps = true;
static atomic_bool s_trySpawn;
static atomic_bool s_tryPush;
thread* bulletManagementThread = nullptr;
thread* bulletSpawnThread = nullptr;
thread* pushbackThread = nullptr;

// struct for bullet data
struct BD {
	float damage;
	float maxDistance;
	float cd;
	float spreadAngle;
	int shotCount;
	bool burst;
	Texture* bulletTexture;
	Sprite* gun;
	Player* player;
}bulletDetails;

// struct for enemy push values
struct EP {
	float velocity;
	Enemy* enemy;
}enemyPush;

#ifdef ADVANCEDCLAMP
Vector2f clamp(Vector2f n, Vector2f lower, Vector2f upper) {
	Vector2f r{};

	r.x = max(lower.x, min(n.x, upper.x));
	
	//cout << max(lower.y, min(n.y, upper.y)) << " " << n.y << " " << lower.y << " " << upper.y << endl;

	r.y = max(lower.y, min(n.y, upper.y));

	return r;
}

Vector2f clamp(Vector2i n, Vector2i lower, Vector2i upper) {
	Vector2f r{};

	r.x = max(lower.x, min(n.x, upper.x));
	r.y = max(lower.y, min(n.y, upper.y));

	return r;
}
#endif

float clamp(float n, float lower, float upper) {
	return max(lower, min(n, upper));
}

void Game::init() {
	spawnBullet();
	ContextSettings settings;

	settings.antialiasingLevel = 8;

	this->window.create(VideoMode(1280, 720), "game", sf::Style::Default, settings);
	
	this->window.setFramerateLimit(0);

	this->squareT.loadFromFile("./assets/square.png");
	this->plrT.loadFromFile("./assets/plr.png");
	this->enemyT.loadFromFile("./assets/enemy.png");
	this->gunT.loadFromFile("./assets/gun.png");
	this->bulletT.loadFromFile("./assets/bullet.png");

	this->squareT.setSmooth(true);
	this->plrT.setSmooth(true);
	this->gunT.setSmooth(true);
	this->bulletT.setSmooth(true);

	this->plr.init(this->plrT, Vector2f(-800, 360), 'p');

	this->healthBar.setFillColor(Color::Green);
	this->healthBar.setSize(Vector2f(400, 30));
	this->healthBar.setPosition(Vector2f(this->window.getSize().x - 420, this->window.getSize().y - 50));

	this->gun.setTexture(this->gunT);
	this->gun.setOrigin(Vector2f(8, 32 + 24));
	this->gun.setPosition(Vector2f(-800, 360));

	this->point.setSize(Vector2f(8, 8));
	this->point.setFillColor(Color::Red);
	this->pointL.setSize(Vector2f(8, 8));
	this->pointL.setFillColor(Color::White);
	this->pointR.setSize(Vector2f(8, 8));
	this->pointR.setFillColor(Color::White);

	for (int i = 0; i < 100; i++) {
		Square* square = new Square;
		square->init(this->squareT, Vector2f(64 * i, 0), 'b');
		this->objects.push_back(square);
	}

	if (spawn) {
		for (int i = 0; i < 10; i++) {
			Enemy* enemy = new Enemy;
			enemy->init(this->enemyT, Vector2f(rand() % 300 + i, rand() % 300 + i), 'e');
			this->enemies.push_back(enemy);
		}
	}

	this->view.setCenter(Vector2f(0, 0));
	this->view.setSize(Vector2f(1920, 1080));

	this->screen.height = this->view.getSize().y;
	this->screen.width = this->view.getSize().x;

	s_trySpawn.store(false);
	s_tryPush.store(false);
	bulletSpawnThread = new thread(&Game::spawnBullet, this);
	bulletSpawnThread->detach();
	pushbackThread = new thread(&Game::animatePushBack, this);
	pushbackThread->detach();
	bulletManagementThread = new thread(&Game::bulletHandler, this);
	bulletManagementThread->detach();

	this->update();
}

inline void Game::update() {
	this->view.setCenter(this->plr.getPosition());

	while (this->window.isOpen()) {
		while (this->window.pollEvent(this->ev)) {
			if (this->ev.type == sf::Event::Closed) {
				this->window.close();
			}
			else if (this->ev.type == Event::MouseButtonPressed && !s_trySpawn.load()) {
				bulletDetails.burst = true;
				bulletDetails.cd = 0.35f;
				bulletDetails.damage = 20.f;
				bulletDetails.maxDistance = 4000.f;
				bulletDetails.shotCount = 1;
				bulletDetails.spreadAngle = 1.f;
				bulletDetails.bulletTexture = &this->bulletT;
				bulletDetails.gun = &this->gun;
				bulletDetails.player = &this->plr;
				s_trySpawn.store(true);
			}
		}

		keyHandler();

		if (this->dashing) {
			for (int i = 0; i < 20; i++) {
				this->plr.move((this->velocity * this->dt) / 20.f);
				if (resolveCollisions(this->plr, 23.5)) {
					this->velocity *= 0.f;
					this->force *= 0.f;
					this->dashing = false;
					break;
				}
			}
			this->velocity *= (1.f - this->dt * 0.99f);

			//cout << this->velocity.x << " " << this->velocity.y << endl;

			if (abs(this->velocity.x) < 1300 && abs(this->velocity.y) < 1300) {
				this->velocity *= 0.f;
				this->force *= 0.f;
				this->dashing = false;
				this->cameraDamping = 3.0f;
			}
		}

		bool outOfRange = true;
		float counter = 1;
		Vector2f averagePosition = this->plr.getPosition();

		for (auto enemy : this->enemies) {
			Vector2f distanceFromPlayer = this->plr.getPosition() - enemy->getPosition();
			float hypotenuse = sqrt(distanceFromPlayer.x * distanceFromPlayer.x + distanceFromPlayer.y * distanceFromPlayer.y);

			if (hypotenuse < 600) {
				counter++;
				averagePosition += enemy->getPosition();
				outOfRange = false;
				if (hypotenuse > 47) {
					enemy->move(distanceFromPlayer / hypotenuse * 155.f, dt);
				}
				else {
					if (this->iTime.getElapsedTime().asSeconds() > 0.4 && !this->dashing) {
						this->iTime.restart();
						this->healthBar.setSize(this->healthBar.getSize() - Vector2f(40, 0));
						this->hp -= 10;

						enemyPush.enemy = enemy;
						enemyPush.velocity = 2500.f;
						s_tryPush.store(true);
					}
				}
			}
		}

		averagePosition /= counter;

		if (counter > 1 && !outOfRange) {
			this->view.move((averagePosition - this->view.getCenter()) * this->cameraDamping * this->dt);
		}
		else {
			this->view.move((this->plr.getPosition() - this->view.getCenter()) * this->cameraDamping * this->dt);
		}

		this->plr.setRotation(atan2(this->plr.getPosition().x - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).x, this->plr.getPosition().y - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).y) * 180 / 3.14159265 * -1);
		this->gun.setRotation(atan2(this->gun.getPosition().x - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).x, this->gun.getPosition().y - this->window.mapPixelToCoords(Mouse::getPosition(this->window)).y) * 180 / 3.14159265 * -1);
		this->gun.setPosition(this->plr.getPosition());

		this->render();

		//cout << "fps: " << 1000 / (this->dt * 1000) << endl;

		this->dt = this->clock.restart().asSeconds();
	}

	bulletSpawnThread = nullptr;
	delete bulletSpawnThread;
	pushbackThread = nullptr;
	delete pushbackThread;
	bulletManagementThread = nullptr;
	delete bulletManagementThread;

}

inline void Game::render() {
	this->window.clear();

	if (this->hp > 0) {
		this->window.setView(this->view);

		for (auto& obj : this->objects) {
			obj->draw(this->window);
		}

		for (auto& enemy : this->enemies) {
			if (enemy->getHp() > 0) {
				enemy->draw(this->window);
			}
		}

		this->plr.draw(this->window);
		
		for (auto& bullet : this->bullets) {
			bullet->draw(this->window);
		}

		this->window.draw(this->gun);
		
		this->window.draw(this->pointL);
		this->window.draw(this->pointR);
		this->window.draw(this->point);

		this->window.setView(this->window.getDefaultView());

		this->window.draw(this->healthBar);

		this->window.setView(this->view);
	}

	this->window.display();
}

void Game::keyHandler() {
	Vector2f dir;

	if (Keyboard::isKeyPressed(Keyboard::A)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(-200, 0) / 5.f, this->dt);
			if (resolveCollisions(this->plr, 23.5)) {
				break;
			}
		}
		dir.x = -1;
	}
	else if (Keyboard::isKeyPressed(Keyboard::D)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(200, 0) / 5.f, this->dt);
			if (resolveCollisions(this->plr, 23.5)) {
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
			if (resolveCollisions(this->plr, 23.5)) {
				break;
			}
		}
		dir.y = -1;

	}
	else if (Keyboard::isKeyPressed(Keyboard::S)) {
		for (int i = 0; i < 5; i++) {
			this->plr.move(Vector2f(0, 200) / 5.f, this->dt);
			if (resolveCollisions(this->plr, 23.5)) {
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
			this->cameraDamping = CAMERADAMPING;

		}
	}

	if (Keyboard::isKeyPressed(Keyboard::Space)) {
		bulletDetails.burst = false;
		bulletDetails.cd = 0.01f;
		bulletDetails.damage = 50;
		bulletDetails.maxDistance = 3000.f;
		bulletDetails.shotCount = 1;
		bulletDetails.spreadAngle = 20;
		bulletDetails.bulletTexture = &this->bulletT;
		bulletDetails.gun = &this->gun;
		bulletDetails.player = &this->plr;
		s_trySpawn.store(true);
	}
}

// spread - spread angle in degrees
void Game::spawnBullet() {
	Clock fireCD;

	while (this->window.isOpen()) {
		if (s_trySpawn.load()) {
			s_trySpawn.store(false);

			float damage = bulletDetails.damage, cd = bulletDetails.cd, maxDistance = bulletDetails.maxDistance, spreadAngle = bulletDetails.spreadAngle;
			int shotCount = bulletDetails.shotCount;
			bool burst = bulletDetails.burst;
			Texture* bulletT = bulletDetails.bulletTexture;

			if (fireCD.getElapsedTime().asSeconds() >= cd) {
				for (int i = 0; i < shotCount; i++) {
					fireCD.restart();

					Vector2f gunPosition = bulletDetails.gun->getPosition(), mousePosition = this->window.mapPixelToCoords(Mouse::getPosition(this->window)), playerPosition = bulletDetails.player->getPosition();
					Vector2f mouseToGun = gunPosition - mousePosition;
					Vector2f mouseToGunNorm = mouseToGun / hypotf(mouseToGun.x, mouseToGun.y);

					Bullet* bullet = new Bullet(damage, maxDistance);
					bullet->setTexture(*bulletT);
					bullet->setOrigin(Vector2f(4, 4));
					bullet->setPosition(gunPosition + mouseToGunNorm * -50.f);

					Vector2f distance = playerPosition - mousePosition;
					float angle = atan2(distance.y, distance.x);

					Vector2f start(playerPosition.x - 500 * cos(angle - spreadAngle * PI / 180), playerPosition.y - 500 * sin(angle - spreadAngle * PI / 180));
					Vector2f end(playerPosition.x - 500 * cos(angle + spreadAngle * PI / 180), playerPosition.y - 500 * sin(angle + spreadAngle * PI / 180));

					float percentage = (float)rand() / RAND_MAX;
					Vector2f spred(start.x * (1.0f - percentage) + end.x * percentage, start.y * (1.0f - percentage) + end.y * percentage);
					Vector2f distanceSpread = playerPosition - spred;

					Vector2f normalized = distanceSpread / hypotf(distanceSpread.x, distanceSpread.y);
					bullet->direction = normalized;

					try {
						this->bullets.push_back(bullet);
					}
					catch (exception& e) {
						cout << "Bullet add exception: " << " " << e.what() << '\n';
					}

					if (burst) {
						sleep(Time(milliseconds(40)));
					}
				}
			}
		}
		sleep(Time(milliseconds(8.3333)));
	}
}

void Game::bulletHandler() {
	Clock c;
	float newDt = 0;

	while (this->window.isOpen()) {
		for (int i = this->bullets.size() - 1; i >= 0; i--) {
			this->bullets[i]->move(this->bullets[i]->direction * -600.f * newDt);
			this->bullets[i]->distance += hypotf((this->bullets[i]->direction * -600.f * newDt).x, (this->bullets[i]->direction * -600.f * newDt).y);
			
			vector<int> indexes = this->resolveCollisionsEnemy(*this->bullets[i], 2);

			try {
				if (!indexes.empty()) {
					if (i < this->bullets.size()) {
						this->bullets.erase(this->bullets.begin() + i);
						cout << "erased" << '\n';
					}
				}
				else if (this->resolveCollisions(*this->bullets[i], 2) || this->bullets[i]->distance > this->bullets[i]->maxDistance) {
					if (i < this->bullets.size()) {
						this->bullets.erase(this->bullets.begin() + i);
						cout << "erased" << '\n';
					}
				}
			}
			catch (exception& e) {
				cout << "Bullet erase exception: " << " " << e.what() << '\n';
			}

			if (!indexes.empty()) {
				cout << "hit" << '\n';
				for (int j = 0; j < indexes.size(); j++) {
					this->enemies[indexes[j]]->takeDamage(this->bullets[i]->damage);
					if (this->enemies[indexes[j]]->getHp() <= 0) {
						try {
							this->enemies.erase(this->enemies.begin() + indexes[j]);
						}
						catch (exception& e) {
							cout << "Enemie erase exception: " << " " << e.what() << '\n';
						}
					}
				}
			}
		}

		newDt = c.restart().asSeconds();
	}

}

bool Game::resolveCollisions(RenderObject& obj, float radius) {
	for (int i = 0; i < 10; i++) {
		for (auto& i : this->objects) {
			if (i->type == 'b') {
				Vector2f pointOnRect;

				pointOnRect.x = clamp(obj.getPosition().x, i->getPosition().x - 32, i->getPosition().x + 32);
				pointOnRect.y = clamp(obj.getPosition().y, i->getPosition().y - 32, i->getPosition().y + 32);

				float length = sqrt((pointOnRect - obj.getPosition()).x * (pointOnRect - obj.getPosition()).x + (pointOnRect - obj.getPosition()).y * (pointOnRect - obj.getPosition()).y);

				if (length < radius) {
					obj.move(Vector2f((obj.getPosition() - pointOnRect).x / radius, (obj.getPosition() - pointOnRect).y / radius));
					return true;
				}
			}
		}
	}
	return false;
}

vector<int> Game::resolveCollisionsEnemy(Bullet& bullet, float size) {
	vector<int> hitIndexes;

	for (size_t i = 0; i < this->enemies.size(); i++) {
		Vector2f pointOnRect;

		pointOnRect.x = clamp(bullet.getPosition().x, this->enemies[i]->getPosition().x - 32, this->enemies[i]->getPosition().x + 32);
		pointOnRect.y = clamp(bullet.getPosition().y, this->enemies[i]->getPosition().y - 32, this->enemies[i]->getPosition().y + 32);

		float length = sqrt((pointOnRect - bullet.getPosition()).x * (pointOnRect - bullet.getPosition()).x + (pointOnRect - bullet.getPosition()).y * (pointOnRect - bullet.getPosition()).y);

		if (length < size) {
			hitIndexes.push_back(i);
		}
	}

	return hitIndexes;
}

void Game::animatePushBack() {
	Clock c;
	float newDt = 0;

	while (this->window.isOpen()) {
		if (s_tryPush.load()) {
			s_tryPush.store(false);
			float velocity = enemyPush.velocity;
			
			for (int i = 0; i < 20; i++) {
				Vector2f distance = this->plr.getPosition() - enemyPush.enemy->getPosition();
				this->plr.move(((distance / hypotf(distance.x, distance.y)) * newDt * velocity) / 2.f);
				velocity *= (1.f - newDt * 0.99f);
				newDt = c.restart().asSeconds();
				sleep(Time(milliseconds(3)));
			}
		}

		newDt = c.restart().asSeconds();
		sleep(Time(milliseconds(8.333)));
	}
}