# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"
# include "Actor.hpp"

// フィールド情報の初期化
Field::Field(size_t h, size_t w) :
	height(h),
	width(w),
	grid(h, Array<char32>(w, '.')){}


void Field::DisplayGrid(void) {
	for (size_t i = 0; i < (this->height * this->width); i++) {
		Rect((i % this->width) * (this->cell_size) + 100, (i / this->height) * (this->cell_size) + 100, 20, 20).drawFrame(1, 1, Palette::Black);
	}
}
