#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include "SDL_image.h"

const int WIDTH = 640;
const int HEIGHT = 480;

Sint16 x = 320, y = 240;

int initSDL(); // Starts up SDL and creates window
void closeSDL(); // Frees media and shuts down SDL

SDL_Window* window = NULL; // The window we'll be rendering to
SDL_Renderer *renderer = NULL; // The renderer contained by the window

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
	window = SDL_CreateWindow("OOP SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
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
	// Destroy renderer	
	// Destroy window	
	// Quit SDL subsystems
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void handleEvent(SDL_Event& e)
{
	/* The method for "Debunce" */

	// If a key was pressed
	// repeat: non-zero if this is a key repeat
	// https://wiki.libsdl.org/SDL_KeyboardEvent
	//if (e.type == SDL_KEYDOWN)
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: y--; break;
		case SDLK_DOWN: y++; break;
		case SDLK_LEFT: x++; break;
		case SDLK_RIGHT: y--; break;
		}
	}
	//If a key was released
	//else if (e.type == SDL_KEYUP)
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: ; break;
		case SDLK_DOWN: ; break;
		case SDLK_LEFT: ; break;
		case SDLK_RIGHT: ; break;
		}
	}
}

int main(int argc, char* args[])
{
	// Start up SDL and create window
	if (initSDL())
	{
		printf("Failed to initialize SDL!\n");
		return -1;
	}

	SDL_Event e;

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
				quit = true; // try to comment this line
			}

			// Handle input for the dot
			handleEvent(e);
		}

		printf("%d %d\n", int(x), int(y));
		SDL_RenderPresent(renderer);
	}


	//Free resources and close SDL
	closeSDL();

	return 0;
}