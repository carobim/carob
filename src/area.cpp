#include <stdio.h>

#include "area.h"

Area::Area(GameWindow* , Resourcer* , Entity* player, std::string )
{
	this->player = player;
}

Area::~Area()
{
}

void Area::button_down(Gosu::Button btn)
{
	if (btn == Gosu::kbRight)
		player->move(1, 0);
	else if (btn == Gosu::kbLeft)
		player->move(-1, 0);
	else if (btn == Gosu::kbUp)
		player->move(0, -1);
	else if (btn == Gosu::kbDown)
		player->move(0, 1);
}

void Area::draw()
{
	player->draw();
}

bool Area::needs_redraw()
{
	return player->needs_redraw();
}

