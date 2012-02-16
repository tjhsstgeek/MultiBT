#include "bubbletrouble.h"
#include <stdio.h>
#include <stdlib.h>
#include <chipmunk.h>
#include <string.h>
struct game *game_create() {
	struct game *g = malloc(sizeof(struct game));
	memset(g, 0, sizeof(struct game));
	g->b = linkedlist_create();
	g->p_q = linkedlist_create();
	g->p_p = linkedlist_create();
	g->clients = linkedlist_create();
	//g->m = SDL_CreateMutex();
	return g;
}
/*void game_create_border(game *g,uint32_t width,uint32_t height) {
	cpSpaceAddStaticShape(g->cp, cpSegmentShapeNew(&g->cp->staticBody, cpv(0,0), cpv(width,0), 0.0f))->e = 1.0f;
	cpSpaceAddStaticShape(g->cp, cpSegmentShapeNew(&g->cp->staticBody, cpv(0,0), cpv(0,height), 0.0f))->e = 1.0f;
	cpSpaceAddStaticShape(g->cp, cpSegmentShapeNew(&g->cp->staticBody, cpv(width,0), cpv(width,height), 0.0f))->e = 1.0f;
}*/
void game_safe_cleanup(struct game *g) {
	if (g->name) {
		free(g->name);
	}
	if (g->l) {
		int a = 0;
		for (;a < g->w_len;a++) {
			if (g->l[a]) {
				wall_remove(g->l[a], g);
				wall_destroy(g->l[a]);
			} else {
				break;
			}
		}
	}
	if (g->b) {
		linkedlist_node *n = g->b->first;
		while (n) {
			struct bubble *b = n->data;
			bubble_remove(b, g);
			bubble_destroy(b);
			linkedlist_node *t = n->next;
			free(n);
			n = t;
		}
		free(g->b);
	}
	if (g->p_p) {
		linkedlist_node *n = g->p_p->first;
		while (n) {
			struct player *b = n->data;
			player_remove(b, g);
			player_destroy(b);
			linkedlist_node *t = n->next;
			free(n);
			n = t;
		}
		free(g->p_p);
	}
	if (g->p_q) {
		linkedlist_node *n = g->p_q->first;
		while (n) {
			struct player *b = n->data;
			player_destroy(b);
			linkedlist_node *t = n->next;
			free(n);
			n = t;
		}
		free(g->p_q);
	}
	if (g->cp)
		cpSpaceDestroy(g->cp);
	free(g);
}
#define LOAD_DIE(res,g) if (!(res)) { \
printf("Died at line %i in file %s\n",__LINE__,__FILE__); \
game_safe_cleanup(g); \
return 0; \
}
struct game *game_load_level(FILE *f) {
	struct game *g = malloc(sizeof(struct game));
	if (!g) {
		ERR_ERRNO();
		return 0;
	}
	memset(g, 0, sizeof(struct game));
	game_init_chipmunk(g);
	uint8_t name_len;
	LOAD_DIE(fread(&name_len, 1, 1, f), g);
	if (name_len) {
		g->name = malloc(name_len + 1);
		if (!g->name) {
			free(g);
			ERR_ERRNO();
			return 0;
		}
		LOAD_DIE(fread(g->name,name_len,1,f),g);
		g->name[name_len] = 0;
	}

	LOAD_DIE(fread(&g->w, 4, 1, f),g);
	LOAD_DIE(fread(&g->h, 4, 1, f),g);
	uint32_t b_len,p_len;
	LOAD_DIE(fread(&g->w_len, 2, 1, f), g);
	LOAD_DIE(fread(&b_len, 2, 1, f), g);
	LOAD_DIE(fread(&p_len, 2, 1, f), g);
	LOAD_DIE(fread(&g->step, 4, 1, f), g);
	g->w = ntohl(g->w);
	g->h = ntohl(g->h);
	g->w_len = ntohs(g->w_len);
	p_len = ntohs(p_len);
	b_len = ntohs(b_len);
	g->step = ntohs(g->step);
	g->l = malloc(sizeof(struct wall *) * g->w_len);
	if (!g->l) {
		ERR_ERRNO();
		if (g->name) {
			free(g->name);
		}
		free(g);
		return 0;
	}
	memset(g->l,0,sizeof(struct wall *) * g->w_len);
	int a = 0;
	for (;a < g->w_len;a++) {
		uint32_t l,r,b,t;
		LOAD_DIE(fread(&l, 4, 1, f),g)
		LOAD_DIE(fread(&r, 4, 1, f),g);
		LOAD_DIE(fread(&b, 4, 1, f),g);
		LOAD_DIE(fread(&t, 4, 1, f),g);
		l = ntohl(l);
		r = ntohl(r);
		b = ntohl(b);
		t = ntohl(t);
		LOAD_DIE((l < r) && (b < t), g);
		struct wall *wa = wall_create(g, l, r, b, t);
		if (!wa) {
			ERR_TRACE();
			game_safe_cleanup(g);
			return 0;
		}
		g->l[a] = wa;
		wall_add(wa, g);
	}
	g->b = linkedlist_create();
	if (!g->b) {
		ERR_TRACE();
		game_safe_cleanup(g);
		return 0;
	}
	a = 0;
	for (;a < b_len;a++) {
		uint32_t v_x,v_y,x,y,r;
		uint8_t l;
		LOAD_DIE(fread(&v_x,4,1,f),g);
		LOAD_DIE(fread(&v_y,4,1,f),g);
		LOAD_DIE(fread(&x,4,1,f),g);
		LOAD_DIE(fread(&y,4,1,f),g);
		LOAD_DIE(fread(&r,4,1,f),g);
		LOAD_DIE(fread(&l,1,1,f),g);
		v_x = ntohl(v_x);
		v_y = ntohl(v_y);
		x = ntohl(x);
		y = ntohl(y);
		r = ntohl(r);
		struct bubble *b = bubble_create(x, y, v_x, v_y, r, l);
		if (!b) {
			ERR_TRACE();
			game_safe_cleanup(g);
			return 0;
		}
		bubble_add(b, g);
	}
	g->p_p = linkedlist_create();
	if (!g->p_p) {
		ERR_TRACE();
		game_safe_cleanup(g);
		return 0;
	}
	g->p_q = linkedlist_create();
	if (!g->p_q) {
		ERR_TRACE();
		game_safe_cleanup(g);
		return 0;
	}
	a = 0;
	for (;a < p_len;a++) {
		uint32_t x,y,w,h,score;
		uint8_t data;
		LOAD_DIE(fread(&x,4,1,f),g);
		LOAD_DIE(fread(&y,4,1,f),g);
		LOAD_DIE(fread(&w,4,1,f),g);
		LOAD_DIE(fread(&h,4,1,f),g);
		LOAD_DIE(fread(&score,4,1,f),g);
		LOAD_DIE(fread(&data,1,1,f),g);
		x = ntohl(x);
		y = ntohl(y);
		w = ntohl(w);
		h = ntohl(h);
		score = ntohl(score);
		struct player *p =player_create(g, x, y, w, h, score, data);
		if (!p) {
			ERR_TRACE();
			game_safe_cleanup(g);
			return 0;
		}
	}
	g->clients = linkedlist_create();
	if (!g->clients) {
		ERR_TRACE();
		game_safe_cleanup(g);
		return 0;
	}
	//game_create_border(g,g->w,g->h);
	return g;
}
uint8_t game_save_level(FILE *f, struct game *g) {
	if (g->name) {
		uint8_t a = strlen(g->name);
		fwrite(&a, 1, 1, f);
		fwrite(g->name, a, 1, f);
	} else {
		uint8_t a = 0;
		fwrite(&a, 1, 1, f);
	}
	uint32_t _w = htonl(g->w);
	uint32_t _h = htonl(g->h);
	uint16_t _wlen = htons(g->w_len);
	uint16_t _blen = htons(g->b->len);
	uint16_t _plen = htons(g->p_q->len);
	uint32_t _step = htonl(g->step);
	fwrite(&_w, 4, 1, f);
	fwrite(&_h, 4, 1, f);
	fwrite(&_wlen, 2, 1, f);
	fwrite(&_blen, 2, 1, f);
	fwrite(&_plen, 2, 1, f);
	fwrite(&_step, 4, 1, f);
	int a = 0;
	for (;a < g->w_len;a++) {
		struct wall *wa = g->l[a];
		cpVect tl = cpPolyShapeGetVert((struct cpShape *)&wa->shape,0);
		cpVect br = cpPolyShapeGetVert((struct cpShape *)&wa->shape,2);
		uint32_t l = htonl((uint32_t)tl.x);
		uint32_t r = htonl((uint32_t)br.x);
		uint32_t b = htonl((uint32_t)br.y);
		uint32_t t = htonl((uint32_t)tl.y);
		fwrite(&l, 4, 1, f);
		fwrite(&r, 4, 1, f);
		fwrite(&b, 4, 1, f);
		fwrite(&t, 4, 1, f);
	}
	linkedlist_node *n = g->b->first;
	for (;n;n = n->next) {
		struct bubble *b = n->data;
		cpVect pos = cpBodyGetPos((struct cpBody *)&b->body);
		cpVect vel = cpBodyGetVel((struct cpBody *)&b->body);
		double radius = cpCircleShapeGetRadius((struct cpShape *)&b->shape);
		uint32_t v_x = htonl((uint32_t)vel.x);
		uint32_t v_y = htonl((uint32_t)vel.y);
		uint32_t x = htonl((uint32_t)pos.x);
		uint32_t y = htonl((uint32_t)pos.y);
		uint32_t r = htonl((uint32_t)radius);
		fwrite(&v_x, 4, 1, f);
		fwrite(&v_y, 4, 1, f);
		fwrite(&x, 4, 1, f);
		fwrite(&y, 4, 1, f);
		fwrite(&r, 4, 1, f);
		fwrite(&b->l, 1, 1, f);
	}
	n = g->p_q->first;
	for (;n;n = n->next) {
		struct player *p = n->data;
		cpVect pos = cpBodyGetPos((struct cpBody *)&p->body);
		cpVect tr = cpPolyShapeGetVert((struct cpShape *)&p->shape, 2);
		uint32_t x = htonl((uint32_t)pos.x);
		uint32_t y = htonl((uint32_t)pos.y);
		uint32_t w = htonl((uint32_t)tr.x);
		uint32_t h = htonl((uint32_t)tr.y);
		uint32_t s = htonl(p->score);
		fwrite(&x, 4, 1, f);
		fwrite(&y, 4, 1, f);
		fwrite(&w, 4, 1, f);
		fwrite(&h, 4, 1, f);
		fwrite(&s, 4, 1, f);
		fwrite(&p->data, 1, 1, f);
	}
	return 0;
}
void game_remove_bubble(struct game *g, struct bubble *b) {
	linkedlist_remove_object(g->b,b);
}
void game_add_bubble(struct game *g, struct bubble *b) {
	linkedlist_add_last(g->b,b);
}
void game_init_chipmunk(struct game *g) {//The physics handler
	cpInitChipmunk();
	cpSpace *s = cpSpaceNew();
	cpSpaceAddCollisionHandler(s, BUBBLE, PLAYER, player_bubble_collision,
	                           0, 0, 0, g);
	cpSpaceAddCollisionHandler(s, BUBBLE, BUBBLE, bubble_bubble_collision,
	                           0, 0, 0, g);
	//cpSpaceSetDefaultCollisionHandler(s,game_test,0,0,0,g);
	g->cp = s;
	s->gravity = cpv(0, -10);
}
char *game_gen_buf(struct game *g, uint32_t *len) {
	uint32_t l = 1;
	l += 2 + g->p_p->len * 16;//4(x) + 4(y) + 4(w) + 4(h)
	l += 2 + g->b->len * 13;//4(x) + 4(y) + 4(r) + 1(l)
	l += 2 + g->w_len * 16;//4(x) + 4(y) + 4(w) + 4(h)
	*len = l;
	char *buf = malloc(l);
	if (!buf) {
		ERR_ERRNO();
		return 0;
	}
	char *write = buf;
	*write++ = 8;
	*(uint16_t *)write = htons((uint16_t)g->p_p->len);
	write += 2;
	*(uint16_t *)write = htons((uint16_t)g->b->len);
	write += 2;
	*(uint16_t *)write = htons((uint16_t)g->w_len);
	write += 2;
	linkedlist_node *n = g->p_p->first;
	for (;n;n = n->next) {
		struct player *p = n->data;
		printf("%i %p\n", p->data, p);
		cpVect pos = cpBodyGetPos(&p->body);
		cpVect tr = cpPolyShapeGetVert((struct cpShape *)&p->shape, 2);
		uint32_t x = htonl((uint32_t)pos.x);
		uint32_t y = htonl((uint32_t)pos.y);
		uint32_t w = htonl((uint32_t)tr.x);
		uint32_t h = htonl((uint32_t)tr.y);
		*(uint32_t *)write = x;
		write += 4;
		*(uint32_t *)write = y;
		write += 4;
		*(uint32_t *)write = w;
		write += 4;
		*(uint32_t *)write = h;
		write += 4;
	}
	n = g->b->first;
	for (;n;n = n->next) {
		struct bubble *b = n->data;
		cpVect pos = cpBodyGetPos(&b->body);
		double radius = cpCircleShapeGetRadius((struct cpShape *)&b->shape);
		uint32_t x = htonl((uint32_t)pos.x);
		uint32_t y = htonl((uint32_t)pos.y);
		uint32_t r = htonl((uint32_t)radius);
		*(uint32_t *)write = x;
		write += 4;
		*(uint32_t *)write = y;
		write += 4;
		*(uint32_t *)write = r;
		write += 4;
		*write++ = b->l;
	}
	int a = 0;
	for (;a < g->w_len;a++) {
		struct wall *wa = g->l[a];
		cpVect tl = cpPolyShapeGetVert((struct cpShape *)&wa->shape, 0);
		cpVect br = cpPolyShapeGetVert((struct cpShape *)&wa->shape, 2);
		uint32_t l = htonl((uint32_t)tl.x);
		uint32_t r = htonl((uint32_t)br.x);
		uint32_t b = htonl((uint32_t)br.y);
		uint32_t t = htonl((uint32_t)tl.y);
		*(uint32_t *)write = l;
		write += 4;
		*(uint32_t *)write = r;
		write += 4;
		*(uint32_t *)write = b;
		write += 4;
		*(uint32_t *)write = t;
		write += 4;
	}
	return buf;
}
int game_leave(struct game *g, struct network_client *nc) {
	if (linkedlist_add_first(g->p_q, nc->p)) {
		ERR_TRACE();
		return -1;
	}
	linkedlist_remove_node(nc->p->node);
	nc->p->client = 0;
	nc->p = 0;
	return 0;
}
struct player *game_join(struct game *g, struct network_client *nc) {
	if (g->p_q->len) {
		struct player *p = g->p_q->last;
		if (linkedlist_add_first(g->p_p, p)) {
			ERR_TRACE();
			return 0;
		}
		linkedlist_remove_last(g->p_q);
		p->client = nc;
		return p;
	}
	return 0;
}

uint32_t game_step(uint32_t time, struct game *g) {
	/*int a = 0;
	for (;a < g->p_len;a++) {
		player *p = g->p[a];
		player_step(p,g);
	}*/
	//linkedlist_node *n = g->b->first;
	/*printf("S\n");
	for(;n;n = n->next) {
		bubble *b = n->data;
		cpVect pos = cpBodyGetPos(b->body);
		printf("%f %f\n",pos.x,pos.y);
	}*/
	cpSpaceStep(g->cp, 0.03);
	//Stuff in between steps happens here
	uint32_t len;
	char *buf = game_gen_buf(g, &len);
	linkedlist_node *client = g->clients->first;
	for (;client;client = client->next) {
		struct network_client *nc = client->data;
		if (nc->p) {
			if (nc->left) {
				if (game_leave(g, nc)) {
					ERR_TRACE();
					SDLNet_TCP_Send(nc->s, "\12", 1);
				}
				continue;
			}
			int vx = 0;
			uint32_t vy = 0;
			if (nc->p->data & 1)
				vx -= 10;
			if (nc->p->data & 2)
				vx += 10;
			if (nc->p->data & 4)
				vy += 10;
			cpBodySetVel(&nc->p->body, cpv(vx, vy));
		} else {
			if (!nc->observe) {
				if (game_join(g, nc)) {
					ERR_TRACE();
					SDLNet_TCP_Send(nc->s, "\12", 1);
				}
			}
		}
		SDLNet_TCP_Send(nc->s, buf, len);
	}
	free(buf);
	return 1;
}

void game_next_level(struct game *g) {
	
}
/*player *game_use_free_player(game *g) {
	if (g->p_q->len) {
		player *p = g->p_q->last;
		if (p->client)
			return 0;
		linkedlist_remove_last(g->p_q);
		linkedlist_add_first(g->p_q,p);
		return p;
	}
	return 0;
}
void game_return_player(game *g,player *p) {
	linkedlist_remove_node(g->p_p,p->node);
	linkedlist_add_last(g->p_q,p);
	p->node = g->p_q->last;
}*/
void game_stop_running(struct game *g) {
	if (g->timer)
		SDL_RemoveTimer(g->timer);
	g->timer = 0;
	SDL_mutexP(g->n->m);//We are changing things the network depends on
	linkedlist_node *q = g->p_p->first;
	while (q) {
		struct player *p = q->data;
		struct network_client *nc = p->client;
		if (nc) {
			nc->p = 0;
			if (nc->left) {
				//Remove them from the game.
			}
		}
		p->client = 0;
		SDLNet_TCP_Send(nc->s,"\7",1);
		q = q->next;
		player_remove(p,g);
	}
	SDL_mutexV(g->n->m);
}
uint32_t game_start_running(struct game *g) {
	//random
	int a = 0;
	for (;a < 10;a++) {
		bubble_add(bubble_create(rand() % 512, rand() % 512,
		                         rand() % 10, 0, 10, 1), g);
	}
	//Add the queued players, so lock 
	SDL_mutexP(g->n->m);
	if (!g->clients->len) {
		return 1;
	}
	uint32_t len = 0;
	char *buf = game_gen_buf(g, &len);//A network buffer with the command
	linkedlist_node *client = g->clients->first;
	linkedlist_node *q = g->p_q->first;
	for (;q && client;client = client->next) {
		struct network_client *nc = client->data;
		SDLNet_TCP_Send(nc->s, "\6", 1);
		if (nc->observe) {
			continue;
		}
		struct player *p = q->data;
		p->client = nc;
		nc->p = p;
		SDLNet_TCP_Send(nc->s, buf, len);
		q = q->next;
		player_add(p, g);
	}
	free(buf);
	g->p_q->first = q;
	if (!q)
		g->p_q->last = 0;
	g->timer = SDL_AddTimer(30,game_step,g);
	if (!g->timer) {
		game_stop_running(g);//Double lock, double release
		return 1;
	}
	SDL_mutexV(g->n->m);
	return 0;
}
/*int main() {
	game *g = game_create();
	g->data = 1;
	g->step = 30;
	game_init_chipmunk(g);
	game_create_border(g,1024,800);
	g->cp->gravity = cpv(0,-9.81);
	int b = 0;
	for (;b < 20;b++) {
		bubble *test1 = bubble_create(g,500,500,0,0,10,2);
		bubble_add(test1,g);
	}
	//bubble *test2 = bubble_create(g,500,400,0,100,20,3);
	//bubble_add(test2,g);
	g->l = malloc(sizeof(wall *) * 1);
	g->l[0] = wall_create(g,400,600,400,450);
	//wall_add(g->l[0],g);
	cpSpaceRehashStatic(g->cp);
	int a = 0;
	for (;a < 1000;a++)
	game_step(g);
}*/
/*int main() {
	FILE *f = fopen("network.c","r");
	game *g = game_load_level(f);
}*/
