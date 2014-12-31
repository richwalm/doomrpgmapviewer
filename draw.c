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

#include <gl/gl.h>

#include "draw.h"
#include "map.h"

extern Map GameMap;

static void DrawGrid()
{
	unsigned int LoopX, LoopY;

	glColor3f(0.25f, 0.25f, 0.25f);
	glBegin(GL_POINTS);

	for (LoopY = 0; LoopY < MAP_SIZE; LoopY++)
		for (LoopX = 0; LoopX < MAP_SIZE; LoopX++)
			glVertex2i(LoopX, LoopY);

	glEnd();

	return;
}

static void DrawVertexes()
{
	unsigned int Loop;

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POINTS);

	for (Loop = 0; Loop < GameMap.NumLines; Loop++) {
		glVertex2i(GameMap.Lines[Loop].Start.X, GameMap.Lines[Loop].Start.Y);
		glVertex2i(GameMap.Lines[Loop].End.X, GameMap.Lines[Loop].End.Y);
	}

	glEnd();

	return;
}

static void DrawMap()
{
	unsigned int Loop;

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	for (Loop = 0; Loop < GameMap.NumLines; Loop++) {
		glVertex2i(GameMap.Lines[Loop].Start.X, GameMap.Lines[Loop].Start.Y);
		glVertex2i(GameMap.Lines[Loop].End.X, GameMap.Lines[Loop].End.Y);
	}

	glEnd();

	return;
}

static void DrawThings()
{
	unsigned int Loop;
	Thing *Thing = GameMap.Things;

	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);

	for (Loop = 0; Loop < GameMap.NumThings; Loop++) {
		glVertex2f(Thing->Pos.X - THING_DRAW_SIZE + 0.5f, Thing->Pos.Y - THING_DRAW_SIZE + 0.5f);
		glVertex2f(Thing->Pos.X + THING_DRAW_SIZE + 0.5f, Thing->Pos.Y + THING_DRAW_SIZE + 0.5f);
		glVertex2f(Thing->Pos.X + THING_DRAW_SIZE + 0.5f, Thing->Pos.Y - THING_DRAW_SIZE + 0.5f);
		glVertex2f(Thing->Pos.X - THING_DRAW_SIZE + 0.5f, Thing->Pos.Y + THING_DRAW_SIZE + 0.5f);

		Thing++;
	}

	glEnd();

	return;
}

static void DrawNodes()
{
	unsigned int Loop;
	Node *Node = GameMap.Nodes;

	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 0.0f, 0.25f);
	glBegin(GL_LINES);

	for (Loop = 0; Loop < GameMap.NumNodes; Loop++) {
		glVertex2i(Node->One.X, Node->One.Y);
		glVertex2i(Node->Two.X, Node->One.Y);

		glVertex2i(Node->Two.X, Node->One.Y);
		glVertex2i(Node->Two.X, Node->Two.Y);

		glVertex2i(Node->Two.X, Node->Two.Y);
		glVertex2i(Node->One.X, Node->Two.Y);

		glVertex2i(Node->One.X, Node->Two.Y);
		glVertex2i(Node->One.X, Node->One.Y);

		Node++;
	}

	glEnd();
	glDisable(GL_BLEND);

	return;
}

/*
static void DrawNode(unsigned int Number)
{
	unsigned int Loop;
	Node *SelNode = &GameMap.Nodes[Number];

	if (SelNode->Type != NODE_LEAF)
		glColor3f(1.0f, 1.0f, 0.0f);
	else
		glColor3f(0.5f, 0.5f, 0.0f);

	glBegin(GL_LINE_LOOP);
		glVertex2i(SelNode->One.X, SelNode->One.Y);
		glVertex2i(SelNode->One.X, SelNode->Two.Y);
		glVertex2i(SelNode->Two.X, SelNode->Two.Y);
		glVertex2i(SelNode->Two.X, SelNode->One.Y);
	glEnd();

	switch (SelNode->Type) {

		// Leaf node.
		case NODE_LEAF:
			glColor3f(0.0f, 1.0f, 0.0f);
			glBegin(GL_LINES);
			for (Loop = 0; Loop < SelNode->ArgTwo; Loop++) {
				glVertex2i(GameMap.Lines[SelNode->ArgOne + Loop].Start.X,
					GameMap.Lines[SelNode->ArgOne + Loop].Start.Y);
				glVertex2i(GameMap.Lines[SelNode->ArgOne + Loop].End.X,
					GameMap.Lines[SelNode->ArgOne + Loop].End.Y);
			}
			glEnd();
			break;

		// Vertical split.
		case NODE_VER_SPLIT:
			glColor3f(0.5f, 0.5f, 0.0f);
			glBegin(GL_LINES);
				glVertex2i(SelNode->Split, SelNode->One.Y);
				glVertex2i(SelNode->Split, SelNode->Two.Y);
			glEnd();
			break;

		// Horizontal split.
		case NODE_HOZ_SPLIT:
			glColor3f(0.5f, 0.5f, 0.0f);
			glBegin(GL_LINES);
				glVertex2i(SelNode->One.X, SelNode->Split);
				glVertex2i(SelNode->Two.X, SelNode->Split);
			glEnd();
			break;
	}

	return;
}
*/

static void DrawBlockmap()
{
	unsigned int Loop, LoopBit, Col, Row;
	int Block;

	Col = Row = 0;

	glColor3f(0.0f, 0.0f, 0.15f);
	glBegin(GL_QUADS);

	for (Loop = 0; Loop < BLOCKMAP_SIZE; Loop++) {

		for (LoopBit = 0; LoopBit < 8; LoopBit += 2) {

			Block = GameMap.Blockmap[Loop] >> LoopBit & 0x03;
			if (Block == 1) {
				glVertex2i(Col, Row);
				glVertex2i(Col + 1, Row);
				glVertex2i(Col + 1, Row + 1);
				glVertex2i(Col, Row + 1);
			}

			Col++;
			if (Col >= MAP_SIZE) {
				Col = 0;
				Row++;
			}

		}

	}

	glEnd();

	return;
}

int Draw(char Grid, char Nodes, char Vertexes, char Things, char Blockmap)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (Blockmap)
		DrawBlockmap();
	if (Grid)
		DrawGrid();
	DrawMap(Vertexes);
	if (Nodes)
//		DrawNode(NodeNumber);
		DrawNodes();
	if (Vertexes)
		DrawVertexes();
	if (Things)
		DrawThings();

	if (glGetError() != GL_NO_ERROR)
		return 0;

	return 1;
}

void GLSetup(unsigned int Width, unsigned int Height)
{
	glViewport(0, 0, Width, Height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0d, MAP_SIZE, MAP_SIZE, 0.0d, 0.0d, 1.0d);
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return;
}
