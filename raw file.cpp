#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include<Windows.h>
#include<conio.h>
#include <iostream>
#include<math.h>
#include <fstream>
#include<string>
#include"Serial.h"
#include"stdafx.h"

#define pwm_min 100
#define pwm_front 60
#define range_high 35
#define range_low_actual 29
#define range_high_actual 29
#define rotate_90 99
#define RX_BUFFSIZE 20
#define go_front 50
#define go_front_slow 40
#define pwm_near 90


void printUsage(_TCHAR progName[]);

using namespace cv;
using namespace std;

Mat originalImg, drawing, canny;
char command[8];
bool pick_or_place = false;
bool hand_motion = false;
VideoCapture cap(1);
/*int ilowH = 10;
int ihighh = 30;
int ilowS = 117;
int ihighS = 255;
int ilowV = 154;
int ihighV = 255;
//double hhh = 0;*/

class obj
{
	int ilowH;
	int ihighh;
	int ilowS;
	int ihighS;
	int ilowV;
	int ihighV;
	static int count1;
	static char message[20];
	static vector<vector<Point>> contour;
	static vector<vector<Point>> contour1;
	static vector<Vec4i> hh;
	vector<double> area;
	static vector<vector<Point>> contours_poly;
	vector<Point2f>center;
	vector<float>radius;
public:
	Point cen;
	float radv;
	float radr;
	Mat thresholdImg;
	int count;
	int lval, hval;
	//int a = 10, int b = 30, int c = 117, int d = 255, int e = 154, int f = 255
	obj(int a = 80, int b = 240)
	{
		radv = 0;
		radr = 0;
		center.push_back(Point2f(0, 0));
		radius.push_back(0);
		lval = a;
		hval = b;
		/*ilowH = a;
		ihighh = b;
		ilowS = c;
		ihighS = d;
		ilowV = e;
		ihighV = f;*/
	}
	void morpho(void);
	bool contourwithfilter(const Mat* originalImg,const char* ch);
	int angle(obj a, obj b);
	//void dist(Point,Point);
	int calibrate(char* ch);
	void read_file(char* ch);
	~obj()
	{}


};
vector<vector<Point>> obj::contour;
vector<vector<Point>> obj::contours_poly;
vector<Vec4i> obj::hh;
char obj::message[20] = {};
int obj::count1 = 0;
void obj::read_file(char* ch)
{
	ifstream o(ch, ios::binary);
	o.read((char*)&ilowH, sizeof(ilowH));
	o.read((char*)&ihighh, sizeof(ihighh));
	o.read((char*)&ilowS, sizeof(ilowS));
	o.read((char*)&ihighS, sizeof(ihighS));
	o.read((char*)&ilowV, sizeof(ilowV));
	o.read((char*)&ihighV, sizeof(ihighV));
	//cout << "value of" << ch << "is               " << ilowH << "   " << ihighh << endl;
	o.close();
}
int obj::calibrate(char* ch)
{
	//const char* FILENAM = "hsv.txt";
	
	//namedWindow("trackbars", 1);
	//createTrackbar("threshold", "original_image", &threshold, 100);
	read_file(ch);

	

	createTrackbar("low hue", "trackbars", &ilowH, 179);
	createTrackbar("high hue", "trackbars", &ihighh, 179);

	createTrackbar("low sat", "trackbars", &ilowS, 255);
	createTrackbar("high sat", "trackbars", &ihighS, 255);

	createTrackbar("low value", "trackbars", &ilowV, 255);
	createTrackbar("high value", "trackbars", &ihighV, 255);
	

	cout << "calibration for " << ch << " press esc when done" << endl;
	while (1)
	{
		bool b = cap.read(originalImg);
		if (!b)
		{
			cout << "cant read the " << endl;
			return -1;
		}
		morpho();
		imshow("trackbars",thresholdImg);
		if (waitKey(33) == 27)
			break;
	}
	cout << "do you want to update value" << endl;
	if (waitKey(0) == 'y')
	{
		ofstream o(ch, ios::binary);
		o.write((char*)&ilowH, sizeof(ilowH));
		o.write((char*)&ihighh, sizeof(ihighh));
		o.write((char*)&ilowS, sizeof(ilowS));
		o.write((char*)&ihighS, sizeof(ihighS));
		o.write((char*)&ilowV, sizeof(ilowV));
		o.write((char*)&ihighV, sizeof(ihighV));
		o.close();
	}
	
	if (count == 3)
		destroyAllWindows();
	count++;
	//cap.release();



}
float dist(Point P1,Point P2,float rad)
{
	float d = 2.6 / rad;
	//cout << "                                                                          " << d << endl;
	char message[24];
	float dis = sqrt(pow((P1.x - P2.x),2)+ pow((P1.y - P2.y),2));
	Point p = (P1 + P2) / 2;
	float disr = dis*d;
	sprintf(message, "real dist=%f",disr);
	putText(drawing,message, Point(350, 350), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
	//cout << "real distance is" << disr << endl;
	return disr;

}
int obj::angle(obj b, obj c)
{

	Point ab = cen - b.cen;
	Point ac = cen - c.cen;
	float dot = float(ab.dot(ac));
	float cross = (float)ab.cross(ac);
	float alpha = atan2(cross, dot);
	int ang = (int)floor(alpha * 180. / 3.14 + 0.5);
	sprintf(message, "angle is %d", ang);
	putText(drawing, message, Point(350,400), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 2);
	return ang;
}
void obj::morpho(void)
{
	GaussianBlur(originalImg, originalImg, Size(3, 3), 2, 2);
	cvtColor(originalImg, canny, COLOR_BGR2HSV);
	inRange(canny, Scalar(ilowH, ilowS, ilowV), Scalar(ihighh, ihighS, ihighV), thresholdImg);
	//morphological opening for eroding the salt noise
	erode(thresholdImg, thresholdImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(thresholdImg, thresholdImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing for removing pepper noise
	dilate(thresholdImg, thresholdImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(thresholdImg, thresholdImg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
}

bool obj::contourwithfilter(const Mat* originalImg1,const char* ch)
{
	int j = 0;
	vector<int>::iterator iter;
	
	findContours(thresholdImg, contour, hh, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	if (contour.size() == 0)
	{
		cout << "no contour found" << endl;
		return false;
	}
	int k = contour.size();
	for (int i = 0; i <k; i++)
	{
		//contours

		Moments moment = moments((Mat)(contour[i]));
		std::cout << "no of countor" << contour.size() << endl;
		area.push_back(moment.m00);
	//	cout << "size=" << area.size() << endl;

		cout << "area of contour" << ch<<"   "<<i << "is           " << area[i] << endl;
		if (area[i] <= 15 || area[i] >= (thresholdImg.rows)*(thresholdImg.cols / 10))
		{
			continue;
		}
		//area.erase(area.begin() + j);
		area.at(j) = area[i];
		//contour.erase(contour.begin()
		contour.at(j) = contour[i];
		j++;
	}
	area.clear();

	if (j != 1)
	{
		cout << "more than one object found" << endl;
		return false;
	}
	contours_poly.push_back(contour.front());
	approxPolyDP(Mat(contour[0]), contours_poly[0], 3, true);
	//cout << "new size of contour" << contours_poly.size() << endl;
	minEnclosingCircle((Mat)contours_poly[0], center[0], radius[0]);
	contours_poly.pop_back();
	circle(drawing, center[0], 3, Scalar(255, 255, 0), -1);
	circle(drawing, center[0], radius[0], Scalar(255, 255, 0), 3);
	cen = center[0];
	radv = radius[0];
	return true;

}
/*void set_hsv_blue()
{
ilowH = 90;
ihighh = 128;
ilowS = 77;
ihighS = 194;
ilowV = 80;
ihighV = 255;
}
void set_hsv_ball()
{
// lll = 30;
//hhh = 60;
ilowH = 20;
ihighh = 40;
ilowS = 54;
ihighS = 131;
ilowV = 117;
ihighV = 255;
}
void set_hsv_green()
{
//l = 230;
//h = 260;
ilowH = 101;
ihighh = 125;
ilowS = 37;
ihighS = 117;
ilowV = 51;
ihighV = 208;
}*/

bool robot(int ang, float dist)
{
	
	char dir='0';
	int pwm1=0, delay=10;
	char side1=3, side2=3;
	if (!pick_or_place&&dist > 50 && dist < 90)
			dir = 'Q';
	if((dist<=range_high&&dist>=range_low_actual)&&((ang>=-179&&ang<=-173)))
	{
		if (!pick_or_place)
			dir = 'W';
		else
		{
			dir = 'Q';
			side1 = 9;
		}
		sprintf(command, "%c%d%d%d%d", dir, side1, side2, pwm1, delay);
		hand_motion = true;
		
		return false;
	}
	else if ((dist <range_low_actual && ((ang >= -175 && ang <= -90) || (ang >=90 && ang <= 180))))
	{
		
		dir = 'a';
		side1 = 2;
		side2 = 2;
		pwm1 = pwm_min;
		delay = rotate_90;
	}
	else if ((dist <range_low_actual && ((ang >-90 && ang <= -1) || (ang >= 1 && ang <90))))
	{
		dir = 'a';
		side1 = 1;
		side2 = 1;
		pwm1 = pwm_min;
		delay = rotate_90;
	}
	else if (ang<170&&ang >= 90)
	{
		dir = 'a';
		side1 = 2;
		side2 = 1;
		pwm1 = pwm_min;
		delay = rotate_90;
	}
	else if (ang >= 0&&ang<90)
	{
		dir = 'a';
		side1 = 2;
		side2 = 1;
		pwm1= pwm_min;
		delay = rotate_90;

	}
	else if (ang >= -90 && ang <= -1)
	{
		dir = 'a';
		side1 = 1;
		side2 = 2;
		pwm1 = pwm_min;
		delay = rotate_90;
	}
	else if (ang >= -160 && ang <= -89)
	{
		dir = 'a';
		side1 = 1;
		side2 = 2;
		pwm1 = pwm_min;
		delay = rotate_90;
	}
	else if (ang >= -173 && ang <= -160)
	{
		dir = 'a';
		side1 = 1;
		side2 = 2;
		pwm1 = pwm_near;
		delay = 15;
	}
	else if (ang > 170 || (ang>=-180 && ang<=-178))
	{
		dir = 'a';
		side1 = 2;
		side2 = 1;
		pwm1 = pwm_near;
		delay = rotate_90;
	}
	else if ((ang <= -174 && ang >= -177))
	{
		dir = 'a';
		side1 = 1;
		side2 = 1;
		pwm1 = pwm_front;
		if (dist < 38 && dist>29)
			delay = go_front_slow;
		else
			delay = go_front;
	}
	sprintf(command, "%c%d%d%d%d", dir, side1, side2, pwm1, delay);
	return true;
}




int _tmain(int argc, _TCHAR* argv[])
{
	obj ball(97,110), blue_circle(228,242), green_circle(130,170), brown_circle;
	int cry = 0;
	bool b_or_ball=true;
	char buffer[RX_BUFFSIZE];
	if (argc != 2)
	{
		printUsage(argv[0]);

		cout << "press any key and enter to quit" << endl;
		char temp;
		cin >> temp;

		return 10;
	}
	/*try
	{
		std::cout << "Opening com port" << endl;
		
		Serial serial(commPortName);
		cout << "Port opened" << endl;
		serial.flush();
	}
	catch (const char *msg)
	{
		cout << msg << endl;
	}*/
	tstring commPortName(argv[1]);
  Serial serial(commPortName);
	serial.flush();


	Mat img_gray, img_hsv, globalthreshold;

	//namedWindow("image", 1);
	namedWindow("trackbars", 1);
	vector<Vec3f> circles;
	namedWindow("original_image", 1);
	cout << "if you want to calibrate the press y" << endl;
	if (waitKey(0) == 'y')
	{
		ball.calibrate("ball");
		blue_circle.calibrate("blue_circle");
		green_circle.calibrate("green_circle");
		brown_circle.calibrate("brown_circle");

	}

	destroyWindow("trackbars");
	if (b_or_ball)
		ball.read_file("ball");
	else
		ball.read_file("brown_circle");
	blue_circle.read_file("blue_circle");
	green_circle.read_file("green_circle");
	brown_circle.read_file("brown_circle");
	while (1)
	{
		
		if(cry)
			 {
				 int bytesWritten =serial.write(command);
				 
				cout << "written           " << bytesWritten << "                       " << command << endl;
				if (hand_motion)
				{
					waitKey(3500);
					hand_motion = false;
				}
	          }
		else
		{
			command[0] = '0';
			int bytesWritten = serial.write("0n000000");
			cout << "written           "<<bytesWritten<<"                       " << command << endl;
		}
		if (waitKey(30) == 27)
			return -1;
		cry = 0;
		cap >>img_hsv;
		//originalImg = img_hsv.clone();
		img_hsv.copyTo( originalImg);
		imshow("original_image", originalImg);
		drawing = Mat::zeros(originalImg.size(), CV_8UC3);
		if (b_or_ball)
		{
			ball.read_file("ball");
			pick_or_place = 1;
		}
		else
			ball.read_file("brown_circle");
		//set_hsv_ball();
		ball.morpho();
		if (!ball.contourwithfilter(&originalImg,"ball"))
		{
			cout << "ball not found" << endl;
			continue;
		}
		//set_hsv_blue();
		blue_circle.morpho();
		if (!blue_circle.contourwithfilter(&originalImg,"blue_circle"))
		{
			cout << "middle circle not found" << endl;
			continue;
		}
		//set_hsv_green();
		green_circle.morpho();
		if (!green_circle.contourwithfilter(&originalImg,"green_circle"))
		{
			cout << "upper circle not found" << endl;
			continue;
		}
		float disr=dist(blue_circle.cen,ball.cen,blue_circle.radv);
	  //  dist(blue_circle.cen,green_circle.cen,blue_circle.radv);
		line(drawing, ball.cen, blue_circle.cen, Scalar(255, 255, 0), 1);
		line(drawing, blue_circle.cen, green_circle.cen, Scalar(255, 255, 0), 1);
		int ang=blue_circle.angle(ball, green_circle);
		
		add(ball.thresholdImg, green_circle.thresholdImg, globalthreshold);
		add(globalthreshold, blue_circle.thresholdImg, globalthreshold);
		imshow("morph", globalthreshold);
		add(drawing, img_hsv,img_gray);
		imshow("original_image", img_gray);
		b_or_ball = robot(ang, disr);
		
		cry = 1;
		//system("CLS");
		//originalImg.release();
		

	}





}
void printUsage(_TCHAR progName[])
{
#if defined(UNICODE)
	wcout << progName << " <comm port>" << endl
		<< "e.g., " << progName << " COM1" << endl;
#else
	cout << progName << " <comm port>" << endl
		<< "e.g., " << progName << " COM1" << endl;
#endif

}