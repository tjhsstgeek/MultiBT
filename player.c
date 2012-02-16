#include "bubbletrouble.h"
#include <stdlib.h>
#include <chipmunk.h>

/**
 * Initializes a new player.
 * The player is placed in the queue of available players.
 */
struct player *player_init(struct player *p, struct game *g, double x, double y,
                           double w, double h, uint32_t score, uint8_t data) {
	cpVect all[4] = {cpv(0,0), cpv(0,h), cpv(w,h), cpv(w,0)};
	cpBody *body = cpBodyInit(&p->body, 10, cpMomentForBox(10, w, h));
	if (!body) {
		ERR_ERRNO();
		return 0;
	}
	cpBodySetPos(body, cpv(x,y));
	//cpShape *shape = cpPolyShapeNew(body,4,all,cpv((p->l+p->r)/2.0,(p->b+p->t)/2.0));
	cpShape *shape = cpPolyShapeInit(&p->shape, body, 4, all, cpv(0, 0));
	if (!shape) {
		ERR_ERRNO();
		cpBodyDestroy(body);
		return 0;
	}
	shape->data = p;
	shape->collision_type = PLAYER;
	if (linkedlist_add_last(g->p_q, p)) {
		ERR_TRACE();
		cpBodyDestroy(body);
		cpShapeDestroy(shape);
		return 0;
	}
	p->x = x;
	p->y = y;
	p->node = g->p_q->last;
	return p;
}
/**
 * Creates a new player by allocating memory and then initializing it. 
 * \sa player_init
 */
struct player *player_create(struct game *g, double x, double y, double w,
                             double h, uint32_t score, uint8_t data) {
	struct player *p = calloc(1, sizeof(struct player));
	if (!p) {
		ERR_ERRNO();
		return 0;
	}
	return player_init(p, g, x, y, w, h, score, data);
}
/**
 * Reset the player's position to the starting values.
 */
void player_respawn(struct player *p) {
	cpBodySetPos(&p->body, cpv(p->x, p->y));
}
/**
 * Adds a player object to the grid.
 * This means moving him from the queue to the playing list.
 */
int player_add(struct player *p, struct game *g) {
	if (p->list != g->p_q) {
		//Player already used
		return -2;
	}
	if (linkedlist_add_last(g->p_p, p)) {
		ERR_TRACE();
		return -1;
	}
	linkedlist_remove_node(g->p_q, p->node);
	p->node = g->p_p->last;
	p->list = g->p_p;
	cpSpaceAddBody(g->cp, &p->body);
	cpSpaceAddShape(g->cp, (struct cpShape *)&p->shape);
	return 0;
}
/**
 * Removes a player object from the grid.
 * This means moving him from the playing list to the queue.
 */
int player_remove(struct player *p, struct game *g) {
	if (p->list != g->p_p) {
		//The player isn't in the playing list
		return -2;
	}
	if (linkedlist_add_last(g->p_q, p)) {
		ERR_TRACE();
		return -1;
	}
	linkedlist_remove_node(g->p_p, p->node);
	cpSpaceRemoveShape(g->cp, (struct cpShape *)&p->shape);
	cpSpaceRemoveBody(g->cp, &p->body);
	p->node = g->p_q->last;
	return 0;
}
/**
 * Free up any internal memory. 
 */
void player_destroy(struct player *p) {
	if (p->client)
		p->client->p = 0;
	cpShapeDestroy((struct cpShape *)&p->shape);
	cpBodyDestroy(&p->body);
}
/**
 * Frees all memory used by a player object.
 */
void player_free(struct player *p) {
	player_destroy(p);
	free(p);
}
int player_hit(struct player *p, struct game *g) {
	if (player_remove(p, g)) {
		ERR_TRACE();
		return -1;
	}
	if (!p->lives) {
		player_respawn(p);
		if (player_add(p, g)) {
			ERR_TRACE();
			return -1;
		}
	} else {
		if (!--p->lives) {
			SDLNet_TCP_Send(p->client->s, "\11", 1);
			player_free(p);
		} else {
			player_respawn(p);
			if (player_add(p, g)) {
				ERR_TRACE();
				return -1;
			}
		}
	}
	return 0;
}
void player_remove_collision(struct cpSpace *space, void *key, void *data) {
	if (player_hit(key,data)) {
		ERR_TRACE();
	}
}
int player_bubble_collision(cpArbiter *arb, struct cpSpace *space, void *data) {
	CP_ARBITER_GET_SHAPES(arb, shape_a, shape_b);
	struct player *p = shape_b->data;
	cpSpaceAddPostStepCallback(space, player_remove_collision, p, data);
	return 0;
}
