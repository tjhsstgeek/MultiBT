#include "bubbletrouble.h"
#include <stdio.h>
#include <stdlib.h>
#include <chipmunk.h>
#include <string.h>

int main() {
	//we need to build all the levels
	struct game *g = game_create();
	game_init_chipmunk(g);
	int a = 0;
	for (;a < 2000;a++) {
		player_create(g,0,0,10,20,0,0);
	}
	g->w = 1024;
	g->h = 800;
	g->name = "network_level.bin";
	FILE *f = fopen("network_level.bin","w");
	game_save_level(f,g);
	fclose(f);
}
