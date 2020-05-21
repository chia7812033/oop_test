#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <algorithm>
#include "SDL_image.h"

const int WIDTH = 1000;
const int HEIGHT = 650;
const int MAIN_HP = 100;

// frame num of mc
const int mc_FRAME = 4;

// frame number of enemy1
const int e1_walk_f = 4;
const int e1_hit_f = 6;

// frame number of enemy2
const int e2_walk_f = 4;

int i;

// Flip type
SDL_RendererFlip no = SDL_FLIP_NONE;
SDL_RendererFlip ho = SDL_FLIP_HORIZONTAL;
SDL_RendererFlip ve = SDL_FLIP_VERTICAL;
SDL_RendererFlip hove = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

struct ImageData
{
	char path[100];
	SDL_Texture *texture;
	int width;
	int height;
};

struct Character
{
	int x_pos;
	int y_pos;
	int x_velo;
	int y_velo;
	int velocity;
	unsigned int health;
	int damage;
	int way = 0;  //0 for right, 1 for left
	int action_flag[10] = { 0 }; // 0:stand, 1:run 2:a, 3:s 4:¡ô+s 5:damage
	int w, h;
};
Character main_ch;
Character *enemy_list;

// 0:up 1:down 2:left 3:right 4:a 5:s
int keyRecord[6] = { 0 };

int initSDL(); // Starts up SDL and creates window
void closeSDL(); // Frees media and shuts down SDL
ImageData loadTexture(char *path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b);
void imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, int cx, int cy,\
	double angle, SDL_RendererFlip flip);
void imgRender_size(SDL_Renderer *renderer, ImageData img, int posX, int posY, int wid, int hei,\
	int cx, int cy, double angle, SDL_RendererFlip flip);
void loadImageSet(ImageData *picture_array, char *path, int frame);
void bg_imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY);
void mc_imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, SDL_RendererFlip flip);
void e_imgRender(SDL_Renderer *renderer, ImageData img, Character e, SDL_RendererFlip flip, int w, int h);
void handleEvent(SDL_Event& e);
void mc_move();
void e_move1(Character &e);
void e_move2(Character &e, Character &mc);
void drawRectangleBound(SDL_Renderer * renderer, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,\
	Sint16 bound, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void showstatus(SDL_Renderer * renderer, ImageData mc_pro, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, \
	Sint16 bound, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Character ch);
int checkaction(Character ch, int n);
int checkcollision(Character *enemies, SDL_RendererFlip flip);
void gotDamage(Character &attacker, Character &opponent);
//function for mc
Uint32  mc_action_stand(Uint32 interval, void *param);
Uint32  mc_action_walk(Uint32 interval, void *param);
Uint32  mc_action_run(Uint32 interval, void *param);
Uint32  mc_attack_A(Uint32 interval, void *param);
Uint32  mc_attack_S(Uint32 interval, void *param);

//function for e1
Uint32  e1_action_walk(Uint32 interval, void *param);
Uint32  e1_action_hit(Uint32 interval, void *param);

//function for e2
Uint32  e2_action(Uint32 interval, void *param);

SDL_Window* window = NULL; // The window we'll be rendering to
SDL_Renderer *renderer = NULL; // The renderer contained by the window

SDL_Texture *bgTexture = NULL; // The background texture
// SDL_Texture *pikaTexture = NULL;
// SDL_Texture *spTexture = NULL;

int initSDL()
{
	// Initialize SDL	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		// Error Handling		
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	// Create window	
	// SDL_WINDOWPOS_UNDEFINED: Used to indicate that you don't care what the window position is.
	window = SDL_CreateWindow("OOP SDL project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 2;
	}

	// Create renderer	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 3;
	}

	return 0;
}

void closeSDL()
{
	SDL_DestroyTexture(bgTexture);
	// Destroy renderer	
	// Destroy window	
	// Quit SDL subsystems
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void drawRectangleBound(SDL_Renderer * renderer, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,\
	Sint16 bound, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (((x2 - x1) >= bound) && ((y2 - y1) >= bound))
		for (int i = 0; i < bound; i++)
			rectangleRGBA(renderer, x1 + i, y1 + i, x2 - i, y2 - i, r, g, b, a);
}

ImageData loadTexture(char *path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b)
{
	ImageData img;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL)
	{
		printf("IMG_Load failed: %s\n", IMG_GetError());
	}
	else
	{
		// Set the color key (transparent pixel) in a surface.
		// https://wiki.libsdl.org/SDL_SetColorKey
		// The color key defines a pixel value that will be treated as transparent in a blit. 
		// It is a pixel of the format used by the surface, as generated by SDL_MapRGB().
		// Use SDL_MapRGB() to map an RGB triple to an opaque pixel value for a given pixel format.
		// https://wiki.libsdl.org/SDL_MapRGB
		SDL_SetColorKey(loadedSurface, ckEnable, SDL_MapRGB(loadedSurface->format, r, g, b));

		// Create texture from surface pixels
		img.texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (img.texture == NULL)
		{
			printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		}

		//Get image dimensions
		img.width = loadedSurface->w;
		img.height = loadedSurface->h;

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//return newTexture;
	return img;
}

void imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, int cx, int cy, \
	double angle, SDL_RendererFlip flip)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	r.w = img.width;
	r.h = img.height;

	SDL_Point center = { cx, cy };
	SDL_RenderCopyEx(renderer, img.texture, NULL, &r, angle, &center, flip);
}

void imgRender_size(SDL_Renderer *renderer, ImageData img, int posX, int posY, int wid, int hei, \
	int cx = 0, int cy = 0, double angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	r.w = wid;
	r.h = hei;

	SDL_Point center = { cx, cy };
	SDL_RenderCopyEx(renderer, img.texture, NULL, &r, angle, &center, flip);
}

void loadImageSet(ImageData *picture_array, char *path, int frame)
{
	for (int i = 0; i < frame; i++)
	{
		char str[100];
		sprintf_s(str, 100, "%s%02d.png", path, i + 1);
		picture_array[i] = loadTexture(str, false, 0xFF, 0xFF, 0xFF);
	}
}

void bg_imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	r.w = WIDTH;
	r.h = HEIGHT;
	SDL_RenderCopy(renderer, img.texture, NULL, &r);
}

void mc_imgRender(SDL_Renderer *renderer, ImageData img, int posX, int posY, SDL_RendererFlip flip)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	if (main_ch.action_flag[2] || main_ch.action_flag[3]) // attack
	{
		r.w = 210;
		r.h = 200;
	}
	else
	{
		r.w = 170;
		r.h = 200;
	}
	SDL_RenderCopyEx(renderer, img.texture, NULL, &r, 0, NULL, flip);
}

void e_imgRender(SDL_Renderer *renderer, ImageData img, Character e, SDL_RendererFlip flip, int w, int h)
{
	SDL_Rect r;
	r.x = e.x_pos;
	r.y = e.y_pos;
	r.w = w;
	r.h = h;
	
	SDL_RenderCopyEx(renderer, img.texture, NULL, &r, 0, NULL, flip);
}

void handleEvent(SDL_Event& e)
{
	/* The method for "Debunce" */
	double ratio;
	// If a key was pressed
	// repeat: non-zero if this is a key repeat
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		static int lasttime = SDL_GetTicks();
		static int curtime = SDL_GetTicks();
		int timediv[5];
		int time_flag;

		lasttime = curtime;
		curtime = SDL_GetTicks();
		timediv[0] = curtime - lasttime;
		if (timediv[0] < 300) // judge if double click
			time_flag = 1;
		else
		{
			time_flag = 0;
			for (int i = 0; i < 6; i++)
				keyRecord[i] = 0;
		}
		if (time_flag)
			ratio = 1.5; // for running
		else
			ratio = 1; // for walk
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_a:
			if (checkaction(main_ch, 2) && main_ch.action_flag[2] < 2)
			//if (main_ch.action_flag[2] < 2)
			{
				main_ch.action_flag[2]++;
				keyRecord[4]++;
			}
			break;
		case SDLK_s:
			if (checkaction(main_ch, 3) && main_ch.action_flag[3] < 2)
			// if (main_ch.action_flag[3] < 2)
			{
				main_ch.action_flag[3]++;
				keyRecord[5]++;
			}
			break;
		case SDLK_UP: 
			if (time_flag && keyRecord[0] == 1)
			{
				if (checkaction(main_ch, 1))
					main_ch.action_flag[1] = 1;
					// main_ch.action_flag[0]++;
			}
			else
				main_ch.action_flag[0]++;
			main_ch.y_velo -= (int)main_ch.velocity * ratio;
			keyRecord[0] = 1;
			break;
		case SDLK_DOWN: 
			if (time_flag && keyRecord[0] == 2)
			{
				if (checkaction(main_ch, 1))
					main_ch.action_flag[1] = 1;
				// main_ch.action_flag[0]++;
			}
			else
				main_ch.action_flag[0]++;
			main_ch.y_velo += (int)main_ch.velocity * ratio;
			keyRecord[0] = 2;
			break;
		case SDLK_LEFT:
			if (time_flag && keyRecord[0] == 3)
			{
				if (checkaction(main_ch, 1))
					main_ch.action_flag[1] = 1;
				// main_ch.action_flag[0]++;
			}
			else
				main_ch.action_flag[0]++;
			main_ch.x_velo -= (int)main_ch.velocity * ratio;
			main_ch.way = 1;
			keyRecord[0] = 3;
			break;
		case SDLK_RIGHT: 
			if (time_flag && keyRecord[0] == 4)
			{
				if (checkaction(main_ch, 1))
					main_ch.action_flag[1] = 1;
				// main_ch.action_flag[0]++;
			}
			else
				main_ch.action_flag[0]++;
			main_ch.x_velo += (int)main_ch.velocity * ratio;
			main_ch.way = 0;
			keyRecord[0] = 4;
			break;
		default:
			break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: 
			main_ch.y_velo = 0;
			if (main_ch.action_flag[0] > 0)
				main_ch.action_flag[0]--;
			main_ch.action_flag[1] = 0;
			break;
		case SDLK_DOWN: 
			main_ch.y_velo = 0;
			if (main_ch.action_flag[0] > 0)
				main_ch.action_flag[0]--;
			main_ch.action_flag[1] = 0;
			break;
		case SDLK_LEFT: 
			main_ch.x_velo = 0;
			if (main_ch.action_flag[0] > 0)
				main_ch.action_flag[0]--;
			main_ch.action_flag[1] = 0;
			break;
		case SDLK_RIGHT: 
			main_ch.x_velo = 0;
			if (main_ch.action_flag[0] > 0)
				main_ch.action_flag[0]--;
			main_ch.action_flag[1] = 0;
			break;
		default:
			break;
		}
	}
}

void mc_move()
{
	//Move the dot left or right
	main_ch.x_pos += main_ch.x_velo;

	//If the dot went too far to the left or right
	if ((main_ch.x_pos <= 0) || (main_ch.x_pos >= WIDTH - 150))
	{
		//Move back
		main_ch.x_pos -= main_ch.x_velo;

	}

	//Move the dot up or down
	main_ch.y_pos += main_ch.y_velo;

	//If the dot went too far up or down
	if ((main_ch.y_pos <= 0) || (main_ch.y_pos >= HEIGHT -200))
	{
		//Move back
		main_ch.y_pos -= main_ch.y_velo;
	}
}

void e_move1(Character &e)
{
	if (!e.action_flag[5])
	{
		e.x_pos += e.x_velo;

		//If the dot went too far to the left or right
		if ((e.x_pos <= 0) || (e.x_pos >= WIDTH - e.w + 50))
		{
			//Move back
			e.x_pos -= e.x_velo;
			e.x_velo = -1 * e.x_velo;
			e.way = (e.way + 1) % 2;
		}

		e.y_pos += e.y_velo;

		//If the dot went too far up or down
		if ((e.y_pos <= 0) || (e.y_pos >= HEIGHT - e.h - 100))
		{
			//Move back
			e.y_pos -= e.y_velo;
			e.y_velo = -1 * e.y_velo;
		}
	}
}

void e_move2(Character &e, Character &mc) {
	if (!e.action_flag[5])
	{
		int mc_way_x = e.x_pos - mc.x_pos;
		int mc_way_y = e.y_pos - mc.y_pos;

		if (mc_way_x >= 100) {
			e.x_pos -= e.x_velo;
			e.way = 1;
		}
		else if (mc_way_x <= -100) {
			e.x_pos += e.x_velo;
			e.way = 0;
		}
		else {

		}

		//If the dot went too far to the left or right
		if ((e.x_pos <= 0) || (e.x_pos >= WIDTH - 150))
		{
			//Move back
			e.x_pos -= e.x_velo;
			e.x_velo = -1 * e.x_velo;
			e.way = (e.way + 1) % 2;
		}

		if (mc_way_y > 0) {
			e.y_pos -= e.y_velo;
		}
		else if (mc_way_y < 0) {
			e.y_pos += e.y_velo;
		}
		else {

		}

		//If the dot went too far up or down
		if ((e.y_pos <= 0) || (e.y_pos >= HEIGHT - 200))
		{
			//Move back
			e.y_pos -= e.y_velo;
			e.y_velo = -1 * e.y_velo;
		}
	}
}

void showstatus(SDL_Renderer * renderer, ImageData mc_pro, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,\
	Sint16 bound, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Character ch)
{
	drawRectangleBound(renderer, x1, y1, x2, y2, bound, 0x00, 0x00, 0x00, 0xFF);
	imgRender_size(renderer, mc_pro, x1 + 2, y1 + 2, 86, 86);

	drawRectangleBound(renderer, x1, y1 + 95, y2, y2 + 25, bound, 0x00, 0x00, 0x00, 0xFF);
	roundedBoxRGBA(renderer, x1 + bound, y1 + 95 + bound, y2 - bound, y2 + 25 - bound, 0, 69, 69, 69, 255);

	drawRectangleBound(renderer, x1, y1 + 120, x2, y2 + 50, bound, 0x00, 0x00, 0x00, 0xFF);
	double hp_ratio;
	if (ch.health <= 0) {
		hp_ratio = 0;
	}
	else {
		hp_ratio = ch.health / MAIN_HP;
	}

	roundedBoxRGBA(renderer, x1 + bound, y1 + 120 + bound, x1 + 90 * (hp_ratio)-bound, y2 + 50 - bound, 0, 255, 255, 0, 255);
}

int checkaction(Character ch, int n)
{
	for (int i = 0; i < 6; i++)
		if (ch.action_flag[i] && i != n)
			return 0;
	return 1;
}

int checkcollision(Character *enemies, SDL_RendererFlip flip)
{
	int m[4]; // x1, x2, y1, y2
	int e_pos[4]; // x1, x2, y1, y2
	if (flip == no)
	{
		m[0] = main_ch.x_pos + 150;
		m[1] = main_ch.y_pos + 50;
		m[2] = 60;
		m[3] = 50;
	}
	else
	{
		m[0] = main_ch.x_pos;
		m[1] = main_ch.y_pos + 50;
		m[2] = 60;
		m[3] = 50;
	}
	for (int i = 0; i < 2; i++)
	{
		e_pos[0] = enemies[i].x_pos;
		e_pos[1] = enemies[i].y_pos;
		e_pos[2] = enemies[i].w;
		e_pos[3] = enemies[i].h;
		if (m[0] + m[3] > e_pos[0] &&
			e_pos[0] + e_pos[2] > m[0] &&
			m[1] + m[3] > e_pos[1] &&
			e_pos[1] + e_pos[3] > m[1]
			)
			return i;
	}
	return -1;
}

void gotDamage(Character &attacker, Character &opponent)
{
	opponent.health -= attacker.damage;
	opponent.action_flag[5]++;
	// printf("%d\n", opponent.health);
}

Uint32  mc_action_stand(Uint32 interval, void *param)
{
	int *index = (int *)param;
	(*index) = ((*index) + 1) % mc_FRAME;

	return interval;
}

Uint32  mc_action_walk(Uint32 interval, void *param)
{
	int *index = (int *)param;
	(*index) = ((*index) + 1) % mc_FRAME;

	return interval;
}

Uint32  mc_action_run(Uint32 interval, void *param)
{
	int *index = (int *)param;
	(*index) = ((*index) + 1) % mc_FRAME;

	return interval;
}

Uint32  mc_attack_A(Uint32 interval, void *param)
{
	int *index = (int *)param;
	
	if (main_ch.action_flag[2])
	{
		if ((*index) == 0 || (*index) == 4)
		{
			if (i != -1)
			{
				gotDamage(main_ch, enemy_list[i]);
				printf("%d\n", (*index));
			}
		}
		if ((*index) < 8)
			(*index) = ((*index) + 1);
	}
	else
		(*index) = 0;
	return interval;
}

Uint32  mc_attack_S(Uint32 interval, void *param)
{
	int *index = (int *)param;

	if (main_ch.action_flag[3])
	{
		if ((*index) == 0 || (*index) == 4)
		{
			if (i != -1)
			{
				gotDamage(main_ch, enemy_list[i]);
				// printf("%d\n", (*index));
			}
		}
		if ((*index) < 8)
			(*index) = ((*index) + 1);
	}
	else
		(*index) = 0;
	return interval;
}

Uint32  e1_action_walk(Uint32 interval, void *param)
{
	int *index = (int *)param;
	(*index) = ((*index) + 1) % e1_walk_f;

	return interval;
}

Uint32  e1_action_hit(Uint32 interval, void *param)
{
	static int counter = 0;
	static int last_index = 0;
	int *index = (int *)param;
	if (last_index != (*index))
	{
		counter = 0;
		last_index = (*index);
	}
	if (enemy_list[0].action_flag[5])
	{
		if (counter == 6)
		{
			enemy_list[0].action_flag[5] = 0;
			counter = 0;
			(*index) = 0;
		}
		counter++;
	}

	return interval;
}

Uint32  e2_action(Uint32 interval, void *param)
{
	int *index = (int *)param;
	(*index) = ((*index) + 1) % e2_walk_f;

	return interval;
}

Uint32  e2_action_hit(Uint32 interval, void *param)
{
	static int counter = 0;
	static int last_index = 0;
	int *index = (int *)param;
	if (last_index != (*index))
	{
		counter = 0;
		last_index = (*index);
	}
	if (enemy_list[1].action_flag[5])
	{
		if (counter == 6)
		{
			enemy_list[1].action_flag[5] = 0;
			counter = 0;
			(*index) = 0;
		}
		counter++;
	}

	return interval;
}

int main(int argc, char* args[])
{
	//Path & index setting
	// path for background
	char imgPath_bg[100] = "../images/bg.png";

	// main character setting
	// path of main character profile
	char mc_pro_Path[100] = "../images/mc_pro.png";
	// path for main character
	char mc_stand_Path[100] = "../images/purple/stand/";
	char mc_walk_Path[100] = "../images/purple/walk/";
	char mc_run_Path[100] = "../images/purple/run/";
	char mc_attack1_Path[100] = "../images/purple/attack1/";
	char mc_attack2_Path[100] = "../images/purple/attack2/";
	
	// index of mc anime
	int mc_stand_index = 0;
	int mc_walk_index = 0;
	int mc_run_index = 0;
	int mc_attack1_index = 0;
	int mc_attack2_index = 0;

	// enemy1
	// path for enemy1
	char e1_Path[2][100] = { "../images/e1/walk/" , "../images/e1/hit/" };
	// enemy1 anime index
	int e1_walk_index = 0;
	int e1_hit_index = 0;

	// enemy2
	// path for enemy2
	char e2_Path[2][100] = { "../images/e2/walk/", "../images/e2/hit/" };
	// enemy2 anime index
	int e2_walk_index = 0;
	int e2_hit_index = 0;

	
	// generate images of each object
	ImageData bg;
	ImageData mc_pro;
	ImageData mc_stand[mc_FRAME], mc_walk[mc_FRAME], mc_run[mc_FRAME], mc_attack1[8], mc_attack2[8];
	ImageData e1_walk[e1_walk_f], e1_hit[e1_hit_f];
	ImageData e2_walk[e2_walk_f] , e2_hit[3];

	SDL_RendererFlip flip_flag;

	// Start up SDL and create window
	if (initSDL())
	{
		printf("Failed to initialize SDL!\n");
		return -1;
	}

	SDL_Event e;

//load png
	//load png of bg
	bg = loadTexture(imgPath_bg, false, 0xFF, 0xFF, 0xFF);

	//load png of mc
	//load profile picture of main character
	mc_pro = loadTexture(mc_pro_Path, false, 0xFF, 0xFF, 0xFF);
	//load png for main character standing
	loadImageSet(mc_stand, mc_stand_Path, mc_FRAME);
	//load png for main character walking
	loadImageSet(mc_walk, mc_walk_Path, mc_FRAME);
	//load png for main character running
	loadImageSet(mc_run, mc_run_Path, mc_FRAME);
	//load png for main character attack1
	loadImageSet(mc_attack1, mc_attack1_Path, 8);
	//load png for main character attack2
	loadImageSet(mc_attack2, mc_attack2_Path, 8);

	//load png of enemy1
	//load png for e1 walk
	loadImageSet(e1_walk, e1_Path[0], e1_walk_f);
	//load png for e1 hit
	loadImageSet(e1_hit, e1_Path[1], 3);

	//load png of enemy2
	//load png for e2 walk
	loadImageSet(e2_walk, e2_Path[0], e2_walk_f);
	loadImageSet(e2_hit, e2_Path[1], 3);

//Timer
	//timer for mc
	SDL_TimerID mc_timerID1_stand = SDL_AddTimer(150, mc_action_stand, &mc_stand_index);
	SDL_TimerID mc_timerID2_walk = SDL_AddTimer(150, mc_action_walk, &mc_walk_index);
	SDL_TimerID mc_timerID3_run = SDL_AddTimer(100, mc_action_run, &mc_run_index);
	SDL_TimerID mc_timerID4_attack1 = SDL_AddTimer(100, mc_attack_A, &mc_attack1_index);
	SDL_TimerID mc_timerID5_attack2 = SDL_AddTimer(150, mc_attack_S, &mc_attack2_index);

	//timer for e1
	SDL_TimerID e1_timerID1_walk = SDL_AddTimer(100, e1_action_walk, &e1_walk_index);
	SDL_TimerID e1_timerID2_hit = SDL_AddTimer(100, e1_action_hit, &e1_hit_index);

	//timer for e2
	SDL_TimerID e2_timerID1_walk = SDL_AddTimer(100, e2_action, &e2_walk_index);
	SDL_TimerID e2_timerID1_hit = SDL_AddTimer(100, e2_action_hit, &e2_hit_index);

//Initialize
	// initialize main character
	main_ch.x_pos = 100;
	main_ch.y_pos = HEIGHT / 2;
	main_ch.x_velo = 0;
	main_ch.y_velo = 0;
	main_ch.velocity = 2;
	main_ch.health = MAIN_HP;
	main_ch.damage = 10;

	enemy_list = new Character[2];
	// initialize enemy1
	enemy_list[0].x_pos = 500;
	enemy_list[0].y_pos = HEIGHT / 2;
	enemy_list[0].x_velo = 2;
	enemy_list[0].y_velo = 1;
	enemy_list[0].way = 1;
	enemy_list[0].health = MAIN_HP;
	enemy_list[0].h= 180;
	enemy_list[0].w = 100;

	// initialize enemy2
	Character e2;
	enemy_list[1].x_pos = 300;
	enemy_list[1].y_pos = 300;
	enemy_list[1].x_velo = 1;
	enemy_list[1].y_velo = 1;
	enemy_list[1].way = 1;
	enemy_list[1].health = MAIN_HP;
	enemy_list[1].h = 180;
	enemy_list[1].w = 150;
	

//Main loop flag
	bool quit = false;

	while (!quit)
	{
		// Handle events on queue		
		while (SDL_PollEvent(&e) != 0)
		{
			// User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true; 
			}

			// Handle input for the dot
			handleEvent(e);
		}

	// character action
		if (!main_ch.action_flag[2] && !main_ch.action_flag[3])
			mc_move();
		e_move1(enemy_list[0]);
		e_move2(enemy_list[1], main_ch);

	// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(renderer);

	//Render handling
		//bg rendering
		bg_imgRender(renderer, bg, 0, 0);

		//mc rendering
		if (main_ch.way)
			flip_flag = ho;
		else
			flip_flag = no;

		
		if (main_ch.action_flag[2]) {
			mc_imgRender(renderer, mc_attack1[mc_attack1_index], main_ch.x_pos, main_ch.y_pos, flip_flag);
			i = checkcollision(enemy_list, flip_flag);
			if (mc_attack1_index == 3)
			{
				if (main_ch.action_flag[2] == 1)
				{
					main_ch.action_flag[2] = 0;
					mc_attack1_index = 0;
				}
			}
			if (mc_attack1_index == 7)
			{
				if (main_ch.action_flag[2] == 2)
				{
					main_ch.action_flag[2] = 0;
					mc_attack1_index = 0;
				}
			}
		}
		else if (main_ch.action_flag[3]) {
			mc_imgRender(renderer, mc_attack2[mc_attack2_index], main_ch.x_pos, main_ch.y_pos, flip_flag);
			i = checkcollision(enemy_list, flip_flag);
			if (mc_attack2_index == 3)
			{
				if (main_ch.action_flag[3] == 1)
				{
					main_ch.action_flag[3] = 0;
					mc_attack2_index = 0;
				}
			}
			if (mc_attack2_index == 7)
			{
				if (main_ch.action_flag[3] == 2)
				{
					main_ch.action_flag[3] = 0;
					mc_attack2_index = 0;
				}
			}
		}
		else if (main_ch.action_flag[1]) {
			mc_imgRender(renderer, mc_run[mc_run_index], main_ch.x_pos, main_ch.y_pos, flip_flag);
		}
		else if (main_ch.action_flag[0]) {
			mc_imgRender(renderer, mc_walk[mc_walk_index], main_ch.x_pos, main_ch.y_pos, flip_flag);
		}
		else {
			mc_imgRender(renderer, mc_stand[mc_stand_index], main_ch.x_pos, main_ch.y_pos, flip_flag);
		}

		
		// printf("%d %d %d %d\n", main_ch.action_flag[0], main_ch.action_flag[1], main_ch.action_flag[2], main_ch.action_flag[3]); // display character status

		//enemy1 rendering
		if (!enemy_list[0].way)
			flip_flag = ho;
		else
			flip_flag = no;

		if (enemy_list[0].action_flag[5]) {
			e1_hit_index = (enemy_list[0].action_flag[5] - 1) % 3;
			e_imgRender(renderer, e1_hit[e1_hit_index], enemy_list[0], flip_flag, enemy_list[0].w + 70, enemy_list[0].h);
		}
		else {
			e_imgRender(renderer, e1_walk[e1_walk_index], enemy_list[0], flip_flag, enemy_list[0].w, enemy_list[0].h);
		}

		//enemy2 rendering
		if (!enemy_list[1].way)
			flip_flag = ho;
		else
			flip_flag = no;

		if (enemy_list[1].action_flag[5]) {
			e2_hit_index = (enemy_list[1].action_flag[5] - 1) % 3;
			e_imgRender(renderer, e2_hit[e2_hit_index], enemy_list[1], flip_flag, enemy_list[1].w, enemy_list[1].h);
		}
		else {
		e_imgRender(renderer, e2_walk[e2_walk_index], enemy_list[1], flip_flag, enemy_list[1].w, enemy_list[1].h);
		}

		//display health of main character
		showstatus(renderer, mc_pro, 20, 20, 110, 110, 3, 0x00, 0x00, 0x00, 0xFF, main_ch);
	
		SDL_RenderPresent(renderer);

	}
	
	//Free resources and close SDL
	closeSDL();

	return 0;
}