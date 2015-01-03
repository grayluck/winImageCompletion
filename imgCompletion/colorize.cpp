
#include "stdafx.h"
#include "colorize.h"

#include <Eigen\SparseLU>

using namespace Eigen;

namespace clrz
{

	Mat img, imgDisp, res;
	Mat cov, allone, col;
	int w, h, n;
	
	const int maxn = 1024;

	double oriYUV[maxn][maxn][3], colYUV[maxn][maxn][3];
	double resYUV[maxn][maxn][3];

	const char* titleSrc = "colorize :: source";
	const char* titleDest = "colorize :: result";
	
	int brushr = 10;

	Scalar colSelected;

	int moused = 0;
	int mousex, mousey;
	
	void rgb2yuv(Vec3b v, double* out)
	{
		out[0] = (0.299*v[2] + 0.587*v[1] + 0.114*v[0]) / 255.0;
		out[1] = (-0.169*v[2] - 0.331*v[1] + 0.5*v[0] + 128) / 255.0;
		out[2] = (0.5*v[2] - 0.419*v[1] - 0.081*v[0] + 128) / 255.0;
	}

	void yuv2rgb(double* in, Vec3b& out)
	{
		double dr = in[0] + 1.13983 * (in[2]-0.5);
		double dg = in[0] - 0.39465 * (in[1]-0.5) - 0.58060 * (in[2]-0.5);
		double db = in[0] + 2.03211 * (in[1]-0.5);
		dr = min(1.0, max(0.0, dr));
		dg = min(1.0, max(0.0, dg));
		db = min(1.0, max(0.0, db));
		out[2] = 255 * dr;
		out[1] = 255 * dg;
		out[0] = 255 * db;
	}

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

	Mat imgRef;

	void openref(CString fname)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fname);
		imgRef = imread(tmps, IMREAD_COLOR);
		imshow("colorize :: color reference", imgRef);
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
			redraw();
		}
		if(event == CV_EVENT_LBUTTONUP || event == CV_EVENT_RBUTTONUP)
		{
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
		/*
		getcol(cov, 0, 0) = Vec3b(1, 1, 1);
		getcol(col, 0, 0) = Vec3b(0, 0, 1);
		*/
		imgDisp = img.clone();
		res = img.clone();
		setMouseCallback(titleSrc, onMouse, 0);
		
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
				rgb2yuv(getcol(img, i, j), oriYUV[i][j]);

		//work();
		redraw();
	}

	int sigx[10], sigy[10];
	double sigv[10];

	/*
	double geti(int x, int y)	// get intensity
	{
		Vec3b v = getcol(img, x, y);
		return (0.299*v[2] + 0.587*v[1]+ 0.114*v[0]) / 255.0;
	}
	*/
	
	typedef Triplet<double> Tripd;
	vector<Tripd> tripletList;
	VectorXd b;
	SparseMatrix<double> A;
	SparseLU<SparseMatrix<double> > solver;

	double debLst[maxn*maxn][3];

#define getn(x, y)	(x * h + y)
	
	DWORD WINAPI workSingleChannel(LPVOID params)
	{
		int channel = (int)params;
		tripletList.clear();
		b = VectorXd(w * h);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				int p = getn(i, j);
				int now = tripletList.size();
				debLst[now][0] = p, debLst[now][1] = p, debLst[now][2] = 1;
				tripletList.push_back(Tripd(p, p, 1));
				b(p) = getcol(cov, i, j)[channel] * colYUV[i][j][channel];
				if(getcol(cov, i, j)[channel])
					continue;
				int cnt = 0;
				double sum = 0, summ = 0;
				double orii = oriYUV[i][j][channel];
				double minn = 1;
				for(int x = i-1; x <= i+1; ++x)
					for(int y = j-1; y <= j+1; ++y)
					{
						if(x < 0 || y < 0 || x >= w || y >= h)
							continue;
						double tmp = oriYUV[x][y][channel];
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
				{
					int now = tripletList.size();
					debLst[now][0] = p, debLst[now][1] = getn(sigx[k], sigy[k]), debLst[now][2] = -sigv[k] / sum;
					tripletList.push_back(Tripd(debLst[now][0], debLst[now][1], debLst[now][2]));
				}
			}
		A = SparseMatrix<double>(n, n);
		A.setFromTriplets(tripletList.begin(), tripletList.end());
		solver.compute(A);
		VectorXd x = solver.solve(b);
		/*
		FILE* fdeb = fopen("debug.txt", "w");
		for(int i = 0; i < w * h; ++i)
		{
			for(int j = 0; j < w * h; ++j)
				fprintf(fdeb, "%7lf ", A.toDense()(i, j));
			fprintf(fdeb, "\n");
		}
		fprintf(fdeb, "\n");
		for(int i = 0; i < w * h; ++i)
			fprintf(fdeb, "%lf\n", b(i));
		fprintf(fdeb, "\n");
		for(int i = 0; i < w * h; ++i)
			fprintf(fdeb, "%lf\n", x(i));
		fclose(fdeb);
		*/
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				int p = getn(i, j);
				resYUV[i][j][channel] = x(p);
			}
	}

	void work()
	{
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
			{
				rgb2yuv(getcol(col, i, j), colYUV[i][j]);
				rgb2yuv(getcol(img, i, j), resYUV[i][j]);
			}
		HANDLE thd[3];
		for(int i = 1; i < 3; ++i)
		{
			//workSingleChannel(i);
			thd[i] = CreateThread(0, 0, workSingleChannel, (void*)i, 0, 0);
		}
		for(int i = 1; i < 3; ++i)
			WaitForSingleObject(thd[i], INFINITE);
		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j)
				yuv2rgb(resYUV[i][j], getcol(res, i, j));
	}
}
