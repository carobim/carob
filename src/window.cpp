/******************************
** Tsunagari Tile Engine     **
** window.cpp                **
** Copyright 2011 OmegaSDG   **
******************************/

#include "resourcer.h"
#include "window.h"

GameWindow::GameWindow(uint x, uint y, bool fullscreen) : Gosu::Window(x, y, fullscreen)
{
	rc = new Resourcer(this);
}

GameWindow::~GameWindow()
{
	delete world;
}

int GameWindow::initEntryWorld(const std::string descriptor) {
	world = new World(this, rc, descriptor);
	return world->init();
}

void GameWindow::buttonDown(Gosu::Button btn)
{
	if (btn == Gosu::kbEscape)
		close();
	else
		world->button_down(btn);
}

void GameWindow::draw()
{
	world->draw();
}

bool GameWindow::needsRedraw()
{
	return world->needs_redraw();
}

void GameWindow::update()
{
}

