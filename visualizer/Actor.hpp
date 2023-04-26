# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;

class Actor{
public:
	Actor(size_t y, size_t x);
protected:
	size_t y_coordinate;
	size_t x_coordinate;
};


class Craftsman : public Actor {
public:
	Craftsman(size_t y, size_t x) : Actor(y, x) {};
	bool Build(Field &field, size_t y, size_t x);
	bool Break(size_t y, size_t x);
	bool Move(int dy, int dx);
};

class Wall : public Actor {

};

class Pond : public Actor {

};

class Castle : public Actor {

};

