#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <algorithm>


#define GRIDSIZE 15
#define judge_black 0
#define judge_white 1
#define grid_blank 0
#define grid_black 1
#define grid_white 7
#define INF 0x3f3f3f3f

using namespace std;

int threshold = 0.98 * (double)CLOCKS_PER_SEC; //CLOCKS_PER_SEC 和clock()的含义可百度
int start_time, current_time;


class point {
public:
	int x = -3;
	int y = -3;
	int a = -3;
	int b = -3;
	double v = 0;
};
point p;
point points[20000];

class maxmin {
public:
	int max;
	int min;
};

int fx = -1, fy = -1, sx = -1, sy = -1;

int currBotColor; // 本方所执子颜色（1为黑，7为白，棋盘状态亦同）
int currEnermyColor;
int gridInfo[GRIDSIZE][GRIDSIZE] = { 0 }; // 先x后y，记录棋盘状态

int turnID;
int maxx = -2, maxy = -2, minx = -2, miny = -2;
int rangemaxx;
int rangeminx;
int rangemaxy;
int rangeminy;

int numberOfEnemyConnected[7];
int numberOfMyConnected[7];
int scoreOfEnemyRoad[7] = { 0,1,5,25,350,250,10000 };
int scoreOfMyRoad[7] = { 0,1,5,20,250,20,10000 };
int scoreOfEnemyLink[2][7] = { {0, 1,10,20,350,250,10000 },{0, 1,10,20,45,30,10000 } };
int scoreOfMyLink[2][7] = { {0, 1,10,15,35,15,10000 } ,{0, 1,10,15,350,15,10000 } };
int baseScore;
double ALERTSCORE;
int black4, white4;
bool attack;//如果敌方为防守型，则attack值为1，表示我方进攻决策


// 比较函数，按照v的大小进行排序
bool compareBymax(const point& a, const point& b) {
	return a.v > b.v;
}
// 比较函数，按照v的大小进行排序
bool compareBymin(const point& a, const point& b) {
	return a.v < b.v;
}


// 判断是否在棋盘内
inline bool inMap(int x, int y)
{
	if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
		return false;
	return true;
}


//统计死4，死5数量
void AH4() {
	for (int i = 0; i < GRIDSIZE; i++) {
		for (int j = 0; j < GRIDSIZE - 5; j++) {
			int S = gridInfo[i][j] + gridInfo[i][j + 1] + gridInfo[i][j + 2] + gridInfo[i][j + 3] + gridInfo[i][j + 4] + gridInfo[i][j + 5];
			if (S == 18 || S == 12 || S == 11) {
				black4++;
			}
			else if (S == 30 || S == 36 || S == 29) {
				white4++;
			}
		}
	}
}
void AV4() {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 0; j < GRIDSIZE; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j] + gridInfo[i + 2][j] + gridInfo[i + 3][j] + gridInfo[i + 4][j] + gridInfo[i + 5][j];
			if (S == 18 || S == 12 || S == 11) {
				black4++;
			}
			else if (S == 30 || S == 36 || S == 29) {
				white4++;
			}
		}
	}
}
void ARD4() {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 0; j < GRIDSIZE - 5; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j + 1] + gridInfo[i + 2][j + 2] + gridInfo[i + 3][j + 3] + gridInfo[i + 4][j + 4] + gridInfo[i + 5][j + 5];
			if (S == 18 || S == 12 || S == 11) {
				black4++;
			}
			else if (S == 30 || S == 36 || S == 29) {
				white4++;
			}
		}
	}
}
void ALD4() {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 5; j < GRIDSIZE; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j - 1] + gridInfo[i + 2][j - 2] + gridInfo[i + 3][j - 3] + gridInfo[i + 4][j - 4] + gridInfo[i + 5][j - 5];
			if (S == 18 || S == 12 || S == 11) {
				black4++;
			}
			else if (S == 30 || S == 36 || S == 29) {
				white4++;
			}
		}
	}
}
void linkAnalyse4() {
	black4 = 0;
	white4 = 0;
	AH4();
	AV4();
	ALD4();
	ARD4();
}


//分析对方为进攻型还是防守型
void AnalyzeAttackorDefence() {
	linkAnalyse4();
	if (currBotColor == 1) {//如果我方是黑
		if (black4 > white4) {//且黑4>白4
			attack = 1;
		}
		else {
			attack = 0;
		}
	}
	else {//我方是白
		if (black4 >= white4) {//且黑4>白4
			attack = 0;
		}
		else {
			attack = 1;
		}
	}
}

//获得警戒分数
void getALERTSCORE(int color) {
	if (attack) {
		if (color == currBotColor) {
			ALERTSCORE = -200;
		}
		else {
			ALERTSCORE = 0;
		}
	}
	else {
		if (color == currBotColor) {
			ALERTSCORE = 0;
		}
		else {
			ALERTSCORE = -200;
		}
	}
	if (color == 7 && ALERTSCORE == 0) {
		ALERTSCORE = -200;
	}
}


// 在坐标处落子，检查是否合法或模拟落子
bool ProcStep(int x0, int y0, int x1, int y1, int grid_color, bool check_only)
{
	if (x1 == -1 || y1 == -1) {
		if (!inMap(x0, y0) || gridInfo[x0][y0] != grid_blank)
			return false;
		if (!check_only) {
			gridInfo[x0][y0] = grid_color;
		}
		return true;
	}
	else {
		if ((!inMap(x0, y0)) || (!inMap(x1, y1)))
			return false;
		if (gridInfo[x0][y0] != grid_blank || gridInfo[x1][y1] != grid_blank)
			return false;
		if (!check_only) {
			gridInfo[x0][y0] = grid_color;
			gridInfo[x1][y1] = grid_color;
		}
		return true;
	}
}


//搜索点周围的25个点
int search25(int x, int y) {
	int ki = 0;
	for (int i = x - 2 > 0 ? x - 2 : 0; i <= (x + 2 < GRIDSIZE ? x + 2 : GRIDSIZE); i++, ki++) {
		int kj = 0;
		for (int j = y - 2 > 0 ? y - 2 : 0; j <= (y + 2 < GRIDSIZE ? y + 2 : GRIDSIZE); j++, kj++) {
			if (gridInfo[i][j] != 0)return 1;
		}
	}
	return 0;
}


//更新边界函数
void updaterange() {
	rangemaxx = maxx;
	rangeminx = minx;
	rangemaxy = maxy;
	rangeminy = miny;
	for (int i = 0; i < 2 && rangemaxx < 14; i++) rangemaxx++;
	for (int i = 0; i < 2 && rangeminx > 0; i++) rangeminx--;
	for (int i = 0; i < 2 && rangemaxy < 14; i++) rangemaxy++;
	for (int i = 0; i < 2 && rangeminy > 0; i++) rangeminy--;
	return;
}


//全局估值函数
void AH(int color) {
	for (int i = 0; i < GRIDSIZE; i++) {
		for (int j = 0; j < GRIDSIZE - 5; j++) {
			int S = gridInfo[i][j] + gridInfo[i][j + 1] + gridInfo[i][j + 2] + gridInfo[i][j + 3] + gridInfo[i][j + 4] + gridInfo[i][j + 5];
			if (S != 0 && (S <= 6 || S % 7 == 0)) {

				if (S < 7) {
					if (color == 1) numberOfMyConnected[S]++;
					else numberOfEnemyConnected[S]++;
				}

				else {
					if (color == 7) numberOfMyConnected[S / 7]++;
					else numberOfEnemyConnected[S / 7]++;
				}
			}
		}
	}
}
void AV(int color) {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 0; j < GRIDSIZE; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j] + gridInfo[i + 2][j] + gridInfo[i + 3][j] + gridInfo[i + 4][j] + gridInfo[i + 5][j];
			if (S != 0 && (S <= 6 || S % 7 == 0)) {

				if (S < 7) {
					if (color == 1) numberOfMyConnected[S]++;
					else numberOfEnemyConnected[S]++;
				}

				else {
					if (color == 7) numberOfMyConnected[S / 7]++;
					else numberOfEnemyConnected[S / 7]++;
				}
			}
		}
	}
}
void ARD(int color) {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 0; j < GRIDSIZE - 5; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j + 1] + gridInfo[i + 2][j + 2] + gridInfo[i + 3][j + 3] + gridInfo[i + 4][j + 4] + gridInfo[i + 5][j + 5];
			if (S != 0 && (S <= 6 || S % 7 == 0)) {

				if (S < 7) {
					if (color == 1) numberOfMyConnected[S]++;
					else numberOfEnemyConnected[S]++;
				}

				else {
					if (color == 7) numberOfMyConnected[S / 7]++;
					else numberOfEnemyConnected[S / 7]++;
				}
			}
		}
	}
}
void ALD(int color) {
	for (int i = 0; i < GRIDSIZE - 5; i++) {
		for (int j = 5; j < GRIDSIZE; j++) {
			int S = gridInfo[i][j] + gridInfo[i + 1][j - 1] + gridInfo[i + 2][j - 2] + gridInfo[i + 3][j - 3] + gridInfo[i + 4][j - 4] + gridInfo[i + 5][j - 5];
			if (S != 0 && (S <= 6 || S % 7 == 0)) {

				if (S < 7) {
					if (color == 1) numberOfMyConnected[S]++;
					else numberOfEnemyConnected[S]++;
				}

				else {
					if (color == 7) numberOfMyConnected[S / 7]++;
					else numberOfEnemyConnected[S / 7]++;
				}
			}
		}
	}
}
void linkAnalyse(int color) {
	for (int i = 0; i < 7; i++) {
		numberOfEnemyConnected[i] = 0;
		numberOfMyConnected[i] = 0;
	}
	AH(color);
	AV(color);
	ALD(color);
	ARD(color);
}


//获得基准分数（当前棋局分数）
void getbaseScore() {
	baseScore = 0;
	for (int i = 1; i < 7; i++) {
		baseScore += (numberOfMyConnected[i] * scoreOfMyRoad[i] - numberOfEnemyConnected[i] * scoreOfEnemyRoad[i]);
	}
}

//获得基准分数（当前棋局分数）
int getScore() {
	int Score = 0;
	for (int i = 1; i < 7; i++) {
		Score += (numberOfMyConnected[i] * scoreOfMyRoad[i] - numberOfEnemyConnected[i] * scoreOfEnemyRoad[i]);
	}
	return Score;
}

//获得全局估值
int getsumvalue() {

	int value = 0;
	int condition = 0;


	if (baseScore > ALERTSCORE) {
		condition = 1;
	}

	for (int i = 1; i < 7; i++) {
		value += (numberOfMyConnected[i] * scoreOfMyLink[condition][i] - numberOfEnemyConnected[i] * scoreOfEnemyLink[condition][i]);
	}

	return value;
}


//局部重新估值
void analysisHorizon(int color, int x, int y, int a, int b) {
	for (int i = x - 5 > 0 ? x - 5 : 0; i <= x && i + 5 < GRIDSIZE; i++) {
		int number = gridInfo[i][y] + gridInfo[i + 1][y] + gridInfo[i + 2][y] + gridInfo[i + 3][y] + gridInfo[i + 4][y] + gridInfo[i + 5][y];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
	for (int i = a - 5 > 0 ? a - 5 : 0; i <= a && i + 5 < GRIDSIZE; i++) {
		if (y == b && (i == x || i + 1 == x || i + 2 == x || i + 3 == x || i + 4 == x || i + 5 == x)) continue;//去重复
		int number = gridInfo[i][b] + gridInfo[i + 1][b] + gridInfo[i + 2][b] + gridInfo[i + 3][b] + gridInfo[i + 4][b] + gridInfo[i + 5][b];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
}
void analysisVertical(int color, int x, int y, int a, int b) {
	for (int i = y - 5 > 0 ? y - 5 : 0; i <= y && i + 5 < GRIDSIZE; i++) {
		int number = gridInfo[x][i] + gridInfo[x][i + 1] + gridInfo[x][i + 2] + gridInfo[x][i + 3] + gridInfo[x][i + 4] + gridInfo[x][i + 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
	for (int i = b - 5 > 0 ? b - 5 : 0; i <= b && i + 5 < GRIDSIZE; i++) {
		if (x == a && (i == y || i + 1 == y || i + 2 == y || i + 3 == y || i + 4 == y || i + 5 == y)) continue;//去重复
		int number = gridInfo[a][i] + gridInfo[a][i + 1] + gridInfo[a][i + 2] + gridInfo[a][i + 3] + gridInfo[a][i + 4] + gridInfo[a][i + 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
}
void analysisLeft(int color, int x, int y, int a, int b) {

	int i, j;

	if (y - 5 > 0 && x - 5 > 0) {
		j = y - 5;
		i = x - 5;
	}
	else {
		if (x > y) {
			j = 0;
			i = x - y;
		}
		else {
			i = 0;
			j = y - x;
		}
	}

	for (; i <= x && i + 5 < GRIDSIZE && j <= y && j + 5 < GRIDSIZE; i++, j++) {

		int number = gridInfo[i][j] + gridInfo[i + 1][j + 1] + gridInfo[i + 2][j + 2] + gridInfo[i + 3][j + 3] + gridInfo[i + 4][j + 4] + gridInfo[i + 5][j + 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}

	if (b - 5 > 0 && a - 5 > 0) {
		j = b - 5;
		i = a - 5;
	}
	else {
		if (a > b) {
			j = 0;
			i = a - b;
		}
		else {
			i = 0;
			j = b - a;
		}
	}

	for (; i <= a && i + 5 < GRIDSIZE && j <= b && j + 5 < GRIDSIZE; i++, j++) {
		if ((i == x && j == y) || (i + 1 == x && j + 1 == y) || (i + 2 == x && j + 2 == y) || (i + 3 == x && j + 3 == y) || (i + 4 == x && j + 4 == y) || (i + 5 == x && j + 5 == y))continue;//去重复
		int number = gridInfo[i][j] + gridInfo[i + 1][j + 1] + gridInfo[i + 2][j + 2] + gridInfo[i + 3][j + 3] + gridInfo[i + 4][j + 4] + gridInfo[i + 5][j + 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
}
void analysisRight(int color, int x, int y, int a, int b) {

	int i, j;
	if (x - 5 > 0 && y + 5 < GRIDSIZE - 1) {
		i = x - 5;
		j = y + 5;
	}
	else {
		if (x > GRIDSIZE - 1 - y) {
			i = x - (GRIDSIZE - 1 - y);
			j = GRIDSIZE - 1;
		}
		else {
			i = 0;
			j = y + x;
		}
	}


	for (; i <= x && i + 5 < GRIDSIZE && j >= y && j - 5 >= 0; i++, j--) {
		int number = gridInfo[i][j] + gridInfo[i + 1][j - 1] + gridInfo[i + 2][j - 2] + gridInfo[i + 3][j - 3] + gridInfo[i + 4][j - 4] + gridInfo[i + 5][j - 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}

	if (a - 5 > 0 && b + 5 < GRIDSIZE - 1) {
		i = a - 5;
		j = b + 5;
	}
	else {
		if (a > GRIDSIZE - 1 - b) {
			i = a - (GRIDSIZE - 1 - b);
			j = GRIDSIZE - 1;
		}
		else {
			i = 0;
			j = b + a;
		}
	}

	for (; i <= a && i + 5 < GRIDSIZE && j >= b && j - 5 >= 0; i++, j--) {
		if ((i == x && j == y) || (i + 1 == x && j - 1 == y) || (i + 2 == x && j - 2 == y) || (i + 3 == x && j - 3 == y) || (i + 4 == x && j - 4 == y) || (i + 5 == x && j - 5 == y))continue;//去重复
		int number = gridInfo[i][j] + gridInfo[i + 1][j - 1] + gridInfo[i + 2][j - 2] + gridInfo[i + 3][j - 3] + gridInfo[i + 4][j - 4] + gridInfo[i + 5][j - 5];
		if (number == 0 || (number > 6 && number % 7 != 0)) {
			continue;
		}
		if (number < 7) {
			if (color == grid_black) {
				numberOfMyConnected[number]++;
			}
			else {
				numberOfEnemyConnected[number]++;
			}
		}
		else {
			if (color == grid_black) {
				numberOfEnemyConnected[number / 7]++;
			}
			else {
				numberOfMyConnected[number / 7]++;
			}
		}
	}
}
void reanalysis(int color, int x, int y, int a, int b) {
	for (int i = 0; i < 7; i++) {
		numberOfEnemyConnected[i] = 0;
		numberOfMyConnected[i] = 0;
	}
	analysisHorizon(color, x, y, a, b);
	analysisVertical(color, x, y, a, b);
	analysisLeft(color, x, y, a, b);
	analysisRight(color, x, y, a, b);
}


//得到x，y的最大最小值
void detectminmax() {
	int i;
	int j;
	for (i = GRIDSIZE - 1; i >= 0; i--) {
		for (j = GRIDSIZE - 1; j >= 0; j--) {
			if (gridInfo[i][j] != 0)break;
		}

		if (j >= 0)break;
	}
	maxx = i;

	for (j = GRIDSIZE - 1; j >= 0; j--) {
		for (i = GRIDSIZE - 1; i >= 0; i--) {
			if (gridInfo[i][j] != 0)break;
		}
		if (i >= 0)break;
	}
	maxy = j;

	for (i = 0; i < GRIDSIZE; i++) {
		for (j = 0; j < GRIDSIZE; j++) {
			if (gridInfo[i][j] != 0)break;
		}
		if (j < GRIDSIZE)break;
	}
	minx = i;


	for (j = 0; j < GRIDSIZE; j++) {
		for (i = 0; i < GRIDSIZE; i++) {
			if (gridInfo[i][j] != 0)break;
		}
		if (i < GRIDSIZE)break;
	}
	miny = j;
}


//模拟下子
void move(int color, int x, int y) {
	gridInfo[x][y] = color;
	if (maxx < x) {
		maxx = x;
	}
	else if (minx > x) {
		minx = x;
	}
	if (maxy < y) {
		maxy = y;
	}
	else if (miny > y) {
		miny = y;
	}
	updaterange();
}
//撤销下子
void remove(int x, int y) {
	gridInfo[x][y] = 0;
	int i;
	int j;
	if (maxx == x) {
		for (i = GRIDSIZE - 1; i >= 0; i--) {
			for (j = GRIDSIZE - 1; j >= 0; j--) {
				if (gridInfo[i][j] != 0)break;
			}

			if (j >= 0)break;
		}
		maxx = i;
	}
	else if (minx == x) {
		for (i = 0; i < GRIDSIZE; i++) {
			for (j = 0; j < GRIDSIZE; j++) {
				if (gridInfo[i][j] != 0)break;
			}
			if (j < GRIDSIZE)break;
		}
		minx = i;
	}
	if (maxy == y) {
		for (j = GRIDSIZE - 1; j >= 0; j--) {
			for (i = GRIDSIZE - 1; i >= 0; i--) {
				if (gridInfo[i][j] != 0)break;
			}
			if (i >= 0)break;
		}
		maxy = j;
	}
	else if (miny == y) {
		for (j = 0; j < GRIDSIZE; j++) {
			for (i = 0; i < GRIDSIZE; i++) {
				if (gridInfo[i][j] != 0)break;
			}
			if (i < GRIDSIZE)break;
		}
		miny = j;
	}

	updaterange();
}


//重新计算全局估值
int regetsumvalue(int movecolor, int mycolor, int x, int y, int a, int b) {

	int vp = 0, va = 0;

	reanalysis(mycolor, x, y, a, b);
	vp = getsumvalue();
	gridInfo[x][y] = movecolor;
	gridInfo[a][b] = movecolor;
	reanalysis(mycolor, x, y, a, b);
	va = getsumvalue();
	gridInfo[x][y] = 0;
	gridInfo[a][b] = 0;

	return va - vp;;
}


double findbest(int color, int depth, int max, int min);
double findworst(int color, int depth, int max, int min);
//递归博弈树极大
double findbest(int color, int depth, int max, int min) {

	double maxval = -INF;
	int maxvalx = -4, maxvaly = -4, maxvala = -4, maxvalb = -4;;
	double maxv = -INF;
	point pointk[21];
	int k = 0;
	//获得全局估值
	getALERTSCORE(color);
	linkAnalyse(currBotColor);
	getbaseScore();

	for (int i = rangeminx; i <= rangemaxx; i++) {
		for (int j = rangeminy; j <= rangemaxy; j++) {
			if (gridInfo[i][j] == 0 && search25(i, j)) {
				int a = i;
				for (int b = j + 1; b <= rangemaxy; b++) {
					if (gridInfo[a][b] == 0 && (a != i || b != j) && search25(a, b)) {
						double v = regetsumvalue(color, color, i, j, a, b);
						if (v > maxv * 0.7) {
							points[k] = { i,j,a,b,v };
							k++;
							if (v > maxv) {
								maxv = v;
							}
						}
					}
				}
				for (int a = i + 1; a <= rangemaxx; a++) {
					for (int b = rangeminy; b <= rangemaxy; b++) {
						if (gridInfo[a][b] == 0 && (a != i || b != j) && search25(a, b)) {
							double v = regetsumvalue(color, color, i, j, a, b);
							if (v > maxv * 0.7) {
								points[k] = { i,j,a,b,v };
								k++;
								if (v > maxv) {
									maxv = v;
								}
							}
						}
					}
				}
			}
		}
	}

	std::sort(points, points + k, compareBymax);//从大到小排序

	//double o = points[20].v / points[0].v;


	if (depth > 0) {
		for (int i = 0; i < 21; i++) {//取前21
			pointk[i].v = points[i].v;
			pointk[i].x = points[i].x;
			pointk[i].y = points[i].y;
			pointk[i].a = points[i].a;
			pointk[i].b = points[i].b;
		}

		if (pointk[0].v > 5000) {
			if (depth == 3) {
				fx = pointk[0].x;
				fy = pointk[0].y;
				sx = pointk[0].a;
				sy = pointk[0].b;
			}
			return  INF / (4 - depth) + pointk[0].v;
		}
		for (int i = 0; i < 8 + 4 * depth && i < k; i++) {//选前x个递归

			move(color, pointk[i].x, pointk[i].y);
			move(color, pointk[i].a, pointk[i].b);

			pointk[i].v = findworst(color, depth - 1, maxval, min);

			current_time = clock();
			if (current_time - start_time > threshold) { //到0.98 秒立即跳出循环
				break;
			}

			if (pointk[i].v >= maxval) {
				maxval = pointk[i].v;
				maxvalx = pointk[i].x;
				maxvaly = pointk[i].y;
				maxvala = pointk[i].a;
				maxvalb = pointk[i].b;
			}

			remove(pointk[i].x, pointk[i].y);
			remove(pointk[i].a, pointk[i].b);

			if (maxval >= min) {
				break;
			}
		}
	}

	else {
		gridInfo[pointk[0].x][pointk[0].y] = color;
		gridInfo[pointk[0].a][pointk[0].b] = color;

		getALERTSCORE(color);
		linkAnalyse(color);
		maxval = getScore();

		gridInfo[pointk[0].x][pointk[0].y] = 0;
		gridInfo[pointk[0].a][pointk[0].b] = 0;
	}

	fx = maxvalx;
	fy = maxvaly;
	sx = maxvala;
	sy = maxvalb;

	return maxval;
}
//递归博弈树极小
double findworst(int color, int depth, int max, int min) {

	double minval = INF;
	double minv = INF;

	int ecolor;
	if (color == 1) {
		ecolor = 7;
	}
	else {
		ecolor = 1;
	}

	point pointk[21];
	int k = 0;
	//获得全局估值
	getALERTSCORE(ecolor);
	linkAnalyse(currBotColor);
	getbaseScore();

	for (int i = rangeminx; i <= rangemaxx; i++) {
		for (int j = rangeminy; j <= rangemaxy; j++) {
			if (gridInfo[i][j] == 0 && search25(i, j)) {
				int a = i;
				for (int b = j + 1; b <= rangemaxy; b++) {
					if (gridInfo[a][b] == 0 && (a != i || b != j) && search25(a, b)) {
						double v = regetsumvalue(ecolor, color, i, j, a, b);
						if (v < minv * 0.7) {
							points[k] = { i,j,a,b,v };
							k++;
							if (v < minv) {
								minv = v;
							}
						}
					}
				}
				for (int a = i + 1; a <= rangemaxx; a++) {
					for (int b = rangeminy; b <= rangemaxy; b++) {
						if (gridInfo[a][b] == 0 && (a != i || b != j) && search25(a, b)) {
							double v = regetsumvalue(ecolor, color, i, j, a, b);
							if (v < minv * 0.7) {
								points[k] = { i,j,a,b,v };
								k++;
								if (v < minv) {
									minv = v;
								}
							}
						}
					}
				}
			}
		}
	}

	std::sort(points, points + k, compareBymin);//从小到大排序

	//double o = points[20].v / points[0].v;

	if (depth > 0) {
		for (int i = 0; i < 21; i++) {//取前21
			pointk[i].v = points[i].v;
			pointk[i].x = points[i].x;
			pointk[i].y = points[i].y;
			pointk[i].a = points[i].a;
			pointk[i].b = points[i].b;
		}

		if (pointk[0].v < -5000) {
			return -INF / (4 - depth) + pointk[0].v;
		}

		for (int i = 0; i < 8 + 4 * depth && i < k; i++) {//选前x个递归

			move(ecolor, pointk[i].x, pointk[i].y);
			move(ecolor, pointk[i].a, pointk[i].b);

			pointk[i].v = findbest(color, depth - 1, max, minval);

			current_time = clock();
			if (current_time - start_time > threshold) { //到0.98 秒立即跳出循环
				break;
			}

			if (pointk[i].v < minval) {
				minval = pointk[i].v;
			}

			remove(pointk[i].x, pointk[i].y);
			remove(pointk[i].a, pointk[i].b);

			if (minval <= max) {
				break;
			}
		}
	}

	else {
		gridInfo[pointk[0].x][pointk[0].y] = ecolor;
		gridInfo[pointk[0].a][pointk[0].b] = ecolor;

		getALERTSCORE(ecolor);
		linkAnalyse(color);
		minval = getScore();

		gridInfo[pointk[0].x][pointk[0].y] = 0;
		gridInfo[pointk[0].a][pointk[0].b] = 0;

	}

	return minval;
}


//主函数
int main() {

	start_time = current_time = clock();
	int x0, y0, x1, y1;

	// 分析自己收到的输入和自己过往的输出，并恢复棋盘状态

	cin >> turnID;
	currBotColor = grid_white; // 先假设自己是白方
	for (int i = 0; i < turnID; i++)
	{
		// 根据这些输入输出逐渐恢复状态到当前回合
		cin >> x0 >> y0 >> x1 >> y1;
		if (x0 == -1) {
			currBotColor = grid_black; // 第一回合收到坐标是-1, -1，说明我是黑方
			if (turnID == 1) {
				srand((unsigned int)time(0));
				cout << 7 << ' ' << 7 << ' ' << -1 << ' ' << -1 << endl;
				return 0;
			}
		}
		if (turnID == 1) {
			if (x1 == -1) {
				if (x0 >= 4 && x0 <= 10 && y0 >= 4 && y0 <= 10) {
					if (x0 < 7) {
						cout << x0 << ' ' << y0 + 1 << ' ' << x0 + 1 << ' ' << y0 - 1 << endl;
					}
					else {
						cout << x0 << ' ' << y0 + 1 << ' ' << x0 - 1 << ' ' << y0 - 1 << endl;
					}
				}
				else {
					if (x0 < 7) {
						if (y0 < 7) {
							if (x0 < y0) {
								cout << 7 << ' ' << 7 << ' ' << 7 << ' ' << 5 << endl;
							}
							else {
								cout << 7 << ' ' << 7 << ' ' << 5 << ' ' << 7 << endl;
							}
						}
						else {
							if (x0 < GRIDSIZE - 1 - y0) {
								cout << 7 << ' ' << 7 << ' ' << 7 << ' ' << 9 << endl;
							}
							else {
								cout << 7 << ' ' << 7 << ' ' << 5 << ' ' << 7 << endl;
							}
						}
					}
					else {
						if (y0 < 7) {
							if (GRIDSIZE - 1 - x0 < y0) {
								cout << 7 << ' ' << 7 << ' ' << 7 << ' ' << 5 << endl;
							}
							else {
								cout << 7 << ' ' << 7 << ' ' << 9 << ' ' << 7 << endl;
							}
						}
						else {
							if (GRIDSIZE - 1 - x0 < GRIDSIZE - 1 - y0) {
								cout << 7 << ' ' << 7 << ' ' << 7 << ' ' << 9 << endl;
							}
							else {
								cout << 7 << ' ' << 7 << ' ' << 9 << ' ' << 7 << endl;
							}
						}
					}
				}
				return 0;
			}
		}


		if (currBotColor == 1) {
			currEnermyColor = 7;
		}
		else {
			currEnermyColor = 1;
		}

		if (x0 >= 0) {
			ProcStep(x0, y0, x1, y1, currEnermyColor, false); // 模拟对方落子
		}
		if (i < turnID - 1) {
			cin >> x0 >> y0 >> x1 >> y1;
			if (x0 >= 0) {
				ProcStep(x0, y0, x1, y1, currBotColor, false); // 模拟己方落子
			}
		}
	}

	//以下是自己代码

	if (turnID == 2 && currBotColor == grid_black) {

		if (gridInfo[7][8] == 7 && gridInfo[6][6] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[8][6] == 7) {
			cout << 8 << ' ' << 7 << ' ' << 6 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[8][6] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[8][8] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[7][6] == 7 && gridInfo[8][8] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[7][6] == 7 && gridInfo[6][8] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[6][8] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[6][6] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}

		//第一种开局
		if (gridInfo[7][6] == 7 && gridInfo[8][7] == 7) {
			cout << 9 << ' ' << 8 << ' ' << 9 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[7][6] == 7 && gridInfo[6][7] == 7) {
			cout << 8 << ' ' << 5 << ' ' << 9 << ' ' << 5 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[7][8] == 7) {
			cout << 5 << ' ' << 5 << ' ' << 5 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[8][7] == 7) {
			cout << 5 << ' ' << 9 << ' ' << 6 << ' ' << 9 << endl;
			return 0;
		}
		//第二种开局
		if (gridInfo[7][6] == 7 && gridInfo[6][5] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[7][6] == 7 && gridInfo[8][5] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[5][8] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[5][6] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[8][9] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[6][9] == 7) {
			cout << 6 << ' ' << 7 << ' ' << 8 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[9][6] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[9][8] == 7) {
			cout << 7 << ' ' << 6 << ' ' << 7 << ' ' << 8 << endl;
			return 0;
		}
		//第三种开局
		if (gridInfo[8][5] == 7 && gridInfo[9][6] == 7) {
			cout << 6 << ' ' << 8 << ' ' << 7 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[6][5] == 7 && gridInfo[5][6] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 7 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[5][8] == 7 && gridInfo[6][9] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 9 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[9][8] == 7 && gridInfo[8][9] == 7) {
			cout << 6 << ' ' << 6 << ' ' << 7 << ' ' << 5 << endl;
			return 0;
		}
		//第四种开局
		if (gridInfo[6][6] == 7 && gridInfo[7][6] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 9 << ' ' << 5 << endl;
			return 0;
		}
		if (gridInfo[8][6] == 7 && gridInfo[7][6] == 7) {
			cout << 6 << ' ' << 6 << ' ' << 5 << ' ' << 5 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[6][8] == 7) {
			cout << 6 << ' ' << 6 << ' ' << 5 << ' ' << 5 << endl;
			return 0;
		}
		if (gridInfo[6][7] == 7 && gridInfo[6][6] == 7) {
			cout << 6 << ' ' << 8 << ' ' << 5 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[8][8] == 7) {
			cout << 6 << ' ' << 8 << ' ' << 5 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[6][8] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 9 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[8][6] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 9 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[8][7] == 7 && gridInfo[8][8] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 9 << ' ' << 5 << endl;
			return 0;
		}
		//第五种开局
		if (gridInfo[8][9] == 7 && gridInfo[9][9] == 7) {
			cout << 6 << ' ' << 8 << ' ' << 5 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[9][8] == 7 && gridInfo[9][9] == 7) {
			cout << 7 << ' ' << 5 << ' ' << 8 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[9][5] == 7 && gridInfo[9][6] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 7 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[9][5] == 7 && gridInfo[8][5] == 7) {
			cout << 6 << ' ' << 6 << ' ' << 5 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[5][5] == 7 && gridInfo[5][6] == 7) {
			cout << 6 << ' ' << 8 << ' ' << 7 << ' ' << 9 << endl;
			return 0;
		}
		if (gridInfo[5][5] == 7 && gridInfo[6][5] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 9 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[5][9] == 7 && gridInfo[5][8] == 7) {
			cout << 6 << ' ' << 6 << ' ' << 7 << ' ' << 5 << endl;
			return 0;
		}
		if (gridInfo[5][9] == 7 && gridInfo[6][9] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 9 << ' ' << 7 << endl;
			return 0;
		}
		//第六种开局
		if (gridInfo[6][8] == 7 && gridInfo[6][9] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 6 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[6][8] == 7 && gridInfo[5][8] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 6 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[8][6] == 7 && gridInfo[9][6] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 6 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[8][6] == 7 && gridInfo[8][5] == 7) {
			cout << 8 << ' ' << 8 << ' ' << 6 << ' ' << 6 << endl;
			return 0;
		}
		if (gridInfo[6][6] == 7 && gridInfo[5][6] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 6 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[6][6] == 7 && gridInfo[6][5] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 6 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[8][8] == 7 && gridInfo[9][8] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 6 << ' ' << 8 << endl;
			return 0;
		}
		if (gridInfo[8][8] == 7 && gridInfo[8][9] == 7) {
			cout << 8 << ' ' << 6 << ' ' << 6 << ' ' << 8 << endl;
			return 0;
		}
		//第七种开局
	}
	if (turnID == 3 && currBotColor == grid_black) {
		if (gridInfo[8][8] == 7 && gridInfo[7][9] == 7 && gridInfo[6][7] == 7 && gridInfo[6][8] == 7) {
			cout << 9 << ' ' << 6 << ' ' << 10 << ' ' << 7 << endl;
			return 0;
		}
		if (gridInfo[7][8] == 7 && gridInfo[8][9] == 7 && gridInfo[9][8] == 7 && gridInfo[10][7] == 7) {
			if (gridInfo[7][7] == 1 && gridInfo[6][7] == 1 && gridInfo[8][7] == 1) {
				cout << 6 << ' ' << 8 << ' ' << 11 << ' ' << 6 << endl;
				return 0;
			}
		}
	}
	if (turnID == 5 && currBotColor == grid_black) {
		if (gridInfo[8][8] == 7 && gridInfo[7][6] == 7 && gridInfo[9][7] == 7 && gridInfo[10][6] == 7 && gridInfo[4][7] == 7 && gridInfo[7][9] == 7 && gridInfo[8][6] == 7 && gridInfo[9][6] == 7) {
			if (gridInfo[6][7] == 1 && gridInfo[8][7] == 1 && gridInfo[7][7] == 1 && gridInfo[6][10] == 1 && gridInfo[11][5] == 1 && gridInfo[5][7] == 1 && gridInfo[6][8] == 1) {
				cout << 6 << ' ' << 6 << ' ' << 11 << ' ' << 6 << endl;
				return 0;
			}
		}
		if (gridInfo[5][5] == 7 && gridInfo[5][6] == 7 && gridInfo[5][7] == 7 && gridInfo[5][8] == 7 && gridInfo[7][6] == 7 && gridInfo[8][6] == 7 && gridInfo[3][9] == 7 && gridInfo[8][9] == 7) {
			if (gridInfo[6][8] == 1 && gridInfo[7][9] == 1 && gridInfo[7][7] == 1 && gridInfo[5][3] == 1 && gridInfo[5][9] == 1 && gridInfo[4][9] == 1 && gridInfo[6][9] == 1) {
				cout << 4 << ' ' << 10 << ' ' << 6 << ' ' << 6 << endl;
				return 0;
			}
		}
		if (gridInfo[6][8] == 7 && gridInfo[7][8] == 7 && gridInfo[6][6] == 7 && gridInfo[6][5] == 7 && gridInfo[5][6] == 7 && gridInfo[6][7] == 7 && gridInfo[5][11] == 7 && gridInfo[10][6] == 7) {
			if (gridInfo[8][8] == 1 && gridInfo[9][9] == 1 && gridInfo[7][7] == 1 && gridInfo[9][7] == 1 && gridInfo[7][9] == 1 && gridInfo[6][4] == 1 && gridInfo[6][10] == 1) {
				cout << 8 << ' ' << 9 << ' ' << 10 << ' ' << 9 << endl;
				return 0;
			}
		}
	}
	if (turnID == 6 && currBotColor == grid_white) {
		if (gridInfo[7][8] == 7 && gridInfo[6][6] == 7 && gridInfo[8][6] == 7 && gridInfo[8][9] == 7 && gridInfo[3][11] == 7 && gridInfo[8][7] == 7 && gridInfo[5][5] == 7 && gridInfo[5][11] == 7 && gridInfo[3][5] == 7 && gridInfo[9][11] == 7) {
			if (gridInfo[6][8] == 1 && gridInfo[5][9] == 1 && gridInfo[7][7] == 1 && gridInfo[4][10] == 1 && gridInfo[5][6] == 1 && gridInfo[5][7] == 1 && gridInfo[5][8] == 1 && gridInfo[7][9] == 1 && gridInfo[8][10] == 1 && gridInfo[5][10] == 1 && gridInfo[6][10] == 1) {
				cout << 8 << ' ' << 5 << ' ' << 7 << ' ' << 10 << endl;
				return 0;
			}
		}
	}

	//弥补弱智走法

	if (turnID < 4) {
		if (currBotColor == grid_white) {
			scoreOfEnemyRoad[2] = 20;
			//scoreOfMyRoad[2] = 20; 
			scoreOfEnemyLink[0][2] = 15;
			scoreOfEnemyLink[1][2] = 15;
			//scoreOfMyLink[0][2] = 5; 
			//scoreOfMyLink[1][2] = 5;
		}
		else {
			scoreOfEnemyRoad[2] = 10;
			//scoreOfMyRoad[2] = 20; 
			scoreOfEnemyLink[0][2] = 8;
			scoreOfEnemyLink[1][2] = 8;
			//scoreOfMyLink[0][2] = 5; 
			//scoreOfMyLink[1][2] = 5;

		}
	}
	AnalyzeAttackorDefence();
	detectminmax();
	updaterange();

	//落子

	findbest(currBotColor, 3, -INF, INF);

	//输出结果
	cout << fx << ' ' << fy << ' ' << sx << ' ' << sy << endl;
	return 0;
}