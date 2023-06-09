# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9

// sとteamに合致するCELLの値を返す
char SwitchCELL(const String& s, const bool team);

// (y,x)を左上とするCELL_SIZEの大きさの四角形
Rect GetGridRect(const Point p);
Circle GetGridCircle(const Point p);

// (y,x)がフィールド内に収まっているか
bool isInField(int y, int x);
bool isInField(const Point& p);

// pの周りでclickされているものを探す
Optional<Point> get_clicked_pos(const Point p, const Array<Point>& dydx);

class Field {
public:
	Field(void);
	void Initialize(size_t pond, size_t castle, size_t craftsman);
	char getGrid(const int y, const int x) const;
	char getGrid(const Point p) const;
	void setGrid(const int y, const int x, const char v);
	void setGrid(const Point p, const char v);
	void DisplayGrid(void) const;
	void DrawActors(void);
	void SearchArea(void);
	size_t CountArea(bool team);

private:
	Array<Array<char>> grid;
};

