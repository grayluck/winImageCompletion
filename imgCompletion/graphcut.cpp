#include "stdafx.h"
#include "graphcut.h"


namespace graphcut
{
	
	int w, h;	// screen width and height
	int ww, hh;	// origin width and height
	int noww, nowh;	// width and height for current patch
	int rest;
	Mat img, imgDisp, ori;

	const int maxn = 1001;
	const int maxm = 100001;
	const int inf = 10000000;	// cannot be 2147483647, it will incur overflow

	int state[maxn][maxn];

	char* title = "graphcut";

	int targe, src;
	
	int next[maxm], fir[maxm];
	int t[maxm], f[maxm];
	int con;
	
	int q[maxm];
	int head, tail;

	int dist[maxm];
	int pre[maxm];

	int n;

	int fin;
	
	DWORD WINAPI drawGraphcut( LPVOID lpParam ) 
	{
		while(1)
		{
			imshow(title, imgDisp);
			Sleep(100);
		}
	}

	Point getPos()
	{
		int x = rand() % w;
		int y = rand() % h;
		return Point(x, y);
	}

	void addline(int st, int en, int inpf)
	{
		if(st >= n || en >= n)
			st ++, st --;
		// bidirection edges
		next[++con] = fir[st], fir[st] = con;
		t[con] = en, f[con] = inpf;
		next[++con] = fir[en], fir[en] = con;
		t[con] = st, f[con] = inpf;
	}

	bool bfs()
	{
		q[head=tail=0] = src;
		for(int i = 0; i < n; ++i)
			dist[i] = inf;
		dist[src] = 0;
		while(head<=tail)
		{
			int p = q[head++];
			for(int i = fir[p]; i; i = next[i])
				if(f[i] && dist[t[i]] > dist[p] + 1)
				{
					dist[t[i]] = dist[p] + 1;
					q[++tail] = t[i];
				}
		}
		return dist[targe] < inf;
	}

	void dfs(int p)
	{
		if(p == targe)
		{
			fin = 1;
			int minn = inf;
			for(int i = targe; i != src; i = t[pre[i]^1])
				minn = min(minn, f[pre[i]]);
			for(int i = targe; i != src; i = t[pre[i]^1])
				f[pre[i]] -= minn, f[pre[i]^1] += minn;
			return;
		}
		for(int i = fir[p]; i && !fin; i = next[i])
			if(f[i] && dist[t[i]] == dist[p] + 1)
			{
				pre[t[i]] = i;
				dfs(t[i]);
			}
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
			noww = w - p.x;
			nowh = h - p.y;
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
									addline(pa, pda, getv(imgDisp, x, y, img, i, j)
										+ getv(	imgDisp, dx, dy,
												img, i + dirx[dir], j + diry[dir]));
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
						addline(pa, targe, inf);
				}

				// do donic
				while(bfs())
				{
					fin = 0;
					dfs(src);
				}

				for(int i = 1; i < n - 1; ++i)
				{
					Point& tmpp = getp(i);
					int x = tmpp.x, y = tmpp.y;
					int px = p.x + x, py = p.y + y;
					if(dist[i] >= inf)
					{
						// is filled by B
						imgDisp.at<Vec3b>(Point(px, py)) = img.at<Vec3b>(Point(x, y));
						if(!state[px][py])
							rest --, state[px][py] = 1;
					}
				}
		}
		return 0;
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
