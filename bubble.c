#include "bubbletrouble.h"
#include <stdlib.h>
#include <chipmunk.h>


/**
 * Initialize a bubble that will bounce around. 
 * \param b The bubble to initialize.
 * \param x The x location of the bubble.
 * \param y The y location of the bubble.
 * \param v_x The x velocity of the bubble.
 * \param v_y The y velocity of the bubble.
 * \param r The radius of the bubble.
 * \param l The bubble's life. 
 */
struct bubble *bubble_init(struct bubble *b, double x, double y, double v_x,
                           double v_y, double r, uint8_t l) {
	cpBody *body = cpBodyInit(&b->body, r,
	                          cpMomentForCircle(r, 0, r, cpv(0, 0)));
	if (!body) {
		ERR_ERRNO();
		return 0;
	}
	cpShape *shape = cpCircleShapeInit(&b->shape, body, r, cpv(0,0));
	if (!shape) {
		cpBodyDestroy(body);
		ERR_ERRNO();
		return 0;
	}
	cpBodySetVel(body, cpv(v_x, v_y));
	cpBodySetPos(body, cpv(x, y));
	shape->e = 1.0;
	shape->data = b;
	shape->collision_type = BUBBLE;
	b->l = l;
	return b;
}
/**
 * Create a bubble that will bounce around the screen.
 * \param g The game in which to create the bubble.
 * \param x The x location of the bubble.
 * \param y The y location of the bubble.
 * \param v_x The x velocity of the bubble.
 * \param v_y The y velocity of the bubble.
 * \param r The radius of the bubble.
 * \param l The bubble's life. 
 */
struct bubble *bubble_create(double x, double y, double v_x, double v_y,
                             double r, uint8_t l) {
	struct bubble *b = calloc(1, sizeof(struct bubble));
	if (!b) {
		ERR_ERRNO();
		return 0;
	}
	return bubble_init(b, x, y, v_x, v_y, r, l);
}
/**
 * Add the bubble into the game.
 */
int bubble_add(struct bubble *b, struct game *g) {
	if (linkedlist_add_last(g->b, b)) {
		ERR_TRACE();
		return -1;
	}
	b->node = g->b->last;
	cpSpaceAddBody(g->cp, &b->body);
	cpSpaceAddShape(g->cp, &b->shape);
	return 0;
}
/**
 * Frees any internal memory of the bubble.
 */
void bubble_destroy(struct bubble *b) {
	cpShapeDestroy(&b->shape);
	cpBodyDestroy(&b->body);
}
/**
 * Frees any memory used by the bubble.
 * Frees the internal and the struct memory.
 */
void bubble_free(struct bubble *b) {
	bubble_destroy(b);
	free(b);
}
/**
 * Remove the bubble from the game.
 */
void bubble_remove(struct bubble *b, struct game *g) {
	cpSpaceRemoveShape(g->cp, &b->shape);
	cpSpaceRemoveBody(g->cp, &b->body);
	linkedlist_remove_node(g->b, b->node);
}
/**
 * Pop the bubble splitting it into two bubbles.
 */
int bubble_pop(struct bubble *b, struct game *g) {
	if (b->l <= 1) {
		bubble_destroy(b);
		return 0;
	}
	double radius = cpCircleShapeGetRadius(&b->shape);
	cpVect pos = cpBodyGetPos(&b->body);
	cpVect vel = cpBodyGetVel(&b->body);
	bubble_remove(b, g);
	bubble_destroy(b);
	b = bubble_init(b, pos.x, pos.y, vel.x + 5, vel.y + 5,
	                radius / 2, b->l - 1);
	if (!b) {
		ERR_TRACE();
		return -1;
	}
	//b->events = b->events;
	if (bubble_add(b, g)) {
		ERR_TRACE();
		return -1;
	}
	struct bubble *c = bubble_create(pos.x, pos.y, vel.x - 5, vel.y + 5,
	                          radius / 2, b->l - 1);
	if (!c) {
		ERR_TRACE();
		return -1;
	}
	//c->events = b->events;
	if (bubble_add(c, g)) {
		ERR_TRACE();
		return -1;
	}
	return 0;
}
int bubble_bubble_collision(cpArbiter *arb, struct cpSpace *space, void *data) {
	struct game *g = data;
	if (g->data & 1) {
		return 1;
	} else {
		return 0;
	}
}
int bubble_popper_collision(cpArbiter *arb, struct cpSpace *space, void *data) {
	//cpCircleShapeGetRadius
}
