# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9

// sとteamに合致するCELLの値を返す
char SwitchCELL(String s, bool team);

// (y,x)を左上とするCELL_SIZEの大きさの四角形
Rect GetGridRect(size_t y, size_t x);
Circle GetGridCircle(size_t y, size_t x);

// (y,x)がフィールド内に収まっているか
bool isInField(size_t y, size_t x);

class Field{
public:
	Field(void);
	void Initialize(size_t pond, size_t castle, size_t craftsman);
	void DisplayGrid(void);
	void DrawActors(void);
	size_t SearchArea(bool team);
	void GetGrid(Array<Array<char>> grid);
	Array<Array<char>> GiveGrid(void);
//private:
	Array<Array<char>> grid;
};

