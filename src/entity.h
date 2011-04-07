/******************************
** Tsunagari Tile Engine     **
** entity.h - Entity         **
** Copyright 2011 OmegaSDG   **
******************************/

#ifndef ENTITY_H
#define ENTITY_H

#include <string>

//#include "area.h"
#include "resourcer.h"
#include "sprite.h"
//#include "tilegrid.h"
//#include "world.h"

class Resourcer;
class Sprite;

class Entity
{
public:
	Entity(Resourcer* rc, const std::string descriptor, const std::string sprite_descriptor);
	~Entity();
	
	int init();

	void draw();
	bool needs_redraw();

	void move(int dx, int dy);

/*
	Entity(World* world, Resource* rc);
	void set_area(Area* area);
	void go_to(Area* area, coord_t coords);
	coord_t get_coords();
	void transport(coord_t coords);
*/

protected:
	Sprite* sprite;
	Resourcer* rc;
	std::string descriptor;
	std::string sprite_descriptor;
	bool redraw;

/*
	World* world;
	TileGrid* grid;
*/
};

#endif

