/*
	Doom RPG Map Viewer
	Written by Richard Walmsley. <richwalm@gmail.com>
	Thanks to Simon "Fraggle" Howard for the specs. (http://www.soulsphere.org/random/doom-rpg-bnf.txt)

	Copyright (C) 2012 Richard Walmsley <richwalm@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "map.h"

int GetNode(Map *Map, unsigned int NodeNumber, unsigned int X, unsigned int Y)
{
	Node *SelNode = &Map->Nodes[NodeNumber];
	int Split;

	if (SelNode->Type == NODE_LEAF)
		return -2;

	if (!(X >= SelNode->One.X && Y >= SelNode->One.Y && X < SelNode->Two.X && Y < SelNode->Two.Y))
		return -1;

	Split = 0;

	switch (SelNode->Type) {

		case NODE_VER_SPLIT:
			if (X < SelNode->Split)
				Split = 1;
			break;

		case NODE_HOZ_SPLIT:
			if (Y < SelNode->Split)
				Split = 1;
			break;
	}

	if (Split)
		return SelNode->ArgTwo;

	return SelNode->ArgOne;
}

int LoadMap(const char *Filename, Map *Map)
{
	FILE *MapFile;
	unsigned int Loop;
	uint16_t Amount;

	MapFile = fopen(Filename, "rb");
	if (!MapFile)
		return 0;

	// Header. Unknown at current so it's skipped.

	if (fseek(MapFile, HEADER_SIZE, SEEK_SET) != 0) {
		fclose(MapFile);
		return 0;
	}

	// Nodes.

	if (fread(&Map->NumNodes, sizeof(Map->NumNodes), 1, MapFile) != 1) {
		fclose(MapFile);
		return 0;
	}

	Map->Nodes = malloc(Map->NumNodes * sizeof(Node));
	if (!Map->Nodes) {
		fclose(MapFile);
		return 0;
	}

	if (fread(Map->Nodes, sizeof(Node), Map->NumNodes, MapFile) != Map->NumNodes) {
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	for (Loop = 0; Loop < Map->NumNodes; Loop++) {
		Map->Nodes[Loop].One.X >>= COORD_SHIFT;
		Map->Nodes[Loop].One.Y >>= COORD_SHIFT;
		Map->Nodes[Loop].Two.X >>= COORD_SHIFT;
		Map->Nodes[Loop].Two.Y >>= COORD_SHIFT;
		Map->Nodes[Loop].Split >>= COORD_SHIFT;
	}

	// Lines.

	if (fread(&Map->NumLines, sizeof(Map->NumLines), 1, MapFile) != 1) {
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	Map->Lines = malloc(Map->NumLines * sizeof(Line));
	if (!Map->Lines) {
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	if (fread(Map->Lines, sizeof(Line), Map->NumLines, MapFile) != Map->NumLines) {
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	for (Loop = 0; Loop < Map->NumLines; Loop++) {
		Map->Lines[Loop].Start.X >>= COORD_SHIFT;
		Map->Lines[Loop].Start.Y >>= COORD_SHIFT;
		Map->Lines[Loop].End.X >>= COORD_SHIFT;
		Map->Lines[Loop].End.Y >>= COORD_SHIFT;
	}

	// Things.

	if (fread(&Map->NumThings, sizeof(Map->NumThings), 1, MapFile) != 1) {
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	Map->Things = malloc(Map->NumThings * sizeof(Thing));
	if (!Map->Things) {
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	for (Loop = 0; Loop < Map->NumThings; Loop++) {

		Map->Things[Loop].Flags = 0;
		if (fread(&Map->Things[Loop], THING_SIZE, 1, MapFile) != 1) {
			free(Map->Lines);
			free(Map->Nodes);
			fclose(MapFile);
			return 0;
		}

		Map->Things[Loop].Pos.X >>= COORD_SHIFT;
		Map->Things[Loop].Pos.Y >>= COORD_SHIFT;
	}

	// Interaction. Skipped.

	if (fread(&Amount, sizeof(Amount), 1, MapFile) != 1) {
		free(Map->Things);
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	if (fseek(MapFile, Amount * INTERACTION_SIZE, SEEK_CUR) != 0) {
		free(Map->Things);
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	// Interaction commands. Skipped.

	if (fread(&Amount, sizeof(Amount), 1, MapFile) != 1) {
		free(Map->Things);
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	if (fseek(MapFile, Amount * INTCOMMAND_SIZE, SEEK_CUR) != 0) {
		free(Map->Things);
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	// Block Map.

	Map->Blockmap = malloc(BLOCKMAP_SIZE);
	if (!Map->Blockmap) {
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	if (fread(Map->Blockmap, BLOCKMAP_SIZE, 1, MapFile) != 1) {
		free(Map->Blockmap);
		free(Map->Things);
		free(Map->Lines);
		free(Map->Nodes);
		fclose(MapFile);
		return 0;
	}

	fclose(MapFile);
	return 1;
}

void FreeMap(Map *Map)
{
	free(Map->Blockmap);
	free(Map->Things);
	free(Map->Lines);
	free(Map->Nodes);

	return;
}
