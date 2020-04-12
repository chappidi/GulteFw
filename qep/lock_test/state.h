#pragma once
#include <sstream>

/*
	Just a return state from Read/Write operation
*/
struct STATE {
	int errors_x{ 0 };
	int errors_y{ 0 };
	int success { 0 };
	int loop{ 0 };
	std::stringstream msg;
	STATE& operator+=(const STATE& rhs) {
		errors_x += rhs.errors_x;
		errors_y += rhs.errors_y;
		success += rhs.success;
		loop += rhs.loop;
		msg << rhs.msg.str();
		return *this;
	}
};