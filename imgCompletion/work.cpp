#include "stdafx.h"
#include "work.h"

// img is the original image
// cov indicates the covering layer
Mat img, cov, imgDisp, tmp;

char* title = "display";

int ffillMode = 1;
int loDiff = 20, upDiff = 20;
int connectivity = 4;
int isColor = true;
bool useMask = false;
int newMaskVal = 255;

int w, h;

int brushr = 20;

int moused = 0;
int mousex, mousey;

static void onMouse( int event, int x, int y, int, void* )
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
		line(cov, Point(mousex, mousey), Point(x, y), (moused==1?Scalar(0,0,0):Scalar(1,1,1)),brushr);
		//circle(cov, Point(x, y), brushr, Scalar(0), CV_FILLED);
		imgDisp = img.clone().mul(cov);
		mousex = x, mousey = y;
	}
}

DWORD WINAPI showChriminisi( LPVOID lpParam ) 
{
	while(1)
	{
		imshow(title, imgDisp);
		//imshow("img", img);
		Sleep(100);
	}
}

void texture_init(CString fname)
{
	char tmps[256];
    strcpy(tmps, (LPCSTR)fname);
	tmp = imread(tmps, IMREAD_COLOR);
	w = tmp.cols * 2;
	h = tmp.rows * 2;
	//namedWindow(title , WINDOW_AUTOSIZE ); // Create a window for display.
	namedWindow(title , WINDOW_NORMAL ); // Create a window for display.
	cov = Mat::ones(h, w, CV_8UC3);
	img = Mat::ones(h, w, CV_8UC3);
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
		{
			Vec3b& col = cov.at<Vec3b>(Point(i,j));
			Vec3b& colimg = img.at<Vec3b>(Point(i,j));
			if(i < w/2 && j < h/2)
			{
				Vec3b& colori = tmp.at<Vec3b>(Point(i,j));
				col[0] = col[1] = col[2] = 1;
				colimg[0] = colori[0];
				colimg[1] = colori[1];
				colimg[2] = colori[2];
			}else
			{
				col[0] = col[1] = col[2] = 0;
				colimg[0] = colimg[1] = colimg[2] = 0;
			}
		}
	imgDisp = img.clone();
	setMouseCallback( title, onMouse, 0 );
	CreateThread(0, 0, showChriminisi, 0, 0, 0);
}

void chrim_init(CString fname)
{	
	char tmps[256];
    strcpy(tmps, (LPCSTR)fname);
	img = imread(tmps, IMREAD_COLOR);
	w = img.cols;
	h = img.rows;
	//namedWindow(title , WINDOW_AUTOSIZE ); // Create a window for display.
	namedWindow(title , WINDOW_NORMAL ); // Create a window for display.
	cov = Mat::ones(h, w, CV_8UC3);
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
		{
			Vec3b& col = cov.at<Vec3b>(Point(i,j));
			col[0] = col[1] = col[2] = 1;
		}
	imgDisp = img.clone();
	setMouseCallback( title, onMouse, 0 );
	CreateThread(0, 0, showChriminisi, 0, 0, 0);
}

const int maxn = 1001;
typedef int timg[maxn][maxn];

timg c, r, g, b;

void mattoarr(Mat& mat, timg arr)
{
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
			c[j][i] = cov.at<Vec3b>(Point(i, j))[0];
}

void arrtomat(timg r, timg g, timg b, Mat& mat)
{
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
		{
			Vec3b& vec = cov.at<Vec3b>(Point(i, j));
			vec[0] = r[j][i];
			vec[1] = g[j][i];
			vec[2] = b[j][i];
		}
}


// paras for chriminisi
int siz = 5;
Mat res, nowcov;

#define iscoved(x, y)	((x) < 0 || (x) >= w || (y) < 0 || (y) >= h || cov.at<Vec3b>(Point((x), (y)))[0] < 0.1)
#define isstillcoved(x, y)	((x) < 0 || (x) >= w || (y) < 0 || (y) >= h || nowcov.at<Vec3b>(Point((x), (y)))[0] < 0.1)

DWORD WINAPI runbruteforce(LPVOID params)
{
	res = imgDisp;
	nowcov = cov.clone();
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
		if(cov.at<Vec3b>(Point(i, j))[0] < 0.1)	// is covered
		{
			double mind = 1e200;
			int minx, miny;
			for(int x = 0; x < w; ++x)
				for(int y = 0; y < h; ++ y)
				{
					if(iscoved(x, y))
						continue;
					bool invalid = 0;
					double tot = 0;
					for(int dx = -siz; dx <= siz && !invalid; ++dx)
						for(int dy = -siz; dy <= siz; ++dy)
						{
							int xx = x + dx, yy = y + dy;
							int px = i + dx, py = j + dy;
							if(isstillcoved(xx, yy) && !isstillcoved(px, py))
							{
								invalid = true;
								break;
							}
							if(isstillcoved(px, py))
								continue;
							Vec3b& c1 = res.at<Vec3b>(Point(xx, yy));
							Vec3b& c2 = res.at<Vec3b>(Point(px, py));
							tot +=	(c1[0] - c2[0]) * (c1[0] - c2[0]) + 
									(c1[1] - c2[1]) * (c1[1] - c2[1]) + 
									(c1[2] - c2[2]) * (c1[2] - c2[2]) * (abs(dx) + abs(dy));
						}
					if(!invalid && tot < mind)
						mind = tot, minx = x, miny = y;
				}
			res.at<Vec3b>(Point(i, j)) = res.at<Vec3b>(Point(minx, miny));
			Vec3b& tmpv = nowcov.at<Vec3b>(Point(i, j));
			tmpv[0] = tmpv[1] = tmpv[2] = 1;
		}
	img = imgDisp.clone();
	for(int i = 0; i < w; ++i)
		for(int j = 0; j < h; ++j)
		{
			Vec3b& col = cov.at<Vec3b>(Point(i,j));
			col[0] = col[1] = col[2] = 1;
		}
	return 0;
}

// not working for borders
void brute_force()
{
	CreateThread(0, 0, runbruteforce, 0, 0, 0);
}

void chrim_run()
{
}
