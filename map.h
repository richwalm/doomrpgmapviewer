#ifndef MAP_H
#define MAP_H

#include <stdint.h>

#define MAP_SIZE	32

#define	HEADER_SIZE	13
#define THING_SIZE	5
#define	INTERACTION_SIZE	4
#define	INTCOMMAND_SIZE		9
#define BLOCKMAP_SIZE		(MAP_SIZE * MAP_SIZE * 2 / 8)

#define COORD_SHIFT	3

enum NodeTypes {
	NODE_LEAF,
	NODE_VER_SPLIT,
	NODE_HOZ_SPLIT
};

typedef struct Coord {
	uint8_t X, Y;
} Coord;

typedef struct Node {
	Coord One, Two;
	uint8_t Type;
	uint8_t Split;
	uint16_t ArgOne, ArgTwo;
} Node;

typedef struct Line {
	Coord Start, End;
	uint16_t Texture;
	uint16_t Flags;
} Line;

typedef struct Thing {
	Coord Pos;
	uint32_t Flags;
} Thing;

typedef struct Map {
	uint16_t NumNodes, NumLines, NumThings;
	Node *Nodes;
	Line *Lines;
	Thing *Things;
	uint8_t *Blockmap;
} Map;

int LoadMap(const char *Filename, Map *Map);
void FreeMap(Map *Map);
int GetNode(Map *Map, unsigned int NodeNumber, unsigned int X, unsigned int Y);

#endif
