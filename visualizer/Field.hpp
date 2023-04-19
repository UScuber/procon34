# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Actor.hpp"


class Field{
public:
	Field(void);
	void DisplayGrid(void);
	void SetCraftsman(void);
	void DrawCraftman(void);
private:
	Array<Array<char>> grid;
	Array<Wall> walls;
	Array<Craftsman> craftsmen;
};

