#include <SDL2/SDL.h>
#include "../entities/entity.h"


void proper_shutdown(SDL_Renderer *renderer, SDL_Window *window)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void handle_key(SDL_KeyboardEvent *keyevent, keystate updown,
	player *tha_playa)
{
	if (keyevent->repeat == 0)
	{
		if (keyevent->keysym.scancode == SDL_SCANCODE_W)
		{
			tha_playa->up = updown;
		}
		if (keyevent->keysym.scancode == SDL_SCANCODE_S)
		{
			tha_playa->down = updown;
		}
		if (keyevent->keysym.scancode == SDL_SCANCODE_A)
		{
			tha_playa->left = updown;
		}
		if (keyevent->keysym.scancode == SDL_SCANCODE_D)
		{
			tha_playa->right = updown;
		}
	}
}

void process_input(player *tha_playa, mouse *tha_mouse, SDL_Renderer *renderer, SDL_Window *window)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				proper_shutdown(renderer, window);
				exit(0);
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					proper_shutdown(renderer, window);
					exit(0);
				}
				else
				{
					handle_key(&event.key, DOWN, tha_playa);
				}
				break;
			case SDL_KEYUP:
				handle_key(&event.key, UP, tha_playa);
				break;
			case SDL_MOUSEMOTION:
					SDL_GetMouseState(&tha_mouse->x, &tha_mouse->y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
						tha_playa->shooting = 1;
						printf("Piew piew \n");
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
						//tha_playa->shooting = 0;
						printf("Holstering...\n");
				}
				break;
			default:
				break;
		}
	}
	// NEW -- Read the mouse position here
}
