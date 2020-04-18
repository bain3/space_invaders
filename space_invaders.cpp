#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "game.h"

olc::Sprite* player_sprite;
olc::Decal* invaders_decal;
olc::Sprite* invaders_sprite;
olc::Sprite* bunker_sprite;
olc::Decal* bunker_decal;
int frameCounter = 5;
float refreshTime = 0.08;
float lastRefresh;
float lastInvaderRefresh;
float shootRefresh;
int INVADER_SPEED = 4;
int PLAYER_SPEED = 1;
float PLAYER_COOLDOWN = 0.1;
bool verticalMove = false;
short front = 4;
float invaderShootTimer;
bool showSplashText = false;
std::string splashText;
short lives = 3;
std::array<std::array<Invader, 14>, 5> invaders;
Player player;
std::list<Bullet> bullets;
Bunker bunkers[4];
int gameLayer;


inline bool collision_check(Bullet& bullet, Invader& invader) {
	return (invader.position.x <= bullet.position.x && bullet.position.x <= (invader.position.x + invader.sprite_wh.x) &&
		invader.position.y <= bullet.position.y && bullet.position.y <= (invader.position.y + 8));
}

inline bool collision_check(Bullet& bullet, Player& player_) {
	return (player_.position.x <= bullet.position.x && bullet.position.x <= (player_.position.x + 13) &&
            player_.position.y <= bullet.position.y && bullet.position.y <= (player_.position.y + 8));
}


class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Space Invaders";
	}

public:
	bool OnUserCreate() override
	{
        gameLayer = CreateLayer();
		invaders_sprite = new olc::Sprite("invaders.png");
		invaders_decal = new olc::Decal(invaders_sprite);
		player_sprite = new olc::Sprite("player.png");
        bunker_sprite = new olc::Sprite("bunker.png");
        bunker_decal = new olc::Decal(bunker_sprite);
		player.position.y = ScreenHeight()-8;
		player.position.x = (int)ScreenWidth() / 2;
		Invader lastInvader;
		int invaderBlock = ScreenWidth() / 12;
		olc::vi2d sprite_wh;
		int sprite_x;
		olc::Pixel tint;


		for (int i = 0; i < invaders.size(); i++) {
			switch (i) {
                case 1:
                case 2:
                    sprite_wh = olc::vi2d{11, 8 };
                    sprite_x = 8;
                    tint = olc::YELLOW;
                    break;
                case 3:
                case 4:
                    sprite_wh = olc::vi2d{12, 8 };
                    sprite_x = 19;
                    tint = olc::RED;
                    break;
				default:
                    sprite_wh = olc::vi2d{8, 8 };
                    sprite_x = 0;
                    tint = olc::GREEN;
                    break;
			}

			for (int j = 0; j < invaders[i].size(); j++) {
				Invader current_invader;
				current_invader.position = olc::vi2d{ 15*j+(15 - sprite_wh.x / 2), 15 * i };
				current_invader.sprite_wh = sprite_wh;
				current_invader.sprite_pos = olc::vi2d{sprite_x, 0 };
				current_invader.tint = tint;
				invaders[i][j] = current_invader;
				lastInvader = current_invader;
			}
		}

        int steps = ScreenWidth()/4;
		for (int i = 0; i < 4; i++) {
		    bunkers[i].position = olc::vi2d{ steps*(i+1)-11-steps/2,ScreenHeight()-30 };
		}
		return true;
	}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
	bool OnUserUpdate(float fElapsedTime) override
	{
	    EnableLayer(gameLayer, !showSplashText);
	    SetDrawTarget(gameLayer);
		lastRefresh += fElapsedTime;
		lastInvaderRefresh += fElapsedTime;
		shootRefresh += fElapsedTime;
		invaderShootTimer += fElapsedTime;

		// splash text render + lock
		if (showSplashText) {
		    SetDrawTarget(nullptr);
			Clear(olc::BLACK);
			DrawString(olc::vi2d{ ScreenWidth()/2 - (int)(splashText.size()*8 / 2),ScreenHeight()/2 - 4 }, splashText);
            return !GetKey(olc::Key::Q).bHeld;
        }


		// invaders render
		Clear(olc::BLACK);
		for (auto & invader : invaders) {
			for (auto & j : invader) {
				if (j.alive) {
					DrawPartialDecal(j.position, invaders_decal, j.sprite_pos, j.sprite_wh, olc::vf2d{ 1.0f, 1.0f }, j.tint);
				}
			}
		}

		// player render
		DrawSprite(player.position, player_sprite);

        // bunker render
        for (const Bunker& bunker : bunkers) {
            DrawDecal(bunker.position, bunker_decal);
        }

		// overlay render
		SetDrawTarget(nullptr);
        Clear(olc::BLANK);
        for (const Bunker& bunker : bunkers) {
            std::string text = std::to_string(bunker.health);
            DrawString(bunker.position+olc::vi2d{8,3}, text);
        }
        DrawString(olc::vi2d{ 10,10 }, std::to_string(lives));
		SetDrawTarget(gameLayer);
		
		// bullet render
		for (const auto& bullet : bullets) {
			if (bullet.player) {
				DrawLine(bullet.position, bullet.position - olc::vi2d{ 0,5 }, olc::GREEN);
			}
			else {
				DrawLine(bullet.position, bullet.position - olc::vi2d{ 0,5 });
			}
		}

        // player input + bullet speed capped at 60 iterations per second
		if (lastRefresh > 0.016) {
			lastRefresh = 0;
			
			if (lives == 0) {
				splashText = "GAME OVER";
				showSplashText = true;
				return true;
			}
			
			
			else if (front == -1) {
				splashText = "YOU WIN!";
				showSplashText = true;
				return true;
			}
			

			// input handling
			if (GetKey(olc::Key::LEFT).bHeld && player.position.x >= 0) {
				player.position.x -= PLAYER_SPEED;
			}
			if (GetKey(olc::Key::RIGHT).bHeld && player.position.x + 13 <= ScreenWidth()) {
				player.position.x += PLAYER_SPEED;
			}
			if (GetKey(olc::Key::X).bHeld && shootRefresh > PLAYER_COOLDOWN) {
				shootRefresh = 0;
				Bullet bullet;
				bullet.player = true;
				bullet.position = player.position + olc::vi2d{ 6,5 };
				bullet.position2 = player.position + olc::vi2d{ 6,0 };
				bullets.push_back(bullet);
			}
			if (GetKey(olc::Key::Q).bHeld) {
				return false;
			}


			// bullet logic
			bullets.remove_if([](const Bullet& n) { return n.to_remove; });
			for (Bullet& bullet : bullets) {
				bullet.position.y += bullet.bulletSpeed;
				bullet.position2.y += bullet.bulletSpeed;
				if (bullet.position.y < 0 || bullet.position.y > ScreenHeight()) {
					bullet.to_remove = true;
					continue;
				}
				bool stop = false;
				if (bullet.player) {
					for (auto & invader : invaders) {
						for (auto & j : invader) {
							if (j.alive && collision_check(bullet, j)) {
								j.alive = false;
								stop = true;
								bullet.to_remove = true;
								break;
							}
						}
						if (stop) { break; }
					}
				}
				else {
					if (collision_check(bullet, player)) {
                        lives--;
                        std::cout << lives << std::endl;
                        bullet.to_remove = true;
                        continue;
                    }
				}
				for (Bullet& bullet_ : bullets) {
					if (bullet.player != bullet_.player && bullet_.position.y == bullet.position.y && 
						bullet_.position.x-2 < bullet.position.x && bullet_.position.x +2 > bullet.position.x) {
						bullet.to_remove = true;
						bullet_.to_remove = true;
						break;
					}
				}
				if (!bullet.to_remove) {
                    for (Bunker &bunker : bunkers) {
                        if (bunker.health > 0 && bunker.position.x < bullet.position.x &&
                            bullet.position.x < bunker.position.x + 22 && bullet.position.y >= bunker.position.y) {
                            bullet.to_remove = true;
                            bunker.health--;
                        }
                    }
                }
			}
		}

        // invaders shooting
		if (invaderShootTimer > 2) {
			invaderShootTimer = 0;
			Bullet bullet;
			bullet.player = false;
			bullet.position = olc::vi2d{ player.position.x+6, 15*(front+1)+invaders[0][0].position.y+5 };
			bullet.bulletSpeed = 1;
			bullets.push_back(bullet);
		}

		// invader moving
		if (lastInvaderRefresh > refreshTime) {
			lastInvaderRefresh = 0;

			if (frameCounter % 5 == 0) { 
				bool stop = false;
				if (verticalMove) {
					verticalMove = !verticalMove;
				}
				for (auto & invader : invaders) { // direction change
					for (auto & j : invader) {
						if (!j.alive) { continue; }
						if (j.position.x + INVADER_SPEED + j.sprite_wh.x > ScreenWidth() || j.position.x + INVADER_SPEED < 0) {
							INVADER_SPEED = -INVADER_SPEED;
							verticalMove = true;
							stop = true;
							break;
						}
					}
					if (stop) { break; }
				}
			}

			auto &currentRow = invaders[frameCounter % 5];
			bool row_alive = false;
			for (auto & i : currentRow) {
				if (i.alive) {
					row_alive = true;
					if (verticalMove) {
						i.position.y += 4;
					}
					else {
						i.position.x += INVADER_SPEED;
					}
					if (i.sprite_pos.y == 0) {
						i.sprite_pos.y = 8;
					}
					else {
						i.sprite_pos.y = 0;
					}
				}
			}
			if (!row_alive && frameCounter % 5 == front) {
				front--;
			}
			frameCounter++;

		}
		return true;
	}
#pragma clang diagnostic pop
};

int main()
{
	Example demo;
	if (demo.Construct(256, 240, 3, 3))
		demo.Start();

	return 0;
}