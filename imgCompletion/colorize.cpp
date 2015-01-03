
#include "stdafx.h"
#include "colorize.h"

namespace clrz
{

	Mat img, imgDisp, res;
	Mat cov, allone, col;
	int w, h;

	const char* titleSrc = "colorize :: source";
	const char* titleDest = "colorize :: result";
	
	int brushr = 10;

	Scalar colSelected;

	int moused = 0;
	int mousex, mousey;

	void redraw()
	{
		imshow(titleSrc, imgDisp);
		imshow(titleDest, res);
	}
	
	void setColor(COLORREF col)
	{
		colSelected = Scalar(GetBValue(col), GetGValue(col), GetRValue(col));
	}

	void setPenWidth(int r)
	{
		brushr = r;
	}

	void onMouse( int event, int x, int y, int, void* )
	{
		int bo = 0;
		switch(event)
		{
		case CV_EVENT_LBUTTONDOWN:
			bo = 1;
			mousex = x, mousey = y;
			moused = 1;
			break;
		case CV_EVENT_LBUTTONUP:
			bo = 0;
			moused = 0;
			break;
		case CV_EVENT_RBUTTONDOWN:
			bo = 1;
			mousex = x, mousey = y;
			moused = 2;
			break;
		case CV_EVENT_RBUTTONUP:
			bo = 0;
			moused = 0;
			break;
		case CV_EVENT_MOUSEMOVE:
			bo = moused;
			break;
		}
		if( bo )
		{
			line(cov, Point(mousex, mousey), Point(x, y), (moused==1?Scalar(1,1,1):Scalar(0,0,0)),brushr);
			line(col, Point(mousex, mousey), Point(x, y), colSelected, brushr);
			//circle(cov, Point(x, y), brushr, Scalar(0), CV_FILLED);
			imgDisp = col.mul(cov) + img.mul(allone - cov);
			mousex = x, mousey = y;
			redraw();
		}
	}

	void init(CString fname)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fname);
		img = imread(tmps, IMREAD_COLOR);
		w = img.cols;
		h = img.rows;
		namedWindow(titleSrc, WINDOW_NORMAL);
		namedWindow(titleDest, WINDOW_NORMAL);
		cov = Mat::zeros(h, w, CV_8UC3);
		allone = Mat::ones(h, w, CV_8UC3);
		col = Mat::zeros(h, w, CV_8UC3);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				Vec3b& col = allone.at<Vec3b>(Point(i,j));
				col[0] = col[1] = col[2] = 1;
			}
		imgDisp = img.clone();
		res = img.clone();
		setMouseCallback(titleSrc, onMouse, 0);
		redraw();
	}
}
