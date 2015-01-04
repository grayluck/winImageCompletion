#include "stdafx.h"
#include "graphcut.h"


namespace graphcut
{
	// 0 for entire match, 1 for random
	int placeMode;
	
	int w, h;	// screen width and height
	int ww, hh;	// origin width and height
	int noww, nowh;	// width and height for current patch
	int rest;
	Mat img, imgDisp, ori;
	Mat imgDeb, tmpImg;

	const int maxn = 1024;
	const int maxm = 1000001;
	const int inf = 10000000;	// cannot be 2147483647, it will incur overflow

	int state[maxn][maxn];

	char* title = "graphcut";

	int targe, src;
	
	int next[maxm], fir[maxm];
	int t[maxm], f[maxm];
	int con;
	
	int q[maxm];

	int dist[maxm];
	int pre[maxm];

	int n;

	int fin;

	// add additional arcs for old seams
	int onedge[maxn][maxn][4];

	FILE* fdeb;
	
	DWORD WINAPI drawGraphcut( LPVOID lpParam ) 
	{
		while(1)
		{
			imshow(title, imgDisp);
			imshow("deb", imgDeb);
			Sleep(10);
		}
	}

	const double k = 1;
	const double sigma = 1;

	Point getPos()
	{
		if(placeMode == 0)
		{
			while(1)
			{
				int cnt = 0;
				int tot = 0;
				int x = rand() % w;
				int y = rand() % h;
				noww = min(w - x, ww);
				nowh = min(h - y, hh);
				for(int i = 0; i < noww; ++i)
					for(int j = 0; j < nowh; ++j)
						if(state[x + i][y + j])
						{
							cnt ++;
							Vec3b& colDisp = imgDisp.at<Vec3b>(Point(x + i, y +j));
							Vec3b& colnew = img.at<Vec3b>(Point(i, j));
							Vec3b tmpv = colDisp - colnew;
							tot += tmpv.dot(tmpv);
						}
				if(!cnt)	continue;
				double c = (double)tot / cnt;
				double p = exp(-c/(sigma*sigma*k));
				if((double)rand()/RAND_MAX < p)
					return Point(x, y);
			}
		}
		int x = rand() % w;
		int y = rand() % h;
		return Point(x, y);
	}

	void addline(int st, int en, int inpf)
	{
		inpf ++;
		// bidirection edges
		next[++con] = fir[st], fir[st] = con;
		t[con] = en, f[con] = inpf;
		next[++con] = fir[en], fir[en] = con;
		t[con] = st, f[con] = inpf;
	}

	bool bfs(int src)
	{
		int head, tail;
		q[head=tail=0] = src;
		for(int i = 0; i < n; ++i)
			dist[i] = inf;
		dist[src] = 0;
		while(head<=tail)
		{
			int p = q[head++];
			for(int i = fir[p]; i; i = next[i])
				if(f[src == 0 ? i : i^1] && dist[t[i]] > dist[p] + 1)
				{
					dist[t[i]] = dist[p] + 1;
					q[++tail] = t[i];
				}
		}
		return dist[targe] < inf;
	}

	int ret;
	int ans;

	void dfs(int p)
	{
		if(p == targe)
		{
			fin = 1;
			int minn = inf;
			for(int i = targe; i != src; i = t[pre[i]^1])
				minn = min(minn, f[pre[i]]);
			ans += minn;
			for(int i = targe; i != src; i = t[pre[i]^1])
			{
				f[pre[i]] -= minn, f[pre[i]^1] += minn;
				if(!f[pre[i]])
					ret = i;
			}
			return;
		}
		for(int i = fir[p]; i; i = next[i])
			if(f[i] && dist[t[i]] == dist[p] + 1)
			{
				pre[t[i]] = i;
				dfs(t[i]);
				if(fin)
					if(ret == i)
						fin = ret =0;
					else
						return;
			}
		dist[p] = -1;
	}

	#define getn(x, y)	((x) * nowh + (y) + 1)
	#define getp(a)		(Point(((a)-1)/nowh, ((a)-1)%nowh))

	int dirx[4] = {1, 0, 0, -1};
	int diry[4] = {0, 1, -1, 0};

	int getv(Mat& a, int ax, int ay, Mat& b, int bx, int by)
	{
		Vec3b& pa = a.at<Vec3b>(Point(ax, ay));
		Vec3b& pb = b.at<Vec3b>(Point(bx, by));
		Vec3b& delta = pa - pb;
		return delta.dot(delta);
	}
	
	DWORD WINAPI workGraphcut( LPVOID lpParam ) 
	{
		while(rest)	// there is still some pixel waits for fill
		{
			Point p = getPos();
			src = 0;
			noww = min(w - p.x, ww);
			nowh = min(h - p.y, hh);
			n = noww * nowh + 2;
			targe = n - 1;
			con = 1;
			for(int i = 0; i < n; ++i)
				fir[i] = 0;
			for(int i = 0; i < noww; ++i)
				for(int j = 0; j < nowh; ++j)
				{
					int x = p.x + i;
					int y = p.y + j;
					int pa = getn(i, j);
					int mark = 0;
					for(int dir = 0; dir < 4; ++dir)
					{
						int dx = x + dirx[dir];
						int dy = y + diry[dir];
						if(dx < 0 || dy < 0 || dx >=  w || dy >= h)
							continue;
						int outsider = (dx < p.x || dy < p.y ||
										dx >= p.x + noww || dy >= p.y + nowh);
						int pda = getn(i+dirx[dir], j+diry[dir]);
						if(state[x][y])
						{
							if(state[dx][dy])
							{
								// are both filled
								if(outsider)
									mark |= 1;
								else
								{
									// it is an inner node
									// add a arc
									if(dir < 2)	// avoid adding an edge twice
										if(!onedge[x][y][dir])
											addline(pa, pda, getv(imgDisp, x, y, img, i, j)
												+ getv(	imgDisp, dx, dy,
														img, i + dirx[dir], j + diry[dir]));
										else
										{
											fir[n] = 0;
											addline(n, targe, onedge[x][y][dir]);
											addline(pa, n,	getv(imgDisp, x, y, img, i, j)
														+	getv(tmpImg, dx, dy, img, i + dirx[dir], j + diry[dir]));
											addline(n, pda,	getv(tmpImg, x, y, img, i, j)
														+	getv(imgDisp, dx, dy, img, i + dirx[dir], j + diry[dir]));
											n++;
										}
								}
							}
							else
								mark |= 2;
						}else
							mark |= 2;
					}
					if(mark == 1)
						addline(src, pa,  inf);
					else
					if(mark == 2)
						addline(pa, targe, inf);
				}

				// do donic
				while(bfs(src))
				{
					fin = 0;
					dfs(src);
				}
				bfs(targe);
				imgDeb = Mat::ones(h, w, CV_8UC3);
				for(int x = 0; x < noww; ++x)
				for(int y = 0; y < nowh; ++y)
				{
					int i = getn(x, y);
					int px = p.x + x, py = p.y + y;
					// get cut-arc info
					for(int dir = 0; dir < 2; ++dir)
					{
						int pxx = px + dirx[dir], pyy = py + diry[dir];
						if(pxx < 0 || pyy < 0 || pxx >= noww || pyy >= nowh)
							continue;
						int j = getn(pxx, pyy);
						if(dist[j] <inf)
						{
							if(dist[i] < inf)
								onedge[px][py][dir] = 0;
							else
							{
								onedge[px][py][dir] = 
										getv(	imgDisp, px, py, img, x, y)
									+	getv(	imgDisp, px + dirx[dir], py + diry[dir],
												img, x + dirx[dir], y + diry[dir]);
								tmpImg.at<Vec3b>(Point(px, py)) = img.at<Vec3b>(Point(x, y));
								tmpImg.at<Vec3b>(Point(pxx, pyy)) = imgDisp.at<Vec3b>(Point(pxx, pyy));
							}
						}else
							if(dist[i] < inf)
							{
								onedge[px][py][dir] = 
										getv(	imgDisp, px, py, img, x, y)
									+	getv(	imgDisp, px + dirx[dir], py + diry[dir],
												img, x + dirx[dir], y + diry[dir]);
								tmpImg.at<Vec3b>(Point(px, py)) = imgDisp.at<Vec3b>(Point(px, py));
								tmpImg.at<Vec3b>(Point(pxx, pyy)) = img.at<Vec3b>(Point(x + dirx[dir], y + diry[dir]));
							}
					}
				}
				for(int x = 0; x < noww; ++x)
				for(int y = 0; y < nowh; ++y)
				{
					int i = getn(x, y);
					int px = p.x + x, py = p.y + y;
					if(dist[i] <inf)
					{
						//char buf[20];
						//sprintf(buf, "%d %d\n", x, y);
						// is filled by B
						//TRACE(buf);
						imgDisp.at<Vec3b>(Point(px, py)) = img.at<Vec3b>(Point(x, y));
						imgDeb.at<Vec3b>(Point(px, py)) = Vec3b(255, 255, 255);
						if(!state[px][py])
							rest --, state[px][py] = 1;
					}
				}
				//Sleep(1000);
		}
		return 0;
	}

	void graphcutReset()
	{
		ans = 0;
		img = Mat::ones(h, w, CV_8UC3);
		imgDeb = Mat::zeros(h, w, CV_8UC3);
		tmpImg = Mat::zeros(h, w, CV_8UC3);
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
					state[i][j] = 1;
				}else
				{
					colimg[0] = colimg[1] = colimg[2] = 0;
					state[i][j] = 0;
				}
			}
		imgDisp = img.clone();
		rest =  w * h * 3 / 4;
	}

	void graphcut_init(CString fname)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fname);
		ori = imread(tmps, IMREAD_COLOR);
		ww = ori.cols;
		hh = ori.rows;
		w = ori.cols * 2;
		h = ori.rows * 2;
		namedWindow(title , WINDOW_AUTOSIZE ); // Create a window for display.
		namedWindow("deb" , WINDOW_AUTOSIZE ); // Create a window for display.
		graphcutReset();
		CreateThread(0, 0, drawGraphcut, 0, 0, 0);
	}
	
	void runGraphcut(int _placeMode)
	{
		placeMode = _placeMode;
		CreateThread(0, 0, workGraphcut, 0, 0, 0);
	}

}
