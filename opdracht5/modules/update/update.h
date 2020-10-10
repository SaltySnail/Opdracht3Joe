#ifndef UPDATE
#define UPDATE

void draw_flash(flash *flash, SDL_Renderer *renderer);
void process_keugel(player *player, keugel *keugel, float angle, mouse *mouse, flash *flash);
void update_player(player *tha_playa, mouse *tha_mouse, keugel *keugel, flash *flash);
float get_angle(int x1, int y1, int x2, int y2);
void get_angle_enemy(player *player, enemy *enemy);

#endif
