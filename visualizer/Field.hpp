# include <Siv3D.hpp> // OpenSiv3D v0.6.9

class Field {
public:
	Field(size_t h, size_t w);
	void DisplayGrid(void);
private:
	size_t height;
	size_t width;
	Array<Array<char32>> grid;
};

