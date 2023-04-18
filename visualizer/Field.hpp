# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Actor.hpp"


class Field {
public:
	Field(size_t h, size_t w);
	void DisplayGrid(void);
private:
	size_t height;
	size_t width;
	size_t cell_size = 20;
	Array<Array<char32>> grid;
	Array<Actor> wall;
	Array<Actor> craftsmen;
};

