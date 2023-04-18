# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"


// フィールド情報の初期化
Field::Field(size_t h, size_t w) :
	height(h),
	width(w),
	grid(h, Array<char32>(w, '.')){}




void Field::DisplayGrid(void) {
	for (size_t i = 0; i < 6; i++) {
		Rect((i % 3) * 50 + 100, (i / 3) * 50 + 100, 50, 50).draw();
	}
}
