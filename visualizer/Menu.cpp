#pragma
# include <Siv3D.hpp>
# include "Menu.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;


MenuRect::MenuRect(Point coordinate, Size size, ColorF colorf, String label, Font &font):font(font) {
	this->coordinate = coordinate;
	this->size = size;
	this->colorf = colorf;
	this->label = label;
	this->rect = Rect{ this->coordinate, this->size};
}

bool MenuRect::isClicked(void) {
	return this->rect.leftClicked();
}

// 四角形と文字
void MenuRect::DrawMenuRect(void) {
	this->rect.draw(this->colorf);
	this->font(this->label).drawAt(this->coordinate + this->size / 2, Palette::Black);
}

MenuTab::MenuTab(size_t number, size_t height, Font &font):font(font) {
	assert(number > 0);
	this->number = number;
	this->height = height;
	for (size_t i = 0; i < number; i++) {
		this->menurects << MenuRect(Point{ 1280 / this->number * i, 0}, Size{  1280 / this->number, this->height }, ColorF{ 0.5, 0.5 }, U"TEST", this->font);
	}
}

void MenuTab::DrawMenuTab(void) {
	for (MenuRect& rect : this->menurects) {
		rect.DrawMenuRect();
	}
}
