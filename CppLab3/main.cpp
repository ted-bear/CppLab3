
#include "include/SDL.h"
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
using namespace std;

int score=0;

class Platform {

public:
	SDL_Rect pos;
	int speed = 5;
	int minSize;

	Platform(int x, int y, int w, int h) {
		pos.x = x;
		pos.y = y;
		pos.w = w;
		pos.h = h;
		minSize = w;
	}

	void Draw(SDL_Renderer* ren) {
		SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderFillRect(ren, &pos);
	}

	void Move(SDL_Event* e, SDL_Rect edge) {

		if (e->type == SDL_KEYDOWN) {
			if (e->key.keysym.sym == SDLK_RIGHT && pos.x+pos.w<edge.w) {
				pos.x += speed;
			}
			if (e->key.keysym.sym == SDLK_LEFT && pos.x>0) {
				pos.x -= speed;
			}
		}

	}

	void changeSize(int x) {
		if (pos.w+x > minSize)
		    pos.w = pos.w + x;
	}
};


int Collision(SDL_Rect a, SDL_Rect b)
{
	if (a.y < b.y + b.h && a.y + a.h > b.y + b.h) {
		if (a.x >= b.x && a.x + a.w <= b.x + b.w)
			return 4;

		if (a.x <= b.x + b.w && a.x + a.w > b.x + b.w) {
			if (b.y + b.h - a.y > b.x + b.w - a.x)
				return 1;
			else return 4;
		}

		if (a.x < b.x  && a.x + a.w >= b.x ) {
			if (b.y + b.h - a.y > a.x + a.w - b.x)
				return 3;
			else return 4;
		}
		
	}

	if (a.y < b.y && a.y + a.h > b.y) {
		if (a.x >= b.x && a.x + a.w <= b.x + b.w) {
			return 2;
		}

		if (a.x <= b.x + b.w && a.x + a.w > b.x + b.w) {
			if (a.y + a.h - b.y > b.x + b.w - a.x)
				return 1;
			else return 2;
		}

		if (a.x < b.x && a.x + a.w >= b.x) {
			if (a.y + a.h > a.x + a.w - b.x)
				return 3;
			else return 2;
		}
		
	}

	if (a.y > b.y  && a.y + a.h < b.y + b.h) {

		if (a.x <= b.x + b.w && a.x + a.w > b.x + b.w) {
			return 1;
		}

		if (a.x < b.x && a.x + a.w >= b.x) {
			return 3;

		}
	}

	return 0;
}




class Brick {
	int color[4][3] =
	{
		{0,0,0},
		{139, 69, 19},
		{112, 128, 144},
		{255, 215, 0}
	};

public:

	int life;
	int immortal = 0;
	SDL_Rect pos;
	int move;
	int speed = 2, dx;

	Brick(int x, int y, int w, int h, int l, int m) {
		pos.x = x;
		pos.y = y;
		pos.w = w;
		pos.h = h;
		life = l;
		if (l == 9) immortal = 1;
		move = m;
		dx = speed;
	}

	void Damage() {
		if (immortal == 0)
		life -= 1;
	}

	void Draw(SDL_Renderer* ren) {
		if (immortal == 0)
		SDL_SetRenderDrawColor(ren, color[life][0], color[life][1], color[life][2], 0xFF);
		else SDL_SetRenderDrawColor(ren, 175, 238, 238, 0xFF);
		SDL_RenderFillRect(ren, &pos);

	}
};

class Ball {

	int dx = 0;
	int dy = 0;
	vector<Brick*>*bricks;
	int minSpeed;

public:
	SDL_Rect pos;
	int speed = 2;

	Ball(int x, int y, int r, vector<Brick*>*b) {
		pos.x = x;
		pos.y = y;
		pos.w = r;
		pos.h = r;
		bricks = b;
		dx = speed;
		dy = -speed;
		minSpeed = speed;
	}

	void Draw(SDL_Renderer* ren) {
		SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderFillRect(ren, &pos);
	}

	int Move(SDL_Event* e, SDL_Rect *screen, SDL_Rect *platform) {
		int fail = 0;
		dx = dx > 0 ? speed : -speed;

		if (pos.x + pos.w/2 >= screen->w)
			dx = -speed;
		if (pos.x - pos.w/2 <= 0)
			dx = speed;
		if (pos.y + pos.w / 2 >= screen->h) {
			dy = -speed;
			fail = 1;
		}
		if (pos.y - pos.w / 2 <=0)
			dy = speed;

		if (pos.y + pos.w >= platform->y && pos.x > platform->x - pos.w && pos.x < platform->x + platform->w)
			dy = -speed;

		for (int i = bricks->size()-1; i >= 0; i--) {
			int checkColl = Collision(pos, bricks->at(i)->pos);
			if (checkColl > 0) {
				bricks->at(i)->Damage();
				if (checkColl == 1 || checkColl == 3)
					dx = -dx;
				if (checkColl == 2 || checkColl == 4)
					dy = -dy;
				break;
			}

		}

		pos.x += dx;
		pos.y += dy;

		return fail;
	}

	void changeSpeed(int x) {
		if (speed + x > minSpeed)
			speed += x;
		else speed = minSpeed;
	}
};


class Bonus {
	int speed = 1;
	int type;
	const int numType = 4;
	int color[5][3] =
	{ {124, 252, 0},
		{0, 206, 209},
		{138, 43, 226},
		{220, 20, 60},
		{255, 215, 0}
	};
	vector<Ball*>* balls;
	vector<Brick*>* bricks;
	Platform* player;
	int* save;

public:
	SDL_Rect pos;
	Bonus(int x, int y, int r, int t, vector<Ball*>* b, vector<Brick*>* br, Platform* pl, int *s) {
		pos.x = x;
		pos.y = y;
		pos.w = r;
		pos.h = r;
		type = t;
		balls = b;
		bricks = br;
		player = pl;
		save = s;

	}

	void Draw(SDL_Renderer* ren) {
		SDL_SetRenderDrawColor(ren, color[type][0], color[type][1], color[type][2], 0xFF);
		SDL_RenderFillRect(ren, &pos);
	}

	int Move(SDL_Rect *edge) {
		if (pos.y + pos.w >= player->pos.y && pos.x > player->pos.x - pos.w && pos.x < player->pos.x + player->pos.w) {
			chooseBonus();
			return 1;
		}

		if (pos.y + pos.h > edge->h)
			return 1;

		pos.y += speed;
		return 0;
	}

	void addBall() {
		balls->push_back(new Ball(player->pos.x, player->pos.y - 10, 10, bricks));
	}

	void incrPlatform() {
		player->changeSize(5);
	}

	void incrBallSpeed() {

		for (auto b : *balls) {
			b->changeSpeed(1);
		}

	}

	void addSave() {
		*save = 1;
	}


	void chooseBonus() {
		if (type == 0) addBall();
		if (type == 1) incrPlatform();
		if (type == 2) incrBallSpeed();
		if (type == 3) addSave();
	}
};


class safeZone {
public:
	SDL_Rect pos;
	safeZone(int y, int w, int h) {
		pos.x = 0;
		pos.y = y;
		pos.h = h;
		pos.w = w;
	}

	void Draw(SDL_Renderer* ren) {
		SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderFillRect(ren, &pos);
	}
};

class Arcanoid {

	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT = 480;
	const int brick_x = 63;
	const int brick_y = 25;
	SDL_Texture* background;
	SDL_Rect bg_rect; //background size
	int bonusChance = 50;
	ifstream in;

public:
	SDL_Window* win = NULL;
	SDL_Renderer* ren = NULL;
	Platform *player;
	//Ball* ball;
	vector<Brick*> bricks;
	vector<Bonus*> bonus;
	vector<Ball*> balls;
	safeZone* saveZ;
    int save = 0;

	bool onLoad() {
		bool success = true;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cout << "init error";
			success = false;
		}
		else
		{
			//Create window
			win = SDL_CreateWindow("Arcanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
			if (win == NULL)
			{
				std::cout << "Window load error";
				success = false;
			}
		}

		ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_PRESENTVSYNC);
		
		if (ren == NULL) {
			std::cout << "Can't create renderer: ";
			success = false;
		}
		
		bg_rect.x = 0;
		bg_rect.y = 0;
		bg_rect.w = SCREEN_WIDTH;
		bg_rect.h = SCREEN_HEIGHT;

		in.open("map.txt");

		return success;
	}


	bool onCreate() {

		SDL_Surface* BMP_background = SDL_LoadBMP("0.bmp");
		if (BMP_background == nullptr) {
			std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
			return true;
		}

		background = SDL_CreateTextureFromSurface(ren, BMP_background);
		SDL_FreeSurface(BMP_background);

		player = new Platform(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, 80, 10);
		if (player == NULL) return false;

		balls.push_back(new Ball(player->pos.x, player->pos.y - 10, 10, &bricks));
		if (balls[0] == NULL) return false;
	

		char c;
		int i = 0, j = 0;
		while (!in.eof()) {
			in >> c;
			if (c <= '9' && c >= '1') {
				int l = c - '0';
				bricks.push_back(new Brick(j * brick_x + 10, i * brick_y + 10, brick_x - 9, brick_y - 9, l, 0));
				if (bricks[bricks.size()-1] == NULL) return false;
			}
			if (c == 'e') {
				i++;
				j = 0;
			}
			
			j++;
		}	

		saveZ = new safeZone(SCREEN_HEIGHT - 4, SCREEN_WIDTH, 4);
	}


	void onUpdate() {

		bool quit = false;
		SDL_Event e;

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}	
			}

			player->Move(&e, bg_rect);

			for (int i = 0; i < balls.size(); i++) {
				if (balls[i]->Move(&e, &bg_rect, &(player->pos))) {
					if (save == 0) {
						if (balls.size() > 1) {
							delete balls[i];
							balls.erase(balls.begin() + i);
						}
						else
						{
							player->changeSize(-10);
							for (auto b : balls) {
								b->changeSpeed(-2);
							}
							cout << --score << endl;
						}
					}
					if (save == 1)
						save = 0;
				}
			}

			for (int i = 0; i < bonus.size(); i++) {
				if (bonus[i]->Move(&bg_rect)) {
					delete bonus[i];
					bonus.erase(bonus.begin() + i);
				}
			}

			/*for (int i = 0; i < bricks.size(); i++) {
				if (bricks[i]->move)
					bricks[i]->Move(&bg_rect);
			}*/

			for (int i = 0; i < bricks.size(); i++) {
				auto b = bricks[i];
				if (b->life == 0) {
					if (rand() % 100 < bonusChance) {
						bonus.push_back(new Bonus(b->pos.x, b->pos.y, 10, rand() % 4, &balls, &bricks, player, &save));
					}
					delete bricks[i];
					bricks.erase(bricks.begin() +i);
					cout << ++score<< endl;
				}
			}
	///////////////////////////////////////////////////////////////////////////////////
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, background, NULL, &bg_rect);

			player->Draw(ren);
			for (int i = 0; i< balls.size(); i++) {
				balls[i]->Draw(ren);
			}
			for (int i = 0; i < bricks.size(); i++) {
				bricks[i]->Draw(ren);	
			}
			for (auto b : bonus)
				b->Draw(ren);

			if (save)
				saveZ->Draw(ren);


			SDL_RenderPresent(ren);
			
			if (bricks.size() == 2)
				quit = true;

		}

	}

	void onClose() {

		//Destroy window
		SDL_DestroyWindow(win);
		win = NULL;

		SDL_DestroyRenderer(ren);
		ren = NULL;

		for (int i = 0; i < bricks.size(); i++) {
			delete bricks[i];
		}

		delete player;
		delete saveZ;
		in.close();

		//Quit SDL subsystems
		SDL_Quit();
	}
};



int main(int argc, char* args[])
{

	Arcanoid game;

	if (!game.onLoad()) {
		cout << "Load Error";
		return 1;
	}
	if (!game.onCreate()) {
		cout << "Create Error";
		return 2;
	}
	game.onUpdate();
	game.onClose();
	return 0;
}