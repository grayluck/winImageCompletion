#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

namespace clrz	// Colorize
{
	void init(CString fname);
	void setColor(COLORREF col);
	void setPenWidth(int r);
}
