#ifndef TEXTURES
#define TEXTURES

//#include "../entities/entity.h"

void init_all_textures(flash *flash, keugel *keugel, mouse *mousepointer, enemy *enemy, player *blorp, SDL_Renderer *renderer);
void init_textures(char *path, char *file_prefix, int num_txtrs, SDL_Texture **txtr_set, SDL_Renderer *renderer);
SDL_Texture *load_texture(char *filename, SDL_Renderer *renderer);

#endif
