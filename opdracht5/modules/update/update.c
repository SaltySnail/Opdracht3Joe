#include <math.h>
#include <SDL2/SDL.h>
#include "../entities/entity.h"
#include "../blit/blit.h"

#define PI							3.14159265358979323846
// Move this much pixels every frame a move is detected:
#define PLAYER_MAX_SPEED			6.0f
// AFTER a movement-key is released, reduce the movement speed for
// every consecutive frame by (1.0 - this amount):
#define PLAYER_DECELERATION			0.9f
#define BULLET_LIFESPAN				300
#define MUZZLE_VEL				20

//static SDL_Window *window = NULL;
//static SDL_Renderer *renderer = NULL;

float get_angle(int x1, int y1, int x2, int y2)
{
	float radius_schouder = 310.0;
	return (float)(360 + atan2(y2 - y1, x2 - x1) * (180.0 / PI) - (float)asin((radius_schouder / sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)))*PI/180) * PI*180);
}

void process_keugel(player *player, keugel *keugel, float angle, mouse *mouse, flash *flash)
{
	int radius = 140;
	if (player->shooting)
	{
		keugel->life = BULLET_LIFESPAN;
		keugel->speed_x = MUZZLE_VEL*(float)(cos(angle*PI/180));
		keugel->speed_y = MUZZLE_VEL*(float)(sin(angle*PI/180));
		angle += 23;
		keugel->x = (int)(player->x + radius * cos(angle*PI/180));
		keugel->y = (int)(player->y + radius * sin(angle*PI/180));
		keugel->angle = (float)(360 + atan2(mouse->y - keugel->y, mouse->x - keugel->x) * (180/PI)); //blorp is magic, blorp can shoot tha bullets backwards
		if (player->state == IDLE)
		{
			player->state = SHOOTING;
		}
		if (player->state == WALKING)
		{
			player->state = SHOOTING_WHILE_WALKING;
		}
		printf("graden: %.2f \n", keugel->angle);
		flash->x = keugel->x;
		flash->y = keugel->y;
		flash->angle = keugel->angle;
		flash->a = 255;	
	}
	if (keugel->life)
	{
		keugel->x += (int)keugel->speed_x;
		keugel->y += (int)keugel->speed_y;	
	}
	if (keugel->life == 0)
	{
		player->state = IDLE;
	}
		//printf("keugel bevindt zich op \n x: %d \n y: %d \n De snelheid is: \n x: %.2f \n y: %.2f \n", keugel->x, keugel->y, keugel->speed_x, keugel->speed_y);
}


void update_player(player *tha_playa, mouse *tha_mouse, keugel *keugel, flash *flash)
{
	if (tha_playa->up)
	{
		tha_playa->y -= (int)PLAYER_MAX_SPEED;
		if (keugel->life >= (BULLET_LIFESPAN - 3))
		{
			flash->y -= (int)PLAYER_MAX_SPEED;	
		}
		//keugel->y += (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->down)
	{
		tha_playa->y += (int)PLAYER_MAX_SPEED;
		if (keugel->life >= (BULLET_LIFESPAN - 3))
		{
			flash->y += (int)PLAYER_MAX_SPEED;
		}
		//keugel->y -= (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->left)
	{
		tha_playa->x -= (int)PLAYER_MAX_SPEED;
		if (keugel->life >= (BULLET_LIFESPAN - 3))
		{
			flash->x -= (int)PLAYER_MAX_SPEED;
		}
		//keugel->x += (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->right)
	{
		tha_playa->x += (int)PLAYER_MAX_SPEED;
		if (keugel->life >= (BULLET_LIFESPAN - 3))
		{
			flash->x += (int)PLAYER_MAX_SPEED;
		}
		//keugel->x -= (int)PLAYER_MAX_SPEED;
	}
	tha_playa->angle = get_angle(tha_playa->x, tha_playa->y,
		tha_mouse->x, tha_mouse->y);
	process_keugel(tha_playa, keugel, tha_playa->angle, tha_mouse, flash);
}


void draw_flash(flash *flash, SDL_Renderer *renderer)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_SetTextureBlendMode(flash->txtr_flash, SDL_BLENDMODE_ADD);
	SDL_SetTextureColorMod(flash->txtr_flash, flash->r, flash->g, flash->b);
	SDL_SetTextureAlphaMod(flash->txtr_flash, flash->a);
	blit_angled(flash->txtr_flash, flash->x, flash->y, flash->angle, -5, renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void get_angle_enemy(player *player, enemy *enemy)
{
	enemy->angle = (float)(360 + atan2(player->y - enemy->y, player->x - enemy->x) * (180.0 / PI));
}

