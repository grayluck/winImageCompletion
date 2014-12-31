#include "stdafx.h"
#include "graphcut.h"

namespace graphcut
{
	
	int w, h;
	int rest;
	Mat img, imgDisp, ori;

	char* title = "graphcut";
	
	DWORD WINAPI drawGraphcut( LPVOID lpParam ) 
	{
		while(1)
		{
			imshow(title, imgDisp);
			Sleep(100);
		}
	}
	
	DWORD WINAPI workGraphcut( LPVOID lpParam ) 
	{
		while(rest)	// there is still some pixel waits for fill
		{
				
		}
	}

	void graphcutReset()
	{
		img = Mat::ones(h, w, CV_8UC3);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				Vec3b& colimg = img.at<Vec3b>(Point(i,j));
				if(i < w/2 && j < h/2)
				{
					Vec3b& colori = ori.at<Vec3b>(Point(i,j));
					colimg[0] = colori[0];
					colimg[1] = colori[1];
					colimg[2] = colori[2];
				}else
					colimg[0] = colimg[1] = colimg[2] = 0;
			}
		imgDisp = img.clone();
		rest =  w * h * 3 / 4;
	}

	void graphcut_init(CString fname)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fname);
		ori = imread(tmps, IMREAD_COLOR);
		w = ori.cols * 2;
		h = ori.rows * 2;
		namedWindow(title , WINDOW_NORMAL ); // Create a window for display.
		graphcutReset();
		CreateThread(0, 0, drawGraphcut, 0, 0, 0);
	}
	
	void runGraphcut()
	{
		CreateThread(0, 0, workGraphcut, 0, 0, 0);
	}

}
