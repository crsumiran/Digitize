#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include<stdio.h>
#include<stdlib.h>
#include<cmath>
#include<time.h>
#include<iostream>
//#include<bits/stdc++.h>

using namespace std;
using namespace cv;
int level_id, snitch_max_speed, bludger_max_speed;
int life;
#define max_levels 15;
#define initialCount 10
#define drawCross( img, center, color, d )\
line(img, Point(center.x - d, center.y - d), Point(center.x + d, center.y + d), color, 2, CV_AA, 0);\
line(img, Point(center.x + d, center.y - d), Point(center.x - d, center.y + d), color, 2, CV_AA, 0 )\


int hx=340 ,hy = 240;
int hl=0,hh=0,sl=0,sh=0,vl=0,vh=0;

double abs_distance(Point a,Point b)
{
    return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

class Snitch{
public:
  static Size imageSize;
  Point cLocation,tLocation,targetError;
  int radius,ballSpeed;
  Snitch()
  {
      radius = 15;
      ballSpeed = rand()%6 + 5;
      cLocation = Point(rand()%(imageSize.width-2*radius)+radius
                        ,rand()%(imageSize.height-2*radius)+radius);
      setLocation();
  }
  void setLocation()
  {
      tLocation = Point(rand()%(imageSize.width-2*radius)+radius
                        ,rand()%(imageSize.height-2*radius)+radius);
      ballSpeed = rand()%(snitch_max_speed)+ 5;
      if(ballSpeed==7)
          ballSpeed = 20;
  }

  void animate()
  {
      targetError.x = tLocation.x - cLocation.x;
      targetError.y = tLocation.y - cLocation.y;
      if(abs(targetError.x) > radius || abs(targetError.y) > radius)
      {
          int minValX = abs(targetError.x)<ballSpeed?abs(targetError.x):ballSpeed;
          int minValY = abs(targetError.y)<ballSpeed?abs(targetError.y):ballSpeed;
          if(targetError.x!=0)
          {
          cLocation.x += (-2*signbit((double)targetError.x)+1)*minValX
                  *cos(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
          cLocation.y += (-2*signbit((double)targetError.y)+1)*minValY
                  *sin(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
          }
          else
          {
              cLocation.y += (-2*signbit((double)targetError.y)+1)*minValY;
          }
      }
      else
      {
          setLocation();
      }
  }
};

class Bludger{
  public:
    static Size imageSize;
    Point cLocation,tLocation,targetError;
    int radius,ballSpeed,prevTargetDir=-1,nextTargetDir;
    Bludger()
    {   
        radius = rand()%13 + 8;
        ballSpeed = rand()%6 + 10;
        cLocation = Point(rand()%(imageSize.width-2*radius)+radius
                          ,rand()%(imageSize.height-2*radius)+radius);
        setLocation();
    }
    void setLocation()
    {
        while((nextTargetDir=rand()%4)==prevTargetDir);

        prevTargetDir = nextTargetDir;

        if(nextTargetDir == 0)
            tLocation = Point(0,rand()%imageSize.height);
        else if(nextTargetDir == 1)
            tLocation = Point(rand()%imageSize.width+radius,0);
        else if(nextTargetDir == 2)
            tLocation = Point(imageSize.width-1,rand()%imageSize.height);
        else
            tLocation = Point(rand()%imageSize.width,imageSize.height-1);

        ballSpeed = rand()%(bludger_max_speed) + 5;

        if(ballSpeed==7)
            ballSpeed = 20;
    }

    void animate()
    {
        targetError.x = tLocation.x - cLocation.x;
        targetError.y = tLocation.y - cLocation.y;
        if(abs(targetError.x) > radius || abs(targetError.y) > radius)
        {
            int minValX = abs(targetError.x)<ballSpeed?abs(targetError.x):ballSpeed;
            int minValY = abs(targetError.y)<ballSpeed?abs(targetError.y):ballSpeed;
            if(targetError.x!=0)
            {
            cLocation.x += (-2*signbit((double)targetError.x)+1)*minValX
                    *cos(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
            cLocation.y += (-2*signbit((double)targetError.y)+1)*minValY
                    *sin(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
            }
            else
            {
                cLocation.y += (-2*signbit((double)targetError.y)+1)*minValY;
            }
        }
        else
        {
            setLocation();
        }
    }
};
Size Bludger::imageSize = Size(0,0);
Size Snitch::imageSize = Size(0,0);
vector<Bludger> obj;
void findHand(Mat orig)
{
    vector<vector<Point> > contours;
    Mat thresh = orig.clone();
    cvtColor(thresh,thresh,CV_BGR2HSV);
    inRange(thresh,Scalar(0,95,66),Scalar(5,200,180),thresh);
    medianBlur(thresh,thresh,7);
    dilate(thresh,thresh,getStructuringElement(MORPH_RECT,Size(5,5)));
    imshow("Thresh",thresh);
    findContours(thresh,contours,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE);
    int max = -1,pos = -1;
    for(int i=0;i<contours.size();i++)
    {
        if(contourArea(contours[i])>max)
        {
            max = contourArea(contours[i]);
            pos = i;
        }
    }
    if(contours.size())
    {
        Moments mu;
        mu = moments( contours[pos], false );
        Point mc;
        mc = Point( mu.m10/mu.m00 , mu.m01/mu.m00 );
        hx = mc.x;
        hy = mc.y;
    }
}
void level_control(int lev, Mat &img)
{
	level_id = lev;
	obj.erase(obj.begin(), obj.end());
	int i;
	switch (level_id)
	{
	case 1: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i < 3; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 1",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 2: { bludger_max_speed = 4;
		snitch_max_speed = 5;
		for (i = 0; i < 4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 2",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 3: { bludger_max_speed = 7;
		snitch_max_speed = 5;
		for (i = 0; i < 5; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 3",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 4: { bludger_max_speed = 8;
		snitch_max_speed = 5;
		for (i = 0; i < 6; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 4",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 5: { bludger_max_speed = 10;
		snitch_max_speed = 7;
		for (i = 0; i<6; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 5",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 6: { bludger_max_speed = 10;
		snitch_max_speed = 8;
		for (i = 0; i<7; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 6",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}

	case 7: { bludger_max_speed = 11;
		snitch_max_speed = 9;
		for (i = 0; i<8; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 7",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 8: { bludger_max_speed = 11;
		snitch_max_speed = 9;
		for (i = 0; i<9; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 8",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 9: { bludger_max_speed = 12;
		snitch_max_speed = 10;
		for (i = 0; i<9; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 9",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	case 10: { bludger_max_speed =14;
		snitch_max_speed = 10;
		for (i = 0; i<10; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}
		cv::putText(img,
			"Level 10",
			cv::Point(240, 240), // Coordinates
			cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			4.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255), // Color
			1, // Thickness
			CV_AA); // Anti-alias
		imshow("Screen", img);
		waitKey(30);

		break;
	}
	default: break;
	/*case 11: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i<4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}


		break;
	}
	case 12: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i<4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}


		break;
	}
	case 13: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i<4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}


		break;
	}
	case 14: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i<4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}


		break;
	}
	case 15: { bludger_max_speed = 4;
		snitch_max_speed = 4;
		for (i = 0; i<4; i++)
		{
			Bludger temp;
			obj.push_back(temp);
		}


		break;
	}*/

	}
}
void death() {}
int main()
{
	Mat voldemort = imread("voldemort.jpg");
	int abs_dist;
	snitch_max_speed = 2;
	bludger_max_speed = 2;
	KalmanFilter KF(4, 2, 0);
	int posx_avg, posy_avg;
	Mat_<float> state(4, 1);
	Mat_<float> processNoise(4, 1, CV_32F);
	Mat_<float> measurement(2, 1); measurement.setTo(Scalar(0));
	int posx[4], posy[4],count_frame=0,kfx,kfy;
	KF.statePre.at<float>(0) = 0;
	KF.statePre.at<float>(1) = 0;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;

	KF.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1); // Including velocity
	KF.processNoiseCov = (cv::Mat_<float>(4, 4) << 0.2, 0, 0.2, 0, 0, 0.2, 0, 0.2, 0, 0, 0.3, 0, 0, 0, 0, 0.3);

	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(KF.errorCovPost, Scalar::all(.1));
    Mat orig,thresh;
    srand(time(NULL));
    VideoCapture cam(0);
    if(cam.isOpened() == false)
    {
        cout << "Camera not found" << endl;
        return 0;
    }
    if(!cam.read(orig))
    {
        cout << "No frame read" << endl;
        return 0;
    }
    Bludger::imageSize = orig.size();
    Snitch::imageSize = orig.size();
    
    Snitch snitch;
	while(!cam.read(orig)){}
	level_control(2, orig);
	waitKey(1000);
	//	level_id = 1;
    namedWindow("Track");
    createTrackbar("HL","Track",&hl,255);
    createTrackbar("HH","Track",&hh,255);
    createTrackbar("SL","Track",&sl,255);
    createTrackbar("SH","Track",&sh,255);
    createTrackbar("VL","Track",&vl,255);
    createTrackbar("VH","Track",&vh,255);
    createTrackbar("Handx","Track",&hx,640);
    createTrackbar("Handy","Track",&hy,480);

    while(1)
	{
		
        if(!cam.read(orig))
        {
            cout << "No frame read" << endl;
            break;
        }
		
			Mat prediction = KF.predict();
			Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
			findHand(orig);
			measurement(0) = hx;
			measurement(1) = hy;
			Point measPt(measurement(0), measurement(1));

			Mat estimated = KF.correct(measurement);
			Point statePt(estimated.at<float>(0), estimated.at<float>(1));
			kfx = estimated.at<float>(0);
			kfy = estimated.at<float>(1);
			if (life < 0)
			{
				imshow("Screen", voldemort);
				waitKey(0);
				break;
				//death();
			}
			posx[count_frame % 4] = kfx;
			posy[count_frame % 4] = kfy;
			circle(orig, Point(kfx, kfy), 10, Scalar(255, 0, 0), -1);
			//drawCross(orig, statePt, Scalar(255, 255, 255), 5);
			if (count_frame > 4)
			{
				if (abs(posx[0] + posx[1] - posx[2] - posx[3]) < 10 && abs(posy[0] + posy[1] - posy[2] - posy[3]) < 10)
				{
					count_frame = 0;
					cv::putText(orig,
						"STOP!",
						cv::Point(240, 240), // Coordinates
						cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
						4.0, // Scale. 2.0 = 2x bigger
						cv::Scalar(255, 255, 255), // Color
						1, // Thickness
						CV_AA); // Anti-alias
					imshow("Screen", orig);
					waitKey(0);

			}

			for (int i = 0; i < obj.size(); i++)
			{
				obj[i].animate();
				circle(orig, obj[i].cLocation, obj[i].radius, Scalar(0, 0, 255), -1);
				cout << obj[i].cLocation << " " << obj[i].tLocation << endl;
			}
			snitch.animate();
			abs_dist = abs_distance(Point(kfx, kfy), snitch.cLocation);
			if (abs_dist < snitch.radius*1.5)
			{
				if (level_id < 10)
					level_control(level_id + 1, orig);


			}
			if (abs_dist < snitch.radius * 5)
			{
				for (int i = 0; i < 2; i++)
				{
					Point target;
					target.x = snitch.cLocation.x + 5 * snitch.radius*(-2 * (rand() % 2) + 1)*((rand() % 100)*1.0 / 100.0);
					target.y = snitch.cLocation.y + 5 * snitch.radius*(-2 * (rand() % 2) + 1)*((rand() % 100)*1.0 / 100.0);
					obj[i].tLocation = target;
					obj[i].ballSpeed = 20;
				}
			}
			for (int i = 0; i < obj.size(); ++i)
			{
				if (abs_distance(Point(kfx, kfy), obj[i].cLocation) < snitch.radius)
				{
					if (level_id < 4)
						life -= 2 * obj[i].radius;
					else
						life -= obj[i].radius;
				}
			}
			
			circle(orig, snitch.cLocation, snitch.radius, Scalar(0, 255, 255), -1);
		}count_frame++;
        flip(orig,orig,1);
        imshow("Screen",orig);
        if(waitKey(10)!=-1)
            break;
    }
}

