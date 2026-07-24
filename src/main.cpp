#include <opencv2/opencv.hpp>
#include <iostream>

#include "../include/helper.h"

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
 *              * Must be a semi-modern computer, the Video Capture card is a 
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
 *      - A window will open showing your video feed and the individual
 *        edge colors
 *      - Exit the program at any time by pressing 'q' with any opened window in focus
 * 
 */

int main(){
            
    cv::Mat screen; // For the image matrix to allocate LEDs
    cv::VideoCapture capture; // Hardware for capturing video 0 is webcam, 1 and up is video capture

    int edgePixels = 60; // Amount of screen the edge peers into

    for(int i = 1; i <= 2; i++){ // Trying indices to see which is the video capture
        capture.open(i, cv::CAP_DSHOW);

        if(capture.isOpened()){
            cv::Mat testScreen;
            capture >> testScreen;
            
            if(!testScreen.empty()){ // If i is video capture, use i
                break;
            }   
        }
    }

    if(!capture.isOpened()){ // Ensures video capture is connected;
        std::cerr << "Video Capture Hardware failed to initialize";
        return -1;
    }

    while(1){ // Loop for video display

        capture >> screen; // Grab current display 
        if(screen.empty()){             
            std::cerr << "Video Capture did not grab what was on the screen";
            break;                   
        }

        int width = screen.cols;
        int height = screen.rows;

        // Divided number for both determines how many squares in that section

        int subwidth = width / 32;      
        int subheight = height / 32;    

        int lightSize = 40; // Size of each individual 'light'

        // One master window will open with four sides of 'lights' surrounding it //

        //  Here will be only the 'lights' which will border the main display

        cv::Mat dashboard(height + (lightSize * 2), 
                          width + (lightSize * 2), 
                          CV_8UC3, 
                          cv::Scalar(0,0,0));

        //  That main display is added here overlapping the border

        screen.copyTo(dashboard(cv::Rect(lightSize,lightSize,width,height)));


        // The loop here will serve to create each of the lights for the display

        for(int i = 0; i < 32; i++){ //  32 for each side

            int currentW = subwidth;
            int currentH = subheight;

            if(i == 31){
                currentW = width - (subwidth * i);
                currentH = height - (subheight * i);
            } 

            // Get the colors of the section of the screen

            cv::Mat leftSlice   = screen(cv::Rect(0, subheight * i, edgePixels, currentH));
            cv::Mat rightSlice  = screen(cv::Rect(width - edgePixels, subheight * i, edgePixels, currentH));
            cv::Mat topSlice    = screen(cv::Rect(subwidth * i, 0, currentW, edgePixels));
            cv::Mat bottomSlice = screen(cv::Rect(subwidth * i, height - edgePixels, currentW, edgePixels));

            // Place each of the colors onto the lights they belong to

            dashboard(cv::Rect(0, lightSize + (subheight * i), lightSize, currentH)).setTo(getVibrantMean(leftSlice)); // Left
            dashboard(cv::Rect(width + lightSize, lightSize + (subheight * i), lightSize, currentH)).setTo(getVibrantMean(rightSlice)); // Right
            dashboard(cv::Rect(lightSize + (subwidth * i), 0, currentW, lightSize)).setTo(getVibrantMean(topSlice)); // Top
            dashboard(cv::Rect(lightSize + (subwidth * i), height + lightSize, currentW, lightSize)).setTo(getVibrantMean(bottomSlice)); // Bottom
        }

        cv::imshow("Ambilight Command Center", dashboard);

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