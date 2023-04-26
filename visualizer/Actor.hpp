# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;

class Actor{
protected:
	size_t y_coordinate;
	size_t x_coordinate;
};


class Craftsman : public Actor {
public:
	Craftsman(Field& field, size_t y, size_t x);
	bool Build(Field& field, size_t y, size_t x);
	bool Break(Field& field, size_t y, size_t x);
	bool Move(Field& field, int dy, int dx);
};

class Wall : public Actor {

};

class Pond : public Actor {

};

class Castle : public Actor {

};

