
#include "stdafx.h"
#include "imageEditing.h"

#include <Eigen\SparseCholesky>

using namespace Eigen;


namespace poi
{
	
	void poi_run();

#define iscov(x, y)	(cov.at<Vec3b>(Point(x,y))[0] == 0)

	char* titleSrc = "poisson source img";
	char* titleDest = "poisson target img";

	Mat imgSrc, imgDest;
	Mat cov;	// a cov layer for img Src
	Mat imgDispSrc;	// the image actually displayed as source img
	Mat imgDispDest;
	Mat res;	// save the process result
	
	int srcw, srch;
	int destw, desth;

	int mode = 1;	// 0: copy; 1: poisson(no holes); 2: poisson(holes)
	
	int moused = 0;
	int mousex, mousey;
	
	int brushr = 20;
	
	int destOffsetX, destOffsetY;

	void redraw()
	{
		imshow(titleSrc, imgDispSrc);
		imgDispDest = imgDest.clone();
		for(int i = 0; i < destw; ++i)
			for(int j = 0; j < desth; ++j)
			{
				int x = i - destOffsetX;
				int y = j - destOffsetY;
				if( x < 0 || y < 0 || x >= srcw || y >= srch)
					continue;
				Vec3b& vec = getcol(cov, x, y);
				if(!vec[0])
					imgDispDest.at<Vec3b>(Point(i, j)) = res.at<Vec3b>(Point(x, y));
			}
		imshow(titleDest, imgDispDest);
	}

	void renewA();

	void onMouseSrc( int event, int x, int y, int, void* )
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
			line(cov, Point(mousex, mousey), Point(x, y), (moused==1?Scalar(0,0,1):Scalar(1,1,1)),brushr);
			//circle(cov, Point(x, y), brushr, Scalar(0), CV_FILLED);
			imgDispSrc = imgSrc.clone().mul(cov);
			mousex = x, mousey = y;
			redraw();
		}
		if(event == CV_EVENT_LBUTTONUP || event == CV_EVENT_RBUTTONUP)
		{
			renewA();
			poi_run();
			redraw();
		}
	}

	void poi_run();

	void onMouseDest( int event, int x, int y, int, void* )
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
		case CV_EVENT_MOUSEMOVE:
			bo = moused;
			break;
		}
		if( bo )
		{
			destOffsetX += x - mousex;
			destOffsetY += y - mousey;
			mousex = x, mousey = y;
			redraw();
		}
		if(event == CV_EVENT_LBUTTONUP)
		{
			poi_run();
			redraw();
		}
	}

	void poi_init(CString fsrc, CString fdest)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fsrc);
		imgSrc = imread(tmps, IMREAD_COLOR);
		strcpy(tmps, (LPCSTR)fdest);
		imgDest = imread(tmps, IMREAD_COLOR);

		srcw = imgSrc.cols;
		srch = imgSrc.rows;

		cov = Mat::ones(srch, srcw, CV_8UC3);

		destw = imgDest.cols;
		desth = imgDest.rows;

		namedWindow(titleSrc , WINDOW_NORMAL ); // Create a window for display.
		namedWindow(titleDest , WINDOW_NORMAL ); // Create a window for display.

		setMouseCallback( titleSrc, onMouseSrc, 0 );
		setMouseCallback( titleDest, onMouseDest, 0 );
		
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
			{
				Vec3b& col = cov.at<Vec3b>(Point(i,j));
				col[0] = col[1] = col[2] = 1;
				/*
				if((i == 1 || i == 2) && (j == 1 || j == 2))
					col[0] = col[1] = 0;
				*/
			}
		imgDispSrc = imgSrc.clone();
		imgDispDest = imgDest.clone();
		renewA();
		poi_run();
		redraw();
	}
	
	void poi_work(int _mode)
	{
		mode = _mode;
		poi_run();
	}
	
	typedef Triplet<double> Tripd;
	vector<Tripd> tripletList;
	VectorXd b;
	SparseMatrix<double> A;
	SimplicialLDLT<SparseMatrix<double> > solver;

#define getn(x, y)	(ind[x][y])
#define getp(i)		(Point(indx[i], indy[i]))
	
	int dirx[4] = {-1, 0, 0, 1};
	int diry[4] = {0, -1, 1, 0};

	const int maxn = 1024;
	int ind[maxn][maxn];
	int indx[maxn*maxn];
	int indy[maxn*maxn];
	int indn;

	void updateInd()
	{
		indn = 0;
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
				if(iscov(i, j))
				{
					ind[i][j] = indn;
					indx[indn] = i;
					indy[indn] = j;
					indn++;
				}
	}

	void renewA()
	{
		tripletList.clear();
		updateInd();
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
			{
				if(!iscov(i, j))
					continue;
				int p = getn(i, j);
				double tmpb = 0;
				int cnt = 0;
				for(int dir = 0;dir < 4; ++dir)
				{
					int x = i + dirx[dir], y = j + diry[dir];
					if( x < 0 || y < 0 || x >= srcw || y >= srch)
						continue;
					cnt ++;
					int pp = getn(x, y);
					if(iscov(x, y))
					{
						// inner q
						tripletList.push_back(Tripd(p, pp, -1));
					}
				}
				tripletList.push_back(Tripd(p, p, cnt));
			}
		A = SparseMatrix<double>(indn, indn);
		A.setFromTriplets(tripletList.begin(), tripletList.end());
		solver.compute(A);
	}

	void poi_singleChannel(int col)
	{
		b = VectorXd(indn);
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
			{
				if(!iscov(i, j))
					continue;
				int p = getn(i, j);
				double tmpb = 0;
				for(int dir = 0;dir < 4; ++dir)
				{
					int x = i + dirx[dir], y = j + diry[dir];
					if( x < 0 || y < 0 || x >= srcw || y >= srch)
						continue;
					int xx = x + destOffsetX, yy = y + destOffsetY;
					int pp = getn(x, y);
					if(!iscov(x, y))
					{
						// border q
						if(xx < 0 || yy < 0 || x >= destw || y >= desth)
							tmpb += getcol(imgSrc, x, y)[col];
						else
							tmpb += getcol(imgDest, x + destOffsetX, y + destOffsetY)[col];
					}else
					{
						tmpb +=  getcol(imgSrc, i, j)[col] - getcol(imgSrc, x, y)[col];
					}
				}
				b(p) = tmpb;
			}
		VectorXd x = solver.solve(b);
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
			{
				if(!iscov(i, j))
					continue;
				int p = getn(i, j);
				getcol(res, i, j)[col] = rectify(x[p]);
			}
	}

	void poi_run()
	{
		res = imgSrc.clone();
		if(mode == 0)
			return;
		for(int i = 0; i < 3; ++i)
			poi_singleChannel(i);
	}

}
