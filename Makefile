minimap: main.c source.c header.h
	gcc main.c source.c -o minimap -lSDL -lSDL_image -lSDL_gfx -lSDL_mixer -lSDL_ttf -g

clean:
	rm -f minimap

