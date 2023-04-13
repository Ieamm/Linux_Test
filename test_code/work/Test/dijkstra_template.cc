#include<vector>
#include<iostream>
#include<cstdio>
#include<queue>
using namespace std;

//顶点 | 边数
int n, m;
//前向指针数组
vector<int> Prev;
//记录该顶点的最短路径是否确认
vector<int> book;
//记录源点到各顶点的最短路径
vector<int> disv;

#define NONE 99999999

class node
{
public:
	node(int d, int w) : d_(d), w_(w)
	{}
	int d_;
	int w_; //源点到d_点的距离

	friend bool operator > (const node& lhs, const node& rhs);
};
bool operator > (const node& lhs, const node& rhs)
{
	return lhs.w_ > rhs.w_;
}

void dijkstra(int source, vector<vector<pair<int, int>>>&graph)
{
	//创建一个最小优先级队列,将距离源点消耗最小的边排在前面
	priority_queue<node,vector<node>, greater<node>> que;
	que.push(node(source, 0));//将源点置入队列
	disv[source] = 0;//源点到源点自然是0权
	
	int count = 0;
	while (!que.empty() && count <= n) //队列不为空 && n个顶点中还有没被找到最短路径的顶点
	{
		node temp = que.top();
		que.pop();
		book[temp.d_] = 1; //将已经得到最短路径的顶点标记
		disv[source] = 0; //源点到源点的距离
		count++; //得到最短路径的顶点+1

		//遍历当前已经确认的最短路顶点为源点的边,进行松弛.
		for (int i = 0; i < graph[temp.d_].size(); ++i)
		{
			//已确认最短路顶点的边
			auto k = graph[temp.d_][i];
			//源点到点k的最短路还未确定 && 源点 -> 点k > 源点->已确认最短路顶点+最短路顶点的联通点k
			if (!book[k.first] && disv[k.first] > disv[temp.d_] + k.second)
			{
				disv[k.first] = disv[temp.d_] + k.second;
				//node(已确定的最短路的联通点k, 源点到点k的距离)
				que.push(node(k.first, temp.w_ + k.second));
			}
		}
	}
}

void init(vector<vector<pair<int, int>>>&graph)
{
	Prev.resize(n+1, 0);
	book.resize(n+1, 0);
	disv.resize(n+1, NONE);

	for (int i = 1; i <= m; ++i)
	{
		int s, d, w;
		cin >> s >> d >> w;
		graph[s].push_back(make_pair(d,w));
	}
}

void Print()
{
	for (int i = 1; i <= n; ++i)
	{
		printf("(1,%d) w[%d] ", i, disv[i]);
	}
	cout << endl;
}

int main()
{
	cin >> n >> m;
	//pair的first作为每条边的dest,second作为每条边的权,下标作为source
	vector<vector<pair<int, int>>> graph(n+1); 
	init(graph);
	dijkstra(1, graph);
	Print();
	return 0;
}

// 6 10
// 1 2 1
// 1 3 3
// 2 3 9
// 2 4 4
// 3 4 1
// 3 5 5
// 4 3 4
// 4 5 13
// 4 6 15
// 5 6 4