#pragma once
#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

void chrim_init(CString fname);
void texture_init(CString fname);
void chrim_run();
void brute_force();
