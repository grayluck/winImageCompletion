
#include "stdafx.h"
#include "colorize.h"

#include <Eigen\SparseCholesky>

using namespace Eigen;

namespace clrz
{

	Mat img, imgDisp, res;
	Mat cov, allone, col;
	int w, h, n;

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

	void work();

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
			work();
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
		n = w * h;
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

	int sigx[10], sigy[10];
	double sigv[10];

	double geti(int x, int y)	// get intensity
	{
		Vec3b v = getcol(img, x, y);
		return (0.299*v[2] + 0.587*v[1]+ 0.114*v[0]) / 255.0;
	}
	
	typedef Triplet<double> Tripd;
	vector<Tripd> tripletList;
	VectorXd b;
	SparseMatrix<double> A;
	SimplicialLDLT<SparseMatrix<double> > solver;

#define getn(x, y)	(x * h + y)

	void workSingleChannel(int channel)
	{
		tripletList.clear();
		b = VectorXd(w * h);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				int p = getn(i, j);
				tripletList.push_back(Tripd(p, p, 1));
				b(p) = getcol(cov, i, j)[channel] * getcol(col, i, j)[channel] / 255.0;
				if(getcol(cov, i, j)[channel])
					continue;
				int cnt = 0;
				double sum = 0, summ = 0;
				double orii = geti(i, j);
				double minn = 1e100;
				for(int x = i-1; x <= i+1; ++x)
					for(int y = j-1; y <= j+1; ++y)
					{
						if(x < 0 || y < 0 || x >= w || y >= h)
							continue;
						double tmp = geti(x, y);
						sum += tmp;
						summ += tmp * tmp;
						if( x == i && y == j)	continue;
						sigx[cnt] = x, sigy[cnt] = y;
						sigv[cnt] = (tmp - orii) * (tmp - orii);
						minn = min(minn, sigv[cnt]);
						cnt++;
					}
				double sigma = (summ - sum * sum / (cnt + 1)) / cnt;
				sigma = max(sigma, 2e-6);
				sigma = max(minn / 4.60517018, sigma);
				sum = 0;
				for(int k = 0; k < cnt; ++k)
				{
					sigv[k] = exp(-sigv[k] / sigma);
					sum += sigv[k];
				}
				for(int k = 0; k < cnt; ++k)
					tripletList.push_back(Tripd(p, getn(sigx[k], sigy[k]), -sigv[k] / sum));
			}
		A = SparseMatrix<double>(n, n);
		A.setFromTriplets(tripletList.begin(), tripletList.end());
		solver.compute(A);
		VectorXd x = solver.solve(b);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				int p = getn(i, j);
				getcol(res, i, j)[channel] = rectify(x[p]) * 255;
			}
	}

	void work()
	{
		for(int i = 0; i < 2; ++i)
			workSingleChannel(i);
	}
}
