#include <opencv2/opencv.hpp>
#include <iostream>

#include <thread> // Allows the program to run two different tasks at the same time
#include <atomic> // Prevents data corruption 
#include <mutex>  // Compliment to thread, makes sure that the 'threads' don't interact

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

 std::atomic<bool> inMotion(true); // Determines if it's safe to change/access variables
 cv::Mat sharedFrame; // communication of main program and capture thread to access/update each other
 std::mutex mutexFrame; // keeps sharedFrame from getting corrupted

 void captureThread(cv::VideoCapture* cap){ // Reduces delay in processing

    cv::Mat tempFrame; // Keeps a copy of the frame in case the original is changed 

    // While not being used by the other processor get a copy of the frame safely 

    while(inMotion){ 

        if(cap -> grab()){
            cap->retrieve(tempFrame);
            if(!tempFrame.empty()){
                std::lock_guard<std::mutex> lock(mutexFrame);
                tempFrame.copyTo(sharedFrame);
            }
        }

    }

 }

int main(){

    int screenWidth  = 0;
    int screenHeight = 0;

    getScreenSize(screenWidth, screenHeight);
            
    cv::Mat screen; // For the image matrix to allocate LEDs
    cv::VideoCapture capture; // Hardware for capturing video 0 is webcam, 1 and up is video capture

    int edgePixels = 60; // Amount of screen the edge peers into

    for(int i = 1; i <= 2; i++){ // Trying indices to see which is the video capture
        capture.open(i, cv::CAP_DSHOW);

        if(capture.isOpened()){
            capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280); // This has a faster frame rate at cost to quality
            capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
            capture.set(cv::CAP_PROP_FPS, 60);
            capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
            capture.set(cv::CAP_PROP_BUFFERSIZE, 1); // Copied frames removed
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

    // Sets up the dashboard once so doesn't need to keep being recreated

    capture >> screen;

    // One master window will open with four sides of 'lights' surrounding it //
    // Here will be only the 'lights' which will border the main display
    int width = screen.cols;
    int height = screen.rows;

    // Divided number for both determines how many squares in that section

    int subwidth = width / 32;      
    int subheight = height / 32;    

    int lightSize = 40; // Size of each individual 'light'

    // This is the entire screen (non zoomed in)
    cv::Mat dashboard(height + (lightSize * 2), 
                        width + (lightSize * 2), 
                        CV_8UC3, 
                        cv::Scalar(0,0,0));

    // The process of creating a window that full screens
    // create the name, have the window open, set the window to its size, have the window created
    std::string windowName = "Ambilight Command Center";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::setWindowProperty(windowName, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    cv::Mat fullScreen; // For display to user

    std::thread backgroundProcess(captureThread, &capture);

    // Downscaling to make easier to calculate border pixels
    cv::Mat compressedScreen;

    // 320 x 180 compression at 10 x 5 pixels reaching 10 pixels in
    int compressedWidth  = 320;
    int compressedHeight = 180;
    int compressedSubwidth = compressedWidth / 32;
    int compressedSubheight = compressedHeight / 32;

    while(1){ // Loop for video display

        {
            std::lock_guard<std::mutex> lock(mutexFrame); // Ensure no changes while grabbing
            if(!sharedFrame.empty()){ // If frame exists, use it
                sharedFrame.copyTo(screen);
            }
        }
        cv::resize(screen, compressedScreen, cv::Size(compressedWidth, compressedHeight), 0, 0, cv::INTER_NEAREST);

        // calculates the compressed edge so it better fits the screen
        int compressedEdge = edgePixels * compressedScreen.cols / width;
        if(compressedEdge < 1) compressedEdge = 1; // Guard


        if(screen.empty()){             
            std::cerr << "Video Capture did not grab what was on the screen";
            break;                   
        }

        //  That main display is added here overlapping the border

        
        screen.copyTo(dashboard(cv::Rect(lightSize,lightSize,width,height)));


        // The loop here will serve to create each of the lights for the display

        for(int i = 0; i < 32; i++){ //  32 for each side

            int currentW = subwidth;

            int compressedCurrentW = compressedSubwidth; // These names are getting too long
            int compressedCurrentH = compressedSubheight;

            if(i == 31){
                currentW = width - (subwidth * i);

                compressedCurrentW = compressedWidth - (compressedSubwidth * i);
                compressedCurrentH = compressedHeight - (compressedSubheight * i);
            } 

            // Height callibration

            int y_start  = (height * i) / 32;
            int y_end    = (height * (i + 1)) / 32;
            int currentH = y_end - y_start;

            int comp_y_start = (compressedHeight * i) / 32;
            int comp_y_end   = (compressedHeight * (i + 1)) / 32;
            int compH        = comp_y_end - comp_y_start;      

            // Get the colors of the section of the screen

            cv::Mat leftSlice  = compressedScreen(cv::Rect(0, comp_y_start, compressedEdge, compH));
            cv::Mat rightSlice = compressedScreen(cv::Rect(compressedWidth - compressedEdge, comp_y_start, compressedEdge, compH));
            cv::Mat topSlice    = compressedScreen(cv::Rect(compressedSubwidth * i, 0, compressedCurrentW, compressedEdge));
            cv::Mat bottomSlice = compressedScreen(cv::Rect(compressedSubwidth * i, compressedHeight - compressedEdge, compressedCurrentW, compressedEdge));

            // Place each of the colors onto the lights they belong to

            dashboard(cv::Rect(0, lightSize + (subheight * i), lightSize, currentH)).setTo(getVibrantMean(leftSlice)); // Left
            dashboard(cv::Rect(width + lightSize, lightSize + (subheight * i), lightSize, currentH)).setTo(getVibrantMean(rightSlice)); // Right
            dashboard(cv::Rect(lightSize + (subwidth * i), 0, currentW, lightSize)).setTo(getVibrantMean(topSlice)); // Top
            dashboard(cv::Rect(lightSize + (subwidth * i), height + lightSize, currentW, lightSize)).setTo(getVibrantMean(bottomSlice)); // Bottom
        }

        cv::resize(dashboard, fullScreen, cv::Size(screenWidth, screenHeight), 0, 0, cv::INTER_LINEAR);

        cv::imshow("Ambilight Command Center", fullScreen);

        // Press q to escape

        if (cv::waitKey(1) == 'q'){
            break;
        }

    }

    // When escaped, stop recording, kill windows, and exit program

    inMotion = false;
    if(backgroundProcess.joinable()){ // If task is running, pause it
        backgroundProcess.join();
    }

    capture.release();
    cv::destroyAllWindows();
    
    return 0;

}