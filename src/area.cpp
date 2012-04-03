/*********************************
** Tsunagari Tile Engine        **
** area.cpp                     **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#include <math.h>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Timing.hpp>

#include "area.h"
#include "common.h"
#include "entity.h"
#include "log.h"
#include "python.h"
#include "resourcer.h"
#include "tile.h"
#include "window.h"
#include "world.h"

#define ASSERT(x)  if (!(x)) return false

/* NOTE: In the TMX map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

Area::Area(Viewport* view,
           Player* player,
           Music* music,
           const std::string& descriptor)
	: view(view),
	  player(player),
	  music(music),
	  dim(0, 0, 0),
	  tileDim(0, 0),
	  loopX(false), loopY(false),
	  beenFocused(false),
	  redraw(true),
	  descriptor(descriptor)
{
}

Area::~Area()
{
}

bool Area::init()
{
	// Abstract method.
	return false;
}

void Area::focus()
{
	if (!beenFocused) {
		beenFocused = true;
		runOnLoads();
	}

	music->setIntro(musicIntro);
	music->setLoop(musicLoop);
}

void Area::buttonDown(const Gosu::Button btn)
{
	if (btn == Gosu::kbRight)
		player->startMovement(ivec2(1, 0));
	else if (btn == Gosu::kbLeft)
		player->startMovement(ivec2(-1, 0));
	else if (btn == Gosu::kbUp)
		player->startMovement(ivec2(0, -1));
	else if (btn == Gosu::kbDown)
		player->startMovement(ivec2(0, 1));
	else if (btn == Gosu::kbSpace)
		player->useTile();
}

void Area::buttonUp(const Gosu::Button btn)
{
	if (btn == Gosu::kbRight)
		player->stopMovement(ivec2(1, 0));
	else if (btn == Gosu::kbLeft)
		player->stopMovement(ivec2(-1, 0));
	else if (btn == Gosu::kbUp)
		player->stopMovement(ivec2(0, -1));
	else if (btn == Gosu::kbDown)
		player->stopMovement(ivec2(0, 1));
}

void Area::draw()
{
	updateTileAnimations();
	drawTiles();
	drawEntities();
	redraw = false;
}

bool Area::needsRedraw() const
{
	if (redraw)
		return true;
	if (player->needsRedraw())
		return true;

	// Do any on-screen tile types need to update their animations?
	BOOST_FOREACH(const TileType& type, tileTypes)
		if (type.needsRedraw(*this))
			return true;
	return false;
}

void Area::requestRedraw()
{
	redraw = true;
}

void Area::update(unsigned long dt)
{
	pythonSetGlobal("area", this);
	player->update(dt);

	if (onUpdateScripts.size()) {
		BOOST_FOREACH(const std::string& script, onUpdateScripts) {
			pythonSetGlobal("area", this);
			Resourcer::getResourcer()->runPythonScript(script);
		}
	}

	view->update(dt);
	music->update();
}

AreaPtr Area::reset()
{
	World* world = World::getWorld();
	AreaPtr newSelf = world->getArea(descriptor, GETAREA_ALWAYS_CREATE);
	if (world->getFocusedArea().get() == this) {
		vicoord c = player->getTileCoords_vi();
		world->focusArea(newSelf, c);
	}
	return newSelf;
}



const Tile& Area::getTile(int x, int y, int z) const
{
	if (loopX)
		x = wrap(0, x, dim.x);
	if (loopY)
		y = wrap(0, y, dim.y);
	return map[z][y][x];
}

const Tile& Area::getTile(int x, int y, double z) const
{
	return getTile(x, y, depthIndex(z));
}

const Tile& Area::getTile(icoord phys) const
{
	return getTile(phys.x, phys.y, phys.z);
}

const Tile& Area::getTile(vicoord virt) const
{
	return getTile(virt2phys(virt));
}

Tile& Area::getTile(int x, int y, int z)
{
	if (loopX)
		x = wrap(0, x, dim.x);
	if (loopY)
		y = wrap(0, y, dim.y);
	return map[z][y][x];
}

Tile& Area::getTile(int x, int y, double z)
{
	return getTile(x, y, depthIndex(z));
}

Tile& Area::getTile(icoord phys)
{
	return getTile(phys.x, phys.y, phys.z);
}

Tile& Area::getTile(vicoord virt)
{
	return getTile(virt2phys(virt));
}

TileType& Area::getTileType(int idx)
{
	return tileTypes[idx];
}



ivec3 Area::getDimensions() const
{
	return dim;
}

ivec2 Area::getTileDimensions() const
{
	return tileDim;
}

icube_t Area::visibleTiles() const
{
	rvec2 screen = view->getVirtRes();
	rvec2 off = view->getMapOffset();

	int x1 = (int)floor(off.x / tileDim.x);
	int y1 = (int)floor(off.y / tileDim.y);
	int x2 = (int)ceil((screen.x + off.x) / tileDim.x);
	int y2 = (int)ceil((screen.y + off.y) / tileDim.y);

	return icube(x1, y1, 0, x2, y2, dim.z);
}

bool Area::inBounds(int x, int y, int z) const
{
	return ((loopX || (0 <= x && x < dim.x)) &&
		(loopY || (0 <= y && y < dim.y)) &&
		          0 <= z && z < dim.z);
}

bool Area::inBounds(int x, int y, double z) const
{
	return inBounds(x, y, depthIndex(z));
}

bool Area::inBounds(icoord phys) const
{
	return inBounds(phys.x, phys.y, phys.z);
}

bool Area::inBounds(vicoord virt) const
{
	return inBounds(virt2phys(virt));
}



bool Area::loopsInX() const
{
	return loopX;
}

bool Area::loopsInY() const
{
	return loopY;
}

const std::string& Area::getDescriptor() const
{
	return descriptor;
}



vicoord Area::phys2virt_vi(icoord phys) const
{
	return vicoord(phys.x, phys.y, indexDepth(phys.z));
}

rcoord Area::phys2virt_r(icoord phys) const
{
	return rcoord(
		(double)phys.x * tileDim.x,
		(double)phys.y * tileDim.y,
		indexDepth(phys.z)
	);
}

icoord Area::virt2phys(vicoord virt) const
{
	return icoord(virt.x, virt.y, depthIndex(virt.z));
}

icoord Area::virt2phys(rcoord virt) const
{
	return icoord(
		(int)(virt.x / tileDim.x),
		(int)(virt.y / tileDim.y),
		depthIndex(virt.z)
	);
}

rcoord Area::virt2virt(vicoord virt) const
{
	return rcoord(
		(double)virt.x * tileDim.x,
		(double)virt.y * tileDim.y,
		virt.z
	);
}

vicoord Area::virt2virt(rcoord virt) const
{
	return vicoord(
		(int)virt.x / tileDim.x,
		(int)virt.y / tileDim.y,
		virt.z
	);
}


int Area::depthIndex(double depth) const
{
	return depth2idx.find(depth)->second;
}

double Area::indexDepth(int idx) const
{
	return idx2depth[idx];
}



void Area::runOnLoads()
{
	std::string onAreaLoadScript = World::getWorld()->getAreaLoadScript();
	if (onAreaLoadScript.size()) {
		pythonSetGlobal("area", this);
		Resourcer::getResourcer()->runPythonScript(onAreaLoadScript);
	}
	BOOST_FOREACH(const std::string& script, onLoadScripts) {
		pythonSetGlobal("area", this);
		Resourcer::getResourcer()->runPythonScript(script);
	}
}

void Area::updateTileAnimations()
{
	const int millis = GameWindow::getWindow().time();
	BOOST_FOREACH(TileType& type, tileTypes)
		type.anim.updateFrame(millis);
}

void Area::drawTiles() const
{
	const icube_t tiles = visibleTiles();
	for (int z = tiles.z1; z < tiles.z2; z++) {
		double depth = idx2depth[z];
		for (int y = tiles.y1; y < tiles.y2; y++) {
			for (int x = tiles.x1; x < tiles.x2; x++) {
				int tx = x, ty = y, tz = z;
				if (loopX)
					tx = wrap(0, tx, dim.x);
				if (loopY)
					ty = wrap(0, ty, dim.y);
				if (inBounds(tx, ty, tz))
					drawTile(map[tz][ty][tx], x, y, depth);
			}
		}
	}
}

void Area::drawTile(const Tile& tile, int x, int y, double depth) const
{
	const TileType* type = tile.type;
	if (type) {
		const Gosu::Image* img = type->anim.frame();
		if (img)
			img->draw((double)x*img->width(),
				  (double)y*img->height(), depth);
	}
}

void Area::drawEntities()
{
	player->draw();
}


void exportArea()
{
	boost::python::class_<Area>("Area", boost::python::no_init)
		.def("request_redraw", &Area::requestRedraw)
		.def("get_tile",
		    static_cast<Tile& (Area::*) (int, int, double)>
		    (&Area::getTile),
		    boost::python::return_value_policy<
		      boost::python::reference_existing_object
		    >()
		)
		.def("in_bounds",
		    static_cast<bool (Area::*) (int, int, double) const>
		    (&Area::inBounds))
		.def("get_tile_type", &Area::getTileType,
		    boost::python::return_value_policy<
		      boost::python::reference_existing_object
		    >()
		)
		.def("reset", &Area::reset);
}

