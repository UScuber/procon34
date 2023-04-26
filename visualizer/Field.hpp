# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9


class Field{
public:
	Field(void);
	void DisplayGrid(void);
	void DrawActors(void);
	size_t SearchArea(void);
//private:
	Array<Array<char>> grid;
};

