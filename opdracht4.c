/*
Copyright (C) 2020
Sander Gieling
Inholland University of Applied Sciences at Alkmaar, the Netherlands
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// Artwork bibliography
// --------------------
// Reticle taken from here:
// https://flyclipart.com/download-png#printable-crosshair-targets-clip-art-clipart-675613.png
// Blorp taken from here:
// https://opengameart.org/content/animated-top-down-survivor-player
// Desert floor tiling taken from here:
// https://www.flickr.com/photos/maleny_steve/8899498324/in/photostream/

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // for IMG_Init and IMG_LoadTexture
#include <math.h> // for atan() function

#define SCREEN_WIDTH				1920
#define SCREEN_HEIGHT				1080
#define PI							3.14159265358979323846
// Move this much pixels every frame a move is detected:
#define PLAYER_MAX_SPEED			6.0f
// AFTER a movement-key is released, reduce the movement speed for
// every consecutive frame by (1.0 - this amount):
#define PLAYER_DECELERATION			0.9f
#define BULLET_LIFESPAN				200
#define MUZZLE_VEL				20

// A mouse structure holds mousepointer coords & a pointer texture:
typedef struct _mouse_ {
	int x;
	int y;
	SDL_Texture *txtr_reticle;
} mouse;

typedef enum _playerstate_ {
	IDLE = 0,
	WALKING = 1,
	SHOOTING = 2,
	SHOOTING_WHILE_WALKING = 3,
	MAX_STATE = 4
}playerstate;

typedef enum _enemystate_ {
	IDLE_E = 0,
	WALKING_E = 1,
	ATTACKING_E = 2,
	MAX_STATE_E = 3
}enemystate;

// Added: speed in both directions and rotation angle:
typedef struct _player_ {
	int x;
	int y;
	float speed_x;
	float speed_y;
	int up;
	int down;
	int left;
	int right;
	float angle;
	int shooting;
	playerstate state;
	SDL_Texture *txtr_body[MAX_STATE][100];
	SDL_Texture *txtr_feet[MAX_STATE][100]; //eerst werkte dit met 20, toen opeens segmentation fault
} player;

typedef struct _enemy_ {
	int x;
	int y;
	float angle;
	SDL_Texture *txtr_body[MAX_STATE_E][100];
	enemystate state;

}enemy;

typedef struct _keugel_ {
		int x;
		int y;
		float speed_x;
		float speed_y;
		float angle;
		int life;
		SDL_Texture *txtr_kogel;	
}keugel;

typedef struct _flash_ {
		int x;
		int y;
		float angle;
		SDL_Texture *txtr_flash;
		Uint8 r;
		Uint8 g;
		Uint8 b;
		Uint8 a;
}flash;

typedef enum _keystate_ {
	UP = 0,
	DOWN = 1
} keystate;

void get_angle_enemy(player *player, enemy *enemy);
void draw_flash(flash *flash);
void process_keugel(player *player, keugel *keugel, float angle, mouse *mouse, flash *flash);
void init_textures(char *path, char *file_prefix, int num_txtrs, SDL_Texture **txtr_set);
void handle_key(SDL_KeyboardEvent *keyevent, keystate updown,
	player *tha_playa);
// This function has changed because mouse movement was added:
void process_input(player *tha_playa, mouse *tha_mouse);
// This function has changed because mouse movement was added:
void update_player(player *tha_playa, mouse *tha_mouse, keugel *keugel, flash *flash);
void proper_shutdown(void);
SDL_Texture *load_texture(char *filename);
// This function has changed because texture rotation was added,
// which means drawing a texture centered on a coordinate is easier:
void blit(SDL_Texture *texture, int x, int y, int center);
// These two functions are new since sdl2a.c:
void blit_angled(SDL_Texture *txtr, int x, int y, float angle, int zoom);
float get_angle(int x1, int y1, int x2, int y2);

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	float prev_enemy_angle;	
	flash flash;
	keugel keugel;
	player blorp;
	enemy enemy;
	enemy.x = SCREEN_HEIGHT/3;
	enemy.y = SCREEN_WIDTH/3;
	blorp.x = SCREEN_HEIGHT / 2;
	blorp.y = SCREEN_WIDTH / 2;
	blorp.speed_x = 0.0;
	blorp.speed_y = 0.0;
	blorp.up = UP;
	blorp.down = UP;
	blorp.left = UP;
	blorp.right = UP;
	blorp.angle = 0.0;
	blorp.state = IDLE;
	blorp.shooting = 0;
	flash.r = 255;
	flash.g = 255;
	flash.b = 255;
	flash.a = 255;
	//player blorp = {(SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), 0.0f, 0.0f, UP, UP, UP, UP, 0.0, NULL, NULL, IDLE};
	// New: Mouse is a type representing a struct containing x and y
	// coords of mouse pointer:
	mouse mousepointer;
	int frame, enemy_frame;
	unsigned int window_flags = 0;
	unsigned int renderer_flags = SDL_RENDERER_ACCELERATED;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	window = SDL_CreateWindow("Blorp is going to F U UP!",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
		SCREEN_HEIGHT, window_flags);
	if (window == NULL)
	{
		printf("Failed to create window -- Error: %s\n",
			SDL_GetError());
		exit(1);
	}
	renderer = SDL_CreateRenderer(window, -1, renderer_flags);
	if (renderer == NULL)
	{
		printf("Failed to create renderer -- Error: %s\n",
			SDL_GetError());
		exit(1);
	}
	IMG_Init(IMG_INIT_PNG);
	//load all the textures
	char path_feet[41], path_body[41], prefix_body[24], prefix_feet[24]; //31+10 & survivor-shoot_handgun_\0 = 24 chars
	for (enemystate state = 0; state < MAX_STATE_E; state++)
	{
		if (state == IDLE_E)
		{
			strcpy(path_body, "gfx/tds_zombie/idle/");
	    		strcpy(prefix_body, "skeleton-idle_");
	    		//printf("%s \n %s \n", path_body, prefix_body);
	    		init_textures(path_body, prefix_body, 17, enemy.txtr_body[state]);
		}
		if (state == WALKING_E)
		{
			strcpy(path_body, "gfx/tds_zombie/walking/");
	    		strcpy(prefix_body, "skeleton-move_");
	    		//printf("%s \n %s \n", path_body, prefix_body);
	    		init_textures(path_body, prefix_body, 17, enemy.txtr_body[state]);

		}
		if (state == ATTACKING_E)
		{
			strcpy(path_body, "gfx/tds_zombie/attack/");
	    		strcpy(prefix_body, "skeleton-attack_");
	    		//printf("%s \n %s \n", path_body, prefix_body);
	    		init_textures(path_body, prefix_body, 9, enemy.txtr_body[state]);

		}
	}

	for (playerstate state = 0; state < MAX_STATE; state++)
	{
		if (state == 0)
		{

			strcpy(path_body, "gfx/idlebody/");
			strcpy(path_feet, "gfx/idlefeet/");
			strcpy(prefix_body, "survivor-idle_handgun_");
			strcpy(prefix_feet, "survivor-idle_");
			//printf("%s \n %s \n", path_feet, path_body);
			init_textures(path_body, prefix_body, 20, blorp.txtr_body[state]);
			init_textures(path_feet, prefix_feet, 1, blorp.txtr_feet[state]);
		}
		if (state == 1)
		{
			strcpy(path_body, "gfx/movebody/");
			strcpy(path_feet, "gfx/walkfeet/");
			strcpy(prefix_body, "survivor-move_handgun_");
			strcpy(prefix_feet, "survivor-walk_");
			//printf("%s \n %s \n", path_feet, path_body);
			init_textures(path_body, prefix_body, 20, blorp.txtr_body[state]);
			init_textures(path_feet, prefix_feet, 20, blorp.txtr_feet[state]);

		}
		if (state == 2)
		{
			strcpy(path_body, "gfx/shoot/");
			strcpy(path_feet, "gfx/idlefeet/");
			strcpy(prefix_body, "survivor-shoot_handgun_");
			strcpy(prefix_feet, "survivor-idle_");
			//printf("%s \n %s \n", path_feet, path_body);
			init_textures(path_body, prefix_body, 3, blorp.txtr_body[state]);
			init_textures(path_feet, prefix_feet, 20, blorp.txtr_feet[state]);

		}
		if (state == 3)
		{
			strcpy(path_body, "gfx/shoot/");
			strcpy(path_feet, "gfx/walkfeet/");
			strcpy(prefix_body, "survivor-shoot_handgun_");
			strcpy(prefix_feet, "survivor-walk_");
			//printf("%s \n %s \n", path_feet, path_body);
			init_textures(path_body, prefix_body, 3, blorp.txtr_body[state]);
			init_textures(path_feet, prefix_feet, 20, blorp.txtr_feet[state]);

		}


	}

	//blorp.txtr_player = load_texture("gfx/blorp.png");
	// New: Load mousepointer texture:
	mousepointer.txtr_reticle = load_texture("gfx/reticle.png");
	keugel.txtr_kogel = load_texture("gfx/bullet20x5.png");
	flash.txtr_flash = load_texture("gfx/muzzle_flashs/m_2.png");

	// New: Turn system mouse cursor off:
	SDL_ShowCursor(0);

	while (1)
	{
		SDL_SetRenderDrawColor(renderer, 120, 144, 156, 255);
		SDL_RenderClear(renderer);

		// # Sensor Reading #
		// Also takes the mouse movement into account:
		process_input(&blorp, &mousepointer);

		// # Applying Game Logic #
		// Also takes the mouse movement into account:
		update_player(&blorp, &mousepointer, &keugel, &flash);
		prev_enemy_angle = enemy.angle;
		get_angle_enemy(&blorp, &enemy);
		if (prev_enemy_angle != enemy.angle)
		{
			enemy.state = WALKING_E;
		}
		else
		{
			enemy.state = IDLE_E;
		}
		if (enemy.state)
		{
			if (prev_enemy_angle < enemy.angle)
			{
				blit_angled(enemy.txtr_body[enemy.state][enemy_frame/3], enemy.x, enemy.y, enemy.angle, 1);
			}
			if (prev_enemy_angle > enemy.angle)
			{
				blit_angled(enemy.txtr_body[enemy.state][17-(enemy_frame/3)], enemy.x, enemy.y, enemy.angle, 1);
			}
		}
		else
		{		
				blit_angled(enemy.txtr_body[enemy.state][enemy_frame/3], enemy.x, enemy.y, enemy.angle, 1);
		}
		if (blorp.up == DOWN || blorp.down == DOWN || blorp.left == DOWN || blorp.right == DOWN)
		{
			if (blorp.state == SHOOTING)
			{
				blorp.state = SHOOTING_WHILE_WALKING;
			}
			else
			{
				blorp.state = WALKING;
				//enemy.state = WALKING_E;
			}
		}	
		if (blorp.up == UP && blorp.down == UP && blorp.left == UP && blorp.right == UP)
		{
			if (blorp.state == SHOOTING)
			{
				blorp.state = SHOOTING;
			}
			else
			{
				blorp.state = IDLE;
				//enemy.state = IDLE_E;
			}
		}
		// # Actuator Output Buffering #
		// Also takes texture rotation into account:-
		
		if (blorp.state) 
		{
			blit_angled(blorp.txtr_feet[blorp.state][frame], blorp.x, blorp.y, blorp.angle, 1);
		} 
		else 
		{
			blit_angled(blorp.txtr_feet[0][0], blorp.x, blorp.y, blorp.angle, 1);
		}
		if (blorp.state == SHOOTING || blorp.state == SHOOTING_WHILE_WALKING)
		{
			if (frame < 15)
			{
				//printf("Gucci fam, frame = %d\n", frame);
				blit_angled(blorp.txtr_body[blorp.state][frame/5], blorp.x, blorp.y, blorp.angle, 1);
				flash.a = (Uint8)((255/15)*(16-frame));
				draw_flash(&flash);
			}
			else
			{
				frame = 18;
				if (blorp.state == SHOOTING)
				{
					blorp.state = IDLE;
				}
				else
				{
					blorp.state = WALKING;
				}
			}
		}
		else
		{	
			blit_angled(blorp.txtr_body[blorp.state][frame], blorp.x, blorp.y, blorp.angle, 1);
		}
		if (keugel.life)
		{
			blit_angled(keugel.txtr_kogel, keugel.x, keugel.y, keugel.angle, 1);
			--keugel.life;
		}
		// New: Redraw mouse pointer centered on the mouse coordinates:
		blit(mousepointer.txtr_reticle, mousepointer.x,
			mousepointer.y, 1);	
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
		enemy_frame++;
		++frame;
		if (enemy_frame == 48)
		{
			enemy_frame = 0;
		}	
		if (frame == 19)
		{
			frame = 0;
			if (blorp.state == SHOOTING)
			{
				blorp.state = IDLE;
			}
			if (blorp.state == SHOOTING_WHILE_WALKING)
			{
				blorp.state = WALKING;
			}
		}

	}

	return 0;
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

void process_input(player *tha_playa, mouse *tha_mouse)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				proper_shutdown();
				exit(0);
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					proper_shutdown();
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
						tha_playa->shooting = 0;
						printf("Holstering...\n");
				}
				break;
			default:
				break;
		}
	}
	// NEW -- Read the mouse position here
}

void update_player(player *tha_playa, mouse *tha_mouse, keugel *keugel, flash *flash)
{
	process_keugel(tha_playa, keugel, tha_playa->angle, tha_mouse, flash);
	if (tha_playa->up)
	{
		tha_playa->y -= (int)PLAYER_MAX_SPEED;
		//keugel->y += (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->down)
	{
		tha_playa->y += (int)PLAYER_MAX_SPEED;
		//keugel->y -= (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->left)
	{
		tha_playa->x -= (int)PLAYER_MAX_SPEED;
		//keugel->x += (int)PLAYER_MAX_SPEED;
	}
	if (tha_playa->right)
	{
		tha_playa->x += (int)PLAYER_MAX_SPEED;
		//keugel->x -= (int)PLAYER_MAX_SPEED;
	}
	tha_playa->angle = get_angle(tha_playa->x, tha_playa->y,
		tha_mouse->x, tha_mouse->y);
}

void proper_shutdown(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Texture *load_texture(char *filename)
{
	//IMG_Init(filename);
	SDL_Texture *txtr;
	txtr = IMG_LoadTexture(renderer, filename);
	return txtr;
}

void blit(SDL_Texture *txtr, int x, int y, int center)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	SDL_QueryTexture(txtr, NULL, NULL, &dest.w, &dest.h);
	// If center != 0, render texture with its center on (x,y), NOT
	// with its top-left corner...
	if (center)
	{
		dest.x -= dest.w / 2;
		dest.y -= dest.h / 2;
	}
	SDL_RenderCopy(renderer, txtr, NULL, &dest);
}

void blit_angled(SDL_Texture *txtr, int x, int y, float angle, int zoom)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	SDL_QueryTexture(txtr, NULL, NULL, &dest.w, &dest.h);
	// Textures that are rotated MUST ALWAYS be rendered with their
	// center at (x, y) to have a symmetrical center of rotation:
	if (zoom > 0)
	{
		dest.w *= zoom;
		dest.h *= zoom;
	}
	if (zoom < 0)
	{
		dest.w /= -zoom;
		dest.h /= -zoom;	
	}
	dest.x -= dest.w / 2;
	dest.y -= dest.h / 2;
	// Look up what this function does. What do these rectangles
	// mean? Why is the source rectangle NULL? What are acceptable
	// values for the `angle' parameter?
	SDL_RenderCopyEx(renderer, txtr, NULL, &dest, angle, NULL,
		SDL_FLIP_NONE);
}

float get_angle(int x1, int y1, int x2, int y2)
{
	float radius_schouder = 310.0/*, print_lange, print_overstaande, print_aansluitende*/;
	//print_lange = (float)sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	//print_overstaande = (float)sqrt(pow(y1 - y2, 2));
	//print_aansluitende = (float)sqrt(pow(x1 - x2, 2));
	//printf("Lange: %.2f \n overstaande: %.2f \n aansluitende: %.2f \n", print_lange, print_overstaande, print_aansluitende);
	// atan2(): a function from math.h, representing the tangent angle:
	// https://en.wikipedia.org/wiki/Atan2
	// where x1,y1 are player coords and x2,y2 are mouse coords.
	// The origin in the wikipedia picture thus represents the player,
	// the (x,y)-pair the mouse coords.
	// The (180.0 / PI) represents 1 radian in degrees, i.e. we're
	// converting from radians (the output of atan2()) to degrees.
	return (float)(360 + atan2(y2 - y1, x2 - x1) * (180.0 / PI) - (float)asin((radius_schouder / sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)))*PI/180) * PI*180);
	// ATAN2 - SIDENOTE:
	// "The atan2() functiona returns a value in the range
	//  -π to π radians"
	// If first arg (delta-y) is POSTIVE, you get a POSITIVE angle
	// between +0.0000...1 and +180 degrees
	// If first arg (delta-y) is NEGATIVE, you get a NEGATIVE angle
	// between 0 and -179.9999...9 degrees
}

void init_textures(char *path, char *file_prefix, int num_txtrs, SDL_Texture **txtr_set)
{
	char filename[128];
	for (int i = 0; i < num_txtrs; i++)
	{
		sprintf(filename, "%s%s%d.png", path, file_prefix, i);
		printf("full file location: %s \n", filename);
		txtr_set[i] = load_texture(filename);
	}
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
		player->shooting = 0;
		player->state = SHOOTING;
		printf("graden: %.2f \n", keugel->angle);
		flash->x = keugel->x;
		flash->y = keugel->y;
		flash->angle = keugel->angle;
	}
	if (keugel->life)
	{
		keugel->x += (int)keugel->speed_x;
		keugel->y += (int)keugel->speed_y;	
	}
//	else
//	{
//		player->state = IDLE;	
	//}
		//printf("keugel bevindt zich op \n x: %d \n y: %d \n De snelheid is: \n x: %.2f \n y: %.2f \n", keugel->x, keugel->y, keugel->speed_x, keugel->speed_y);
}

void draw_flash(flash *flash)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_SetTextureBlendMode(flash->txtr_flash, SDL_BLENDMODE_ADD);
	SDL_SetTextureColorMod(flash->txtr_flash, flash->r, flash->g, flash->b);
	SDL_SetTextureAlphaMod(flash->txtr_flash, flash->a);
	blit_angled(flash->txtr_flash, flash->x, flash->y, flash->angle, -5);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void get_angle_enemy(player *player, enemy *enemy)
{
	enemy->angle = (float)(360 + atan2(player->y - enemy->y, player->x - enemy->x) * (180.0 / PI));
}
