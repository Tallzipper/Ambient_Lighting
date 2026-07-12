#include <opencv2/opencv.hpp>
#include <iostream>

int main(){

    int index = -1;              
    cv::Mat screen;             // For the image matrix to allocate LEDs
    cv::VideoCapture capture;   // Hardware for capturing video, 
                                 //     0 is webcam, 1 and up is video capture

    int edgePixels = 60;         // Adjustable, grabbs edge of screen and 
                                 //     gives that to the LEDs

    for(int i = 1; i <= 2; i++){ // Trying indices to see which is the video capture
        capture.open(i, cv::CAP_DSHOW);

        if(capture.isOpened()){
            cv::Mat testScreen;
            capture >> testScreen;
            
            if(!testScreen.empty()){    // If i is video capture, use i
                index = i;
                break;
            }   
        }
    }

    if(!capture.isOpened()){     // Ensures video capture is connected;
        std::cerr << "Video Capture Hardware failed to initialize";
        return -1;
    }

    while(1){

        capture >> screen;       // Grab the current set of pixels on screen
        if(screen.empty()){      // Verifys that screen had something
            std::cerr << "Video Capture did not grab what was on the screen";
            break;                   
        }

        int width = screen.cols;
        int height = screen.rows;

        // Creates an area of each of the pixels 

        cv::Rect leftArea(0, 0, edgePixels, height);
        cv::Rect rightArea(width - edgePixels, 0, edgePixels, height);
        cv::Rect topArea(0, 0, width, edgePixels);
        cv::Rect bottomArea(0, height - edgePixels, width, edgePixels);

        // Creates a sub-image from the area of the pixels with its own colors

        cv::Mat leftPixels      = screen(leftArea);
        cv::Mat rightPixels     = screen(rightArea);
        cv::Mat topPixels       = screen(topArea);
        cv::Mat bottomPixels    = screen(bottomArea);

        // Creates a sort of array holding color values

        cv::Scalar leftMean     = cv::mean(leftPixels);
        cv::Scalar rightMean    = cv::mean(rightPixels);
        cv::Scalar topMean      = cv::mean(topPixels);
        cv::Scalar bottomMean   = cv::mean(bottomPixels);

        // Colors for the borders

        int leftB               = static_cast<int>(leftMean[0]);
        int leftG               = static_cast<int>(leftMean[1]);
        int leftR               = static_cast<int>(leftMean[2]);

        int rightB              = static_cast<int>(rightMean[0]);
        int rightG              = static_cast<int>(rightMean[1]);
        int rightR              = static_cast<int>(rightMean[2]);

        int topB                = static_cast<int>(topMean[0]);
        int topG                = static_cast<int>(topMean[1]);
        int topR                = static_cast<int>(topMean[2]);

        int bottomB             = static_cast<int>(bottomMean[0]);
        int bottomG             = static_cast<int>(bottomMean[1]);
        int bottomR             = static_cast<int>(bottomMean[2]);

        cv::Mat leftLight(100, 100, CV_8UC3, leftMean);
        cv::Mat rightLight(100, 100, CV_8UC3, rightMean);
        cv::Mat topLight(100, 100, CV_8UC3, topMean);
        cv::Mat bottomLight(100, 100, CV_8UC3, bottomMean);

        std::cout << "\033[2J\033[1;1H"; 

        std::cout << "Left Edge BGR:   (" << leftB << ", " << leftG << ", " << leftR << ")\n";
        std::cout << "Right Edge BGR:  (" << rightB << ", " << rightG << ", " << rightR << ")\n";
        std::cout << "Top Edge BGR:    (" << topB << ", " << topG << ", " << topR << ")\n";
        std::cout << "Bottom Edge BGR: (" << bottomB << ", " << bottomG << ", " << bottomR << ")\n";

        std::cout << std::flush;
        

        cv::imshow("Ambient Light", screen);

        cv::imshow("Left Strip", leftLight);
        cv::imshow("Right Strip", rightLight);
        cv::imshow("Top Strip", topLight);
        cv::imshow("Bottom Strip", bottomLight);

        if (cv::waitKey(30) == 'q'){
            break;
        }

    }

    capture.release();
    cv::destroyAllWindows();
    
    return 0;

}