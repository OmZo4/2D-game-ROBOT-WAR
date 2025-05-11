#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

typedef struct {
    SDL_Surface *image;
} Image;

typedef struct {
    SDL_Surface *image;
    SDL_Surface *hoverImage;
    int x, y;
    int isSelected;
} Button;

typedef struct {
    SDL_Rect camera1, camera2;
    SDL_Rect posEcran1, posEcran2;
    int map_width, map_height;
} Background;

typedef struct {
    SDL_Surface *walk_right_frames[4], *walk_left_frames[4];
    SDL_Surface *fight_right_frames[4], *fight_left_frames[4];
    SDL_Surface *life_state_frames[4], *death_frame;
    SDL_Rect position;
    int lives;
    int facingRight;
    int walking, fighting;
    int walkFrame, fightFrame;
    int isAlive;
    int mode; // 0: idle, 1: chase, 2: fight
    int spawn_x, spawn_y;
    Uint32 last_damage_time;
} Enemy;

typedef struct {
    SDL_Surface *image_back, *backgroundMask, *image_minimap;
    SDL_Surface *image_bonhomme, *image_bonhomme2;
    SDL_Surface *image_boom, *image_rocket, *image_saved;
    SDL_Surface *image_gameover;
    SDL_Surface *image_countdown[4];
    SDL_Surface *walk_frames[4], *jump_frames[4], *walkl_frames[4], *jumpl_frames[4];
    SDL_Surface *fight_frames[4], *fightl_frames[4];
    SDL_Surface *walk_frames_p2[4], *jump_frames_p2[4], *walkl_frames_p2[4], *jumpl_frames_p2[4];
    SDL_Surface *fight_frames_p2[4], *fightl_frames_p2[4];
    SDL_Surface *life_icons_p1[3]; // v1.png, v2.png, v3.png for Player 1
    SDL_Surface *life_icons_p2[3]; // v1.png, v2.png, v3.png for Player 2
    SDL_Surface *digit_images[10]; // 00.png, 11.png, 22.png, 33.png, 44.png, 55.png, 66.png, 77.png, 88.png, 99.png
    SDL_Surface *colon_image; // colon.png
    SDL_Rect position_back, position_minimap, position_bonhomme;
    SDL_Rect position_minimap2, position_bonhomme2;
    SDL_Rect position_boom, position_rocket, position_saved, position_gameover;
    SDL_Rect position_countdown;
    SDL_Rect position_perso, position_perso2;
    SDL_Rect position_life_p1, position_life_p2; // Positions for life icons
    int facingRight, walking, jumping, fighting;
    int walkFrame, jumpStep, fightFrame;
    int lives_p1;
    int facingRight_p2, walking_p2, jumping_p2, fighting_p2;
    int walkFrame_p2, jumpStep_p2, fightFrame_p2;
    int lives_p2;
    float game_timer;
    int current_level;
    Background bg;
    int keysheld[322];
    int splitScreen;
    int show_saved;
    Uint32 saved_timer;
    Mix_Music *music;
    Mix_Music *gameover_music;
    int rocket_active;
    int rocket_speed_y;
    int countdown_active;
    int blinking_p1, blinking_p2;
    Uint32 blink_start_time_p1, blink_start_time_p2;
    int blink_visible_p1, blink_visible_p2;
    Uint32 last_danger_time_p1, last_danger_time_p2;
    Enemy enemy;
int skin;
} minimap;

void initBackground(Image *background, char *filename);
void initButton(Button *button, char *filename, char *hoverFilename, int x, int y);
void drawButton(SDL_Surface *screen, Button *button);
int isMouseOverButton(Button *button, int x, int y);
void blitImage(SDL_Surface *image, SDL_Surface *screen, int x, int y);
void InitialiserMinimap(minimap *m, int level, int skin);
int skinSelectionMenu(SDL_Surface *screen);
void display_countdown(minimap *m, SDL_Surface *screen);
void savePlayerPositions(minimap *m);
void loadPlayerPositions(minimap *m, int splitScreen, int skin);
void Liberer(minimap *m);
SDL_Color GetPixel(SDL_Surface *surface, int x, int y);
int check_level_transition(minimap *m, SDL_Rect pos);
int collision_danger(SDL_Surface *masque, SDL_Rect pos);
int collision_wall(SDL_Surface *masque, SDL_Rect pos);
void handleScrolling(Background *bg, SDL_Rect *playerPos, int dx, int dy, int speed, int playerNum);
int collision_boundingbox(SDL_Rect a, SDL_Rect b);
void check_collisions(minimap *m, SDL_Rect *posPerso, int *exploded_rocket, int *exploded_barrel, int playerNum);
void afficher(minimap m, SDL_Surface *screen, int exploded_barrel);
int sauvegardeMenu(SDL_Surface *screen);
int mainMenu(SDL_Surface *screen);
void sousMenuOption(SDL_Surface **screen, Mix_Music **currentMusic);
void bestScoreMenu(SDL_Surface **screen, Mix_Music **currentMusic);
void storyMenu(SDL_Surface **screen, Mix_Music **currentMusic);
int modeSelectionMenu(SDL_Surface *screen);

#endif
