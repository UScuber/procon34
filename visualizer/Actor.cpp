# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Actor.hpp"


// 座標初期化
Actor::Actor(size_t y, size_t x, size_t h, size_t w, Texture image) :
	y_coordinate(y), x_coordinate(x), height(h), width(w), image(image) {}

