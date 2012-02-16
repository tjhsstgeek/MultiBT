#ifndef __BUBBLETROUBLE_H__
#define __BUBBLETROUBLE_H__ 1
struct _bubble;
struct _bubble_event;
#include <stdint.h>
#include "linkedlist.h"
#include <chipmunk.h>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <stdio.h>
#include <errno.h>

#define BUBBLE 1
#define WALL 2
#define PLAYER 3

typedef struct _bubble_event {
	void (*created)(struct _bubble_event *,struct _bubble *);
	void (*destroyed)(struct _bubble_event *,struct _bubble *);
	void *data;
} bubble_event;

struct network {
	/**
	 * The list of all the users.
	 */
	linkedlist *users;
	/**
	 * The list of all the games.
	 */
	linkedlist *games;
	SDL_mutex *m;
};
struct network_client {
	/**
	 * The socket we are communicating on.
	 */
	TCPsocket s;
	/**
	 * A pointer to the network struct.
	 */
	struct network *n;
	/**
	 * The game we are currently in or NULL.
	 */
	struct game *g;
	/**
	 * The player we are currently playing as or NULL.
	 */
	struct player *p;
	/**
	 * True if the player is logged in.
	 */
	uint8_t in:1;//logged in
	/**
	 * True if the player wants to observe the game. 
	 * False if they want to play.
	 * This flag doesn't reset, meaning players only have to click start once.
	 * Default value is true.
	 */
	uint8_t observe:1;
	/**
	 * True if the player left the game and we are waiting for cleanup.
	 */
	uint8_t left:1;
};

typedef struct {
	uint8_t data;
} game_object;
struct bubble {
	/**
	 * The body of the bubble.
	 * Aka mass and i.
	 */
	cpBody body;
	/**
	 * The actual shape.
	 * Aka circle, etc.
	 */
	struct cpCircleShape shape;
	/**
	 * What node are we in the linked list.
	 */
	linkedlist_node *node;
	linkedlist *events;
	/**
	 * The level of the ball. 
	 * A ball of level 1 is the smallest. No children are produced.
	 * Anything larger produces children of (l - 1)
	 */
	uint8_t l;
};

struct wall {
	struct cpPolyShape shape;
};

struct player {
	/**
	 * The body of the player.
	 * Aka mass and i.
	 */
	cpBody body;
	/**
	 * The actual shape.
	 * Aka rectangle, etc.
	 */
	struct cpPolyShape shape;
	/**
	 * Who is controlling the player. 
	 */
	struct network_client *client;
	/**
	 * What node are we in the linked list.
	 */
	linkedlist_node *node;
	/**
	 * What linked list is this node in.
	 */
	linkedlist *list;
	/**
	 * What x value did we start at.
	 */
	double x;
	/**
	 * What y value did we start at.
	 */
	double y;
	/**
	 * Our score for popping bubbles.
	 */
	uint32_t score;
	/**
	 * How many more lives for this player.
	 * Zero indicates the player has infinite lives. 
	 */
	uint32_t lives;
	/**
	 * Bit 1 (& 1) - (on) Moving left (off) Not moving left.
	 * Bit 2 (& 2) - (on) Moving right (off) Not moving right.
	 * Bit 3 (& 4) - (on) Jumping (off) Not jumping.
	 * Bit 4 (& 8) - (on) Shoot (off) Don't shoot.
	 */
	uint8_t data;
};

struct map {
	/**
	 * The name of the map.
	 */
	char *name;
};

struct game {
	/**
	 * The name of the game.
	 */
	char *name;
	
	struct wall **l;
	//linkedlist of bubbles
	linkedlist *b;
	linkedlist *p_p;//players playing
	linkedlist *p_q;//free players
	linkedlist *clients;//
	struct network *n;
	struct cpSpace *cp;
	SDL_TimerID timer;
	//SDL_Mutex *m;
	uint32_t ident;
	int32_t w;
	int32_t h;
	uint32_t step;
	uint16_t w_len;
	uint16_t p_len;
	/**
	Bit 0-bubble-bubble collision
	*/
	uint8_t data;
};

extern int player_bubble_collision(cpArbiter *, struct cpSpace *, void *);
extern int bubble_bubble_collision(cpArbiter *, struct cpSpace *, void *);
extern struct bubble *bubble_create(double, double, double,
                                    double, double, uint8_t);
extern struct wall *wall_create(struct game *, double, double, double, double);
extern struct player *player_create(struct game *g, double, double, double,
                                    double, uint32_t, uint8_t);

#define ERR_TRACE() fprintf(stderr,"%s::%s->%i: called from here\n", \
                            __FILE__, __func__, __LINE__);
#define ERR_ERRNO() fprintf(stderr,"%s::%s->%i: %s\n", __FILE__, __func__, \
                            __LINE__, strerror(errno));

#endif
