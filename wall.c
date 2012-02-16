#include "bubbletrouble.h"
#include <stdlib.h>
#include <chipmunk.h>

struct wall *wall_init(struct wall *wa, struct game *g, double l, double r,
                         double b, double t) {
	cpVect all[4] = {cpv(l,t), cpv(r,t), cpv(r,b), cpv(l,b)};
	//cpShape *shape = cpPolyShapeNew(body,4,all,cpv((wa->l+wa->r)/2.0,(wa->b+wa->t)/2.0));
	cpShape *shape = cpPolyShapeInit(&wa->shape, &g->cp->staticBody, 4, all, cpvzero);
	shape->data = wa;
	shape->e = 1.0;
	shape->collision_type = WALL;
	return wa;
}
struct wall *wall_create(struct game *g, double l, double r, double b, double t) {
	struct wall *wa = calloc(1, sizeof(struct wall));
	if (!wa) {
		ERR_ERRNO();
		return 0;
	}
	return wall_init(wa, g, l, r, b, t);
}
void wall_add(struct wall *w, struct game *g) {
	cpSpaceAddStaticShape(g->cp, &w->shape);
}
void wall_remove(struct wall *w, struct game *g) {
	cpSpaceRemoveStaticShape(g->cp, &w->shape);
}
void wall_destroy(struct wall *w) {
	cpShapeDestroy(&w->shape);
}
void wall_free(struct wall *w) {
	wall_destroy(w);
	free(w);
}
