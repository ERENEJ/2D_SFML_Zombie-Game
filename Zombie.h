#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

class Zombie
{
private:
	const float BLOATER_SPEED = 200;
	const float CHASER_SPEED = 400;
	const float CRAWLER_SPEED = 2500;

	const float BLOATER_HEALTH = 5;
	const float CHASER_HEALTH = 1;
	const float CRAWLER_HEALTH = 3;

	const int MAX_VARRIANCE = 30;
	const int OFFSET = 101 - MAX_VARRIANCE;

	Vector2f m_Position;
	Sprite m_Sprite;
	float m_Speed;
	float m_Health;

	bool m_Alive;

public: 

	// function called as "hit" but it returns if the zombie dead by the hit or not
	bool hit();

	bool isAlive();
	void spawn(float startX, float startY, int type, int seed);

	FloatRect getPosition();
	Sprite getSprite();
	
	void update(float elapsedTime, Vector2f playerLocation);
};