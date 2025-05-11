#include "header.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initBackground(Image *background, char *filename) {
    background->image = IMG_Load(filename);
    if (!background->image) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
    }
}

void initButton(Button *button, char *filename, char *hoverFilename, int x, int y) {
    button->image = IMG_Load(filename);
    if (!button->image) {
        printf("Erreur de chargement de l'image du bouton : %s\n", IMG_GetError());
    }
    button->hoverImage = IMG_Load(hoverFilename);
    if (!button->hoverImage) {
        printf("Erreur de chargement de l'image de survol du bouton : %s\n", IMG_GetError());
    }
    button->x = x;
    button->y = y;
    button->isSelected = 0;
}

void drawButton(SDL_Surface *screen, Button *button) {
    SDL_Rect dest = {button->x, button->y, 0, 0};
    if (button->isSelected && button->hoverImage) {
        SDL_BlitSurface(button->hoverImage, NULL, screen, &dest);
    } else if (button->image) {
        SDL_BlitSurface(button->image, NULL, screen, &dest);
    }
}

int isMouseOverButton(Button *button, int x, int y) {
    if (!button->image) return 0;
    return x >= button->x && x <= button->x + button->image->w &&
           y >= button->y && y <= button->y + button->image->h;
}

void playSound(char *filename) {
    Mix_Chunk *sound = Mix_LoadWAV(filename);
    if (!sound) {
        printf("Erreur de chargement du son : %s\n", Mix_GetError());
        return;
    }
    Mix_PlayChannel(-1, sound, 0);
    Mix_FreeChunk(sound);
}

void playMusic(char *filename, Mix_Music **currentMusic) {
    if (*currentMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(*currentMusic);
        *currentMusic = NULL;
    }
    *currentMusic = Mix_LoadMUS(filename);
    if (!*currentMusic) {
        printf("Erreur de chargement de la musique : %s\n", Mix_GetError());
        return;
    }
    Mix_PlayMusic(*currentMusic, -1);
}

void drawVolumeBar(SDL_Surface *screen, int volume) {
    SDL_Rect bar = {350, 200, volume * 2, 20};
    SDL_FillRect(screen, &bar, SDL_MapRGB(screen->format, 0, 255, 0));
}

void blitImage(SDL_Surface *image, SDL_Surface *screen, int x, int y) {
    if (image) {
        SDL_Rect dest = {x, y, 0, 0};
        SDL_BlitSurface(image, NULL, screen, &dest);
    }
}

void sousMenuOption(SDL_Surface **screen, Mix_Music **currentMusic) {
    Image background;
    initBackground(&background, "submenu.png");
    Button buttons[5];
    initButton(&buttons[0], "decrease.png", "decrease_hover.png", 150, 200);
    initButton(&buttons[1], "increase.png", "increase_hover.png", 550, 200);
    initButton(&buttons[2], "new/ffullscreen.png", "new/ffullscreen_hover.png", 150, 350);	//ici
    initButton(&buttons[3], "new/fullscreen.png", "new/fullscreen_hover.png", 550, 350);
    initButton(&buttons[4], "new/retour.png", "new/retour_hover.png", 350, 500);

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0};
    SDL_Surface *textVolume = TTF_RenderText_Solid(font, "VOLUME :", textColor);
    SDL_Surface *textDisplayMode = TTF_RenderText_Solid(font, "MODE D'AFFICHAGE :", textColor);

    int volume = 50, running = 1, isFullscreen = 0;
    SDL_Event event;
    int wasMouseOver[5] = {0};

    playMusic("music.mp3", currentMusic);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_PLUS && volume < 100) {
                    volume += 10;
                    Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
                    playSound("beep.wav");
                } else if (event.key.keysym.sym == SDLK_MINUS && volume > 0) {
                    volume -= 10;
                    Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
                    playSound("beep.wav");
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                for (int i = 0; i < 5; i++) {
                    buttons[i].isSelected = isMouseOverButton(&buttons[i], event.motion.x, event.motion.y);
                    if (buttons[i].isSelected && !wasMouseOver[i]) {
                        playSound("beep.wav");
                        wasMouseOver[i] = 1;
                    } else if (!buttons[i].isSelected) {
                        wasMouseOver[i] = 0;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (isMouseOverButton(&buttons[0], event.button.x, event.button.y) && volume > 0) {
                    volume -= 10;
                    Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
                    playSound("beep.wav");
                } else if (isMouseOverButton(&buttons[1], event.button.x, event.button.y) && volume < 100) {
                    volume += 10;
                    Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
                    playSound("beep.wav");
                } else if (isMouseOverButton(&buttons[2], event.button.x, event.button.y) && isFullscreen) {
                    SDL_Surface *newScreen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
                    if (newScreen) {
                        SDL_FreeSurface(*screen);
                        *screen = newScreen;
                        isFullscreen = 0;
                        playSound("beep.wav");
                    } else {
                        printf("Failed to switch to normal mode: %s\n", SDL_GetError());
                    }
                } else if (isMouseOverButton(&buttons[3], event.button.x, event.button.y) && !isFullscreen) {
                    SDL_Surface *newScreen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
                    if (newScreen) {
                        SDL_FreeSurface(*screen);
                        *screen = newScreen;
                        isFullscreen = 1;
                        playSound("beep.wav");
                    } else {
                        printf("Failed to switch to fullscreen: %s\n", SDL_GetError());
                    }
                } else if (isMouseOverButton(&buttons[4], event.button.x, event.button.y)) {
                    running = 0;
                }
            }
        }

        SDL_FillRect(*screen, NULL, SDL_MapRGB((*screen)->format, 0, 0, 0));
        blitImage(background.image, *screen, 0, 0);
        drawButton(*screen, &buttons[0]);
        drawButton(*screen, &buttons[1]);
        drawButton(*screen, &buttons[2]);
        drawButton(*screen, &buttons[3]);
        drawButton(*screen, &buttons[4]);
        blitImage(textVolume, *screen, 100, 150);
        blitImage(textDisplayMode, *screen, 100, 300);
        drawVolumeBar(*screen, volume);
        SDL_Flip(*screen);
    }

    SDL_FreeSurface(background.image);
    for (int i = 0; i < 5; i++) {
        SDL_FreeSurface(buttons[i].image);
        SDL_FreeSurface(buttons[i].hoverImage);
    }
    SDL_FreeSurface(textVolume);
    SDL_FreeSurface(textDisplayMode);
    TTF_CloseFont(font);
    Mix_HaltMusic();
}

void InitialiserMinimap(minimap *m, int level, int skin) {
    srand(time(NULL));
    memset(m, 0, sizeof(minimap));
    m->saved_timer = 0;
    m->current_level = level;
    m->last_danger_time_p1 = 0;
    m->last_danger_time_p2 = 0;
    m->blinking_p1 = 0;
    m->blinking_p2 = 0;
    m->blink_visible_p1 = 1;
    m->blink_visible_p2 = 1;
    m->skin = skin; // Store selected skin

    // Load digit images for timer
    char *digit_filenames[10] = {
        "00.png", "11.png", "22.png", "33.png", "44.png",
        "55.png", "66.png", "77.png", "88.png", "99.png"
    };
    for (int i = 0; i < 10; i++) {
        m->digit_images[i] = IMG_Load(digit_filenames[i]);
        if (!m->digit_images[i]) {
            printf("Warning: Failed to load %s: %s\n", digit_filenames[i], IMG_GetError());
        }
    }

    // Load colon image
    m->colon_image = IMG_Load("colon.png");
    if (!m->colon_image) {
        printf("Warning: Failed to load colon.png: %s\n", IMG_GetError());
    }

    if (level == 1) {
        m->image_back = IMG_Load("background.jpg");
        m->backgroundMask = SDL_LoadBMP("backgroundMasque.bmp");
        m->image_minimap = IMG_Load("map.jpg");
    } else {
        m->image_back = IMG_Load("background2.jpg");
        m->backgroundMask = SDL_LoadBMP("backgroundMasque2.bmp");
        m->image_minimap = IMG_Load("map2.jpg");
    }

    if (!m->image_back || !m->backgroundMask || !m->image_minimap) {
        printf("Critical asset load failed for level %d:\n", level);
        if (!m->image_back) printf("Failed to load background: %s\n", IMG_GetError());
        if (!m->backgroundMask) printf("Failed to load background mask: %s\n", SDL_GetError());
        if (!m->image_minimap) printf("Failed to load minimap: %s\n", IMG_GetError());
        exit(1);
    }

    m->position_back.x = 0;
    m->position_back.y = 0;

    m->image_bonhomme = IMG_Load("dot.png");
    if (!m->image_bonhomme) {
        printf("Error: Failed to load dot.png: %s\n", IMG_GetError());
        exit(1);
    }

    m->image_bonhomme2 = IMG_Load("dot.png");
    if (!m->image_bonhomme2) {
        printf("Error: Failed to load dot.png for Player 2: %s\n", IMG_GetError());
        exit(1);
    }

    m->image_boom = IMG_Load("boom.png");
    m->image_rocket = IMG_Load("rocket.png");
    m->image_saved = IMG_Load("saved.png");
    m->image_gameover = IMG_Load("gameover.jpg");
    m->music = Mix_LoadMUS("music.mp3");
    m->gameover_music = Mix_LoadMUS("gameover.mp3");

    m->position_boom.x = 1400;
    m->position_boom.y = 550;
    m->position_rocket.x = rand() % (1920 - (m->image_rocket ? m->image_rocket->w : 0));
    m->position_rocket.y = 0;
    m->rocket_active = 1;
    m->rocket_speed_y = 15;
    m->position_saved.x = 1920 - (m->image_saved ? m->image_saved->w : 0);
    m->position_saved.y = 1080 - (m->image_saved ? m->image_saved->h : 0);
    m->position_gameover.x = (1920 - (m->image_gameover ? m->image_gameover->w : 0)) / 2;
    m->position_gameover.y = (1080 - (m->image_gameover ? m->image_gameover->h : 0)) / 2;

    m->position_minimap.x = 20;
    m->position_minimap.y = 20;
    m->position_minimap2.x = 1920 / 2 + 20;
    m->position_minimap2.y = 20;

    m->position_bonhomme.x = m->position_minimap.x;
    m->position_bonhomme.y = m->position_minimap.y;
    m->position_bonhomme2.x = m->position_minimap2.x;
    m->position_bonhomme2.y = m->position_minimap2.y;

    // Load animation frames based on selected skin
    char *suffix = (skin == 1) ? "_p2" : "";
    char filename[32];

    // Player 1 animations
    for (int i = 0; i < 4; i++) {
        sprintf(filename, "walk%d%s.png", i+1, suffix);
        m->walk_frames[i] = IMG_Load(filename);
        if (!m->walk_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "jump%d%s.png", i+1, suffix);
        m->jump_frames[i] = IMG_Load(filename);
        if (!m->jump_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "walkl%d%s.png", i+1, suffix);
        m->walkl_frames[i] = IMG_Load(filename);
        if (!m->walkl_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "jumpl%d%s.png", i+1, suffix);
        m->jumpl_frames[i] = IMG_Load(filename);
        if (!m->jumpl_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "fight%d%s.png", i+1, suffix);
        m->fight_frames[i] = IMG_Load(filename);
        if (!m->fight_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "fightl%d%s.png", i+1, suffix);
        m->fightl_frames[i] = IMG_Load(filename);
        if (!m->fightl_frames[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());
    }

    // Player 2 animations (use same skin as Player 1)
    for (int i = 0; i < 4; i++) {
        sprintf(filename, "walk%d%s.png", i+1, suffix);
        m->walk_frames_p2[i] = IMG_Load(filename);
        if (!m->walk_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "jump%d%s.png", i+1, suffix);
        m->jump_frames_p2[i] = IMG_Load(filename);
        if (!m->jump_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "walkl%d%s.png", i+1, suffix);
        m->walkl_frames_p2[i] = IMG_Load(filename);
        if (!m->walkl_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "jumpl%d%s.png", i+1, suffix);
        m->jumpl_frames_p2[i] = IMG_Load(filename);
        if (!m->jumpl_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "fight%d%s.png", i+1, suffix);
        m->fight_frames_p2[i] = IMG_Load(filename);
        if (!m->fight_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());

        sprintf(filename, "fightl%d%s.png", i+1, suffix);
        m->fightl_frames_p2[i] = IMG_Load(filename);
        if (!m->fightl_frames_p2[i]) printf("Warning: Failed to load %s: %s\n", filename, IMG_GetError());
    }

    // Load life icons for Player 1
    m->life_icons_p1[0] = IMG_Load("v1.png"); // 3 lives
    m->life_icons_p1[1] = IMG_Load("v2.png"); // 2 lives
    m->life_icons_p1[2] = IMG_Load("v3.png"); // 1 life
    for (int i = 0; i < 3; i++) {
        if (!m->life_icons_p1[i]) {
            printf("Warning: Failed to load v%d.png for Player 1: %s\n", i+1, IMG_GetError());
        }
    }

    // Load life icons for Player 2
    m->life_icons_p2[0] = IMG_Load("v1.png"); // 3 lives
    m->life_icons_p2[1] = IMG_Load("v2.png"); // 2 lives
    m->life_icons_p2[2] = IMG_Load("v3.png"); // 1 life
    for (int i = 0; i < 3; i++) {
        if (!m->life_icons_p2[i]) {
            printf("Warning: Failed to load v%d.png for Player 2: %s\n", i+1, IMG_GetError());
        }
    }

    // Set positions for life icons
    m->position_life_p1.x = 10;   // Top-left corner for Player 1
    m->position_life_p1.y = 170;
    m->position_life_p2.x = 1000; // Adjust for Player 2 (split-screen or single-screen)
    m->position_life_p2.y = 170;

    // Initialize enemy for level 1 only
    if (level == 1) {
        m->enemy.walk_right_frames[0] = IMG_Load("marche1/1.png");
        m->enemy.walk_right_frames[1] = IMG_Load("marche1/2.png");
        m->enemy.walk_right_frames[2] = IMG_Load("marche1/3.png");
        m->enemy.walk_right_frames[3] = IMG_Load("marche1/4.png");
        m->enemy.walk_left_frames[0] = IMG_Load("marche/1.png");
        m->enemy.walk_left_frames[1] = IMG_Load("marche/2.png");
        m->enemy.walk_left_frames[2] = IMG_Load("marche/3.png");
        m->enemy.walk_left_frames[3] = IMG_Load("marche/4.png");
        m->enemy.fight_right_frames[0] = IMG_Load("fight1/1.png");
        m->enemy.fight_right_frames[1] = IMG_Load("fight1/2.png");
        m->enemy.fight_right_frames[2] = IMG_Load("fight1/3.png");
        m->enemy.fight_right_frames[3] = IMG_Load("fight1/4.png");
        m->enemy.fight_left_frames[0] = IMG_Load("fight/1.png");
        m->enemy.fight_left_frames[1] = IMG_Load("fight/2.png");
        m->enemy.fight_left_frames[2] = IMG_Load("fight/3.png");
        m->enemy.fight_left_frames[3] = IMG_Load("fight/4.png");
        m->enemy.life_state_frames[0] = IMG_Load("etaten/1.png");
        m->enemy.life_state_frames[1] = IMG_Load("etaten/2.png");
        m->enemy.life_state_frames[2] = IMG_Load("etaten/3.png");
        m->enemy.life_state_frames[3] = IMG_Load("etaten/4.png");
        m->enemy.death_frame = IMG_Load("mort/1.png");
        m->enemy.position.x = 5450;
        m->enemy.position.y = 200;
        m->enemy.position.w = m->enemy.walk_right_frames[0] ? m->enemy.walk_right_frames[0]->w : 0;
        m->enemy.position.h = m->enemy.walk_right_frames[0] ? m->enemy.walk_right_frames[0]->h : 0;
        m->enemy.spawn_x = m->enemy.position.x;
        m->enemy.spawn_y = m->enemy.position.y;
        m->enemy.lives = 5;
        m->enemy.facingRight = 1;
        m->enemy.walking = 0;
        m->enemy.fighting = 0;
        m->enemy.walkFrame = 0;
        m->enemy.fightFrame = 0;
        m->enemy.isAlive = 1;
        m->enemy.mode = 0;
        m->enemy.last_damage_time = 0;
    }

    m->image_countdown[0] = IMG_Load("3.png");
    m->image_countdown[1] = IMG_Load("2.png");
    m->image_countdown[2] = IMG_Load("1.png");
    m->image_countdown[3] = IMG_Load("go.png");
    m->position_countdown.x = 1920 / 2;
    m->position_countdown.y = 1080 / 2;
    m->countdown_active = 1;

    if (level == 2) {
        m->position_perso.x = 0;
        m->position_perso.y = 500;
        m->position_perso2.x = 0;
        m->position_perso2.y = 500;
    } else {
        m->position_perso.x = 300;
        m->position_perso.y = 790;
        m->position_perso2.x = 500;
        m->position_perso2.y = 790;
    }

    m->position_perso.w = m->walk_frames[0] ? m->walk_frames[0]->w : 0;
    m->position_perso.h = m->walk_frames[0] ? m->walk_frames[0]->h : 0;
    m->position_perso2.w = m->walk_frames_p2[0] ? m->walk_frames_p2[0]->w : 0;
    m->position_perso2.h = m->walk_frames_p2[0] ? m->walk_frames_p2[0]->h : 0;

    m->facingRight = 1;
    m->walking = 0;
    m->jumping = 0;
    m->fighting = 0;
    m->walkFrame = 0;
    m->jumpStep = 0;
    m->fightFrame = 0;
    m->lives_p1 = 3;

    m->facingRight_p2 = 1;
    m->walking_p2 = 0;
    m->jumping_p2 = 0;
    m->fighting_p2 = 0;
    m->walkFrame_p2 = 0;
    m->jumpStep_p2 = 0;
    m->fightFrame_p2 = 0;
    m->lives_p2 = 3;

    m->game_timer = 0.0f;

    m->bg.camera1.x = 0;
    m->bg.camera1.y = 0;
    m->bg.camera1.w = 1920;
    m->bg.camera1.h = 1080;
    m->bg.camera2.x = 0;
    m->bg.camera2.y = 0;
    m->bg.camera2.w = 1920/2;
    m->bg.camera2.h = 1080;
    m->bg.posEcran1.x = 0;
    m->bg.posEcran1.y = 0;
    m->bg.posEcran1.w = 1920/2;
    m->bg.posEcran1.h = 1080;
    m->bg.posEcran2.x = 1920/2;
    m->bg.posEcran2.y = 0;
    m->bg.posEcran2.w = 1920/2;
    m->bg.posEcran2.h = 1080;
    m->bg.map_width = m->image_back ? m->image_back->w : 1920;
    m->bg.map_height = m->image_back ? m->image_back->h : 1080;

    for (int i = 0; i < 322; i++) {
        m->keysheld[i] = 0;
    }

    m->splitScreen = 0;
}

int skinSelectionMenu(SDL_Surface *screen) {
    Image background;
    initBackground(&background, "submenu.png");

    Button buttons[2];
    initButton(&buttons[0], "jump1.png", "jump1.png", 760, 400); // Default skin
    initButton(&buttons[1], "jump1_p2.png", "jump1_p2.png", 960, 400); // Alternate skin

    SDL_Event event;
    int running = 1;
    int selectedSkin = -1; // -1: no selection, 0: default, 1: alternate
    int wasMouseOver[2] = {0};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
                selectedSkin = -1; // Cancel selection
            } else if (event.type == SDL_MOUSEMOTION) {
                for (int i = 0; i < 2; i++) {
                    buttons[i].isSelected = isMouseOverButton(&buttons[i], event.motion.x, event.motion.y);
                    if (buttons[i].isSelected && !wasMouseOver[i]) {
                        playSound("beep.wav");
                        wasMouseOver[i] = 1;
                    } else if (!buttons[i].isSelected) {
                        wasMouseOver[i] = 0;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (isMouseOverButton(&buttons[0], event.button.x, event.button.y)) {
                    selectedSkin = 0; // Default skin
                    running = 0;
                    playSound("beep.wav");
                } else if (isMouseOverButton(&buttons[1], event.button.x, event.button.y)) {
                    selectedSkin = 1; // Alternate skin
                    running = 0;
                    playSound("beep.wav");
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        if (background.image) {
            SDL_BlitSurface(background.image, NULL, screen, NULL);
        }
        for (int i = 0; i < 2; i++) {
            drawButton(screen, &buttons[i]);
        }
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    if (background.image) SDL_FreeSurface(background.image);
    for (int i = 0; i < 2; i++) {
        if (buttons[i].image) SDL_FreeSurface(buttons[i].image);
        if (buttons[i].hoverImage) SDL_FreeSurface(buttons[i].hoverImage);
    }

    return selectedSkin;
}

void display_countdown(minimap *m, SDL_Surface *screen) {
    m->countdown_active = 1;
    int images_loaded = 1;
    for (int i = 0; i < 4; i++) {
        if (!m->image_countdown[i]) {
            images_loaded = 0;
            printf("Warning: Skipping countdown: image %d.png not loaded\n", 3-i);
            break;
        }
    }

    if (images_loaded && m->image_back && screen) {
        for (int i = 0; i < 4; i++) {
            Uint32 start_time = SDL_GetTicks();
            while (SDL_GetTicks() - start_time < 1000) {
                SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 100, 100, 255));
                afficher(*m, screen, 0);
                int screen_x = m->splitScreen ? 1920 / 4 : 1920 / 2;
                int screen_y = 1080 / 2;
                m->position_countdown.x = screen_x - (m->image_countdown[i]->w / 2);
                m->position_countdown.y = screen_y - (m->image_countdown[i]->h / 2);
                blitImage(m->image_countdown[i], screen, m->position_countdown.x, m->position_countdown.y);
                SDL_Flip(screen);
                SDL_PollEvent(NULL);
                SDL_Delay(10);
            }
        }
    } else {
        printf("Warning: Countdown skipped due to missing assets or screen\n");
    }
    m->countdown_active = 0;
}

void savePlayerPositions(minimap *m) {
    FILE *file = fopen("data.txt", "w");
    if (file) {
        fprintf(file, "Level: %d\n", m->current_level);
        fprintf(file, "Player 1 position: %d,%d\n", m->position_perso.x, m->position_perso.y);
        fprintf(file, "Player 1 lives: %d\n", m->lives_p1);
        if (m->splitScreen) {
            fprintf(file, "Player 2 position: %d,%d\n", m->position_perso2.x, m->position_perso2.y);
            fprintf(file, "Player 2 lives: %d\n", m->lives_p2);
        }
        fprintf(file, "Game timer: %.2f\n", m->game_timer);
        fclose(file);
        m->show_saved = 1;
        m->saved_timer = SDL_GetTicks();
        printf("Level, positions, lives, and timer saved!\n");
    } else {
        printf("Failed to save data!\n");
    }
}

void loadPlayerPositions(minimap *m, int splitScreen, int skin) {
    FILE *file = fopen("data.txt", "r");
    if (file) {
        int x, y, lives, level;
        float timer;
        if (fscanf(file, "Level: %d\n", &level) == 1) {
            m->current_level = level;
            Liberer(m);
            InitialiserMinimap(m, level, skin); // Use provided skin
            m->splitScreen = splitScreen; // Set splitScreen before loading Player 2 data
        }
        if (fscanf(file, "Player 1 position: %d,%d\n", &x, &y) == 2) {
            m->position_perso.x = x;
            m->position_perso.y = y;
            m->bg.camera1.x = x - m->bg.camera1.w / 2;
            m->bg.camera1.y = y - m->bg.camera1.h / 2;
            if (m->bg.camera1.x < 0) m->bg.camera1.x = 0;
            if (m->bg.camera1.y < 0) m->bg.camera1.y = 0;
            if (m->bg.camera1.x + m->bg.camera1.w > m->bg.map_width)
                m->bg.camera1.x = m->bg.map_width - m->bg.camera1.w;
            if (m->bg.camera1.y + m->bg.camera1.h > m->bg.map_height)
                m->bg.camera1.y = m->bg.map_height - m->bg.camera1.h;
            printf("Loaded Player 1: pos(%d,%d), camera1(%d,%d)\n", x, y, m->bg.camera1.x, m->bg.camera1.y);
        }
        if (fscanf(file, "Player 1 lives: %d\n", &lives) == 1) {
            m->lives_p1 = lives;
        }
        if (splitScreen) {
            if (fscanf(file, "Player 2 position: %d,%d\n", &x, &y) == 2) {
                m->position_perso2.x = x;
                m->position_perso2.y = y;
                m->bg.camera2.x = x - m->bg.camera2.w / 2;
                m->bg.camera2.y = y - m->bg.camera2.h / 2;
                if (m->bg.camera2.x < 0) m->bg.camera2.x = 0;
                if (m->bg.camera2.y < 0) m->bg.camera2.y = 0;
                if (m->bg.camera2.x + m->bg.camera2.w > m->bg.map_width)
                    m->bg.camera2.x = m->bg.map_width - m->bg.camera2.w;
                if (m->bg.camera2.y + m->bg.camera2.h > m->bg.map_height)
                    m->bg.camera2.y = m->bg.map_height - m->bg.camera2.h;
                printf("Loaded Player 2: pos(%d,%d), camera2(%d,%d)\n", x, y, m->bg.camera2.x, m->bg.camera2.y);
            }
            if (fscanf(file, "Player 2 lives: %d\n", &lives) == 1) {
                m->lives_p2 = lives;
            }
        }
        if (fscanf(file, "Game timer: %f\n", &timer) == 1) {
            m->game_timer = timer;
        }
        fclose(file);
        printf("Level, positions, lives, timer, and camera positions loaded!\n");
    } else {
        printf("Failed to load data, using defaults!\n");
        m->current_level = 1;
        m->lives_p1 = 3;
        m->lives_p2 = 3;
        m->game_timer = 0.0f;
        m->bg.camera1.x = 0;
        m->bg.camera1.y = 0;
        m->bg.camera2.x = 0;
        m->bg.camera2.y = 0;
        m->splitScreen = splitScreen; // Ensure splitScreen is set even on failure
    }
}

void Liberer(minimap *m) {
    // Free life icons for Player 1 and Player 2
    for (int i = 0; i < 3; i++) {
        if (m->life_icons_p1[i]) SDL_FreeSurface(m->life_icons_p1[i]);
        if (m->life_icons_p2[i]) SDL_FreeSurface(m->life_icons_p2[i]);
    }

    // Free digit images
    for (int i = 0; i < 10; i++) {
        if (m->digit_images[i]) SDL_FreeSurface(m->digit_images[i]);
    }

    // Free colon image
    if (m->colon_image) SDL_FreeSurface(m->colon_image);

    if (m->image_back) SDL_FreeSurface(m->image_back);
    if (m->backgroundMask) SDL_FreeSurface(m->backgroundMask);
    if (m->image_minimap) SDL_FreeSurface(m->image_minimap);
    if (m->image_bonhomme) SDL_FreeSurface(m->image_bonhomme);
    if (m->image_bonhomme2) SDL_FreeSurface(m->image_bonhomme2);
    if (m->image_boom) SDL_FreeSurface(m->image_boom);
    if (m->image_rocket) SDL_FreeSurface(m->image_rocket);
    if (m->image_saved) SDL_FreeSurface(m->image_saved);
    if (m->image_gameover) SDL_FreeSurface(m->image_gameover);

    for (int i = 0; i < 4; i++) {
        if (m->walk_frames[i]) SDL_FreeSurface(m->walk_frames[i]);
        if (m->jump_frames[i]) SDL_FreeSurface(m->jump_frames[i]);
        if (m->walkl_frames[i]) SDL_FreeSurface(m->walkl_frames[i]);
        if (m->jumpl_frames[i]) SDL_FreeSurface(m->jumpl_frames[i]);
        if (m->fight_frames[i]) SDL_FreeSurface(m->fight_frames[i]);
        if (m->fightl_frames[i]) SDL_FreeSurface(m->fightl_frames[i]);
        if (m->walk_frames_p2[i]) SDL_FreeSurface(m->walk_frames_p2[i]);
        if (m->jump_frames_p2[i]) SDL_FreeSurface(m->jump_frames_p2[i]);
        if (m->walkl_frames_p2[i]) SDL_FreeSurface(m->walkl_frames_p2[i]);
        if (m->jumpl_frames_p2[i]) SDL_FreeSurface(m->jumpl_frames_p2[i]);
        if (m->fight_frames_p2[i]) SDL_FreeSurface(m->fight_frames_p2[i]);
        if (m->fightl_frames_p2[i]) SDL_FreeSurface(m->fightl_frames_p2[i]);
        if (m->image_countdown[i]) SDL_FreeSurface(m->image_countdown[i]);
        if (m->enemy.walk_right_frames[i]) SDL_FreeSurface(m->enemy.walk_right_frames[i]);
        if (m->enemy.walk_left_frames[i]) SDL_FreeSurface(m->enemy.walk_left_frames[i]);
        if (m->enemy.fight_right_frames[i]) SDL_FreeSurface(m->enemy.fight_right_frames[i]);
        if (m->enemy.fight_left_frames[i]) SDL_FreeSurface(m->enemy.fight_left_frames[i]);
        if (m->enemy.life_state_frames[i]) SDL_FreeSurface(m->enemy.life_state_frames[i]);
    }
    if (m->enemy.death_frame) SDL_FreeSurface(m->enemy.death_frame);

    if (m->music) Mix_FreeMusic(m->music);
    if (m->gameover_music) Mix_FreeMusic(m->gameover_music);

    memset(m, 0, sizeof(minimap));
}

SDL_Color GetPixel(SDL_Surface *surface, int x, int y) {
    SDL_Color color = {0, 0, 0, 0};
    if (surface && x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
        int col = 0;
        char *pPosition = (char *)surface->pixels;
        pPosition += (surface->pitch * y);
        pPosition += (surface->format->BytesPerPixel * x);
        memcpy(&col, pPosition, surface->format->BytesPerPixel);
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return color;
}

int check_level_transition(minimap *m, SDL_Rect pos) {
    if (m->current_level != 1 || !m->backgroundMask) return 0;

    SDL_Color colors[4] = {
        GetPixel(m->backgroundMask, pos.x + pos.w/2, pos.y + pos.h/2),
        GetPixel(m->backgroundMask, pos.x + pos.w/4, pos.y + pos.h),
        GetPixel(m->backgroundMask, pos.x + 3*pos.w/4, pos.y + pos.h),
        GetPixel(m->backgroundMask, pos.x + pos.w/2, pos.y + pos.h)
    };

    for (int i = 0; i < 4; i++) {
        if (colors[i].r >= 0 && colors[i].r <= 5 &&
            colors[i].g >= 250 && colors[i].g <= 255 &&
            colors[i].b >= 0 && colors[i].b <= 5) {
            printf("Green pixel detected at point %d! Transitioning to level 2.\n", i);
            return 1;
        }
    }
    return 0;
}

int collision_danger(SDL_Surface *masque, SDL_Rect pos) {
    if (!masque) return 0;

    SDL_Color colors[8] = {
        GetPixel(masque, pos.x + pos.w/2, pos.y + pos.h/2),
        GetPixel(masque, pos.x, pos.y),
        GetPixel(masque, pos.x + pos.w, pos.y),
        GetPixel(masque, pos.x, pos.y + pos.h),
        GetPixel(masque, pos.x + pos.w, pos.y + pos.h),
        GetPixel(masque, pos.x + pos.w/4, pos.y + pos.h),
        GetPixel(masque, pos.x + 3*pos.w/4, pos.y + pos.h),
        GetPixel(masque, pos.x + pos.w/2, pos.y + pos.h)
    };

    for (int i = 3; i < 8; i++) {
        printf("Point %d (bottom-related) RGB: (%d, %d, %d)\n", i, colors[i].r, colors[i].g, colors[i].b);
    }

    for (int i = 0; i < 8; i++) {
        if (colors[i].r >= 250 && colors[i].g <= 5 && colors[i].b <= 5) {
            printf("Danger collision detected at point %d! Position: (%d, %d)\n", i, pos.x, pos.y);
            return 1;
        }
    }
    return 0;
}

int collision_wall(SDL_Surface *masque, SDL_Rect pos) {
    if (!masque) return 0;

    SDL_Color colors[4] = {
        GetPixel(masque, pos.x, pos.y + pos.h/2),
        GetPixel(masque, pos.x + pos.w, pos.y + pos.h/2),
        GetPixel(masque, pos.x + pos.w/2, pos.y),
        GetPixel(masque, pos.x + pos.w/2, pos.y + pos.h)
    };

    for (int i = 0; i < 4; i++) {
        if (colors[i].r == 255 && colors[i].g == 255 && colors[i].b == 255) {
            return 1;
        }
    }
    return 0;
}

void handleScrolling(Background *bg, SDL_Rect *playerPos, int dx, int dy, int speed, int playerNum) {
    if (playerNum == 1) {
        if (dx > 0 && bg->camera1.x + bg->camera1.w < bg->map_width)
            bg->camera1.x += speed;
        if (dx < 0 && bg->camera1.x > 0)
            bg->camera1.x -= speed;
        if (dy < 0 && bg->camera1.y > 0)
            bg->camera1.y -= speed;
        if (dy > 0 && bg->camera1.y + bg->camera1.h < bg->map_height)
            bg->camera1.y += speed;
    } else {
        if (dx > 0 && bg->camera2.x + bg->camera2.w < bg->map_width)
            bg->camera2.x += speed;
        if (dx < 0 && bg->camera2.x > 0)
            bg->camera2.x -= speed;
        if (dy < 0 && bg->camera2.y > 0)
            bg->camera2.y -= speed;
        if (dy > 0 && bg->camera2.y + bg->camera2.h < bg->map_height)
            bg->camera2.y += speed;
    }
}

int collision_boundingbox(SDL_Rect a, SDL_Rect b) {
    if (a.x + a.w < b.x || a.x > b.x + b.w || a.y + a.h < b.y || a.y > b.y + b.h)
        return 0;
    else
        return 1;
}

void check_collisions(minimap *m, SDL_Rect *posPerso, int *exploded_rocket, int *exploded_barrel, int playerNum) {
    if (!m->backgroundMask) return;

    Uint32 current_time = SDL_GetTicks();
    const Uint32 danger_cooldown = 500;

    if (collision_danger(m->backgroundMask, *posPerso)) {
        if (playerNum == 1) {
            if (m->lives_p1 > 0 && (current_time - m->last_danger_time_p1 >= danger_cooldown)) {
                m->lives_p1--;
                m->blinking_p1 = 1;
                m->blink_start_time_p1 = current_time;
                m->blink_visible_p1 = 0;
                m->last_danger_time_p1 = current_time;
                printf("Player 1 lost a life! Lives remaining: %d. Blinking started.\n", m->lives_p1);
            }
        } else if (playerNum == 2) {
            if (m->lives_p2 > 0 && (current_time - m->last_danger_time_p2 >= danger_cooldown)) {
                m->lives_p2--;
                m->blinking_p2 = 1;
                m->blink_start_time_p2 = current_time;
                m->blink_visible_p2 = 0;
                m->last_danger_time_p2 = current_time;
                printf("Player 2 lost a life! Lives remaining: %d. Blinking started.\n", m->lives_p2);
            }
        }
    }

    if (m->rocket_active && !*exploded_rocket && !m->countdown_active) {
        m->position_rocket.x -= 5;
        m->position_rocket.y += m->rocket_speed_y;

        if (m->position_rocket.y > 1080 || m->position_rocket.x + (m->image_rocket ? m->image_rocket->w : 0) < 0) {
            m->position_rocket.x = rand() % (5760 - (m->image_rocket ? m->image_rocket->w : 0));
            m->position_rocket.y = 0;
            printf("Rocket respawned at (%d, %d)\n", m->position_rocket.x, m->position_rocket.y);
        }

        if (collision_boundingbox(m->position_rocket, *posPerso)) {
            if (playerNum == 1) {
                m->lives_p1 = 0;
                printf("Player 1 hit by rocket! Instant game over.\n");
            } else if (playerNum == 2) {
                m->lives_p2 = 0;
                printf("Player 2 hit by rocket! Instant game over.\n");
            }
        }
    }
}

void afficher(minimap m, SDL_Surface *screen, int exploded_barrel) {
    if (!screen || !m.image_back) return;

    Uint32 current_time = SDL_GetTicks();
    if (m.blinking_p1) {
        if (current_time - m.blink_start_time_p1 >= 2000) {
            m.blinking_p1 = 0;
            m.blink_visible_p1 = 1;
        } else {
            if ((current_time - m.blink_start_time_p1) % 200 < 100) {
                m.blink_visible_p1 = 1;
            } else {
                m.blink_visible_p1 = 0;
            }
        }
    }
    if (m.blinking_p2) {
        if (current_time - m.blink_start_time_p2 >= 2000) {
            m.blinking_p2 = 0;
            m.blink_visible_p2 = 1;
        } else {
            if ((current_time - m.blink_start_time_p2) % 200 < 100) {
                m.blink_visible_p2 = 1;
            } else {
                m.blink_visible_p2 = 0;
            }
        }
    }

    if (m.splitScreen) {
        SDL_Rect src1 = {m.bg.camera1.x, m.bg.camera1.y, m.bg.camera1.w, m.bg.camera1.h};
        SDL_Rect dst1 = {m.bg.posEcran1.x, m.bg.posEcran1.y, m.bg.posEcran1.w, m.bg.posEcran1.h};
        SDL_BlitSurface(m.image_back, &src1, screen, &dst1);

        SDL_Rect src2 = {m.bg.camera2.x, m.bg.camera2.y, m.bg.camera2.w, m.bg.camera2.h};
        SDL_Rect dst2 = {m.bg.posEcran2.x, m.bg.posEcran2.y, m.bg.posEcran2.w, m.bg.posEcran2.h};
        SDL_BlitSurface(m.image_back, &src2, screen, &dst2);

        if (m.rocket_active && m.image_rocket) {
            SDL_Rect rocket_pos1 = {m.position_rocket.x - m.bg.camera1.x, m.position_rocket.y - m.bg.camera1.y, 0, 0};
            SDL_BlitSurface(m.image_rocket, NULL, screen, &rocket_pos1);
            SDL_Rect rocket_pos2 = {m.position_rocket.x - m.bg.camera2.x + 1920/2, m.position_rocket.y - m.bg.camera2.y, 0, 0};
            SDL_BlitSurface(m.image_rocket, NULL, screen, &rocket_pos2);
        }

        SDL_Surface **current_frames1 = m.facingRight ? 
            (m.fighting ? m.fight_frames : (m.jumping ? m.jump_frames : m.walk_frames)) :
            (m.fighting ? m.fightl_frames : (m.jumping ? m.jumpl_frames : m.walkl_frames));
        int frame1 = m.fighting ? (m.fightFrame / 3) : (m.jumping ? (m.jumpStep / 3 > 3 ? 3 : m.jumpStep / 3) : (m.walking ? m.walkFrame : 0));
        SDL_Rect perso_pos1 = {m.position_perso.x - m.bg.camera1.x, m.position_perso.y - m.bg.camera1.y, 0, 0};
        if (current_frames1[frame1] && m.blink_visible_p1) {
            printf("Player 1: facingRight=%d, fighting=%d, frame=%d, using %s frame\n", 
                   m.facingRight, m.fighting, frame1, m.facingRight ? "fight" : "fightl");
            SDL_BlitSurface(current_frames1[frame1], NULL, screen, &perso_pos1);
        }

        SDL_Surface **current_frames2 = m.facingRight_p2 ? 
            (m.fighting_p2 ? m.fight_frames_p2 : (m.jumping_p2 ? m.jump_frames_p2 : m.walk_frames_p2)) :
            (m.fighting_p2 ? m.fightl_frames_p2 : (m.jumping_p2 ? m.jumpl_frames_p2 : m.walkl_frames_p2));
        int frame2 = m.fighting_p2 ? (m.fightFrame_p2 / 3) : (m.jumping_p2 ? (m.jumpStep_p2 / 3 > 3 ? 3 : m.jumpStep_p2 / 3) : (m.walking_p2 ? m.walkFrame_p2 : 0));
        SDL_Rect perso_pos2 = {m.position_perso2.x - m.bg.camera2.x + 1920/2, m.position_perso2.y - m.bg.camera2.y, 0, 0};
        if (current_frames2[frame2] && m.blink_visible_p2) {
            printf("Player 2: facingRight_p2=%d, fighting_p2=%d, frame=%d, using %s frame\n", 
                   m.facingRight_p2, m.fighting_p2, frame2, m.facingRight_p2 ? "fight" : "fightl");
            SDL_BlitSurface(current_frames2[frame2], NULL, screen, &perso_pos2);
        }

        if (exploded_barrel && m.image_boom) {
            SDL_Rect boom_pos1 = {m.position_boom.x - m.bg.camera1.x, m.position_boom.y - m.bg.camera1.y, 0, 0};
            SDL_BlitSurface(m.image_boom, NULL, screen, &boom_pos1);
            SDL_Rect boom_pos2 = {m.position_boom.x - m.bg.camera2.x + 1920/2, m.position_boom.y - m.bg.camera2.y, 0, 0};
            SDL_BlitSurface(m.image_boom, NULL, screen, &boom_pos2);
        }

        // Render enemy in split-screen
        if (m.current_level == 1 && m.enemy.isAlive) {
            SDL_Surface **enemy_frames = m.enemy.facingRight ? 
                (m.enemy.fighting ? m.enemy.fight_right_frames : m.enemy.walk_right_frames) :
                (m.enemy.fighting ? m.enemy.fight_left_frames : m.enemy.walk_left_frames);
            int enemy_frame = m.enemy.fighting ? (m.enemy.fightFrame / 3) : (m.enemy.walking ? m.enemy.walkFrame : 0);
            SDL_Rect enemy_pos1 = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y, 0, 0};
            SDL_Rect enemy_pos2 = {m.enemy.position.x - m.bg.camera2.x + 1920/2, m.enemy.position.y - m.bg.camera2.y, 0, 0};
            if (enemy_frames[enemy_frame]) {
                SDL_BlitSurface(enemy_frames[enemy_frame], NULL, screen, &enemy_pos1);
                SDL_BlitSurface(enemy_frames[enemy_frame], NULL, screen, &enemy_pos2);
            }
            // Render life state indicator above enemy
            int life_frame = m.enemy.lives <= 3 ? m.enemy.lives : 3;
            if (m.enemy.life_state_frames[life_frame]) {
                SDL_Rect life_pos1 = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y - 20, 0, 0};
                SDL_Rect life_pos2 = {m.enemy.position.x - m.bg.camera2.x + 1920/2, m.enemy.position.y - m.bg.camera2.y - 20, 0, 0};
                SDL_BlitSurface(m.enemy.life_state_frames[life_frame], NULL, screen, &life_pos1);
                SDL_BlitSurface(m.enemy.life_state_frames[life_frame], NULL, screen, &life_pos2);
            }
        } else if (m.current_level == 1 && !m.enemy.isAlive && m.enemy.death_frame) {
            SDL_Rect death_pos1 = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y, 0, 0};
            SDL_Rect death_pos2 = {m.enemy.position.x - m.bg.camera2.x + 1920/2, m.enemy.position.y - m.bg.camera2.y, 0, 0};
            SDL_BlitSurface(m.enemy.death_frame, NULL, screen, &death_pos1);
            SDL_BlitSurface(m.enemy.death_frame, NULL, screen, &death_pos2);
        }
    } else {
        SDL_Rect src = {m.bg.camera1.x, m.bg.camera1.y, m.bg.camera1.w, m.bg.camera1.h};
        SDL_Rect dst = {0, 0, m.bg.camera1.w, m.bg.camera1.h};
        SDL_BlitSurface(m.image_back, &src, screen, &dst);

        if (m.rocket_active && m.image_rocket) {
            SDL_Rect rocket_pos = {m.position_rocket.x - m.bg.camera1.x, m.position_rocket.y - m.bg.camera1.y, 0, 0};
            SDL_BlitSurface(m.image_rocket, NULL, screen, &rocket_pos);
        }

        SDL_Surface **current_frames = m.facingRight ? 
            (m.fighting ? m.fight_frames : (m.jumping ? m.jump_frames : m.walk_frames)) :
            (m.fighting ? m.fightl_frames : (m.jumping ? m.jumpl_frames : m.walkl_frames));
        int frame = m.fighting ? (m.fightFrame / 3) : (m.jumping ? (m.jumpStep / 3 > 3 ? 3 : m.jumpStep / 3) : (m.walking ? m.walkFrame : 0));
        SDL_Rect perso_pos = {m.position_perso.x - m.bg.camera1.x, m.position_perso.y - m.bg.camera1.y, 0, 0};
        if (current_frames[frame] && m.blink_visible_p1) {
            printf("Player 1 (single-screen): facingRight=%d, fighting=%d, frame=%d, using %s frame\n", 
                   m.facingRight, m.fighting, frame, m.facingRight ? "fight" : "fightl");
            SDL_BlitSurface(current_frames[frame], NULL, screen, &perso_pos);
        }

        if (exploded_barrel && m.image_boom) {
            SDL_Rect boom_pos = {m.position_boom.x - m.bg.camera1.x, m.position_boom.y - m.bg.camera1.y, 0, 0};
            SDL_BlitSurface(m.image_boom, NULL, screen, &boom_pos);
        }

        // Render enemy in single-screen
        if (m.current_level == 1 && m.enemy.isAlive) {
            SDL_Surface **enemy_frames = m.enemy.facingRight ? 
                (m.enemy.fighting ? m.enemy.fight_right_frames : m.enemy.walk_right_frames) :
                (m.enemy.fighting ? m.enemy.fight_left_frames : m.enemy.walk_left_frames);
            int enemy_frame = m.enemy.fighting ? (m.enemy.fightFrame / 3) : (m.enemy.walking ? m.enemy.walkFrame : 0);
            SDL_Rect enemy_pos = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y, 0, 0};
            if (enemy_frames[enemy_frame]) {
                SDL_BlitSurface(enemy_frames[enemy_frame], NULL, screen, &enemy_pos);
            }
            // Render life state indicator above enemy
            int life_frame = m.enemy.lives <= 3 ? m.enemy.lives : 3;
            if (m.enemy.life_state_frames[life_frame]) {
                SDL_Rect life_pos = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y - 20, 0, 0};
                SDL_BlitSurface(m.enemy.life_state_frames[life_frame], NULL, screen, &life_pos);
            }
        } else if (m.current_level == 1 && !m.enemy.isAlive && m.enemy.death_frame) {
            SDL_Rect death_pos = {m.enemy.position.x - m.bg.camera1.x, m.enemy.position.y - m.bg.camera1.y, 0, 0};
            SDL_BlitSurface(m.enemy.death_frame, NULL, screen, &death_pos);
        }
    }

    // Render life icon for Player 1
    if (m.lives_p1 > 0 && m.lives_p1 <= 3 && m.life_icons_p1[3 - m.lives_p1]) {
        SDL_BlitSurface(m.life_icons_p1[3 - m.lives_p1], NULL, screen, &m.position_life_p1);
    }

    // Render life icon for Player 2 (if split-screen)
    if (m.splitScreen && m.lives_p2 > 0 && m.lives_p2 <= 3 && m.life_icons_p2[3 - m.lives_p2]) {
        SDL_BlitSurface(m.life_icons_p2[3 - m.lives_p2], NULL, screen, &m.position_life_p2);
    }

    // Render timer (MM:SS) using digit and colon images
    int all_digits_loaded = 1;
    for (int i = 0; i < 10; i++) {
        if (!m.digit_images[i]) {
            all_digits_loaded = 0;
            break;
        }
    }
    if (all_digits_loaded && m.colon_image) {
        int total_seconds = (int)m.game_timer;
        int minutes = total_seconds / 60;
        int seconds = total_seconds % 60;

        int min_tens = minutes / 10;
        int min_ones = minutes % 10;
        int sec_tens = seconds / 10;
        int sec_ones = seconds % 10;

        // Assume digit images are 24x24 pixels, colon is 12x24 pixels
        int digit_width = m.digit_images[0] ? m.digit_images[0]->w : 24;
        int colon_width = m.colon_image ? m.colon_image->w : 12;
        int total_width = 4 * digit_width + colon_width;
        int start_x = (1920 - total_width) / 2; // Center horizontally
        int y = 150; // Top of screen

        SDL_Rect pos;

        // Minutes tens
        pos.x = start_x;
        pos.y = y;
        SDL_BlitSurface(m.digit_images[min_tens], NULL, screen, &pos);

        // Minutes ones
        pos.x += digit_width;
        SDL_BlitSurface(m.digit_images[min_ones], NULL, screen, &pos);

        // Colon
        pos.x += digit_width;
        SDL_BlitSurface(m.colon_image, NULL, screen, &pos);

        // Seconds tens
        pos.x += colon_width;
        SDL_BlitSurface(m.digit_images[sec_tens], NULL, screen, &pos);

        // Seconds ones
        pos.x += digit_width;
        SDL_BlitSurface(m.digit_images[sec_ones], NULL, screen, &pos);
    } else {
        printf("Warning: Timer not displayed due to missing digit or colon images\n");
    }
}

int mainMenu(SDL_Surface *screen) {
    Image background;
    initBackground(&background, "submenu.png");

    Button buttons[5];
initButton(&buttons[0], "new/play.png", "new/play_hover.png", 860, 400); // Play button
initButton(&buttons[1], "new/option.png", "new/option_hover.png", 860, 600); // Options button
initButton(&buttons[2], "histoire.png", "histoire_hover.png", 860, 700); // Story button
initButton(&buttons[3], "scores.png", "scores_hover.png", 860, 800); // Scores button
initButton(&buttons[4], "quitter.png", "quitter_hover.png", 860, 900); // Quit button

    Mix_Music *currentMusic = NULL;
    SDL_Event event;
    int running = 1;
    int proceedToGame = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                proceedToGame = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
                proceedToGame = 0;
            } else if (event.type == SDL_MOUSEMOTION) {
                for (int i = 0; i < 5; i++) {
                    buttons[i].isSelected = isMouseOverButton(&buttons[i], event.motion.x, event.motion.y);
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		if (isMouseOverButton(&buttons[0], event.button.x, event.button.y)) {
		    proceedToGame = 1;
		    running = 0;
		} else if (isMouseOverButton(&buttons[1], event.button.x, event.button.y)) {
		    sousMenuOption(&screen, &currentMusic);
		} else if (isMouseOverButton(&buttons[2], event.button.x, event.button.y)) {
		    storyMenu(&screen, &currentMusic);
		} else if (isMouseOverButton(&buttons[3], event.button.x, event.button.y)) {
		    bestScoreMenu(&screen, &currentMusic);
		} else if (isMouseOverButton(&buttons[4], event.button.x, event.button.y)) {
		    running = 0;
		    proceedToGame = 0; // Exit the game completely
		}
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        if (background.image) {
            SDL_BlitSurface(background.image, NULL, screen, NULL);
        }
        for (int i = 0; i < 5; i++) {
            drawButton(screen, &buttons[i]);
        }
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    if (background.image) SDL_FreeSurface(background.image);
    for (int i = 0; i < 5; i++) {
        if (buttons[i].image) SDL_FreeSurface(buttons[i].image);
        if (buttons[i].hoverImage) SDL_FreeSurface(buttons[i].hoverImage);
    }
    if (currentMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
    }

    return proceedToGame;
}

int modeSelectionMenu(SDL_Surface *screen) {
    Image background;
    initBackground(&background, "submenu.png");

    Button buttons[2];
    initButton(&buttons[0], "mono.png", "mono_hover.png", 860, 400); // Single-player button
    initButton(&buttons[1], "multi.png", "multi_hover.png", 860, 600); // Multiplayer button

    SDL_Event event;
    int running = 1;
    int mode = 0; // 0: exit, 1: single-player, 2: multiplayer
    int wasMouseOver[2] = {0};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
                mode = 0;
            } else if (event.type == SDL_MOUSEMOTION) {
                for (int i = 0; i < 2; i++) {
                    buttons[i].isSelected = isMouseOverButton(&buttons[i], event.motion.x, event.motion.y);
                    if (buttons[i].isSelected && !wasMouseOver[i]) {
                        playSound("beep.wav");
                        wasMouseOver[i] = 1;
                    } else if (!buttons[i].isSelected) {
                        wasMouseOver[i] = 0;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (isMouseOverButton(&buttons[0], event.button.x, event.button.y)) {
                    mode = 1; // Single-player
                    running = 0;
                    playSound("beep.wav");
                } else if (isMouseOverButton(&buttons[1], event.button.x, event.button.y)) {
                    mode = 2; // Multiplayer
                    running = 0;
                    playSound("beep.wav");
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        if (background.image) {
            SDL_BlitSurface(background.image, NULL, screen, NULL);
        }
        for (int i = 0; i < 2; i++) {
            drawButton(screen, &buttons[i]);
        }
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    if (background.image) SDL_FreeSurface(background.image);
    for (int i = 0; i < 2; i++) {
        if (buttons[i].image) SDL_FreeSurface(buttons[i].image);
        if (buttons[i].hoverImage) SDL_FreeSurface(buttons[i].hoverImage);
    }

    return mode;
}

void bestScoreMenu(SDL_Surface **screen, Mix_Music **currentMusic) {
    Image background;
    initBackground(&background, "submenu.png");

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0};
    SDL_Surface *textScores = TTF_RenderText_Solid(font, "Best scores:", textColor);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }

        SDL_FillRect(*screen, NULL, SDL_MapRGB((*screen)->format, 0, 0, 0));
        blitImage(background.image, *screen, 0, 0);
        blitImage(textScores, *screen, 100, 100); // Position text at (100, 100)
        SDL_Flip(*screen);
    }

    SDL_FreeSurface(background.image);
    SDL_FreeSurface(textScores);
    TTF_CloseFont(font);
}



void storyMenu(SDL_Surface **screen, Mix_Music **currentMusic) { 
Image background; initBackground(&background, "submenu.png");

TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
SDL_Color textColor = {0, 0, 0};

const char *storyLines[] = {
    "In the heart of Tunisia, at the prestigious engineering school ESPRIT,",
    "students are working on a revolutionary Artificial Intelligence project.",
    "One night, something goes terribly wrong. A rogue faction of rival",
    "university professors launches an attack using an army of autonomous",
    "mini robots designed to steal research and shut down the ESPRIT mainframe.",
    " ",
    "But the ESPRIT students are ready.",
    " ",
    "They ve secretly been building Project Guardian, a powerful combat robot",
    "equipped with adaptive AI, built from scavenged tech hidden across the",
    "campus. Now, with the university under siege, it s up to you, an elite",
    "student engineer, to guide the Guardian across the school grounds,",
    "collecting essential parts, dodging traps, and battling enemy bots.",
    " ",
    "Navigate through the campus using your minimap, avoid obstacles with",
    "pixel-perfect precision, and interact with the environment:",
    " ",
    "- Barrels may hide parts or explode, tread carefully.",
    "- Rockets fly in for sudden ambushes, dodge them or face destruction.",
    "- Use your jump and movement skills, switching between walking and",
    "  jumping animations, to survive.",
    " ",
    "But you re not alone...",
    " ",
    "In 2-player mode, your teammate can join in split-screen style to help",
    "you build the Guardian faster and defend ESPRIT from total collapse.",
    "Coordinate, explore, and battle together to unlock new zones and",
    "outsmart the invaders.",
    " ",
    "And remember:",
    "If the enemy reaches the lab before you do, it s game over, not just",
    "for ESPRIT, but for the future of student innovation."
};
int numLines = sizeof(storyLines) / sizeof(storyLines[0]);

SDL_Surface *textSurfaces[32];
for (int i = 0; i < numLines; i++) {
    textSurfaces[i] = TTF_RenderText_Solid(font, storyLines[i], textColor);
    if (!textSurfaces[i]) {
        printf("Error rendering text line %d: %s\n", i, TTF_GetError());
    }
}

SDL_Event event;
int running = 1;

while (running) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            running = 0;
        }
    }

    SDL_FillRect(*screen, NULL, SDL_MapRGB((*screen)->format, 0, 0, 0));
    blitImage(background.image, *screen, 0, 0);
    for (int i = 0; i < numLines; i++) {
        if (textSurfaces[i]) {
            blitImage(textSurfaces[i], *screen, 100, 100 + i * 30);
        }
    }
    SDL_Flip(*screen);
}

SDL_FreeSurface(background.image);
for (int i = 0; i < numLines; i++) {
    if (textSurfaces[i]) SDL_FreeSurface(textSurfaces[i]);
}
TTF_CloseFont(font);

}


int sauvegardeMenu(SDL_Surface *screen) {
    Image background;
    initBackground(&background, "submenu.png");

    Button boutonNouvellePartie, boutonCharger;
    initButton(&boutonNouvellePartie, "new.png", "new_h.png", 900, 500);
    initButton(&boutonCharger, "load.png", "load_h.png", 200, 500);

    SDL_Event event;
    int running = 1;
    int gameMode = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                gameMode = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
                gameMode = 0;
            } else if (event.type == SDL_MOUSEMOTION) {
                boutonNouvellePartie.isSelected = isMouseOverButton(&boutonNouvellePartie, event.motion.x, event.motion.y);
                boutonCharger.isSelected = isMouseOverButton(&boutonCharger, event.motion.x, event.motion.y);
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (isMouseOverButton(&boutonNouvellePartie, event.button.x, event.button.y)) {
                    gameMode = 1;
                    running = 0;
                } else if (isMouseOverButton(&boutonCharger, event.button.x, event.button.y)) {
                    gameMode = 2;
                    running = 0;
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        if (background.image) {
            SDL_BlitSurface(background.image, NULL, screen, NULL);
        }
        drawButton(screen, &boutonNouvellePartie);
        drawButton(screen, &boutonCharger);
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    if (background.image) SDL_FreeSurface(background.image);
    if (boutonNouvellePartie.image) SDL_FreeSurface(boutonNouvellePartie.image);
    if (boutonNouvellePartie.hoverImage) SDL_FreeSurface(boutonNouvellePartie.hoverImage);
    if (boutonCharger.image) SDL_FreeSurface(boutonCharger.image);
    if (boutonCharger.hoverImage) SDL_FreeSurface(boutonCharger.hoverImage);

    return gameMode;
}
