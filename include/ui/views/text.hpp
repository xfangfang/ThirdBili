#pragma once
#include <string>
#include "view.hpp"
#include "../ui_common.hpp"

// simple horizontal line
struct TextView : public View {
	UI::FlexibleString<TextView> text;
	double font_size = 0.5;
	double interval = DEFAULT_FONT_INTERVAL;
public :
	using View::View;
	
	TextView *set_text(UI::FlexibleString<TextView> text) {
		this->text = text;
		return this;
	}
	TextView *set_font_size(double font_size, double interval) {
		this->font_size = font_size;
		this->interval = interval;
		return this;
	}
	
	void draw() const override {
		int y = (int)((y0 + y1 - interval) / 2);
		Draw(text, x0 + SMALL_MARGIN, y, font_size, font_size, DEF_DRAW_BLACK);
	}
	void update(Hid_info key) override {}
};
