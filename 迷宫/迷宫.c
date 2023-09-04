#define _CRT_SECURE_NO_WARNINGS 1

//===========================================================================
//
//  版权所有者： 刘新国，浙江大学计算机科学与技术学院
//                       CAD&CG国家重点实验室
//               xgliu@cad.zju.edu.cn
//  最近修改：2020年3月25日 
//  初次创建：2019年3月，用于<<程序设计专题>>课程教学
//
//===========================================================================

#include <windows.h>
#include <winuser.h>
#include "graphics.h"
#include "extgraph.h"
#include "imgui.h"
#include "linkedlist.h"

//地图元素类型
#define MAP_TYPE_BLANK 1001
#define MAP_TYPE_WALL 1002
#define MAP_TYPE_START 1003
#define MAP_TYPE_END 1004
//当前交互方式
#define MODEL_FILE 2002
#define MODEL_EDIT_MAP 2002
#define MODEL_SOLVE 2003
#define MODEL_HELP 2004
// 计时器
#define MY_TIMER_1  1
#define MY_TIMER_2  2
//地图大小
#define MAP_WIDTH 20
#define MAP_HEIGHT 20
//保存地图数组大小, 总比地图大2
#define ARRAY_WIDTH 22
#define ARRAY_HEIGHT 22

//步骤
struct Point {
	int x;
	int y;
	int step;
	int pre_x;
	int pre_y;
};

//初始化一些内容
void myInitColor();

//引入需要使用的函数
void DisplayClear(void);
void startTimer(int id, int timeinterval);

// 显示画面
void display();
void drawMenu(); //绘制菜单
void displayMap(); //显示地图
void displayInfo();	//信息框
void displayEditInfo();	//编辑地图信息框
void displaySolve();	//求解
void displayHelp();	//帮助


//处理用户事件
void CharEventProcess(char ch);// 用户的字符事件响应函数
void KeyboardEventProcess(int key, int event);// 用户的键盘事件响应函数
void MouseEventProcess(int x, int y, int button, int event);// 用户的鼠标事件响应函数
void TimerEventProcess(int timerID);//定时器处理
void EditMouseEventProcess(int x, int y, int button, int event);//修改界面点击处理
void SolveMouseEventProcess(int x, int y, int button, int event);//求解界面点击处理


//数据处理
void read(const char* file);
void save(const char* file);
void dealMenuFile(int selection);
void dealMenuEdit(int selection);
void dealMenuSolve(int selection);
void dealMenuHelp(int selection);
void dealMove(int ch);
void CreateMaze(int maze[ARRAY_HEIGHT][ARRAY_WIDTH], int x, int y);
void editMap(int x, int y);
int checkMap();
int PathSearch();
int Check2(int i, int j, int k);
void PathSearchAll(int i, int j, int dep);
void Output2();
// 全局变量
double g_win_width, g_win_height;   // 窗口尺寸
double g_size = 6.0/20;   // 地图尺寸
int g_map[ARRAY_HEIGHT][ARRAY_WIDTH] = {0};   // 地图数组
int g_visit[ARRAY_HEIGHT][ARRAY_WIDTH] = {0};
char g_now_file[999] = { 0 };
int g_rank = 0;
int g_model = MODEL_EDIT_MAP;
int g_now_element = MAP_TYPE_BLANK;
char g_title[999] = "信息框标题";
char g_msg[999] = "提醒框内容";
int g_player_x = 0;
int g_player_y = 0;
int g_play_x = 0;
int g_play_y = 0;
int g_end_x = 0;
int g_end_y = 0;
int g_help = 0;
int g_auto_solve = -1;
int g_auto_run = 0;
int g_ax, g_ay;              //ax，ay为从队列前部取出的对应值
int g_path[ARRAY_HEIGHT * ARRAY_WIDTH][2];
int g_move = 0;
int g_count = 0;
int g_count_now = 0;
linkedlistADT g_head = NULL;

void Main()
{
	Randomize();
	// 初始化窗口和图形系统
	SetWindowTitle("迷宫");
	InitGraphics();

	// 获得窗口尺寸
	g_win_width = GetWindowWidth();
	g_win_height = GetWindowHeight();

	//定义颜色
	myInitColor();

	//默认随机地图
	dealMenuEdit(1);

	// 注册时间响应函数
	registerCharEvent(CharEventProcess);        // 字符
	registerKeyboardEvent(KeyboardEventProcess);// 键盘
	registerMouseEvent(MouseEventProcess);      // 鼠标
	registerTimerEvent(TimerEventProcess);      // 定时器

	// 开启定时器
	startTimer(MY_TIMER_1, 50);
	startTimer(MY_TIMER_2, 500);

	// 打开控制台，方便用printf/scanf输出/入变量信息，方便调试
	// InitConsole(); 
}


// 用户的字符事件响应函数
void CharEventProcess(char ch)
{
	uiGetChar(ch); // GUI字符输入
	display(); //刷新显示
}

// 用户的键盘事件响应函数
void KeyboardEventProcess(int key, int event)
{
	uiGetKeyboard(key, event); // GUI获取键盘
	if(g_model == MODEL_SOLVE && event == KEY_DOWN)
		dealMove(key);
	display(); // 刷新显示
}

// 用户的鼠标事件响应函数
void MouseEventProcess(int x, int y, int button, int event)
{
	uiGetMouse(x, y, button, event); //GUI获取鼠标
	EditMouseEventProcess(x, y, button, event); //GUI获取鼠标
	display(); // 刷新显示
}
//定时器处理
void TimerEventProcess(int timerID)
{
	if (timerID == MY_TIMER_1)
	{
		display(); // 刷新显示
	}
	if (timerID == MY_TIMER_2)
	{
		if (g_model == MODEL_SOLVE && g_auto_solve != -1 && g_auto_run) {
			if (g_path[g_auto_solve][0] == -1) {
				g_auto_solve = -1;
			}
			else {
				g_player_x = g_path[g_auto_solve][0];
				g_player_y = g_path[g_auto_solve][1];
				g_auto_solve++;
			}
		}
		display(); // 刷新显示
	}
}

void EditMouseEventProcess(int x, int y, int button, int event)
{
	double px = ScaleXInches(x);
	double py = ScaleYInches(y);
	double fH = GetFontHeight();
	double yy = 6 - fH * 3.5;
	double left = 0.8;
	double start_x = g_win_width - g_size * MAP_WIDTH - 0.5;
	double start_y = g_size;
	if (BUTTON_DOWN == event) {
		if (0.5 <= px && px <= 2.5) {
			if (yy - g_size * 6 <= py && py <= yy - g_size * 4.5) {
				g_now_element = MAP_TYPE_END;
			}
			else if (yy - g_size * 4.5 <= py && py <= yy - g_size * 3.0) {
				g_now_element = MAP_TYPE_START;
			}
			else if (yy - g_size * 3 <= py && py <= yy - g_size * 1.5) {
				g_now_element = MAP_TYPE_WALL;
			}
			else if (yy - g_size * 1.5 <= py && py <= yy) {
				g_now_element = MAP_TYPE_BLANK;
			}
		}
		x = (px - start_x) / g_size;
		y = (py - start_y) / g_size;

		if (0 <= x && x < MAP_WIDTH && 0 <= y && y < MAP_HEIGHT) {
			editMap(x + 1, MAP_HEIGHT - y);
		}
	}
}

void SolveMouseEventProcess(int x, int y, int button, int event)
{

}

void read(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fread(g_map, sizeof(int) * ARRAY_WIDTH * ARRAY_HEIGHT, 1, fp);
	fclose(fp);
}

void save(const char* file)
{
	FILE* fp = fopen(file, "wb");
	fwrite(g_map, sizeof(int) * ARRAY_WIDTH * ARRAY_HEIGHT, 1, fp);
	fclose(fp);
}

void dealMenuFile(int selection)
{
	OPENFILENAMEA file;
	int i, j;
	strcpy(g_title, "文件操作");
	g_model = MODEL_FILE;
	if (selection == 1) {
		for (i = 0; i < ARRAY_WIDTH; i++) {
			for (j = 0; j < ARRAY_HEIGHT; j++) {
				g_map[j][i] = MAP_TYPE_WALL;
			}
		}
		g_now_file[0] = 0;
	}
	if (selection == 2) {
		memset(&file, 0, sizeof(file));
		file.lpstrFile = (char*)malloc(512);
		file.lpstrFileTitle = (char*)malloc(512);
		memset(file.lpstrFile, 0, 512);
		memset(file.lpstrFileTitle, 0, 512);

		file.hwndOwner = NULL;
		file.hInstance = NULL;
		file.lStructSize = sizeof(file);
		file.lpstrFilter = "数据文件(*.mymap)\0*.mymap\0所有文件\0*.*\0\0";
		file.nFilterIndex = 1;
		file.nMaxFile = 512;
		file.nMaxFileTitle = 512;
		file.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
		if (GetOpenFileName(&file)) {
			strcpy(g_now_file, file.lpstrFile);
			read(g_now_file);
		}
		free(file.lpstrFile);
		free(file.lpstrFileTitle);
	}
	if (selection == 3) {
		memset(&file, 0, sizeof(file));
		file.lpstrFile = (char*)malloc(512);
		file.lpstrFileTitle = (char*)malloc(512);
		memset(file.lpstrFile, 0, 512);
		memset(file.lpstrFileTitle, 0, 512);

		file.hwndOwner = NULL;
		file.hInstance = NULL;
		file.lStructSize = sizeof(file);
		file.lpstrFilter = "数据文件(*.mymap)\0*.mymap\0\0";
		file.nFilterIndex = 1;
		file.nMaxFile = 512;
		file.nMaxFileTitle = 512;
		file.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
		if (g_now_file[0] == 0) {
			if (GetSaveFileName(&file)) {
				strcpy(g_now_file, file.lpstrFile);
				save(g_now_file);
			}
		}
		free(file.lpstrFile);
		free(file.lpstrFileTitle);
	}
	if (selection == 4)
		exit(-1);
}

void dealMenuEdit(int selection)
{
	int i, j;
	g_auto_solve = -1;

	strcpy(g_title, "编辑地图");
	g_model = MODEL_EDIT_MAP;
	if (selection == 1) {
		g_rank = 0;
		//全部填充墙
		for (i = 0; i < ARRAY_WIDTH; i++) {
			for (j = 0; j < ARRAY_HEIGHT; j++) {
				g_map[j][i] = MAP_TYPE_WALL;
			}
		}
		//边框先变成路
		for (int i = 0; i < ARRAY_WIDTH; i++) {
			g_map[0][i] = MAP_TYPE_BLANK;
			g_map[ARRAY_HEIGHT - 1][i] = MAP_TYPE_BLANK;
		}
		for (int i = 0; i < ARRAY_HEIGHT; i++) {
			g_map[i][0] = MAP_TYPE_BLANK;
			g_map[i][ARRAY_WIDTH - 1] = MAP_TYPE_BLANK;
		}
		//递归挖
		CreateMaze(g_map, 1, 1);
		//设置终点
		g_map[1][1] = MAP_TYPE_START;
		for (int i = ARRAY_HEIGHT - 3; i >= 0; i--) {
			if (g_map[i][ARRAY_WIDTH - 3] == MAP_TYPE_BLANK) {
				g_map[i][ARRAY_WIDTH - 2] = MAP_TYPE_END;
				break;
			}
		}
		//边框变回强
		for (int i = 0; i < ARRAY_WIDTH; i++) {
			g_map[0][i] = MAP_TYPE_WALL;
			g_map[ARRAY_HEIGHT - 1][i] = MAP_TYPE_WALL;
		}
		for (int i = 0; i < ARRAY_HEIGHT; i++) {
			g_map[i][0] = MAP_TYPE_WALL;
			g_map[i][ARRAY_WIDTH - 1] = MAP_TYPE_WALL;
		}
	}
	else if (selection == 2) {
		if (g_model != MODEL_EDIT_MAP) {
			g_model = MODEL_EDIT_MAP;
			strcpy(g_msg, "点击地图填充");
		}
		else {
			g_model = MODEL_FILE;
		}
	}
	else if (selection == 3) {
		if (g_model != MODEL_EDIT_MAP) {
			g_model = MODEL_EDIT_MAP;
		}
		strcpy(g_title, "编辑地图");
		strcpy(g_msg, "点击地图填充");
		if (g_now_element == MAP_TYPE_END) {
			g_now_element = MAP_TYPE_BLANK;
		}
		else {
			g_now_element++;
		}
	}
}

void dealMenuSolve(int selection)
{
	int i, j;
	int** path;
	g_count = 0;
	g_count_now = 0;
	g_auto_run = 0;
	g_move = 0;
	strcpy(g_title, "地图求解");
	g_model = MODEL_SOLVE;
	g_auto_solve = -1;
	g_path[0][0] = -1;
	g_path[0][1] = -1;
	if (checkMap() == 0) {
		strcpy(g_msg, "地图没有起点或终点");
		return;
	}
	strcpy(g_msg, "开始求解...");
	if (selection == 1) {
		g_move = 1;
	}
	else if (selection == 2) {
		g_auto_solve = 0;
		g_auto_run = 1;
		PathSearch();
	}
	else if (selection == 3) {
		if (g_head != NULL) {
			FreeLinkedList(g_head);
		}
		g_head = NewLinkedList();
		g_visit[g_player_y][g_player_x] = -1;
		PathSearchAll(g_player_y, g_player_x, 3);
		path = (int**)ithNodeobj(g_head, g_count_now+1);
		if (path != NULL) {
			for (i = 0; ; i++) {
				g_path[i][0] = path[i][0];
				g_path[i][1] = path[i][1];
				if (path[i][0] == -1)
					break;
			}
			path = path;
		}
	}
}

void dealMenuHelp(int selection)
{
	strcpy(g_title, "地图求解");
	g_model = MODEL_HELP;
	if (selection == 1) {
		strcpy(g_msg, "帮助");
		g_help = 0;
	}
	else if (selection == 2) {
		strcpy(g_msg, "关于");
		g_help = 1;
	}
}

void dealMove(int ch)
{
	int i;
	int **path;
	if (g_move) {
		if (ch == 38) {
			if (g_player_y <= 0)
				return;
			if (g_map[g_player_y - 1][g_player_x] != MAP_TYPE_WALL)
				g_player_y--;
		}
		else if (ch == 40) {
			if (g_player_y >= ARRAY_HEIGHT - 1)
				return;
			if (g_map[g_player_y + 1][g_player_x] != MAP_TYPE_WALL)
				g_player_y++;
		}
		else if (ch == 37) {
			if (g_player_x <= 0)
				return;
			if (g_map[g_player_y][g_player_x - 1] != MAP_TYPE_WALL)
				g_player_x--;
		}
		else if (ch == 39) {
			if (g_player_y >= ARRAY_WIDTH - 1)
				return;
			if (g_map[g_player_y][g_player_x + 1] != MAP_TYPE_WALL)
				g_player_x++;
		}
	}
	if (ch == ' ') {
		g_auto_run = !g_auto_run;
	}
	if (g_count > 0) {

		if (ch == 37) {
			g_count_now = (g_count_now - 1 + g_count) % g_count;
		}
		else if (ch == 39) {
			g_count_now = (g_count_now + 1) % g_count;
		}


		path = (int**)ithNodeobj(g_head, g_count_now + 1);
		if (path != NULL) {
			for (i = 0; ; i++) {
				g_path[i][0] = path[i][0];
				g_path[i][1] = path[i][1];
				if (path[i][0] == -1)
					break;
			}
		}
	}
		
}

void CreateMaze(int maze[ARRAY_HEIGHT][ARRAY_WIDTH], int x, int y)
{
	//确保四个方向随机
	int direction[4][2] = { { 1,0 },{ -1,0 },{ 0,1 },{ 0,-1 } };
	int i, r, temp, dx, dy, range, count, j, k;
	for (i = 0; i < 4; i++) {
		r = rand() % 4;
		temp = direction[0][0];
		direction[0][0] = direction[r][0];
		direction[r][0] = temp;

		temp = direction[0][1];
		direction[0][1] = direction[r][1];
		direction[r][1] = temp;
	}

	//向四个方向开挖
	for (i = 0; i < 4; i++) {
		dx = x;
		dy = y;

		//控制挖的距离，由Rank来调整大小
		range = 1 + (g_rank == 0 ? 0 : rand() % g_rank);
		while (range > 0) {
			dx += direction[i][0];
			dy += direction[i][1];

			//排除掉回头路
			if (maze[dx][dy] == MAP_TYPE_BLANK) {
				break;
			}

			//判断是否挖穿路径
			count = 0;
			for (j = dx - 1; j < dx + 2; j++) {
				for (k = dy - 1; k < dy + 2; k++) {
					//abs(j - dx) + abs(k - dy) == 1 确保只判断九宫格的四个特定位置
					if (abs(j - dx) + abs(k - dy) == 1 && maze[j][k] == MAP_TYPE_BLANK) {
						count++;
					}
				}
			}

			if (count > 1) {
				break;
			}

			//确保不会挖穿时，前进
			--range;
			maze[dx][dy] = MAP_TYPE_BLANK;
		}

		//没有挖穿危险，以此为节点递归
		if (range <= 0) {
			CreateMaze(maze, dx, dy);
		}
	}
}

void editMap(int x, int y)
{
	int flag_x = -1;
	int flag_y = -1;
	int i, j;
	if (g_now_element == MAP_TYPE_START) {
		for (i = 0; i < ARRAY_WIDTH; i++) {
			for (j = 0; j < ARRAY_HEIGHT; j++) {
				if (g_map[j][i] == MAP_TYPE_START) {
					g_map[j][i] = MAP_TYPE_BLANK;
					i = ARRAY_WIDTH;
					break;
				}
			}
		}
		g_map[y][x] = g_now_element;
	}else if (g_now_element == MAP_TYPE_END) {
		for (i = 0; i < ARRAY_WIDTH; i++) {
			for (j = 0; j < ARRAY_HEIGHT; j++) {
				if (g_map[j][i] == MAP_TYPE_END) {
					g_map[j][i] = MAP_TYPE_BLANK;
					i = ARRAY_WIDTH;
					break;
				}
			}
		}
		g_map[y][x] = g_now_element;
	}
	else {
		g_map[y][x] = g_now_element;
	}

}

int checkMap()
{
	int i, j;
	int n = 0;
	for (i = 0; i < ARRAY_WIDTH; i++) {
		for (j = 0; j < ARRAY_HEIGHT; j++) {
			g_visit[j][i] = (MAP_TYPE_WALL == g_map[j][i]);
		}
	}
	for (i = 0; i < ARRAY_WIDTH; i++) {
		for (j = 0; j < ARRAY_HEIGHT; j++) {
			if (g_map[j][i] == MAP_TYPE_START) {
				n++;
				g_player_x = i;
				g_player_y = j;
				g_play_x = i;
				g_play_y = j;
				g_visit[j][i] = -1;
				i = ARRAY_WIDTH;
				break;
			}
		}
	}
	for (i = 0; i < ARRAY_WIDTH; i++) {
		for (j = 0; j < ARRAY_HEIGHT; j++) {
			if (g_map[j][i] == MAP_TYPE_END) {
				n++;
				g_end_x = i;
				g_end_y = j;
				i = ARRAY_WIDTH;
				break;
			}
		}
	}

	return n==2;
}

void display() {
	DisplayClear();
	displayMap();
	if (g_model == MODEL_EDIT_MAP)
		displayEditInfo();
	if (g_model == MODEL_SOLVE)
		displaySolve();
	if (g_model == MODEL_HELP)
		displayHelp();
	displayInfo();
	drawMenu();
}



// 菜单演示程序
void drawMenu()
{
	static char* menuListFile[] = { "文件",
		"新建地图 | Ctrl-N", // 快捷键必须采用[Ctrl-X]格式，放在字符串的结尾
		"打开地图 | Ctrl-O",
		"保存地图 | Ctrl-S",
		"退出   | Ctrl-E" };
	static char* menuListMapEdit[] = { "地图编辑",
		"随机生成  | Ctrl-R",
		"手动编辑",
		"元素选择  | Ctrl-W" };
	//static char* menuListSolve[] = { "地图求解",
	//	"手动求解  | Ctrl-Q",
	//	"程序求解  | Ctrl-A",
	//	"展示所有可行解  | Ctrl-D"};
	static char* menuListSolve[] = { "地图求解",
		"手动求解  | Ctrl-Q",
		"程序求解  | Ctrl-A",
		"展示所有可行解  | Ctrl-D"};
	static char* menuListHelp[] = { "帮助",
		"使用说明  | Ctrl-H",
		"关于  | Ctrl-P"};
	static char* selectedLabel = NULL;

	double fH = GetFontHeight();
	double x = 0; //fH/8;
	double y = g_win_height;
	double h = fH * 1.5; // 控件高度
	double w = TextStringWidth("a"); // 控件宽度
	double wid = 0;
	double xindent = g_win_height / 20; // 缩进
	int    selection;

	// menu bar
	drawMenuBar(0, y - h, g_win_width, h);

	// File 菜单
	wid = strlen(menuListFile[1]) * w;
	selection = menuList(GenUIID(0), x, y - h, wid/2, wid, h, menuListFile, sizeof(menuListFile) / sizeof(menuListFile[0]));
	if (selection > 0) {
		dealMenuFile(selection);
	}
	x += wid/2;

	wid = strlen(menuListMapEdit[3]) * w;
	selection = menuList(GenUIID(0), x, y - h, wid*2/3, wid, h, menuListMapEdit, sizeof(menuListMapEdit) / sizeof(menuListMapEdit[0]));
	if (selection > 0)
		dealMenuEdit(selection);
	x += wid * 2 / 3;


	wid = strlen(menuListSolve[3]) * w;
	selection = menuList(GenUIID(0), x , y - h, wid * 2 / 3, wid, h, menuListSolve, sizeof(menuListSolve) / sizeof(menuListSolve[0]));
	if (selection > 0)
		dealMenuSolve(selection);
	x += wid * 2 / 3;

	wid = strlen(menuListHelp[1]) * w;
	selection = menuList(GenUIID(0), x, y - h, wid * 2 / 3, wid, h, menuListHelp, sizeof(menuListHelp) / sizeof(menuListHelp[0]));
	if (selection > 0)
		dealMenuHelp(selection);

}

void displayMap()
{
	double fH = GetFontHeight();
	double menu_h = fH * 1.5;
	double start_x = g_win_width - g_size * MAP_WIDTH - 0.5;
	double start_y = g_size;
	int i, j;
	int value;
	for (i = 0; i < MAP_WIDTH; i++) {
		for (j = 0; j < MAP_HEIGHT; j++) {
			value = g_map[j+1][i+1];
			if (value == MAP_TYPE_BLANK) {
				SetPenColor("line_grey");
				drawRectangle(start_x + i * g_size, start_y + (MAP_HEIGHT - j - 1) * g_size, g_size, g_size, 1);
			}
			else  if (value == MAP_TYPE_WALL) {
				SetPenColor("line_wall");
				drawRectangle(start_x + i * g_size, start_y + (MAP_HEIGHT - j - 1) * g_size, g_size, g_size, 1);
			}
			else  if (value == MAP_TYPE_START) {
				SetPenColor("line_start");
				drawBox(start_x + i * g_size, start_y + (MAP_HEIGHT - j - 1) * g_size, g_size, g_size, 1, "起", 'L', "white");
			}
			else  if (value == MAP_TYPE_END) {
				SetPenColor("line_start");
				drawBox(start_x + i * g_size, start_y + (MAP_HEIGHT - j - 1) * g_size, g_size, g_size, 1, "终", 'L', "white");
			}
		}
	}	
	if (g_model == MODEL_SOLVE) {
		SetPenColor("green");
		for (i = 0;  g_path[i][0] != -1; i++) {
			drawRectangle(start_x + g_path[i][0] * g_size - g_size, start_y + (MAP_HEIGHT - g_path[i][1] - 1) * g_size + g_size, g_size, g_size, 1);
		}
	}
	SetPenColor("red");
	drawRectangle(start_x + g_player_x * g_size - g_size*0.8, start_y + (MAP_HEIGHT - g_player_y - 1) * g_size + g_size*1.2, g_size*0.6, g_size*0.6, 1);
	MovePen(start_x + g_play_x * g_size  - g_size*0.5, start_y + (MAP_HEIGHT - g_play_y - 1) * g_size + g_size * 1.5);
	for (i = 1; i < g_auto_solve; i++) {
		DrawLine((g_path[i][0] - g_path[i-1][0])*g_size, (g_path[i-1][1] - g_path[i][1]) * g_size);
	}	


	SetPenColor("line_grey");
	for (i = 0; i < MAP_WIDTH + 1; i++) {
		MovePen(start_x + i * g_size, start_y);
		DrawLine(0, MAP_HEIGHT * g_size);
	}
	for (i = 0; i < MAP_HEIGHT + 1; i++) {
		MovePen(start_x, start_y + i * g_size);
		DrawLine(MAP_WIDTH * g_size, 0);
	}
}
void displayInfo()
{
	double fH = GetFontHeight();
	double y = 6 - fH*1.5;
	double left = 0.7;
	SetPenColor("line_grey");
	drawRectangle(0.5, 0.5, 2.5, 5.5, 0);
	drawLabel(0.5, 6 + fH / 2, g_title);
	drawLabel(left, y, g_msg);

	
}
void displayEditInfo()
{
	double fH = GetFontHeight();
	double y = 6 - fH * 3.5;
	double left = 0.8;

	y -= g_size;
	SetPenColor("line_grey");
	drawRectangle(left,y, g_size, g_size, 1);

	y -= g_size*1.5;
	SetPenColor("line_wall");
	drawRectangle(left,y, g_size, g_size, 1);

	y -= g_size*1.5;
	SetPenColor("line_start");
	drawBox(left,y, g_size, g_size, 1, "起", 'L', "white");

	y -= g_size*1.5;
	SetPenColor("line_start");
	drawBox(left,y, g_size, g_size, 1, "终", 'L', "white");

	SetPenColor("line_grey");
	y = 6 - fH * 3.5;
	if (g_now_element == MAP_TYPE_BLANK) {
		y -= g_size * 1.5;
	}else if (g_now_element == MAP_TYPE_WALL) {
		y -= g_size * 3;
	}else if (g_now_element == MAP_TYPE_START) {
		y -= g_size * 4.5;
	}else if (g_now_element == MAP_TYPE_END) {
		y -= g_size * 6;
	} 
	drawLabel(left - TextStringWidth("a")*2, y + fH / 2 + g_size / 2, ">");


	y = 6 - fH * 3.5;
	SetPenColor("line_grey");

	y -= g_size * 1.5;
	drawLabel(left + g_size * 1.5, y + fH / 2 + g_size / 2, "路");
	y -= g_size * 1.5;
	drawLabel(left + g_size * 1.5, y + fH / 2 + g_size / 2, "墙");
	y -= g_size * 1.5;
	drawLabel(left + g_size * 1.5, y + fH / 2 + g_size / 2, "起点");
	y -= g_size * 1.5;
	drawLabel(left + g_size * 1.5, y + fH/2 + g_size/2, "终点");

}
void displaySolve()
{
	double fH = GetFontHeight();
	double y = 6 - fH * 3.5;
	double left = 0.8;
	char buf[999];

	y -= g_size * 1.5;
	drawLabel(left, y + fH / 2 + g_size / 2, "空格暂停/继续运行");
	if (g_count>0) {

		y -= g_size * 1.5;
		sprintf(buf,"共%d种解法",g_count);
		drawLabel(left, y + fH / 2 + g_size / 2, buf);
		y -= g_size * 1.5;
		sprintf(buf,"当前使用第%d种解法",g_count_now+1);
		drawLabel(left, y + fH / 2 + g_size / 2, buf);
		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "方向键左右切换解放");
	}
}
void displayHelp()
{
	double fH = GetFontHeight();
	double y = 6 - fH * 3.5;
	double left = 0.8;

	if (g_help == 0) {

		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "帮助界面");
		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "点击对应菜单选择对应功能");
		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "可以使用快捷键");
	}
	else {

		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "关于界面");
		y -= g_size * 1.5;
		drawLabel(left, y + fH / 2 + g_size / 2, "迷宫系统v1.0");
	}
}
void myInitColor() {
	char colorName[20] = "line_grey";
	char colorName2[20] = "line_wall";
	char colorName3[20] = "line_start";
	char colorName4[20] = "red";
	char colorName5[20] = "green";

	DefineColor(colorName, 0.5, 0.5, 0.5);
	DefineColor(colorName2, 1, 178.0/255, 102.0/255);
	DefineColor(colorName3, 102.0 / 255, 1, 1);
	DefineColor(colorName5, 0.5, 1, 0);
}


int fx[4] = { -1,1,0,0 };
int fy[4] = { 0,0,-1,1 };
struct {			//存放路径坐标和前一个坐标的位置
	int x, y, pre;
}sq[ARRAY_WIDTH * ARRAY_HEIGHT*100];


int Check(int i, int j);			//检查当前坐标是否可行
void Output(int qe);				//输出路径坐标


int PathSearch() {
	int i, j, k, qh = 0, qe = 1;
	g_visit[g_player_y][g_player_x] = -1;				//置-1表示当前坐标已走过
	sq[1].pre = 0;
	sq[1].x = g_player_x;					//保存当前坐标
	sq[1].y = g_player_y;
	while (qh != qe) {				//出现qh=qe则说明该迷宫走不通，跳出循环
		qh = qh + 1;
		for (k = 0; k < 4; k++) {	//搜索当前坐标的四个扩展方向
			i = sq[qh].x + fx[k];
			j = sq[qh].y + fy[k];
			if (Check(i, j) == 1) {	//检查是否可行
				qe = qe + 1;
				sq[qe].x = i;
				sq[qe].y = j;
				sq[qe].pre = qh;
				g_visit[j][i] = -1;
				if (sq[qe].x == g_end_x && sq[qe].y == g_end_y) {	//走到(8,8)即出口则结束搜索，输出路径并返回
					Output(qe);
					return 1;
				}
			}
		}
	}
	return -1;
}

/*检查当前坐标是否可行*/
int Check(int j, int i) {
	int flag = 1;
	if (i < 1 || i > ARRAY_HEIGHT -1  || j < 1 || j > ARRAY_WIDTH-1)		//是否在迷宫内
		flag = 0;
	if (g_visit[i][j] == 1 || g_visit[i][j] == -1)	//是否有路，是否走过
		flag = 0;
	return flag;
}

/*输出路径坐标*/
void Output(int qe) {
	int path_x[ARRAY_HEIGHT* ARRAY_WIDTH], path_y[ARRAY_HEIGHT * ARRAY_WIDTH], i = 2, j = 0;
	int n = 0;
	path_x[1] = sq[qe].x;
	path_y[1] = sq[qe].y;
	while (sq[qe].pre != 0) {					//将sq中保存的路径保存到path_x和path_y中以便正序输出，若直接输出sq则是倒序
		qe = sq[qe].pre;
		path_x[i] = sq[qe].x;
		path_y[i] = sq[qe].y;
		i++;
	}
	for (i = i - 1; i >= 1; i--) {				//正序输出路径坐标
		g_path[n][0] = path_x[i];
		g_path[n][1] = path_y[i];
		n++;
	}
	g_path[n][0] = -1;
	g_path[n][0] = -1;
}
void PathSearchAll(int y, int x, int dep) {
	int k, newy, newx;
	for (k = 0; k < 4; k++) {				
		if (Check2(y, x, k) == 1) {
			newy = y + fx[k];
			newx = x + fy[k];
			g_visit[newy][newx] = dep;			
			if (newy == g_end_y && newx == g_end_x) {
				g_count++;
				Output2();
			}
			else							
				PathSearchAll(newy, newx, dep + 1);
			g_visit[newy][newx] = 0;			
		}
	}
}

/*检查当前坐标是否可行*/
int Check2(int i, int j, int k) {
	int flag = 1;
	i = i + fx[k];
	j = j + fy[k];
	if (i < 1 || i > ARRAY_HEIGHT - 1 || j < 1 || j > ARRAY_WIDTH - 1)		//是否在迷宫内
		flag = 0;
	if (g_visit[i][j] != 0)					//是否可行
		flag = 0;
	return flag;
}
void Output2() {
	int path_x[ARRAY_HEIGHT * ARRAY_WIDTH], path_y[ARRAY_HEIGHT * ARRAY_WIDTH], d[ARRAY_HEIGHT * ARRAY_WIDTH];
	int len = 0;
	int i, j;
	int temp;
	int** path;
	for ( i = 0; i < ARRAY_WIDTH; i++) {
		for ( j = 0; j < ARRAY_HEIGHT; j++) {
			if (g_visit[j][i] >= 2) {
				path_x[len] = j;
				path_y[len] = i;
				d[len] = g_visit[j][i];
				len++;
			}
		}
	}
	for (i = 0; i < len; i++) {
		for (j = 0; j < i; j++) {
			if (d[i] < d[j]) {
				temp = d[i];
				d[i] = d[j];
				d[j] = temp;
				
				temp = path_x[i];
				path_x[i] = path_x[j];
				path_x[j] = temp;
				
				temp = path_y[i];
				path_y[i] = path_y[j];
				path_y[j] = temp;

			}
		}
	}
	path = (int**)malloc(sizeof(int*) * (len + 1));
	for (i = 0; i < len ; i++) {
		path[i] = (int*)malloc(sizeof(int) * 2);
		path[i][0] = path_y[i];
		path[i][1] = path_x[i];
	}
	path[i] = (int*)malloc(sizeof(int) * 2);
	path[i][0] = -1;
	path[i][1] = -1;

	InsertNode(g_head, NULL, path);
}
