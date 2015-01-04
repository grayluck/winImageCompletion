#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

namespace dec
{
	void init(CString fname);
	void onThetaChanged(int _theta);
	void onAlphaChanged(int _alpha);
}
