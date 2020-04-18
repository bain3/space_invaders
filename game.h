#pragma once
#include "olcPixelGameEngine.h"

struct Invader {
    olc::vi2d position;
	olc::vi2d sprite_pos;
	olc::vi2d sprite_wh;
	bool alive = true;
	olc::Pixel tint;
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

struct Bunker {
    olc::vi2d position;
    int health = 5;
};