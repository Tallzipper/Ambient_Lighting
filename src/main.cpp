#include <opencv2/opencv.hpp>
#include <iostream>

/*
 * How to use: 
 * 
 *  1. Hardware:
 * 
 *      [REQUIRED]
 *          - HDMI Splitter
 *              * Must be active power and downscale the feed in order to be 
 *                handled by your computer
 *              * My HDMI Splitter takes in one input and has two outputs:
 *                  - One of the outputs is for your tv/screen and the other
 *                    is for the capture card
 *                  - You can have two or more inputs in case you want to switch
 *                    your display without needing to switch cables
 * 
 *          - Computer
 *              * Must be a semi-modern computer; the Video Capture card is a 
 *                bit of a resource junkie so be warned
 *                  - If you'll use a Raspberry Pi, have it be 4 or above
 *              * My current computer runs this program fine:
 *                  - CPU:           Intel(R) Core(TM) Ultra 7 155U (1.70) GHz
 *                  - RAM:           32.0 GB
 *                  - Graphics Card: 128 MB
 * 
 *          - Video Capture Card
 *              * This is optional if you only want to test it since you can tweak 
 *                the code to remove the device-scanning loop and point the capture
 *                connection directly to your webcam instead
 *              * Must have a good frame rate or will lag behind noticeably
 *              * The one I used is 4K USB 3.0 HDMI to USB C at 1080P 60FPS
 *                because it's quick and reliable, but the 4k part isn't needed,
 *                only that it outputs to the system at 1080P
 * 
 *  2. Calibrations:
 * 
 *      - Where the `edgePixels` variable is declared, you can edit that to be any
 *        number you'd like as it determines deep into the screen the lights will 
 *        grab what is being displayed
 * 
 *      - If you have too many things plugged in, you can change the condition in the 
 *        hardware setup `for` loop and increase the index maximum to as high as you 
 *        need for the video capture system to find the card
 * 
 *  3. Controls:
 *  
 *      - Five preview windows will open showing your video feed and the individual
 *        edge colors
 *      - Exit the program at any time by pressing 'q' with any opened window in focus
 * 
 */

int main(){

    int index = -1;              
    cv::Mat screen;             // For the image matrix to allocate LEDs
    cv::VideoCapture capture;   // Hardware for capturing video, 
                                //     0 is webcam, 1 and up is video capture

    int edgePixels = 60;         // Adjustable, grabs edge of screen and 
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

        // Displays color information of the screen into the terminal in order of Blue Green Red

        std::cout << "\033[2J\033[1;1H"; // current terminal will be replaced with new information

        std::cout << "Left Edge BGR:   (" << leftB << ", " << leftG << ", " << leftR << ")\n";
        std::cout << "Right Edge BGR:  (" << rightB << ", " << rightG << ", " << rightR << ")\n";
        std::cout << "Top Edge BGR:    (" << topB << ", " << topG << ", " << topR << ")\n";
        std::cout << "Bottom Edge BGR: (" << bottomB << ", " << bottomG << ", " << bottomR << ")\n";

        std::cout << std::flush; // Information is pushed out quickly due to speed of program

        // Updating/Creating windows 

        cv::imshow("Ambient Light", screen);

        cv::imshow("Left Strip", leftLight);
        cv::imshow("Right Strip", rightLight);
        cv::imshow("Top Strip", topLight);
        cv::imshow("Bottom Strip", bottomLight);

        // Press q to escape

        if (cv::waitKey(30) == 'q'){
            break;
        }

    }

    // When escaped, stop recording, kill windows, and exit program

    capture.release();
    cv::destroyAllWindows();
    
    return 0;

}