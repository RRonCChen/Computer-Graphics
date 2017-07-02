// MidtermProject.cpp : Defines the entry point for the console application.
//

#include "include\GL\glew.h"
#include "include\GL\freeglut.h"
#include "include\opencv\opencv.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <math.h>


using namespace cv;
using namespace std;

#define WINDOW_TITLE_PREFIX "Advanced CG Example"

int CurrentWidth = 800, CurrentHeight = 600, WindowHandle = 0;
GLuint myIndex;
GLubyte myLists[10];

GLuint textureID = -1, textureIDArray[10];

unsigned FrameCount = 0;

struct point {
	float x;
	float y;
	float z;
};

vector<point> points_upper;
vector<point> points_lower;

int lookAt_Y = 10;
int lookAt_Z = 0;
int lookAt_X = 10;
float lookZoom = 500.0;

int degree = 0;
float deg = 1.0;
bool mouseLeftPressed = 0;
bool mouseCenterPressed = 0;
int mouseClickX, mouseClickY;

void ResizeFunction(int, int);//2
void RenderFunction(void); //1
void TimerFunction(int);
void IdleFunction(void);
void ImportTexture();
void drawSphere_upper();
void drawSphere_lower();
float linearInterPolate(float p0, float p1, float mu);
void MouseFunc(int button, int state, int x, int y);
void MouseMotion(int x, int y);


int main(int argc, char* argv[])
{
	

	//初始化 glut
	glutInit(&argc, argv);

	//以下使用 Context 功能
	/*	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);*/

	//設定 glut 畫布尺寸 與color / depth模式
	glutInitWindowSize(CurrentWidth, CurrentHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	//根據已設定好的 glut (如尺寸,color,depth) 向window要求建立一個視窗，接著若失敗則退出程式
	WindowHandle = glutCreateWindow("M10509111 Final Project");
	if (WindowHandle < 1) { printf("ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE); }

	
	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"ResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
	//glutIdleFunc(RenderFunction);
	/*glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMotion);*/

	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMotion);

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) { fprintf(stderr, "ERROR: %s\n", glewGetErrorString(GlewInitResult));	exit(EXIT_FAILURE); }

	//背景顏色黑
	glClearColor(1.0f,1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat MaterialAmbient[] = { 0.4,0.4,0.4,1.0f };
	GLfloat MaterialDiffuse[] = { 1.0,1.0,1.0,1.0f };
	GLfloat MaterialSpecular[] = { 1.0,1.0,1.0, 1.0f };
	GLfloat AmbientLightPosition[] = { -1000,0,0,1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, MaterialAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, MaterialDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, MaterialSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPosition);
	//glEnable(GL_DEPTH_TEST);

	ImportTexture();

	glEnable(GL_COLOR_MATERIAL);

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
}


void RenderFunction(void)
{	
	
	++FrameCount;
	printf("Render - %d\n", FrameCount);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-CurrentWidth / lookZoom, CurrentWidth / lookZoom, -CurrentHeight / lookZoom, CurrentHeight / lookZoom, 1, 5000);
	gluLookAt(0, 0, 0, lookAt_X, lookAt_Y, lookAt_Z, 0, 1, 0);


	glShadeModel(GL_SMOOTH);
	
	glRotatef(-90, 1.0, 0.0, 0.0);
	
	glTranslated(0.0, 0.0, 0.0);
	drawSphere_upper();
	glPushMatrix();
	glRotatef(180, 1.0, 0.0, 0.0);
		drawSphere_lower();
	glPopMatrix();
	

	glFlush();
	glutSwapBuffers();
	
}

void IdleFunction(void)
{

}

void TimerFunction(int Value)
{
	if (0 != Value) {
		char* TempString = (char*)
			malloc(512 + strlen(WINDOW_TITLE_PREFIX));

		printf(
			TempString,
			"%s: %d Frames Per Second @ %d x %d",
			WINDOW_TITLE_PREFIX,
			FrameCount * 4,
			CurrentWidth,
			CurrentHeight
		);

		glutSetWindowTitle(TempString);
		free(TempString);
	}

	FrameCount = 0;
	glutTimerFunc(250, TimerFunction, 1);
}

void ImportTexture()
{
	Mat src1,src2;
	Mat textureBitmap1(2048, 2048, CV_8UC3);
	Mat textureBitmap2(2048, 2048, CV_8UC3);
	

	Mat textureBitmapFlip1(2048, 2048, CV_8UC3);
	Mat textureBitmapFlip2(2048, 2048, CV_8UC3);
	

	src1 = imread("L000.jpg", 1);
	src2 = imread("R000.jpg", 1);
	
	
	resize(src1, textureBitmap1, textureBitmap1.size(), 0, 0, INTER_LINEAR);
	resize(src2, textureBitmap2, textureBitmap2.size(), 0, 0, INTER_LINEAR);
		
	glGenTextures(3, &textureIDArray[0]); 

	glBindTexture(GL_TEXTURE_2D, textureIDArray[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2048, 2048, 0, GL_BGR, GL_UNSIGNED_BYTE, textureBitmap1.data);

	glBindTexture(GL_TEXTURE_2D, textureIDArray[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2048, 2048, 0, GL_BGR, GL_UNSIGNED_BYTE, textureBitmap2.data);


}

void drawSphere_upper() {
	int radius = 200; // 半徑

	float x = 0.0;
	float y = 0.0;
	float z = 0.0;

	struct point p;

	Mat point_mat = cv::Mat::zeros(4, 1, CV_32FC1);
	Mat point_next_mat = cv::Mat::zeros(4, 1, CV_32FC1);

	//對Y軸旋轉5度
	Mat rotateY = Mat::eye(4, 4, CV_32F);
	float dx = float(5) / 180.0*3.14159;
	float cs = cos(dx);
	float sn = sin(dx);

	rotateY.at<float>(0, 0) = cs;
	rotateY.at<float>(0, 2) = sn;
	rotateY.at<float>(2, 0) = -sn;
	rotateY.at<float>(2, 2) = cs;


	//upper	
	for (int i = 0; i < 72; i++) {

		for (int j = 0; j < 19; j++) {
			//upper sphere
			if (i == 0) {
				p.x = radius*cos(5 * j * 3.14159265 / 180);
				p.y = radius*sin(5 * j * 3.14159265 / 180);
				p.z = 0.0;

				points_upper.push_back(p);
			}
			else {
				point_mat.at<float>(0, 0) = points_upper[(i - 1) * 19 + j].x;
				point_mat.at<float>(1, 0) = points_upper[(i - 1) * 19 + j].y;
				point_mat.at<float>(2, 0) = points_upper[(i - 1) * 19 + j].z;

				point_next_mat = rotateY * point_mat;

				p.x = point_next_mat.at<float>(0, 0);
				p.y = point_next_mat.at<float>(1, 0);
				p.z = point_next_mat.at<float>(2, 0);

				points_upper.push_back(p);
			}
		}
	}

	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureIDArray[0]);
	glBegin(GL_QUADS);
	for (int k = 0; k < 72; k++) {
		//貼圖座標
		float texture_x = 0.5 + 0.4 * cos(5 * k * 3.14159265 / 180);
		float texture_y = 0.5 + 0.4* sin(5 * k * 3.14159265 / 180);

		//下一點貼圖座標
		float texture_next_x = 0.5 + 0.4 * cos(5 * (k + 1) * 3.14159265 / 180);
		float texture_next_y = 0.5 + 0.4 * sin(5 * (k + 1) * 3.14159265 / 180);

		
		for (int l = 0; l < 18; l++) {
			
				// upper sphere
					
				glTexCoord2d(linearInterPolate(texture_x,0.5,(l/18.0)), linearInterPolate(texture_y,0.5,(l/18.0)));
				glVertex3f(points_upper[(k * 19) + l].x, points_upper[(k * 19) + l].y, points_upper[(k * 19) + l].z);     //0
				//printf("x%d : %f y%d : %f\n", l,linearInterPolate(texture_x, 0.5, (l / 18.0)), l,linearInterPolate(texture_y, 0.5, (l / 18.0)));
						
				glTexCoord2d(linearInterPolate(texture_x,0.5,((l+1)/18.0)), linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));
				glVertex3f(points_upper[(k * 19) + l+1].x, points_upper[(k * 19) + l+1].y, points_upper[(k * 19) + l+1].z);	    //1
				//printf("x%d : %f y%d : %f\n", l+1,linearInterPolate(texture_x, 0.5, (l + 1) / 18.0),l+1 ,linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));
				
				
				if (k != 71) {
					glTexCoord2d(linearInterPolate(texture_next_x,0.5, ((l + 1) / 18.0)), linearInterPolate(texture_next_y, 0.5, ((l + 1) / 18.0)));
					glVertex3f(points_upper[((k + 1) * 19) + l + 1].x, points_upper[((k + 1) * 19) + l + 1].y, points_upper[((k + 1) * 19) + l + 1].z);  //20
					//printf("x%d : %f y%d : %f\n", ((k + 1) * 19) + l + 1,linearInterPolate(texture_next_x, 0.5, ((l + 1) / 18.0)), ((k + 1) * 19) + l + 1,linearInterPolate(texture_next_y, 0.5, ((l + 1) / 18.0)));
					
					glTexCoord2d(linearInterPolate(texture_next_x, 0.5, (l / 18.0)), linearInterPolate(texture_next_y, 0.5, (l / 18.0)));
					glVertex3f(points_upper[((k + 1) * 19) + l].x, points_upper[((k + 1) * 19) + l].y, points_upper[((k + 1) * 19) + l].z);	 //19
					//printf("x%d : %f y%d : %f\n\n", ((k + 1) * 19) + l,linearInterPolate(texture_next_x, 0.5, (l / 18.0)), ((k + 1) * 19) + l,linearInterPolate(texture_next_y, 0.5, (l / 18.0)));
				}
				else {
					//貼圖座標
					 texture_x = 0.5 + 0.4 * cos(0);
					 texture_y = 0.5 + 0.4 * sin(0);
					
					glTexCoord2d(linearInterPolate(texture_x, 0.5, ((l + 1) / 18.0)), linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));
					glVertex3f(points_upper[l+1].x, points_upper[l+1].y, points_upper[l+1].z);

					glTexCoord2d(linearInterPolate(texture_x, 0.5, (l / 18.0)), linearInterPolate(texture_y, 0.5, (l / 18.0)));
					glVertex3f(points_upper[l].x, points_upper[l].y, points_upper[l].z);
				}			

			}
		}

		glEnd();

		
}


void drawSphere_lower() {

	int radius = 200; // 半徑

	float x = 0.0;
	float y = 0.0;
	float z = 0.0;

	struct point p;

	Mat point_mat = cv::Mat::zeros(4, 1, CV_32FC1);
	Mat point_next_mat = cv::Mat::zeros(4, 1, CV_32FC1);

	//對Y軸旋轉5度
	Mat rotateY = Mat::eye(4, 4, CV_32F);
	float dx = float(5) / 180.0*3.14159;
	float cs = cos(dx);
	float sn = sin(dx);

	rotateY.at<float>(0, 0) = cs;
	rotateY.at<float>(0, 2) = sn;
	rotateY.at<float>(2, 0) = -sn;
	rotateY.at<float>(2, 2) = cs;

	//lower
	for (int i = 0; i < 72; i++) {

		for (int j = 0; j < 19; j++) {
			//lower sphere
			if (i == 0) {
				p.x = radius*cos(5 * j * 3.14159265 / 180);
				p.y = radius*sin(5 * j * 3.14159265 / 180);
				p.z = 0.0;

				points_lower.push_back(p);
			}
			else {
				point_mat.at<float>(0, 0) = points_lower[(i - 1) * 19 + j].x;
				point_mat.at<float>(1, 0) = points_lower[(i - 1) * 19 + j].y;
				point_mat.at<float>(2, 0) = points_lower[(i - 1) * 19 + j].z;

				point_next_mat = rotateY * point_mat;

				p.x = point_next_mat.at<float>(0, 0);
				p.y = point_next_mat.at<float>(1, 0);
				p.z = point_next_mat.at<float>(2, 0);

				points_lower.push_back(p);
			}
		}
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureIDArray[1]);
	glBegin(GL_QUADS);
	for (int k = 0; k < 72; k++) {
		//貼圖座標
		float texture_x = 0.5 - 0.4 * cos(5 * k * 3.14159265 / 180);
		float texture_y = 0.5 - 0.4* sin(5 * k * 3.14159265 / 180);

		//下一點貼圖座標
		float texture_next_x = 0.5 - 0.4 * cos(5 * (k + 1) * 3.14159265 / 180);
		float texture_next_y = 0.5 - 0.4 * sin(5 * (k + 1) * 3.14159265 / 180);


		for (int l = 0; l < 18; l++) {

			// lower sphere

			glBindTexture(GL_TEXTURE_2D, textureIDArray[1]);

			glTexCoord2d(linearInterPolate(texture_x, 0.5, (l / 18.0)), linearInterPolate(texture_y, 0.5, (l / 18.0)));
			glVertex3f(points_lower[(k * 19) + l].x, points_lower[(k * 19) + l].y, points_lower[(k * 19) + l].z);     //0
																													  //printf("x%d : %f y%d : %f\n", l, linearInterPolate(texture_x, 0.5, (l / 18.0)), l, linearInterPolate(texture_y, 0.5, (l / 18.0)));

			glTexCoord2d(linearInterPolate(texture_x, 0.5, ((l + 1) / 18.0)), linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));
			glVertex3f(points_lower[(k * 19) + l + 1].x, points_lower[(k * 19) + l + 1].y, points_lower[(k * 19) + l + 1].z);	    //1
																																	//printf("x%d : %f y%d : %f\n", l + 1, linearInterPolate(texture_x, 0.5, (l + 1) / 18.0), l + 1, linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));


			if (k != 71) {
				glTexCoord2d(linearInterPolate(texture_next_x, 0.5, ((l + 1) / 18.0)), linearInterPolate(texture_next_y, 0.5, ((l + 1) / 18.0)));
				glVertex3f(points_lower[((k + 1) * 19) + l + 1].x, points_lower[((k + 1) * 19) + l + 1].y, points_lower[((k + 1) * 19) + l + 1].z);  //20
																																					 //printf("x%d : %f y%d : %f\n", ((k + 1) * 19) + l + 1, linearInterPolate(texture_next_x, 0.5, ((l + 1) / 18.0)), ((k + 1) * 19) + l + 1, linearInterPolate(texture_next_y, 0.5, ((l + 1) / 18.0)));

				glTexCoord2d(linearInterPolate(texture_next_x, 0.5, (l / 18.0)), linearInterPolate(texture_next_y, 0.5, (l / 18.0)));
				glVertex3f(points_lower[((k + 1) * 19) + l].x, points_lower[((k + 1) * 19) + l].y, points_lower[((k + 1) * 19) + l].z);	 //19
																																		 //printf("x%d : %f y%d : %f\n\n", ((k + 1) * 19) + l, linearInterPolate(texture_next_x, 0.5, (l / 18.0)), ((k + 1) * 19) + l, linearInterPolate(texture_next_y, 0.5, (l / 18.0)));
			}
			else {
				//貼圖座標
				texture_x = 0.5 - 0.4 * cos(0);
				texture_y = 0.5 - 0.4 * sin(0);

				glTexCoord2d(linearInterPolate(texture_x, 0.5, ((l + 1) / 18.0)), linearInterPolate(texture_y, 0.5, ((l + 1) / 18.0)));
				glVertex3f(points_lower[l + 1].x, points_lower[l + 1].y, points_lower[l + 1].z);

				glTexCoord2d(linearInterPolate(texture_x, 0.5, (l / 18.0)), linearInterPolate(texture_y, 0.5, (l / 18.0)));
				glVertex3f(points_lower[l].x, points_lower[l].y, points_lower[l].z);
			}

		}
	}
	glEnd();

}


float linearInterPolate(float p0 , float p1 , float mu) {
	return  p0 *(1 - mu) + p1*mu;
}


void MouseFunc(int button, int state, int x, int y)
{
	printf("button = %d, state = %d\n", button, state);
	if (state)
	{
		if (button == 0) { mouseLeftPressed = 0; }
		
	}
	else
	{
		if (button == 0) { mouseLeftPressed = 1; mouseClickX = x; mouseClickY = y; }
	    if (button == 3) { 
			mouseCenterPressed = 1; 
		    lookZoom += 10.0;
		    RenderFunction();
		}
		if (button == 4) {
			mouseCenterPressed = 1;
			if (lookZoom > 240) {
				lookZoom -= 10.0;
				RenderFunction();
			}
			
		}
		
	}
}

void MouseMotion(int x, int y)
{
	printf("x = %d , y = %d\n", x, y);
	if (mouseLeftPressed)
	{
		lookAt_Z += (x - mouseClickX )/3;
		
		lookAt_Y += (mouseClickY- y )/3;


		mouseClickX = x;  //重新記錄目前位置
		mouseClickY = y;
		RenderFunction();
	}
	
}



