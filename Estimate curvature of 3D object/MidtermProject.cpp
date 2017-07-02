// MidtermProject.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
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


unsigned FrameCount = 0;
struct point {
	float x;
	float y;
	float z;
};
struct face_vertex {
	int vertex1;
	int vertex2;
	int vertex3;
};


vector<point> points;
vector<face_vertex> face_vertexs;
vector<point> vertex_normal;
vector<float> curvature;
void findNeibor();
void findVertexNormal();
void ResizeFunction(int, int);//2
void RenderFunction(void); //1
void TimerFunction(int);
void readOBJ(vector<point> &points, vector<face_vertex> &face_vertexs);
void IdleFunction(void);
void drawOBJ();
void normalize();
void find_curvature();
void ColorMap(float InValue, float Lowerbound, float Upperbound, float *OutputRGB);
void writeobj();


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
	WindowHandle = glutCreateWindow("This is my openGL Example");
	if (WindowHandle < 1) { printf("ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE); }

	readOBJ(points, face_vertexs);

	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"ResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
									  //glutIdleFunc(IdleFunction);		  //閒置時...請系統執行"IdleFunction"

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) { fprintf(stderr, "ERROR: %s\n", glewGetErrorString(GlewInitResult));	exit(EXIT_FAILURE); }

	//背景顏色黑
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

	int i;
	float x, y, z, r, g, b, xyz[3];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-float(CurrentWidth) / 2.0, float(CurrentWidth) / 2.0, -float(CurrentHeight) / 2.0, float(CurrentHeight) / 2.0, -CurrentHeight*10.0, CurrentHeight*10.0);
	gluLookAt(0, 0, 1000, 0, 0, 0, 0, 1, 0);

	glPointSize(1);
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	drawOBJ();

	glFlush();
	glutSwapBuffers();
	//printf("count = %d\n",FrameCount);
	// 
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

void readOBJ(vector<point> &points, vector<face_vertex> &face_vertexs) {
	ifstream file;
	file.open("dragon.obj");
	string str;
	float point[3];
	int vertex[3];
	char v;
	char f;
	struct point p;
	struct face_vertex fv;

	while (!file.eof()) {
		getline(file, str);
		if (str[0] == 'v') {
			char *c = const_cast<char*>(str.c_str());
			sscanf(c, "%c %f %f %f", &v, &point[0], &point[1], &point[2]);
			p.x = point[0];
			p.y = point[1];
			p.z = point[2];
			points.push_back(p);

		}
		else if (str[0] == 'f') {
			char *c = const_cast<char*>(str.c_str());
			sscanf(c, "%c %d %d %d", &f, &vertex[0], &vertex[1], &vertex[2]);
			fv.vertex1 = vertex[0] - 1;
			fv.vertex2 = vertex[1] - 1;
			fv.vertex3 = vertex[2] - 1;
			face_vertexs.push_back(fv);
		}
	}
	//findNeibor(); //找點的鄰居
	findVertexNormal();//算出每個點的法向量
	normalize();//正規化法向量
	find_curvature();//向下旋轉到(0,0,-1)
	writeobj();
}

void drawOBJ() {
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < face_vertexs.size(); i++) {
		glColor3f(1.0, 1.0, 1.0);
		glVertex3d(points[face_vertexs[i].vertex1].x, points[face_vertexs[i].vertex1].y, points[face_vertexs[i].vertex1].z);
		glVertex3d(points[face_vertexs[i].vertex2].x, points[face_vertexs[i].vertex2].y, points[face_vertexs[i].vertex2].z);
		glVertex3d(points[face_vertexs[i].vertex3].x, points[face_vertexs[i].vertex3].y, points[face_vertexs[i].vertex3].z);
	}
	glEnd();
}


//找出每個點的鄰近點
void findNeibor() {

	set<int> *neibor = new set<int>[points.size()];
	for (int i = 0; i < face_vertexs.size(); i++) {
		neibor[face_vertexs[i].vertex1].insert(face_vertexs[i].vertex2);
		neibor[face_vertexs[i].vertex1].insert(face_vertexs[i].vertex3);

		neibor[face_vertexs[i].vertex2].insert(face_vertexs[i].vertex1);
		neibor[face_vertexs[i].vertex2].insert(face_vertexs[i].vertex3);

		neibor[face_vertexs[i].vertex3].insert(face_vertexs[i].vertex1);
		neibor[face_vertexs[i].vertex3].insert(face_vertexs[i].vertex2);

	}
}

void findVertexNormal() {

	
	vertex_normal.assign(points.begin(), points.end());
	for (int z = 0; z < points.size(); z++) {
		vertex_normal[z].x = 0.0;
		vertex_normal[z].y = 0.0;
		vertex_normal[z].z = 0.0;
	}

	point face_normal;

	for (int i = 0; i < face_vertexs.size(); i++) {
		float x1 = points[face_vertexs[i].vertex1].x;
		float x2 = points[face_vertexs[i].vertex2].x;
		float x3 = points[face_vertexs[i].vertex3].x;
		float y1 = points[face_vertexs[i].vertex1].y;
		float y2 = points[face_vertexs[i].vertex2].y;
		float y3 = points[face_vertexs[i].vertex3].y;
		float z1 = points[face_vertexs[i].vertex1].z;
		float z2 = points[face_vertexs[i].vertex2].z;
		float z3 = points[face_vertexs[i].vertex3].z;

		//找面的normal
		face_normal.x = (y2 - y1)*(z3 - z1) - (y3 - y1)*(z2 - z1);
		face_normal.y = -(x2 - x1)*(z3 - z1) + (x3 - x1)*(z2 - z1);
		face_normal.z = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

		//std::cout << face_normal.x << " " << face_normal.y << " " << face_normal.z<< std::endl;

		//將面的normal加給面上的所有的點
		vertex_normal[face_vertexs[i].vertex1].x += face_normal.x;
		vertex_normal[face_vertexs[i].vertex1].y += face_normal.y;
		vertex_normal[face_vertexs[i].vertex1].z += face_normal.z;

		vertex_normal[face_vertexs[i].vertex2].x += face_normal.x;
		vertex_normal[face_vertexs[i].vertex2].y += face_normal.y;
		vertex_normal[face_vertexs[i].vertex2].z += face_normal.z;

		vertex_normal[face_vertexs[i].vertex3].x += face_normal.x;
		vertex_normal[face_vertexs[i].vertex3].y += face_normal.y;
		vertex_normal[face_vertexs[i].vertex3].z += face_normal.z;
	}
	/*for (int k = 0; k < vertex_normal.size(); k++) {
		std::cout << vertex_normal[k].x << " " << vertex_normal[k].y << " " << vertex_normal[k].z << std::endl;
	}*/

}

void normalize() {
	//正規化每個點的normal
	for (int i = 0; i < vertex_normal.size(); i++) {
		float x = vertex_normal[i].x;
		float y = vertex_normal[i].y;
		float z = vertex_normal[i].z;
		float normal_length = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

		vertex_normal[i].x = vertex_normal[i].x / normal_length;
		vertex_normal[i].y = vertex_normal[i].y / normal_length;
		vertex_normal[i].z = vertex_normal[i].z / normal_length;
		//std::cout << vertex_normal[i].x << " " << vertex_normal[i].y << " " << vertex_normal[i].z << std::endl;
	}

}


void find_curvature() {
	//參考 http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d 旋轉

	
	point vpoint;

	cv::Mat V, R, I, neiborV, term, term2,test,test2;
	term2 = cv::Mat::zeros(3, 1, CV_32FC1);
	I = (Mat_<float>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
	R = cv::Mat::zeros(3, 3, CV_32FC1);
	test = cv::Mat::zeros(3, 1, CV_32FC1);
	test2 = cv::Mat::zeros(3, 1, CV_32FC1);
	
	//找鄰近點
	set<int> *neibor = new set<int>[points.size()];
	for (int i = 0; i < face_vertexs.size(); i++) {
		neibor[face_vertexs[i].vertex1].insert(face_vertexs[i].vertex2);
		neibor[face_vertexs[i].vertex1].insert(face_vertexs[i].vertex3);

		neibor[face_vertexs[i].vertex2].insert(face_vertexs[i].vertex1);
		neibor[face_vertexs[i].vertex2].insert(face_vertexs[i].vertex3);

		neibor[face_vertexs[i].vertex3].insert(face_vertexs[i].vertex1);
		neibor[face_vertexs[i].vertex3].insert(face_vertexs[i].vertex2);

	}

	for (int k = 0; k < vertex_normal.size(); k++) {
		//a cross b
		vpoint.x = -vertex_normal[k].y;
		vpoint.y = vertex_normal[k].x;
		vpoint.z = 0;

		//a dot b 
		float c; //a dot b
		c = -vertex_normal[k].z;

		V = (Mat_<float>(3, 3) << 0, -vpoint.z, vpoint.y, vpoint.z, 0, -vpoint.x, -vpoint.y, vpoint.x, 0);
		R = I + V + V*V*(1 / (1 + c)); //旋轉到(0,0,-1)的旋轉矩陣
		
		neiborV = cv::Mat::zeros(3, neibor[k].size(), CV_32FC1);

		//鄰近點轉移回原點，旋轉
		int j = 0;
		for (set<int>::iterator it = neibor[k].begin(); it != neibor[k].end(); it++) {
			term = (Mat_<float>(3, 1) << points[*it].x - points[k].x, points[*it].y - points[k].y, points[*it].z - points[k].z);
			term2 = R*term;
			neiborV.at<float>(0, j) = term2.at<float>(0, 0);
			neiborV.at<float>(1, j) = term2.at<float>(1, 0);
			neiborV.at<float>(2, j) = term2.at<float>(2, 0);
			j++;
		}

		//解出C
		cv::Mat BIG_U, BIG_d;

		BIG_U = cv::Mat::zeros(neibor[k].size(), 3, CV_32FC1);

		BIG_d = cv::Mat::zeros(neibor[k].size(), 1, CV_32FC1);

		for (int l = 0; l < neibor[k].size(); l++)
		{
			BIG_U.at<float>(l, 0) = neiborV.at<float>(0, l)*neiborV.at<float>(0, l);
			BIG_U.at<float>(l, 1) = 2 * neiborV.at<float>(0, l)*neiborV.at<float>(1, l);
			BIG_U.at<float>(l, 2) = neiborV.at<float>(1, l)*neiborV.at<float>(1, l);

			BIG_d.at<float>(l, 0) = neiborV.at<float>(2, l);
		}
		cv::Mat C_matrix;
		C_matrix = (BIG_U.t()*BIG_U).inv() * BIG_U.t()*BIG_d;

		//printf("C = [%f %f %f]^T\n", C_matrix.at<float>(0, 0), C_matrix.at<float>(1, 0), C_matrix.at<float>(2, 0));
		float Cx = C_matrix.at<float>(0, 0);
		float Cy = C_matrix.at<float>(1, 0);
		float Cz = C_matrix.at<float>(2, 0);
		
		cv::Mat CM;
		cv::Mat eigenValue;
		CM = (Mat_<float>(2, 2) << Cx, Cy, Cy, Cz);
		cv::eigen(CM, eigenValue);
		/*cout << "k1 : " << eigenValue.at<float>(0, 0);
		cout << "k2 : " << eigenValue.at<float>(1, 0) << endl;*/

		//解 eigenvalue
		float k1 = eigenValue.at<float>(0, 0);
		float k2 = eigenValue.at<float>(1, 0);
		curvature.push_back((k1 + k2) / 2);
	}
}

void writeobj() {
	vector<float> curvature_copy;
	curvature_copy.assign(curvature.begin(), curvature.end());
	std::sort(curvature.begin(), curvature.end());

	int middle = int(curvature.size() / 2);
	float RGB[3];
	fstream file;
	file.open("M10509111_dragon.obj", ios::out | ios::app);
	for (int i = 0; i < curvature.size(); i++) {
		ColorMap(curvature_copy[i], curvature[middle] - 0.25, curvature[middle] + 0.25, RGB);

		if (file.fail()) {
			cout << "無法開檔\n";
		}
		else {
			file << "v " << points[i].x << " " << points[i].y << " " << points[i].z << " ";
			file << RGB[0] << " " << RGB[1] << " " << RGB[2] << endl;
		}
	}

	for (int j = 0; j < face_vertexs.size(); j++) {
		if (file.fail()) {
			cout << "無法開檔\n";
		}
		else {
			file << "f " << face_vertexs[j].vertex1 + 1 << " " << face_vertexs[j].vertex2 + 1 << " " << face_vertexs[j].vertex3 + 1 << endl;

		}
	}

	file.close();
}


void ColorMap(float InValue, float Lowerbound, float Upperbound, float *OutputRGB)
{
	unsigned char RGB_table[][3] = { { 0,0,8 },
	{ 0,1,14 },
	{ 0,2,20 },
	{ 0,3,26 },
	{ 0,4,32 },
	{ 0,5,38 },
	{ 0,6,44 },
	{ 0,7,50 },
	{ 0,8,56 },
	{ 0,9,62 },
	{ 0,10,68 },
	{ 0,11,74 },
	{ 0,12,80 },
	{ 0,13,86 },
	{ 0,14,92 },
	{ 0,15,98 },
	{ 0,16,104 },
	{ 0,17,110 },
	{ 0,18,116 },
	{ 0,19,122 },
	{ 0,20,128 },
	{ 0,21,134 },
	{ 0,22,140 },
	{ 0,23,146 },
	{ 0,24,152 },
	{ 0,25,158 },
	{ 0,26,164 },
	{ 0,27,170 },
	{ 0,28,176 },
	{ 0,29,182 },
	{ 0,30,188 },
	{ 0,31,194 },
	{ 0,32,200 },
	{ 0,36,199 },
	{ 0,40,198 },
	{ 0,44,197 },
	{ 0,48,196 },
	{ 0,52,195 },
	{ 0,56,194 },
	{ 0,60,193 },
	{ 0,64,192 },
	{ 0,68,191 },
	{ 0,72,190 },
	{ 0,76,189 },
	{ 0,80,188 },
	{ 0,84,187 },
	{ 0,88,186 },
	{ 0,92,185 },
	{ 0,96,184 },
	{ 0,100,183 },
	{ 0,104,182 },
	{ 0,108,181 },
	{ 0,112,180 },
	{ 0,116,179 },
	{ 0,120,178 },
	{ 0,124,177 },
	{ 0,128,176 },
	{ 0,132,175 },
	{ 0,136,174 },
	{ 0,140,173 },
	{ 0,144,172 },
	{ 0,148,171 },
	{ 0,152,170 },
	{ 0,156,169 },
	{ 0,160,168 },
	{ 0,162,165 },
	{ 0,164,162 },
	{ 0,166,159 },
	{ 0,168,156 },
	{ 0,170,153 },
	{ 0,172,150 },
	{ 0,174,147 },
	{ 0,176,144 },
	{ 0,178,141 },
	{ 0,180,138 },
	{ 0,182,135 },
	{ 0,184,132 },
	{ 0,186,129 },
	{ 0,188,126 },
	{ 0,190,123 },
	{ 0,192,120 },
	{ 0,194,117 },
	{ 0,196,114 },
	{ 0,198,111 },
	{ 0,200,108 },
	{ 0,202,105 },
	{ 0,204,102 },
	{ 0,206,99 },
	{ 0,208,96 },
	{ 0,210,93 },
	{ 0,212,90 },
	{ 0,214,87 },
	{ 0,216,84 },
	{ 0,218,81 },
	{ 0,220,78 },
	{ 0,222,75 },
	{ 0,224,70 },
	{ 8,225,68 },
	{ 16,226,66 },
	{ 24,227,64 },
	{ 32,228,62 },
	{ 40,229,60 },
	{ 48,230,58 },
	{ 56,231,56 },
	{ 64,232,54 },
	{ 72,233,52 },
	{ 80,234,50 },
	{ 88,235,48 },
	{ 96,236,46 },
	{ 104,237,44 },
	{ 112,238,42 },
	{ 120,239,40 },
	{ 128,240,38 },
	{ 136,241,36 },
	{ 144,242,34 },
	{ 152,243,32 },
	{ 160,244,30 },
	{ 168,245,28 },
	{ 176,246,26 },
	{ 184,247,24 },
	{ 192,248,22 },
	{ 200,249,20 },
	{ 208,250,18 },
	{ 216,251,16 },
	{ 224,252,14 },
	{ 232,253,12 },
	{ 240,254,10 },
	{ 248,255,8 },
	{ 255,255,0 },
	{ 255,251,0 },
	{ 255,247,0 },
	{ 255,243,0 },
	{ 255,239,0 },
	{ 255,235,0 },
	{ 255,231,0 },
	{ 255,227,0 },
	{ 255,223,0 },
	{ 255,219,0 },
	{ 255,215,0 },
	{ 255,211,0 },
	{ 255,207,0 },
	{ 255,203,0 },
	{ 255,199,0 },
	{ 255,195,0 },
	{ 255,191,0 },
	{ 255,187,0 },
	{ 255,183,0 },
	{ 255,179,0 },
	{ 255,175,0 },
	{ 255,171,0 },
	{ 255,167,0 },
	{ 255,163,0 },
	{ 255,159,0 },
	{ 255,155,0 },
	{ 255,151,0 },
	{ 255,147,0 },
	{ 255,143,0 },
	{ 255,139,0 },
	{ 255,135,0 },
	{ 255,131,0 },
	{ 255,124,0 },
	{ 255,120,2 },
	{ 255,116,4 },
	{ 255,112,6 },
	{ 255,108,8 },
	{ 255,104,10 },
	{ 255,100,12 },
	{ 255,96,14 },
	{ 255,92,16 },
	{ 255,88,18 },
	{ 255,84,20 },
	{ 255,80,22 },
	{ 255,76,24 },
	{ 255,72,26 },
	{ 255,68,28 },
	{ 255,64,30 },
	{ 255,60,32 },
	{ 255,56,34 },
	{ 255,52,36 },
	{ 255,48,38 },
	{ 255,44,40 },
	{ 255,40,42 },
	{ 255,36,44 },
	{ 255,32,46 },
	{ 255,28,48 },
	{ 255,24,50 },
	{ 255,20,52 },
	{ 255,16,54 },
	{ 255,12,56 },
	{ 255,8,58 },
	{ 255,4,60 },
	{ 255,0,62 },
	{ 255,0,64 },
	{ 255,2,70 },
	{ 255,4,76 },
	{ 255,6,82 },
	{ 255,8,88 },
	{ 255,10,94 },
	{ 255,12,100 },
	{ 255,14,106 },
	{ 255,16,112 },
	{ 255,18,118 },
	{ 255,20,124 },
	{ 255,22,130 },
	{ 255,24,136 },
	{ 255,26,142 },
	{ 255,28,148 },
	{ 255,30,154 },
	{ 255,32,160 },
	{ 255,34,166 },
	{ 255,36,172 },
	{ 255,38,178 },
	{ 255,40,184 },
	{ 255,42,190 },
	{ 255,44,196 },
	{ 255,46,202 },
	{ 255,48,208 },
	{ 255,50,214 },
	{ 255,52,220 },
	{ 255,54,226 },
	{ 255,56,232 },
	{ 255,58,238 },
	{ 255,60,244 },
	{ 255,62,250 },
	{ 255,63,255 },
	{ 255,69,255 },
	{ 255,75,255 },
	{ 255,81,255 },
	{ 255,87,255 },
	{ 255,93,255 },
	{ 255,99,255 },
	{ 255,105,255 },
	{ 255,111,255 },
	{ 255,117,255 },
	{ 255,123,255 },
	{ 255,129,255 },
	{ 255,135,255 },
	{ 255,141,255 },
	{ 255,147,255 },
	{ 255,153,255 },
	{ 255,159,255 },
	{ 255,165,255 },
	{ 255,171,255 },
	{ 255,177,255 },
	{ 255,183,255 },
	{ 255,189,255 },
	{ 255,195,255 },
	{ 255,201,255 },
	{ 255,207,255 },
	{ 255,213,255 },
	{ 255,219,255 },
	{ 255,225,255 },
	{ 255,231,255 },
	{ 255,237,255 },
	{ 255,243,255 },
	{ 255,249,255 } };
	float tmp;
	if (Lowerbound > Upperbound)
	{
		tmp = Upperbound;   Upperbound = Lowerbound; Lowerbound = tmp;
	} //Make sure Lowerbound < Upperbound;

	if (InValue <= Lowerbound) {
		for (int i = 0; i < 3; i++)
			OutputRGB[i] = float(RGB_table[0][i]) / 255.0;
		return;
	}
	if (InValue >= Upperbound) {
		for (int i = 0; i < 3; i++)
			OutputRGB[i] = float(RGB_table[255][i]) / 255.0;
		return;
	}
	int index = (int)(255.0*(InValue - Lowerbound) / (Upperbound - Lowerbound));

	if (index < 0) index = 0;
	if (index > 255) index = 255;

	for (int i = 0; i < 3; i++)
		OutputRGB[i] = float(RGB_table[index][i]) / 255.0;
	return;
}

