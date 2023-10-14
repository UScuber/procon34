# pragma once
# include <Siv3D.hpp>

class ProgressBar {
public:
	/// @brief プログレスバーの座標を大きさを指定
	/// @param p 四角形の左上の大きさ
	/// @param height 縦方向の長さ
	/// @param length 横方向の最大長
	ProgressBar(const Point p, const int height, const int length);

	void draw(const double &percentage);
	void draw(const double &numerator, const double &denominator);

	void set_color_background(const Color &color_background);
private:
	// プログレスバーの高さと長さ(最大値)
	int length = 0;
	int height = 0;
	// 左上の座標
	Point leftup;
	// プログレスバーの四角形
	Rect rect;
	// プログレスバーの背景色
	Color color_background = Palette::Gray;
	// thresholdsの閾値で表示する色
	Array<Color> colors = { Palette::Green, Palette::Yellow, Palette::Red };
	Array<double> thresholds = { 1.0, 0.5, 0.2, 0.0 };
};

ProgressBar::ProgressBar(const Point p, const int height, const int length){
	assert(colors.size() + 1 == thresholds.size());
	rect = Rect(p, length, height);
	this->leftup = p;
	this->height = height;
	this->length = length;
}

void ProgressBar::set_color_background(const Color &color_background){
	this->color_background = color_background;
}

void ProgressBar::draw(const double &percentage){
	double tmp_percentage = percentage;
	tmp_percentage = Min(tmp_percentage, 1.0);
	rect.draw(color_background);
	for(int i = 0; i < (int)colors.size(); i++){
		if(thresholds[i+1] < tmp_percentage and tmp_percentage <= thresholds[i]){
			rect.scaledAt(leftup, tmp_percentage, 1.0).draw(colors[i]);
			break;
		}
	}
}

void ProgressBar::draw(const double &numerator, const double &denominator){
	this->draw(numerator / denominator);
}
