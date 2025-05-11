#include "header.h"

int main(int argc, char *argv[]) {
    SDL_Surface *screen;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur d'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Erreur d'initialisation de SDL_ttf : %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        printf("Erreur d'initialisation de SDL_mixer : %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    screen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        printf("Erreur de création de la fenêtre : %s\n", SDL_GetError());
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_WM_SetCaption("Minimap Test", NULL);

    int programRunning = 1;
    while(programRunning) {
        // Start with the main menu
        int proceed = mainMenu(screen);
        if (!proceed) {
            programRunning = 0;
            break;
        }

        // Proceed to mode selection menu
        int mode = modeSelectionMenu(screen);
        if (mode == 0) {
            continue; // Return to main menu
        }

        // Proceed to the load/new game menu
        int gameMode = sauvegardeMenu(screen);
        if (gameMode == 0) {
            continue; // Return to main menu
        }

	// Proceed to skin selection menu
	int selectedSkin = skinSelectionMenu(screen);
	if (selectedSkin == -1) {
	    continue; // Return to main menu if skin selection is cancelled
	}

        minimap m;
        InitialiserMinimap(&m, gameMode == 1 ? 1 : 0, selectedSkin);
        m.splitScreen = (mode == 2) ? 1 : 0; // Set split-screen based on mode
        int savedSplitScreen = m.splitScreen; // Save splitScreen state

	if (gameMode == 2) {
            loadPlayerPositions(&m, m.splitScreen, selectedSkin); // Pass selectedSkin
            m.splitScreen = savedSplitScreen;
        }

        if (m.music && (m.current_level == 1 || m.current_level == 2)) {
            Mix_HaltMusic();
            if (Mix_PlayMusic(m.music, -1) == -1) {
                printf("Failed to play music.mp3: %s\n", Mix_GetError());
            }
        }

        display_countdown(&m, screen);

        SDL_Event event;
        int continuer = 1;
        int exploded_barrel = 0;
        int exploded_rocket = 0;

        int rightHeld = 0, leftHeld = 0, upHeld = 0;
        int rightHeld_p2 = 0, leftHeld_p2 = 0, upHeld_p2 = 0;
        int jumping = 0, jumpStep = 0;
        int jumping_p2 = 0, jumpStep_p2 = 0;
        int walking = 0, walkFrame = 0;
        int walking_p2 = 0, walkFrame_p2 = 0;
        int fighting = 0, fightFrame = 0;
        int fighting_p2 = 0, fightFrame_p2 = 0;
        int facingRight = 1;
        int facingRight_p2 = 1;

        SDL_Rect posPerso = {
            m.position_perso.x,
            m.position_perso.y,
            m.walk_frames[0] ? m.walk_frames[0]->w : 0,
            m.walk_frames[0] ? m.walk_frames[0]->h : 0
        };
        
        SDL_Rect posPerso2 = {
            m.position_perso2.x,
            m.position_perso2.y,
            m.walk_frames_p2[0] ? m.walk_frames_p2[0]->w : 0,
            m.walk_frames_p2[0] ? m.walk_frames_p2[0]->h : 0
        };
        
        int initialPosY = posPerso.y;
        int initialPosY_p2 = posPerso2.y;
        Uint32 lastTime = SDL_GetTicks();

        while (continuer) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            if (!m.countdown_active) {
                m.game_timer += deltaTime / 1000.0f;

                if (m.show_saved && (currentTime - m.saved_timer) >= 1000) {
                    m.show_saved = 0;
                }

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        continuer = 0;
                        programRunning = 0;
                    }

                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_v) {
                        savePlayerPositions(&m);
                    }

                    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                        int keyState = (event.type == SDL_KEYDOWN) ? 1 : 0;
                        m.keysheld[event.key.keysym.sym] = keyState;
                    }
                }

                // Player 1 controls
                if (!fighting) {
                    if (m.keysheld[SDLK_d]) {
                        rightHeld = 1; 
                        facingRight = 1;
                        walking = 1;
                    } else {
                        rightHeld = 0;
                        if (!leftHeld) walking = 0;
                    }
                    
                    if (m.keysheld[SDLK_a]) {
                        leftHeld = 1; 
                        facingRight = 0;
                        walking = 1;
                    } else {
                        leftHeld = 0;
                        if (!rightHeld) walking = 0;
                    }
                    
                    if (m.keysheld[SDLK_w]) {
                        upHeld = 1; 
                        if (!jumping) {
                            jumping = 1;
                            jumpStep = 0;
                            initialPosY = posPerso.y;
                        }
                    } else {
                        upHeld = 0;
                    }
                    
                    if (m.keysheld[SDLK_e] && !jumping) {
                        fighting = 1;
                        fightFrame = 0;
                        walking = 0;
                        jumping = 0;
                    }
                } else {
                    fightFrame++;
                    if (fightFrame >= 12) {
                        fighting = 0;
                        fightFrame = 0;
                    }
                }

                // Player 2 controls
                if (m.splitScreen) {
                    if (!fighting_p2) {
                        if (m.keysheld[SDLK_RIGHT]) {
                            rightHeld_p2 = 1; 
                            facingRight_p2 = 1;
                            walking_p2 = 1;
                        } else {
                            rightHeld_p2 = 0;
                            if (!leftHeld_p2) walking_p2 = 0;
                        }
                        
                        if (m.keysheld[SDLK_LEFT]) {
                            leftHeld_p2 = 1; 
                            facingRight_p2 = 0;
                            walking_p2 = 1;
                        } else {
                            leftHeld_p2 = 0;
                            if (!rightHeld_p2) walking_p2 = 0;
                        }
                        
                        if (m.keysheld[SDLK_UP]) {
                            upHeld_p2 = 1; 
                            if (!jumping_p2) {
                                jumping_p2 = 1;
                                jumpStep_p2 = 0;
                                initialPosY_p2 = posPerso2.y;
                            }
                        } else {
                            upHeld_p2 = 0;
                        }
                        
                        if (m.keysheld[SDLK_l] && !jumping_p2) {
                            fighting_p2 = 1;
                            fightFrame_p2 = 0;
                            walking_p2 = 0;
                            jumping_p2 = 0;
                        }
                    } else {
                        fightFrame_p2++;
                        if (fightFrame_p2 >= 12) {
                            fighting_p2 = 0;
                            fightFrame_p2 = 0;
                        }
                    }
                }

                // Enemy logic (only for level 1)
                if (m.current_level == 1 && m.enemy.isAlive) {
                    int closest_player_x = m.splitScreen ? (abs(m.enemy.position.x - posPerso.x) < abs(m.enemy.position.x - posPerso2.x) ? posPerso.x : posPerso2.x) : posPerso.x;
                    int distance = abs(m.enemy.position.x - closest_player_x);

                    // Determine enemy mode
                    if (collision_boundingbox(m.enemy.position, posPerso) || (m.splitScreen && collision_boundingbox(m.enemy.position, posPerso2))) {
                        m.enemy.mode = 2; // Fight mode
                        m.enemy.fighting = 1;
                        m.enemy.walking = 0;
                    } else if (distance < 300) {
                        m.enemy.mode = 1; // Chase mode
                        m.enemy.fighting = 0;
                        m.enemy.walking = 1;
                    } else if (distance > 500) {
                        m.enemy.mode = 0; // Idle mode (return to spawn)
                        m.enemy.fighting = 0;
                        m.enemy.walking = 1;
                    } else {
                        m.enemy.mode = 0; // Idle (stay in place)
                        m.enemy.fighting = 0;
                        m.enemy.walking = 0;
                    }

                    // Enemy movement
                    if (m.enemy.mode == 1) { // Chase
                        if (closest_player_x > m.enemy.position.x) {
                            m.enemy.position.x += 3;
                            m.enemy.facingRight = 1;
                            if (collision_wall(m.backgroundMask, m.enemy.position)) {
                                m.enemy.position.x -= 3;
                            }
                        } else {
                            m.enemy.position.x -= 3;
                            m.enemy.facingRight = 0;
                            if (collision_wall(m.backgroundMask, m.enemy.position)) {
                                m.enemy.position.x += 3;
                            }
                        }
                    } else if (m.enemy.mode == 0 && (m.enemy.position.x != m.enemy.spawn_x || m.enemy.position.y != m.enemy.spawn_y)) {
                        // Return to spawn
                        if (m.enemy.position.x < m.enemy.spawn_x) {
                            m.enemy.position.x += 3;
                            m.enemy.facingRight = 1;
                            if (collision_wall(m.backgroundMask, m.enemy.position)) {
                                m.enemy.position.x -= 3;
                            }
                        } else if (m.enemy.position.x > m.enemy.spawn_x) {
                            m.enemy.position.x -= 3;
                            m.enemy.facingRight = 0;
                            if (collision_wall(m.backgroundMask, m.enemy.position)) {
                                m.enemy.position.x += 3;
                            }
                        }
                    }

                    // Gravity for enemy
                    if (!m.enemy.fighting) {
                        m.enemy.position.y += 10;
                        if (collision_wall(m.backgroundMask, m.enemy.position)) {
                            m.enemy.position.y -= 10;
                        }
                    }

                    // Update animation frames
                    if (m.enemy.walking && deltaTime > 0) {
                        m.enemy.walkFrame = (m.enemy.walkFrame + 1) % 4;
                    }
                    if (m.enemy.fighting) {
                        m.enemy.fightFrame++;
                        if (m.enemy.fightFrame >= 12) {
                            m.enemy.fightFrame = 0;
                        }
                    }

                    // Enemy damage to players
                    if (m.enemy.mode == 2 && currentTime - m.enemy.last_damage_time >= 4000) {
                        if (collision_boundingbox(m.enemy.position, posPerso) && m.lives_p1 > 0) {
                            m.lives_p1--;
                            m.blinking_p1 = 1;
                            m.blink_start_time_p1 = currentTime;
                            m.blink_visible_p1 = 0;
                            printf("Player 1 hit by enemy! Lives remaining: %d\n", m.lives_p1);
                        }
                        if (m.splitScreen && collision_boundingbox(m.enemy.position, posPerso2) && m.lives_p2 > 0) {
                            m.lives_p2--;
                            m.blinking_p2 = 1;
                            m.blink_start_time_p2 = currentTime;
                            m.blink_visible_p2 = 0;
                            printf("Player 2 hit by enemy! Lives remaining: %d\n", m.lives_p2);
                        }
                        m.enemy.last_damage_time = currentTime;
                    }

                    // Player attacks on enemy
                    if (m.keysheld[SDLK_e] && collision_boundingbox(m.enemy.position, posPerso) && m.enemy.lives > 0) {
                        m.enemy.lives--;
                        printf("Enemy hit by Player 1! Enemy lives: %d\n", m.enemy.lives);
                        if (m.enemy.lives <= 0) {
                            m.enemy.isAlive = 0;
                            printf("Enemy defeated!\n");
                        }
                    }
                    if (m.splitScreen && m.keysheld[SDLK_l] && collision_boundingbox(m.enemy.position, posPerso2) && m.enemy.lives > 0) {
                        m.enemy.lives--;
                        printf("Enemy hit by Player 2! Enemy lives: %d\n", m.enemy.lives);
                        if (m.enemy.lives <= 0) {
                            m.enemy.isAlive = 0;
                            printf("Enemy defeated!\n");
                        }
                    }
                }

                // Player 1 movement
                if (!fighting) {
                    int moved = 0; // Flag to track if movement was successful
                    if (rightHeld) {
                        posPerso.x += 5;
                        if (!collision_wall(m.backgroundMask, posPerso)) {
                            handleScrolling(&m.bg, &posPerso, 5, 0, 5, 1);
                            moved = 1;
                        } else {
                            posPerso.x -= 5; // Revert position if collision
                        }
                    }
                    if (leftHeld) {
                        posPerso.x -= 5;
                        if (!collision_wall(m.backgroundMask, posPerso)) {
                            handleScrolling(&m.bg, &posPerso, -5, 0, 5, 1);
                            moved = 1;
                        } else {
                            posPerso.x += 5; // Revert position if collision
                        }
                    }
                    if (walking && deltaTime > 0 && moved) {
                        walkFrame = (walkFrame + 1) % 4;
                    }

                    if (jumping) {
                        jumpStep++;
                        moved = 0;
                        if (rightHeld) {
                            posPerso.x += 5;
                            if (!collision_wall(m.backgroundMask, posPerso)) {
                                handleScrolling(&m.bg, &posPerso, 5, 0, 5, 1);
                                moved = 1;
                            } else {
                                posPerso.x -= 5;
                            }
                        } else if (leftHeld) {
                            posPerso.x -= 5;
                            if (!collision_wall(m.backgroundMask, posPerso)) {
                                handleScrolling(&m.bg, &posPerso, -5, 0, 5, 1);
                                moved = 1;
                            } else {
                                posPerso.x += 5;
                            }
                        }
                        if (jumpStep < 12) {
                            posPerso.y -= 20;
                            if (collision_wall(m.backgroundMask, posPerso)) {
                                posPerso.y += 20;
                                jumping = 0;
                            }
                        } else {
                            posPerso.y += 20;
                            if (collision_wall(m.backgroundMask, posPerso)) {
                                posPerso.y -= 20;
                                jumping = 0;
                            }
                        }
                        if (jumpStep >= 24) {
                            jumping = 0;
                            posPerso.y = initialPosY;
                        }
                    } else {
                        posPerso.y += 10;
                        if (collision_wall(m.backgroundMask, posPerso)) {
                            posPerso.y -= 10;
                        }
                    }
                }

                // Player 2 movement
                if (m.splitScreen && !fighting_p2) {
                    int moved_p2 = 0; // Flag to track if movement was successful
                    if (rightHeld_p2) {
                        posPerso2.x += 5;
                        if (!collision_wall(m.backgroundMask, posPerso2)) {
                            handleScrolling(&m.bg, &posPerso2, 5, 0, 5, 2);
                            moved_p2 = 1;
                        } else {
                            posPerso2.x -= 5; // Revert position if collision
                        }
                    }
                    if (leftHeld_p2) {
                        posPerso2.x -= 5;
                        if (!collision_wall(m.backgroundMask, posPerso2)) {
                            handleScrolling(&m.bg, &posPerso2, -5, 0, 5, 2);
                            moved_p2 = 1;
                        } else {
                            posPerso2.x += 5; // Revert position if collision
                        }
                    }
                    if (walking_p2 && deltaTime > 0 && moved_p2) {
                        walkFrame_p2 = (walkFrame_p2 + 1) % 4;
                    }

                    if (jumping_p2) {
                        jumpStep_p2++;
                        moved_p2 = 0;
                        if (rightHeld_p2) {
                            posPerso2.x += 5;
                            if (!collision_wall(m.backgroundMask, posPerso2)) {
                                handleScrolling(&m.bg, &posPerso2, 5, 0, 5, 2);
                                moved_p2 = 1;
                            } else {
                                posPerso2.x -= 5;
                            }
                        } else if (leftHeld_p2) {
                            posPerso2.x -= 5;
                            if (!collision_wall(m.backgroundMask, posPerso2)) {
                                handleScrolling(&m.bg, &posPerso2, -5, 0, 5, 2);
                                moved_p2 = 1;
                            } else {
                                posPerso2.x += 5;
                            }
                        }
                        if (jumpStep_p2 < 12) {
                            posPerso2.y -= 20;
                            if (collision_wall(m.backgroundMask, posPerso2)) {
                                posPerso2.y += 20;
                                jumping_p2 = 0;
                            }
                        } else {
                            posPerso2.y += 20;
                            if (collision_wall(m.backgroundMask, posPerso2)) {
                                posPerso2.y -= 20;
                                jumping_p2 = 0;
                            }
                        }
                        if (jumpStep_p2 >= 24) {
                            jumping_p2 = 0;
                            posPerso2.y = initialPosY_p2;
                        }
                    } else {
                        posPerso2.y += 10;
                        if (collision_wall(m.backgroundMask, posPerso2)) {
                            posPerso2.y -= 10;
                        }
                    }
                }

                // Update minimap positions
                m.position_bonhomme.x = m.position_minimap.x + (posPerso.x * m.image_minimap->w / m.bg.map_width);
                m.position_bonhomme.y = m.position_minimap.y + (posPerso.y * m.image_minimap->h / m.bg.map_height);
                if (m.splitScreen) {
                    m.position_bonhomme2.x = m.position_minimap2.x + (posPerso2.x * m.image_minimap->w / m.bg.map_width);
                    m.position_bonhomme2.y = m.position_minimap2.y + (posPerso2.y * m.image_minimap->h / m.bg.map_height);
                }

                // Update minimap structure
                m.position_perso = posPerso;
                m.position_perso2 = posPerso2;
                m.facingRight = facingRight;
                m.walking = walking;
                m.jumping = jumping;
                m.fighting = fighting;
                m.walkFrame = walkFrame;
                m.jumpStep = jumpStep;
                m.fightFrame = fightFrame;
                m.facingRight_p2 = facingRight_p2;
                m.walking_p2 = walking_p2;
                m.jumping_p2 = jumping_p2;
                m.fighting_p2 = fighting_p2;
                m.walkFrame_p2 = walkFrame_p2;
                m.jumpStep_p2 = jumpStep_p2;
                m.fightFrame_p2 = fightFrame_p2;

                // Check collisions
                check_collisions(&m, &posPerso, &exploded_rocket, &exploded_barrel, 1);
                if (m.splitScreen) {
                    check_collisions(&m, &posPerso2, &exploded_rocket, &exploded_barrel, 2);
                }

                // Check for level transition
		if (check_level_transition(&m, posPerso) || (m.splitScreen && check_level_transition(&m, posPerso2))) {
		    int savedSplitScreen = m.splitScreen; // Save splitScreen state
		    int savedSkin = m.skin; // Save skin state
		    Liberer(&m);
		    InitialiserMinimap(&m, 2, savedSkin); // Pass saved skin
		    m.splitScreen = savedSplitScreen; // Restore splitScreen state
		    posPerso.x = 0;
		    posPerso.y = 500;
		    posPerso2.x = 0;
		    posPerso2.y = 500;
		    m.position_perso = posPerso;
		    m.position_perso2 = posPerso2;
		    m.bg.camera1.x = 0;
		    m.bg.camera1.y = 0;
		    m.bg.camera2.x = 0;
		    m.bg.camera2.y = 0;
		    m.lives_p1 = 3;
		    m.lives_p2 = 3;
		    m.game_timer = 0.0f;
		    display_countdown(&m, screen);
		    Mix_HaltMusic();
		    if (m.music) {
			if (Mix_PlayMusic(m.music, -1) == -1) {
			    printf("Failed to play music.mp3: %s\n", Mix_GetError());
			}
		    }
		}

                // Check for game over
                if (m.lives_p1 <= 0 || (m.splitScreen && m.lives_p2 <= 0)) {
                    Mix_HaltMusic();
                    if (m.gameover_music) {
                        if (Mix_PlayMusic(m.gameover_music, 0) == -1) {
                            printf("Failed to play gameover.mp3: %s\n", Mix_GetError());
                        }
                    }
                    if (m.image_gameover) {
                        SDL_BlitSurface(m.image_gameover, NULL, screen, &m.position_gameover);
                        SDL_Flip(screen);
                        SDL_Delay(3000);
                    }
                    continuer = 0;
                }
            }

            // Render the scene
            afficher(m, screen, exploded_barrel);

            // Render minimap
            if (m.image_minimap && m.image_bonhomme) {
                SDL_BlitSurface(m.image_minimap, NULL, screen, &m.position_minimap);
                SDL_BlitSurface(m.image_bonhomme, NULL, screen, &m.position_bonhomme);
                if (m.splitScreen && m.image_bonhomme2) {
                    SDL_BlitSurface(m.image_minimap, NULL, screen, &m.position_minimap2);
                    SDL_BlitSurface(m.image_bonhomme2, NULL, screen, &m.position_bonhomme2);
                }
            }

            // Render saved message
            if (m.show_saved && m.image_saved) {
                SDL_BlitSurface(m.image_saved, NULL, screen, &m.position_saved);
            }

            SDL_Flip(screen);
            SDL_Delay(10);
        }

        Liberer(&m);
    }

    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
