#include "bubbletrouble.h"
#include <stdlib.h>
#include <chipmunk.h>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <time.h>

char *network_read_string_1(TCPsocket sock) {
	uint8_t len;
	SDLNet_TCP_Recv(sock,&len,1);
	int read = 0;
	char *text = malloc(len+1);
	while (read != len) {
		int more = SDLNet_TCP_Recv(sock,text,len-read);
		if (more < 1)
			return 0;
		read += more;
	}
	text[len] = 0;
	return text;
}
void network_game_leave(struct network *n, struct network_client *nc) {
	SDL_mutexP(n->m);
	if (nc->p)
		nc->p->client = 0;
	linkedlist_remove_object(nc->g->clients,nc);
	nc->g = 0;
	SDL_mutexV(n->m);
}
void network_leave(struct network *n, struct network_client *nc) {
	SDL_mutexP(n->m);//can fail
	if (nc->p)
		nc->p->client = 0;
	if (nc->g)
		linkedlist_remove_object(nc->g->clients,nc);
	nc->g = 0;
	linkedlist_remove_object(n->users,nc);
	SDL_mutexV(n->m);
}
struct game *network_create_game(struct network *n, FILE *f) {
	struct game *g = game_load_level(f);
	if (!g) {
		return 0;
	}
	g->ident = rand();
	printf("%i\n",g->ident);
	g->n = n;
	uint8_t name_len = 0;
	if (g->name)
		name_len = strlen(g->name);
	char *data = malloc(6 + name_len);
	*data = 5;
	*(data + 1) = name_len;
	*(uint32_t *)(data + 2) = htonl(g->ident);
	memcpy(data+6,g->name,name_len);

	SDL_mutexP(n->m);//can fail
	linkedlist_add_last(n->games,g);
	linkedlist_node *nn = n->users->first;
	for (;nn;nn = nn->next) {
		struct network_client *nc = nn->data;
		SDLNet_TCP_Send(nc->s, data, name_len + 6);
	}
	SDL_mutexV(n->m);
	free(data);
	return g;
}
uint32_t network_join_game(struct network *n, struct network_client *nc,
                           int ident) {
	//Lock mutex to prevent concurrency issues
	SDL_mutexP(n->m);
	linkedlist_node *nn = n->games->first;
	for (;nn;nn = nn->next) {
		struct game *g = nn->data;
		if (g->ident == ident) {
			break;
		}
	}
	if (nn) {
		struct game *g = nn->data;
		//Can't have more people than possible slots.
		if (g->p_q->len > g->clients->len) {
			nc->g = g;
			linkedlist_add_last(g->clients, nc);
			SDL_mutexV(n->m);
		} else {
			SDL_mutexV(n->m);
			return 1;
		}
	} else {
		SDL_mutexV(n->m);
		return 2;
	}
	return 0;
}
uint32_t network_start_game(struct game *g) {
	game_start_running(g);
}
int network_listen(void *v) {
	struct network_client *nc = v;
	TCPsocket sock = nc->s;
	struct network *n = nc->n;
	uint8_t command;
	while (1) {
		if (SDLNet_TCP_Recv(sock,&command,1) < 1) {
			network_leave(n,nc);
			free(nc);
			return 1;
		}
		switch (command) {
			case 1:;//login
				char *pass = network_read_string_1(sock);
				if (!pass)
					break;
				if (!strcmp(pass,"926452")) {
					nc->in = 1;
					SDLNet_TCP_Send(sock,"\1\0",2);
				} else {
					SDLNet_TCP_Send(sock,"\1\1",2);
				}
				free(pass);
				break;
			case 2:;//Create a game 
				char *name = network_read_string_1(sock);
				if (!name) {
					break;
				}
				if (!nc->in) {
					SDLNet_TCP_Send(sock,"\2\1",2);
					break;
				}
				FILE *f = fopen(name, "r");
				free(name);
				if (!f) {
					SDLNet_TCP_Send(sock,"\2\2",2);
					break;
				}
				struct game *g = network_create_game(n, f);
				fclose(f);
				if (!g) {
					SDLNet_TCP_Send(sock,"\2\3",2);
					break;
				}
				SDLNet_TCP_Send(sock,"\2\0",2);
				break;
			case 3://Start the game (must be creator)
				if (!nc->in) {
					SDLNet_TCP_Send(sock,"\3\3",2);
					break;
				}
				if (!nc->g) {
					SDLNet_TCP_Send(sock,"\3\1",2);
					break;
				}
				if (nc->g->timer) {
					SDLNet_TCP_Send(sock,"\3\2",2);
					break;
				}
				network_start_game(nc->g);
				SDLNet_TCP_Send(sock,"\3\0",2);
				break;
			case 4:;//Join a game
				uint32_t ident;
				SDLNet_TCP_Recv(sock,&ident,4);
				ident = ntohl(ident);
				if (network_join_game(n,nc,ident)) {
					SDLNet_TCP_Send(sock,"\4\1",2);
					break;
				}
				SDLNet_TCP_Send(sock,"\4\0",2);
				break;
			case 5://list game, [client]
				break;
			case 6://now playing, [client]
				break;
			case 7://game over, [client]
				break;
			case 8://data, [client]
				break;
			case 9:;//Action. See player.data for possible transmitted data
				uint8_t dir;
				SDLNet_TCP_Recv(sock, &dir, 1);
				if (nc->p) {
					nc->p->data = (nc->p->data & ~0xf) | (dir & 0xf);
				}
				break;
			case 10://map list, [client]
				break;
			case 11://Player died
				break;
			/*case 9://move right
				printf("Right\n");
				if (nc->p) {
					//cpBodySetVel(&nc->p->body,cpv(40, 0));
					nc->p->data |= 0x2;
				}
				break;
			case 10://move left
				printf("Left\n");
				if (nc->p) {
					//cpBodySetVel(&nc->p->body,cpv(-40, 0));
					nc->p->data |= 0x1;
				}
				break;
			case 11://stop
				printf("Stop\n");
				if (nc->p) {
					cpBodySetVel(&nc->p->body, cpv(0, 0));
					nc->p->data |= 0x3;
				}
				break;*/
		}
	}
}
int network_join() {
	
}
int network_create() {
	struct network *n = malloc(sizeof(struct network));
	if (!n)
		return 1;
	n->users = linkedlist_create();
	if (!n->users) {
		free(n);
		return 1;
	}
	n->games = linkedlist_create();
	if (!n->games) {
		free(n->users);
		free(n);
		return 1;
	}
	n->m = SDL_CreateMutex();//I think this doesn't fail
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, 0, 3737)) {
		free(n->games);
		free(n->users);
		free(n);
		return 1;
	}
	TCPsocket m = SDLNet_TCP_Open(&ip);
	if (!m) {
		free(n->games);
		free(n->users);
		free(n);
		return 1;
	}
	while (1) {
		TCPsocket temp = SDLNet_TCP_Accept(m);
		if (temp) {
			struct network_client *nc = malloc(sizeof(struct network_client));
			if (!nc) {
				//No more memory
				SDLNet_TCP_Close(temp);
				continue;
			}
			memset(nc, 0, sizeof(struct network_client));
			nc->s = temp;
			nc->n = n;
			linkedlist_add_last(n->users,nc);
			//Send over the list of games, and their names
			linkedlist_node *ga = n->games->first;
			for (;ga;ga = ga->next) {
				struct game *g = ga->data;
				uint8_t name_len = 0;
				if (g->name) {
					uint32_t len = strlen(g->name);
					if (len > 0xff)
						name_len = 0xff;
					else
						name_len = len;
				}
				uint8_t data [262];//256 + 6
				*data = 5;
				*(data + 1) = name_len;
				//Copy the game identification
				*(uint32_t *)(data + 2) = htonl(g->ident);
				//Copy the name
				memcpy(data + 6, g->name, name_len);
				SDLNet_TCP_Send(temp, data, name_len + 6);
			}
			SDL_CreateThread(network_listen, nc);
		}
		SDL_Delay(500);
	}
	return 0;
}
int network_start() {
	srand(time(0));
	if (SDL_Init(SDL_INIT_TIMER))
		return 1;
	if (SDLNet_Init())
		return 1;
	return 0;
}
