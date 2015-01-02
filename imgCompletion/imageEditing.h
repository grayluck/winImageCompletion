#pragma once

//poisson

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

namespace poi
{
	void poi_init(CString fSrc, CString fDest);
	void poi_work(int _mode);
}
