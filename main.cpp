#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<bits/stdc++.h>

using namespace std;
using namespace cv;

#define initialCount 10

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
      ballSpeed = rand()%6 + 5;
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
          cLocation.x += (-2*signbit(targetError.x)+1)*minValX
                  *cos(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
          cLocation.y += (-2*signbit(targetError.y)+1)*minValY
                  *sin(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
          }
          else
          {
              cLocation.y += (-2*signbit(targetError.y)+1)*minValY;
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

        ballSpeed = rand()%6 + 5;

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
            cLocation.x += (-2*signbit(targetError.x)+1)*minValX
                    *cos(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
            cLocation.y += (-2*signbit(targetError.y)+1)*minValY
                    *sin(abs(atan((targetError.y*1.0)/(targetError.x*1.0))));
            }
            else
            {
                cLocation.y += (-2*signbit(targetError.y)+1)*minValY;
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

int main()
{
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
    vector<Bludger> obj;
    Snitch snitch;
    for(int i=0;i<initialCount;i++)
    {
        Bludger temp;
        obj.push_back(temp);
    }
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
        findHand(orig);
        circle(orig,Point(hx,hy),10,Scalar(255,0,0),-1);
        for(int i=0;i<obj.size();i++)
        {
            obj[i].animate();
            circle(orig,obj[i].cLocation,obj[i].radius,Scalar(0,0,255),-1);
            cout<<obj[i].cLocation<<" "<<obj[i].tLocation<<endl;
        }
        snitch.animate();
        if(abs_distance(Point(hx,hy),snitch.cLocation)<snitch.radius*5)
        {
            for(int i=0;i<obj.size()/2;i++)
            {
                Point target;
                target.x = snitch.cLocation.x + 5*snitch.radius*(-2*(rand()%2)+1)*((rand()%100)*1.0/100.0);
                target.y = snitch.cLocation.y + 5*snitch.radius*(-2*(rand()%2)+1)*((rand()%100)*1.0/100.0);
                obj[i].tLocation = target;
                obj[i].ballSpeed = 20;
            }
        }
        circle(orig,snitch.cLocation,snitch.radius,Scalar(0,255,255),-1);
        flip(orig,orig,1);
        imshow("Screen",orig);
        if(waitKey(10)!=-1)
            break;
    }
}

