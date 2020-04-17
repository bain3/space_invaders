#pragma once
#include "olcPixelGameEngine.h"

struct Invader {
	olc::vi2d position;
	olc::vi2d spritepos;
	olc::vi2d spritewh;
	bool alive = true;
};

struct Player {
	olc::vi2d position;
};

struct Bullet {
	olc::vi2d position;
	olc::vi2d position2;
	bool player;
	bool to_remove = false;
	float bulletSpeed = -2;
};