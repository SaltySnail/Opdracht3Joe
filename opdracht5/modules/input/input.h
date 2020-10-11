#ifndef INPUT
#define INPUT

void proper_shutdown(SDL_Renderer *renderer, SDL_Window *window);
void process_input(player *tha_playa, mouse *tha_mouse, SDL_Renderer *renderer, SDL_Window *window);
void handle_key(SDL_KeyboardEvent *keyevent, keystate updown,
	player *tha_playa);

#endif
