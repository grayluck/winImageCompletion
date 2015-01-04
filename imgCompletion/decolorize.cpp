
#include "stdafx.h"
#include "decolorize.h"

namespace dec
{

const int maxn = 2048;

Mat img, imgRes;

int h, w;

int res[maxn][maxn];

char* titleSrc = "Decolorize :: source";
char* titleDest = "Decolorize :: result";

void work();

double theta, alpha;

void redraw()
{
	imshow(titleSrc, img);
	imshow(titleDest, imgRes);
}

void init(CString fname)
{
	char tmps[256];
	strcpy(tmps, (LPCSTR)fname);
	img = imread(tmps, IMREAD_COLOR);
	namedWindow(titleSrc, WINDOW_NORMAL);
	namedWindow(titleDest, WINDOW_NORMAL);
	h = img.rows;
	w = img.cols;
	imgRes = Mat::zeros(h, w, CV_8UC1);
	work();
	redraw();
}

void work()
{

}

}
