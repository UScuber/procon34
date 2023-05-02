# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;

class Actor{
public:
	size_t y_coordinate;
	size_t x_coordinate;
};


class Craftsman : public Actor {
public:
	Craftsman(Field& field, size_t y, size_t x, bool team);
	void Initialize(void);
	bool isActed = false;
	bool isTarget = false;
	bool team = true;
	bool Build(Field& field, int dy, int dx);
	bool Break(Field& field, int dy, int dx);
	bool Move(Field& field, int dy, int dx);
	size_t Direction = 0;
	String Act = U"";
};

class Wall : public Actor {

};

class Pond : public Actor {

};

class Castle : public Actor {

};

