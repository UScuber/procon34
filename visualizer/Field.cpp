# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"
# include "Actor.hpp"


enum {
	POND = 1 << 0,
	WALL_ENEM = 1 << 1,
	WALL_ALLY = 1 << 2,
	AREA_ENEM = 1 << 3,
	AREA_ALLY = 1 << 4,
	ENEM = 1 << 5,
	ALLY = 1 << 6,
	CASTLE = 1 << 7
};


extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;


Field::Field(void) {
	this->grid.resize(HEIGHT, Array<char>(WIDTH));
}

void Field::DisplayGrid(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).drawFrame(1, 1, Palette::Black);
	}
}

void Field::DrawCraftman(void) {
	for (auto& craftman : this->craftsmen) {
		craftman.Draw();
	}
}

void Field::SetCraftsman(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		if (Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).leftClicked()) {
			(this->craftsmen) << Craftsman((i / WIDTH) * CELL_SIZE + 100, (i % WIDTH) * CELL_SIZE + 100, Texture{ U"👷"_emoji });
		}
	}
}
