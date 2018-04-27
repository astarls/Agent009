// Agent009.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <conio.h>
#include <graphics.h>
#include <math.h>
#include<ctype.h>

#define PI 3.1415926536		// 圆周率
#define WIDETH 20
#define HEIGHT 15
#define MoveCoolDownTime 1//每次移动冷却时间

DWORD* g_bufMask;			// 指向“建筑物”IMAGE 的指针
DWORD* g_bufRender;			// 指向渲染 IMAGE 的指针

LOGFONT f;                 // 新建字体格式结构体用于后面输出文本



struct role
{
	int x;
	int y;//表示人物坐标
	int por_num;
};
struct cell
{
	bool downwall;
	bool rightwall;//表示各方向墙的有无，0表示没有，1表示有
	bool portal;//表示传送设备的有无，0表示无，1表示有
	bool exit;//表示出口
	bool guard;//表示有没有守卫
};
struct cell map[HEIGHT][WIDETH];//15*20的地图
								//声明10个player 将会在每关地图生成时对player进行具体初始化
struct role player;
//定义守卫结构体并声明3个实例
struct guard_still
{
	int x;
	int y;
	char direction;//表示守卫朝向，d表示右，s表示下，a表示左，w表示上
	int aim_x;
	int aim_y;
	int alarm;//表示是否引起警觉，值越高警觉性越高
};
struct guard_still guard[3];

// 创建渲染对象
IMAGE imgRender(800, 600);


//保存用户正在游玩的关卡
int mission_num = 0;
//key=1则表示通关
int key = 0;
//储存当前守卫数目
int guard_num = 0;
//force表示能力是否开启 将会被用于绘制游戏界面故声明为全局变量
bool force;
//elecShock表示
bool elecShock;
//控制玩家移动速度的冷却
short int MoveCoolDown = 0;
//设定重试游戏为全局变量 避免函数自身调用导致程序崩溃
bool retrygame = FALSE;
//设定重选关卡为全局变量 避免函数自身调用导致程序崩溃
bool rechoosegame = FALSE;




void mapping_border()//将地图的边界设置为墙
{
	for (int i = 1; i < WIDETH - 1; i++)
	{
		map[0][i].downwall = 1;
		map[HEIGHT - 2][i].downwall = 1;
	}
	for (int i = 1; i < HEIGHT - 1; i++)
	{
		map[i][0].rightwall = 1;
		map[i][WIDETH - 2].rightwall = 1;
	}

}

//所有map函数应当对玩家及守卫的所有初始值进行赋值
//第0关地图
void mapping0()
{
	for (int i = 4; i <= 13; i++)
		map[i][3].rightwall = 1;
	for (int i = 1; i <= 13; i++)
	{
		map[i][10].rightwall = 1;
		map[i][15].rightwall = 1;
	}
	map[7][7].portal = 1;
	map[13][6].portal = 1;
	map[13][7].portal = 1;
	map[13][8].portal = 1;
	/*该段用于测试
	map[13][1].portal = 1; map[13][2].portal = 1;
	map[13][3].portal = 1; map[13][4].portal = 1;
	map[13][5].portal = 1; map[13][9].portal = 1;
	map[13][10].portal = 1; map[13][11].portal = 1;
	map[13][12].portal = 1; map[13][13].portal = 1;
	map[13][14].portal = 1; map[13][15].portal = 1;
	*/
	map[7][13].portal = 1;//传送设备位置
	map[3][17].exit = 1;//出口位置
	player.x = 2;
	player.y = 12;//玩家初始位置
	player.por_num = 0;//初始化传送设备数目
	guard_num = 0;
}
//第1关地图
void mapping1()
{
	for (int i = 1; i < 19; i++)
		map[8][i].downwall = 1;
	for (int i = 15; i < 19; i++)
		map[9][i].downwall = 1;
	for (int i = 10; i < 14; i++)
		map[i][14].rightwall = 1;
	map[12][16].rightwall = 1;
	map[13][16].rightwall = 1;
	map[11][17].downwall = 1;
	map[11][18].downwall = 1;
	map[3][14].portal = 1;
	map[13][6].portal = 1;
	map[13][7].portal = 1;
	map[13][8].portal = 1;
	map[10][13].portal = 1;
	map[13][18].exit = 1;
	player.x = 2;
	player.y = 2;
	player.por_num = 0;//初始化传送设备数目
	guard[0].x = 8;
	guard[0].y = 1;
	guard[0].direction = 's';
	map[1][8].guard = 1;
	guard[1].x = 15;
	guard[1].y = 3;
	guard[1].direction = 'd';
	map[3][15].guard = 1;
	guard[2].x = 12;
	guard[2].y = 9;
	guard[2].direction = 's';
	map[9][12].guard = 1;
	guard_num = 3;
	
}


void rightmove()
{
	int *p, *q, *r;
	r = &mission_num;
	p = &player.x; q = &player.y;
	if (force == 0)//判断行动是否强化
	{
		if (map[*q][*p].rightwall)
			return;
		else
		{
			player.x++;
			//移动陷入冷却
			MoveCoolDown = MoveCoolDownTime;

			return;
		}
	}
	else if (force == 1)
	{
		player.x++;
		player.por_num--;
		force = 0;
		MoveCoolDown = MoveCoolDownTime;
		return;
	}
}
void leftmove()
{
	int *p, *q, *r;
	r = &mission_num;
	p = &player.x; q = &player.y;
	if (force == 0)//判断行动是否强化
	{
		if (map[*q][*p - 1].rightwall)
			return;
		else
		{
			player.x--;
			MoveCoolDown = MoveCoolDownTime;
			return;
		}
	}
	else if (force == 1)
	{
		player.x--;
		player.por_num--;
		MoveCoolDown = MoveCoolDownTime;
		force = 0;
		return;
	}
}
void upmove()
{
	int *p, *q, *r;
	r = &mission_num;
	p = &player.x; q = &player.y;
	if (force == 0)//判断行动是否强化
	{
		if (map[*q - 1][*p].downwall)
			return;
		else
		{
			player.y--;
			MoveCoolDown = MoveCoolDownTime;
			return;
		}
	}
	else if (force == 1)
	{
		player.y--;
		player.por_num--;
		MoveCoolDown = MoveCoolDownTime;
		force = 0;
		return;
	}
}
void downmove()
{
	int *p, *q, *r;
	r = &mission_num;
	p = &player.x; q = &player.y;
	if (force == 0)//判断行动是否强化
	{
		if (map[*q][*p].downwall)
			return;
		else
		{
			player.y++;
			MoveCoolDown = MoveCoolDownTime;
			return;
		}
	}
	else if (force == 1)
	{
		player.y++;
		player.por_num--;
		MoveCoolDown = MoveCoolDownTime;
		force = 0;
		return;
	}
}

void test()
{
	int *p, *q, *r;
	r = &mission_num;
	p = &player.x; q = &player.y;
	if (map[*q][*p].portal == 1)
	{
		player.por_num++;
		map[*q][*p].portal = 0;
	}
	if (map[*q][*p].exit == 1)
		key = 1;
}




//任务介绍函数   待完善 未完成关卡默认使用第一关
void intro_mission(void)
{
	//确定文字输出范围
	RECT r = { 50,50,760,560 };
	//将文字在新的IMAGE上绘制 避免原背景被删除
	IMAGE text(800, 600);
	SetWorkingImage(&text);

	/*文字背景图片是否采用待定
	IMAGE textbg;//建立IMAGE储存文字背景
	loadimage(&textbg, _T("textbg2.jpg"));
	putimage(0, 0, &textbg);*/

	//clearrectangle(0,0,800,400);

	gettextstyle(&f);                     // 获取当前字体设置
	f.lfHeight = 30;                      // 设置字体高度为 30
	_tcscpy_s(f.lfFaceName, _T("楷体"));    // 设置字体
	f.lfQuality = ANTIALIASED_QUALITY;    // 设置输出效果为抗锯齿  
	settextstyle(&f);                     // 设置字体样式

	settextcolor(0xfffa57);

	switch (mission_num)
	{
	case 0:
		drawtext(_T("欢迎回来！Agent009！\n\
			你在上一次执行任务时头部受创，可能导致你的记忆\n\
			出现了缺失，怕你忘记了怎么行动，就再说一遍吧：\n\
			使用WSAD移动,F键可以用来开启/或关闭传送能力，\n\
			那是我们公司的机密科技,可以让你无视墙壁阻碍到\n\
			达附近区域。蓝色的圆圈可以补充你的剩余传送次数\n\
			。绿色的圆即是出口，到达那里即可完成任务。\n\
            按F键关闭本条消息"), &r, DT_LEFT);
	default:
		drawtext(_T("欢迎回来！Agent009！\n\
			你在上一次执行任务时头部受创，可能导致你的记忆\n\
			出现了缺失，怕你忘记了怎么行动，就再说一遍吧：\n\
			使用WSAD移动,F键可以用来开启/或关闭传送能力，\n\
			那是我们公司的机密科技,可以让你无视墙壁阻碍到\n\
			达附近区域。蓝色的圆圈可以补充你的剩余传送次数\n\
			。绿色的圆即是出口，到达那里即可完成任务。\n\
            按F键关闭本条消息"), &r, DT_LEFT);



	}
	SetWorkingImage(NULL);
	putimage(0, 0, &text, SRCPAINT);
}

// 在指定位置“照明”,direction只能为'w','a','s','d'.  _x _y均为cell坐标 在函数内改变为像素坐标
void Lighting(int _x, int _y, char direction, int guard_index)
{
	int i;	// 定义循环变量
	double delta_direction;
	double sidelen = 180.0;//这里边长实际为正方形的边长的1/4
	int range;

	//将cell坐标改变为像素坐标
	_x = 20 + _x * 40;
	_y = 20 + _y * 40;

	//根据朝向控制循环的初始弧度
	switch (direction)
	{
	case 's':
		delta_direction = PI / 4.0;
		break;
	case 'w':
		delta_direction = 5.0* PI / 4.0;
		break;
	case 'a':
		delta_direction = 3.0* PI / 4.0;
		break;
	case 'd':
		delta_direction = 7.0* PI / 4.0;
		break;
	}


	// 计算灯光照亮的区域
	for (double a = delta_direction; a < PI / 2.0 + delta_direction; a += PI / 180)	// 圆周循环
	{

		switch (direction)
		{
		case 's':
			range = (int)sidelen / sin(a);
			break;
		case 'w':
			range = -(int)sidelen / sin(a);
			break;
		case 'a':
			range = -(int)sidelen / cos(a);
			break;
		case 'd':
			range = (int)sidelen / cos(a);
			break;

		}



		for (int r = 0; r < range; r++)				// 半径循环
		{
			// 计算照射到的位置
			int x = (int)(_x + cos(a) * r);
			int y = (int)(_y + sin(a) * r);

			// 光线超出屏幕范围，终止
			// （为了简化，不处理最上和最下一行，实际也不可能影响最下一边界行）
			if (x < 0 || x >= 800 || y <= 0 || y >= 600 - 1)
				break;

			// 光线碰到建筑物，终止
			if (g_bufMask[y * 800 + x])
				break;

			// 光线叠加
			g_bufRender[y * 800 + x] += 0x504000;
			//临时量储存守卫信息 该守卫对应蓝色字节增加量为2^n
			int temp = 1;
			for (int i = 0; i < guard_index; i++)
			{
				temp *= 2;
			}
			g_bufRender[y * 800 + x] += temp;	// 这个地方需要根据具体守卫具体处理
		}
	}

	// 将光线模糊处理（避开建筑物）
	for (i = 800; i < 800 * (600 - 1); i++)
		if (!g_bufMask[i])
			for (int j = 0; j < 2; j++)
			{
				g_bufRender[i] = RGB(
					(GetRValue(g_bufRender[i - 800]) + GetRValue(g_bufRender[i - 1]) + GetRValue(g_bufRender[i])
						+ GetRValue(g_bufRender[i + 1]) + GetRValue(g_bufRender[i + 800])) / 5,
						(GetGValue(g_bufRender[i - 800]) + GetGValue(g_bufRender[i - 1]) + GetGValue(g_bufRender[i])
							+ GetGValue(g_bufRender[i + 1]) + GetGValue(g_bufRender[i + 800])) / 5, GetBValue(g_bufRender[i]));
				/*(GetBValue(g_bufRender[i - WIDTH]) + GetBValue(g_bufRender[i - 1]) + GetBValue(g_bufRender[i])
				+ GetBValue(g_bufRender[i + 1]) + GetBValue(g_bufRender[i + WIDTH])) / 5);*/ //这部分原本为模糊信息所用 但因为蓝色字节在游戏中用于储存守卫数据不可模糊
			}
}


//显示守卫视觉的函数
void GenerateGuardsVision(void)
{
	memset(g_bufRender, 0, 800 * 600 * 4);
	for (int i = 0; i < guard_num; i++)
	{
		Lighting(guard[i].x, guard[i].y, guard[i].direction, i);
	}
	//建立指针储存屏幕显存
	DWORD* bufScreen = GetImageBuffer(NULL);
	//建立缓冲图层 用缓冲图层储存RB信息 进行模糊反曝光操作 避免对Render层的Blue信息造成影响
	IMAGE cusion(800, 600);
	
	//建立指针储存cusion显存
	DWORD* bufCusion = GetImageBuffer(&cusion);
	//清空cusion内存
	memset(bufCusion, 0, 800 * 600);
	//将Render层的信息存至Cusion层
	//cusion = imgRender;
	for (int i = 0; i < 800 * 600 - 1; i++)
	{
		if (g_bufRender[i])
		{
			bufCusion[i] = g_bufRender[i];
			//bufCusion[i] /= 256;
			//bufCusion[i] *= 256;
		}
	}
	

	// 修正曝光过度的点
	for (int i = 800 * 600 - 1; i >= 0; i--)
		if (bufCusion[i] > 0xffffff)
			bufCusion[i] = 0xffffff;

	// 将光线模糊处理（避开建筑物）
	for (int i = 800; i < 800 * (600 - 1); i++)
		if (!g_bufMask[i])
			for (int j = 0; j < 2; j++)
			{
				bufCusion[i] = RGB(
					(GetRValue(bufCusion[i - 800]) + GetRValue(bufCusion[i - 1]) + GetRValue(bufCusion[i])
						+ GetRValue(bufCusion[i + 1]) + GetRValue(bufCusion[i + 800])) / 5,
						(GetGValue(bufCusion[i - 800]) + GetGValue(bufCusion[i - 1]) + GetGValue(bufCusion[i])
							+ GetGValue(bufCusion[i + 1]) + GetGValue(bufCusion[i + 800])) / 5,
							(GetBValue(bufCusion[i - 800]) + GetBValue(bufCusion[i - 1]) + GetBValue(bufCusion[i])
								+ GetBValue(bufCusion[i + 1]) + GetBValue(bufCusion[i + 800])) / 5);
			}

	//针对每个显存单位如果有cusion数据 绘制在屏幕上
	for (int i = 0; i < 800 * 600 - 1; i++)
	{
		if (bufCusion[i])
		{
			bufScreen[i] = bufCusion[i];
		}
	}


}



//常规绘制游戏画面 每次刷新调用一次 将在函数外部flushbatchdraw.
void drawgame(void)
{
	int i, j;//循环变量

	//绘制守卫视野
	GenerateGuardsVision();

	 // 绘制玩家   暂时先以实心方格代表玩家
	setfillcolor(WHITE);
	solidrectangle(player.x * 40 + 10, player.y * 40 + 10, player.x * 40 + 30, player.y * 40 + 30);
	//绘制守卫 暂时以红色方格代表守卫
	setfillcolor(RED);
	for (int i = 0; i < guard_num; i++)
	{
		solidrectangle(guard[i].x * 40 + 10, guard[i].y * 40 + 10, guard[i].x * 40 + 30, guard[i].y * 40 + 30);
	}
	


	//遍历地图每个方格 进行绘制
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 20; j++)
		{
			//绘制传送设备 暂时以蓝色圆球代表		
			if (map[i][j].portal == TRUE)
			{
				setfillcolor(BLUE);
				solidcircle(j * 40 + 20, i * 40 + 20, 10);
			}
			//绘制出口 暂时以绿色圆球代表
			if (map[i][j].exit == TRUE)
			{
				setfillcolor(GREEN);
				solidcircle(j * 40 + 20, i * 40 + 20, 10);
			}

		}
	}

	//绘制电击是否开启 同时考虑关卡是否达到要求 较前的关卡将不显示
	if (mission_num >= 2)
	{
		if (elecShock)
		{
			outtextxy(40, 5, _T("电击能力：ON"));
		}
		else
		{
			outtextxy(40, 5, _T("电击能力：OFF"));
		}
	}



	//绘制传送能力是否开启及剩余次数
	settextcolor(WHITE);
	settextstyle(30, 15, _T("Courier"));

	if (force)
	{
		outtextxy(40, 565, _T("传送能力：ON"));
	}
	else
	{
		outtextxy(40, 565, _T("传送能力：OFF"));
	}
	//定义变量储存用户剩余传送次数
	int PortalToUse = player.por_num;

	RECT r = { 280, 565, 800, 600 };
	drawtext(_T("剩余传送次数: "), &r, DT_LEFT);
	//图形库自带输出变量能力受限 以switch输出剩余传送次数
	moveto(480, 565);
	if (PortalToUse > 9)
	{
		int temp = PortalToUse / 10;
		switch (temp)
		{
		case 1:
			outtext(_T("1"));
			break;
		case 2:
			outtext(_T("2"));
			break;
		case 3:
			outtext(_T("3"));
			break;
		case 4:
			outtext(_T("4"));
			break;
		case 5:
			outtext(_T("5"));
			break;
		case 6:
			outtext(_T("6"));
			break;
		case 7:
			outtext(_T("7"));
			break;
		case 8:
			outtext(_T("8"));
			break;
		case 9:
			outtext(_T("9"));
			break;
		}
	}
	int temp = PortalToUse % 10;
	switch (temp)
	{
	case 0:
		outtext(_T("0"));
		break;
	case 1:
		outtext(_T("1"));
		break;
	case 2:
		outtext(_T("2"));
		break;
	case 3:
		outtext(_T("3"));
		break;
	case 4:
		outtext(_T("4"));
		break;
	case 5:
		outtext(_T("5"));
		break;
	case 6:
		outtext(_T("6"));
		break;
	case 7:
		outtext(_T("7"));
		break;
	case 8:
		outtext(_T("8"));
		break;
	case 9:
		outtext(_T("9"));
		break;
	}



}

//检测玩家是否被看到的函数
void test_ifseen(void)
{
	int x_px, y_px;//储存玩家坐标对应的中心像素点
	x_px = player.x * 40 + 20;
	y_px = player.y * 40 + 20;
	//检测用户坐标中心及周围8个像素是否被守卫看到
	/*if(g_bufRender[y_px * 800 + x_px]|| g_bufRender[y_px * 800 + x_px-1] || g_bufRender[y_px * 800 + x_px+1]
	|| g_bufRender[(y_px-1) * 800 + x_px] || g_bufRender[(y_px - 1) * 800 + x_px-1] || g_bufRender[(y_px - 1) * 800 + x_px+1]
	|| g_bufRender[(y_px + 1) * 800 + x_px] || g_bufRender[(y_px + 1) * 800 + x_px - 1] || g_bufRender[(y_px + 1) * 800 + x_px + 1])*/
	for (int i = x_px - 1; i <= x_px + 1; i++)
	{
		for (int j = y_px - 1; j <= y_px + 1; j++)
		{
			//如果用户所在附近像素会被敌人看到
			if (g_bufRender[j * 800 + i])
			{
				//获取该像素点的B Value 以此判别被那位守卫看到
				int bValue = GetBValue(g_bufRender[j * 800 + i]);
				for (int k = 0; k < 8; k++)
				{
					//判断第k位守卫是否看到 如看到bValue%2==1将等于1
					if (bValue % 2 == 1)
					{
						//todo:  将第k位守卫的目的地改为当前玩家坐标 
					}
					bValue = bValue / 2;
				}
			}
		}
	}
}


IMAGE* drawgamebuildings(void)
{
	//声明static IMAGE 以确保在函数外可使用该变量
	static IMAGE g_imgMask(800, 600);
	//获取绘图设备的显存指针并储存于全局变量 用于之后的渲染判断
	g_bufMask = GetImageBuffer(&g_imgMask);
	// 设置绘图目标
	SetWorkingImage(&g_imgMask);
	//防止之前关卡叠加
	//cleardevice();
	memset(g_bufMask, 0, 800 * 600 * 4);
	//设置划线（墙）颜色及样式
	setlinecolor(WHITE);
	setlinestyle(PS_SOLID, 3);

	int i, j;     //i,j用于循环控制
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 20; j++)
		{
			if (map[i][j].rightwall == TRUE)
			{
				//绘制每个方格右边的墙
				line(40 + 40 * j, 40 * i, 40 + 40 * j, 40 + 40 * i);
			}
			if (map[i][j].downwall == TRUE)
			{
				//绘制每个方格下边的墙
				line(40 * j, 40 + 40 * i, 40 + 40 * j, 40 + 40 * i);
			}
		}
	}

	// 恢复绘图目标为默认窗口
	SetWorkingImage(NULL);
	return &g_imgMask;
}

//定义gameover函数
void gameover(void)
{
	settextcolor(0x999999);
	f.lfWidth = 30;
	settextstyle(&f);
	outtextxy(250, 150, _T("Game over"));
	f.lfWidth = 20;
	settextstyle(&f);
	outtextxy(250, 250, _T("按r重新开始本局"));
	outtextxy(250, 350, _T("按c重选关卡"));
	outtextxy(250, 450, _T("按q退出游戏"));
	FlushBatchDraw();
	BeginBatchDraw();
	char kin;
	while (TRUE)
	{
		Sleep(200);
		if (_kbhit())
		{
			kin = _getch();
			switch (kin)
			{
			case 'r':
				retrygame = TRUE;
				cleardevice();
				return;
			
			case 'q':
				exit(0);
			case 'c':
				rechoosegame = TRUE;
				cleardevice();
				return;
			}
		}
	}
}

void pause(void)
{
	//设置文字颜色为灰色
	settextcolor(0x999999);
	settextstyle(50, 0, _T("Courier"));
	outtextxy(350, 50, _T("暂停"));
	settextstyle(30, 0, _T("Courier"));
	outtextxy(300, 150, _T("按r重新开始本局"));
	outtextxy(300, 250, _T("按p恢复游戏"));
	outtextxy(300, 350, _T("按c重选关卡"));
	outtextxy(300, 450, _T("按q退出游戏"));

	FlushBatchDraw();
	BeginBatchDraw();
	char kin;
	while (TRUE)
	{
		Sleep(200);
		if (_kbhit())
		{
			kin = _getch();
			switch (kin)
			{
			case 'r':
				retrygame = TRUE;
				cleardevice();
				return;
			case 'p':
				return;
			case 'q':
				exit(0);
			case 'c':
				rechoosegame = TRUE;
				cleardevice();
				return;
			}
		}
	}
}

int choose_mission(void)
{
	//声明字符型变量储存用户输入
	char kin;
	//声明整型变量储存mission_num_chosen用户所选择的关卡序号
	int mission_num_chosen = 0;
	//新建IMAGE对象ChooseMission并载入图片
	IMAGE ChooseMission;
	loadimage(&ChooseMission, _T("choose_mission.jpg"));

	//改变工作区至默认窗口
	SetWorkingImage(NULL);
	//设定填充颜色为黄色
	setfillcolor(0x00a0a0);
	//进入循环 直到用户按'f'后跳出 0.1s刷新一次
	while (TRUE)
	{
		//清空屏幕准备好下一次绘制
		cleardevice();
		//对用户输入进行处理
		if (_kbhit())
		{
			kin = _getch();
			kin = tolower(kin);
			//对用户输入进行判断并改变被选关卡
			switch (kin)
			{
			case 'w':
				if (mission_num_chosen >= 5)
				{
					mission_num_chosen -= 5;
				}
				break;
			case 's':
				if (mission_num_chosen <= 4)
				{
					mission_num_chosen += 5;
				}
				break;
			case 'a':
				if (mission_num_chosen != 0 && mission_num_chosen != 5)
				{
					mission_num_chosen--;
				}
				break;
			case 'd':
				if (mission_num_chosen != 4 && mission_num_chosen != 9)
				{
					mission_num_chosen++;
				}
				break;
			case 'f':
				return mission_num_chosen;
			}
		}

		// 根据所选择的关卡序号填充方格
		fillrectangle(80 + 60 * (mission_num_chosen % 5), 240 + 60 * (mission_num_chosen / 5), 140 + 60 * (mission_num_chosen % 5), 300 + 60 * (mission_num_chosen / 5));

		//将图片展示在窗口 与默认窗口‘或’操作（srcpaint）
		putimage(0, 0, &ChooseMission, SRCPAINT);
		//使用batchdraw和Sleep 避免屏闪
		FlushBatchDraw();
		Sleep(100);
		BeginBatchDraw();

	}
}


//Tip 进入Ingame之前已经开始batchdraw
void ingame(void)
{
	//初始化玩家状态
	player.por_num = 0;
	player.x = 0;
	player.y = 0;
	for (int i = 0; i < guard_num; i++)
	{
		guard[i].alarm = 0;
	}


	//初始化地图
	memset(map, 0, WIDETH * HEIGHT * sizeof(cell));
	//生成地图边界
	mapping_border();
	//这里switch语句判断用户选择的关卡 需要补全
	switch (mission_num)
	{
	case 0:
		mapping0();
		break;
	case 1:
		mapping1();
		break;
	default:
		mapping0();
		break;
	}


	//使用IMAGE指针变量储存drawgamebuildings函数返回的值 以用于后续的putimage函数
	IMAGE* maskpt = drawgamebuildings();
	//显示buildings
	putimage(0, 0, maskpt, SRCPAINT);
	//绘制游戏界面
	drawgame();

	//输出任务介绍
	intro_mission();

	//刷新界面
	FlushBatchDraw();
	//声明字符型变量储存用户输入
	char input;

	//检测用户输入‘f'以关闭任务介绍
	do
	{
		input = _getch();
	} while (input != 'f');

	BeginBatchDraw();

	//进入游戏循环
	while (key == 0)
	{
		//移动冷却恢复
		if (MoveCoolDown > 0)
		{
			MoveCoolDown--;
		}
		//判断是否被看到
		test_ifseen();
		//判断是否被守卫抓住
		for (int i = 0; i < guard_num; i++)
		{
			if (guard[i].x == player.x&&guard[i].y == player.y)
			{
				gameover();
				if (retrygame || rechoosegame)
				{
					return;
				}				
			}
		}


		if (_kbhit() && MoveCoolDown == 0)
		{
			input = _getch();

			switch (input)
			{
			case 'w':
				upmove();
				test();
				break;
			case 'a':
				leftmove();
				test();
				break;
			case 's':
				downmove();
				test();
				break;
			case 'd':
				rightmove();
				test();
				break;
			case 'f'://按f强化下一次行动，若已强化，则取消
				if (player.por_num != 0 && force == 0)
					force = 1;
				else if (force == 1)
				{
					force = 0;
				}
				break;
			case 'p'://按p进入暂停
				pause();
				if (retrygame || rechoosegame)
				{
					return;
				}
				break;
			case 'e':
				if (elecShock)
					elecShock = FALSE;
				else
					elecShock = TRUE;
				break;
			}
		}
		cleardevice();
		drawgame();
		putimage(0, 0, maskpt, SRCPAINT);
		//控制刷新频率
		Sleep(50);
		FlushBatchDraw();
		BeginBatchDraw();
	}
	settextcolor(WHITE);
	settextstyle(50, 0, _T("Courier"));
	outtextxy(150, 250, _T("Mission Complete!"));
	FlushBatchDraw();
	mission_num++;
	BeginBatchDraw();
	_getch();
	return;
}


int main()
{
	//声明字符型变量‘kin’存储用户输入
	char kin;

	//初始化窗口，分辨率 800 * 600.
	initgraph(800, 600);

	g_bufRender = GetImageBuffer(&imgRender);

	//新建IMAGE对象startmenu并载入图片
	IMAGE startmenu;
	loadimage(&startmenu, _T("agent009menu.jpg"));
	//将图片展示在窗口
	putimage(0, 0, &startmenu);
	//进行循环直到用户输入'f’
	do
	{
		kin = _getch();
	} while (kin != 'f' && kin != 'F');

	//进入choose_mission函数 获取用户选择的关卡序号 并储存在mission_num变量中 函数末尾开始batchdraw 未flush
	mission_num = choose_mission();

	ingame();
	//使用循环变量控制重新开始游戏 避免递归
	while (retrygame || rechoosegame)
	{
		while (retrygame)
		{
			retrygame = FALSE;
			ingame();
		}
		while (rechoosegame)
		{
			mission_num = choose_mission();
			rechoosegame = FALSE;
			ingame();
		}
	}


	//关闭绘图设备
	closegraph();
	//该部分用于调试
	/*printf("mission number:%d",mission_num);
	_getch();*/
	return 0;
}

