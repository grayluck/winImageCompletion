#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

namespace graphcut
{
	void graphcut_init(CString fname);	
	void runGraphcut();
}
