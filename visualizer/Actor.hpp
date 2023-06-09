# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;


class Craftsman {
public:
	Craftsman(Field& field, int y, int x, bool team);
	void Initialize(void);
	bool isActed = false;
	bool isTarget = false;
	bool team = true;
	bool Build(Field& field, const Point d);
	bool Break(Field& field, const Point d);
	bool Move(Field& field, const Point d);
	size_t Direction = 0;
	String Act = U"";
	Point pos;
};
