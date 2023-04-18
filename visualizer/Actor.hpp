# include <Siv3D.hpp> // OpenSiv3D v0.6.9


class Actor {
public:
	Actor(size_t y, size_t x, size_t h, size_t w, Texture image);
private:
	size_t y_coordinate;
	size_t x_coordinate;
	size_t height;
	size_t width;
	Texture image;
};

class Craftsman : Actor {
public:
	bool Build(size_t y, size_t x);
	bool Break(size_t y, size_t x);
	bool Move(int dy, int dx);
};

class Wall : Actor {

};

class Pond : Actor {

};

class Castle : Actor {

};

