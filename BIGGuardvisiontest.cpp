// BIGGuardvisiontest.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <graphics.h>
#include <conio.h>
#include <math.h>

#define RANGE 80			// 灯光照射半径
#define WIDTH 640			// 场景宽度
#define HEIGHT 480			// 场景高度
#define ZOOM 1				// 显示时的缩放倍数
#define PI 3.1415926536		// 圆周率

DWORD* g_bufMask;			// 指向“建筑物”IMAGE 的指针
DWORD* g_bufRender;			// 指向渲染 IMAGE 的指针


							// 创建“仓库”
IMAGE* MakeMask()
{
	// 创建“仓库” IMAGE 对象
	static IMAGE g_imgMask(WIDTH, HEIGHT);
	g_bufMask = GetImageBuffer(&g_imgMask);

	// 设置绘图目标
	SetWorkingImage(&g_imgMask);

	// 绘制“仓库”
	settextstyle(100, 0, _T("Courier"));
	outtextxy(8, 60, _T("1234"));

	// 恢复绘图目标为默认窗口
	SetWorkingImage(NULL);
	return &g_imgMask;
}


// 在指定位置“照明”,direction只能为'w','a','s','d'.
void Lighting(int _x, int _y, char direction)
{
	int i;	// 定义循环变量
	double delta_direction;
	double eclip_a = 200.0, eclip_b = 30.0;//elip_a和eclip_b分别为视野照亮的椭圆的长半轴长和短半轴长
	switch (direction)
	{
	case 's':
		delta_direction =  PI / 4.0;
		eclip_a = 30.0;
		eclip_b = 200.0;
		break;
	case 'w':
		delta_direction = 5.0* PI / 4.0;
		eclip_a = 30.0;
		eclip_b = 200.0;
		break;
	case 'a':
		delta_direction = 3.0* PI / 4.0;
		eclip_b = 30.0;
		eclip_a = 200.0;
		break;
	case 'd':
		delta_direction = 7.0* PI / 4.0;
		eclip_b = 30.0;
		eclip_a = 200.0;
		break;

	}
			// 清空 Render 对象
	memset(g_bufRender, 0, WIDTH * HEIGHT * 4);

	// 计算灯光照亮的区域
	for (double a = delta_direction; a < PI/2.0 + delta_direction; a += PI / 180)	// 圆周循环
	{
		
		double tem = sqrt(eclip_a*eclip_a*sin(a)*sin(a) + eclip_b * eclip_b*cos(a)*cos(a));//用于储存过长的分母
		int range = (int)eclip_a*eclip_b/tem;//椭圆极坐标方程中r关于θ的表达式
		for (int r = 0; r < range; r++)				// 半径循环
		{
			// 计算照射到的位置
			int x = (int)(_x + cos(a) * r);
			int y = (int)(_y + sin(a) * r);

			// 光线超出屏幕范围，终止
			// （为了简化全凭模糊运算，不处理最上和最下一行）
			if (x < 0 || x >= WIDTH || y <= 0 || y >= HEIGHT - 1)
				break;

			// 光线碰到建筑物，终止
			if (g_bufMask[y * WIDTH + x])
				break;

			// 光线叠加
			g_bufRender[y * WIDTH + x] += 0x202000;	// 0x101000 是很淡的黄色
		}
	}

	// 修正曝光过度的点
	for (i = WIDTH * HEIGHT - 1; i >= 0; i--)
		if (g_bufRender[i] > 0xffffff)
			g_bufRender[i] = 0xffffff;

	// 将光线模糊处理（避开建筑物）
	for (i = WIDTH; i < WIDTH * (HEIGHT - 1); i++)
		if (!g_bufMask[i])
			for (int j = 0; j < 2; j++)
			{
				g_bufRender[i] = RGB(
					(GetRValue(g_bufRender[i - WIDTH]) + GetRValue(g_bufRender[i - 1]) + GetRValue(g_bufRender[i])
						+ GetRValue(g_bufRender[i + 1]) + GetRValue(g_bufRender[i + WIDTH])) / 5,
						(GetGValue(g_bufRender[i - WIDTH]) + GetGValue(g_bufRender[i - 1]) + GetGValue(g_bufRender[i])
							+ GetGValue(g_bufRender[i + 1]) + GetGValue(g_bufRender[i + WIDTH])) / 5,
							(GetBValue(g_bufRender[i - WIDTH]) + GetBValue(g_bufRender[i - 1]) + GetBValue(g_bufRender[i])
								+ GetBValue(g_bufRender[i + 1]) + GetBValue(g_bufRender[i + WIDTH])) / 5);
			}
}


// 主函数
void main()
{
	// 初始化绘图窗口
	initgraph(WIDTH * ZOOM, HEIGHT * ZOOM);
	BeginBatchDraw();
	DWORD* bufScreen = GetImageBuffer(NULL);

	int gx = 200, gy = 200;//guard position
	// 制作建筑
	IMAGE* maskpt = MakeMask();//maskpt是指向墙壁IMAGE的指针
	

	// 创建渲染对象
	IMAGE imgRender(WIDTH, HEIGHT);
	g_bufRender = GetImageBuffer(&imgRender);

	// 定义鼠标消息变量
	//MOUSEMSG msg;
	char kin;//kin存储输入
	char dir = 'w';// dir 为朝向，默认方向初始为上
	while (true)
	{
		// 获取一条鼠标移动的消息
		
		if (_kbhit())
		{
			kin = _getch();
			if (kin == 'w'||kin == 's'||kin == 'a'||kin == 'd')
			{
				dir = kin;
				switch (kin)
				{
				case 'w':gy-=10;
					break;
				case 's':gy+=10;
					break;
				case 'a':gx-=10;
					break;
				case 'd':gx+=10;
					break;
				}
			}
		} 
		
		// 在鼠标位置模拟灯光
		Lighting(gx, gy,dir);

		// 将渲染的内容拉伸后显示在绘图窗口中
		int ps = 0, pr = 0;
		for (int y = 0; y < HEIGHT; y++)
		{
			for (int x = 0; x < WIDTH; x++, pr++)
			{
				ps = y * ZOOM * WIDTH * ZOOM + x * ZOOM;
				for (int zy = 0; zy < ZOOM; zy++)
				{
					for (int zx = 0; zx < ZOOM; zx++)
						bufScreen[ps++] = g_bufRender[pr];
					ps += ZOOM * (WIDTH - 1);
				}
			}
		}
		putimage(0, 0, maskpt, SRCPAINT);

		// 刷新显示，并延时
		FlushBatchDraw();
		Sleep(20);

		// 按任意键退出
		/*if (_kbhit())
		{
			EndBatchDraw();
			closegraph();
		}*/
	}
}