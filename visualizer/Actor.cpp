# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Actor.hpp"


extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;

// 初期化
Actor::Actor(size_t y, size_t x, Texture image) :
	y_coordinate(y), x_coordinate(x), image(image) {}

void Actor::Draw(void) {
	this->image.resized(CELL_SIZE).draw(x_coordinate, y_coordinate, Palette::Black);
}
