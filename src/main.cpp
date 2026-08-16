#include <opencv2/opencv.hpp>
#include <iostream>

#include <algorithm> // Meant for min and max functions
#include <cmath>

#include <thread> // Allows the program to run two different tasks at the same time
#include <atomic> // Prevents data corruption 
#include <mutex>  // Compliment to thread, makes sure that the 'threads' don't interact

#include <windows.h>

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

    HANDLE hSerial = initSerial("\\\\.\\COM6", 115200);


    bool borders = true; // Determines whether the LEDs on the border is shown or not

    int screenWidth  = 0;
    int screenHeight = 0;

    getScreenSize(screenWidth, screenHeight);
            
    cv::Mat screen; // For the image matrix to allocate LEDs
    cv::VideoCapture capture; // Hardware for capturing video 0 is webcam, 1 and up is video capture

    int edgePixels = 60; // Amount of screen the edge peers into

    for(int i = 0; i <= 2; i++){ // Trying indices to see which is the video capture
        capture.open(i, cv::CAP_DSHOW);

        if(capture.isOpened()){
            capture.set(cv::CAP_PROP_FRAME_WIDTH, 1920); // This has a faster frame rate at cost to quality
            capture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
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

    int lightSize = 40; // Size of each individual 'light'

    // for the inside of the LED frame
    int innerWidth = screenWidth - (lightSize * 2);
    int innerHeight = screenHeight - (lightSize * 2);

    // Divided number for both determines how many squares in that section
    int subwidth = innerWidth / 32;      
    int subheight = innerHeight / 32;   

    // This is the entire screen (non zoomed in)
    cv::Mat dashboard(  screenHeight, 
                        screenWidth, 
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

    // Inside of the LED frame's feed
    cv::Mat innerDisplay;

    std::vector<cv::Vec3b> adalightPayload(75, cv::Vec3b(0, 0, 0)); // FIX Hardcoded for my system. Will make const

    while(1){ // Loop for video display

        {
            std::lock_guard<std::mutex> lock(mutexFrame); // Ensure no changes while grabbing
            if(!sharedFrame.empty()){ // If frame exists, use it
                sharedFrame.copyTo(screen);
            }
        }
        
        // Makes the quality of the display better
        cv::resize(screen, compressedScreen, cv::Size(compressedWidth, compressedHeight), 0, 0, cv::INTER_LINEAR);
        cv::GaussianBlur(compressedScreen, compressedScreen, cv::Size(5, 5), 0);

        // calculates the compressed edge so it better fits the screen
        int compressedEdge = edgePixels * compressedScreen.cols / width;
        if(compressedEdge < 1) compressedEdge = 1; // Guard


        if(screen.empty()){             
            std::cerr << "Video Capture did not grab what was on the screen";
            break;                   
        }

        if(borders){ // Show border

            dashboard.setTo(cv::Scalar(0, 0, 0)); // Clear canvas
            
            // That main display is added here within the border
            cv::resize(screen, innerDisplay, cv::Size(innerWidth, innerHeight));
            innerDisplay.copyTo(dashboard(cv::Rect(lightSize, lightSize, innerWidth, innerHeight)));

            // Store edge slice colors to map onto physical 75 LEDs
            std::vector<cv::Vec3b> leftSlices(32), topSlices(32), rightSlices(32), bottomSlices(32);

            // The loop here will serve to create each of the lights for the display

            for(int i = 0; i < 32; i++){ //  32 for each side

                int currentW = subwidth;

                int compressedCurrentW = compressedSubwidth; // These names are getting too long
                int compressedCurrentH = compressedSubheight;

                // Edge case
                if(i == 31){
                    currentW = innerWidth - (subwidth * i);

                    compressedCurrentW = compressedWidth - (compressedSubwidth * i);
                    compressedCurrentH = compressedHeight - (compressedSubheight * i);
                } 

                // Height callibration

                int currentH = 0;
                int compH = 0;

                if(i == 31){
                    currentH = innerHeight - (subheight * i);
                    compH = compressedHeight - (compressedSubheight * i);
                }
                else{
                    currentH = subheight;
                    compH = compressedSubheight;
                }

                int comp_y_start = compressedSubheight * i;
                
                // Get the colors of the section of the screen

                cv::Mat leftSlice   = compressedScreen(cv::Rect(0, comp_y_start, compressedEdge, compH));
                cv::Mat rightSlice  = compressedScreen(cv::Rect(compressedWidth - compressedEdge, comp_y_start, compressedEdge, compH));
                cv::Mat topSlice    = compressedScreen(cv::Rect(compressedSubwidth * i, 0, compressedCurrentW, compressedEdge));
                cv::Mat bottomSlice = compressedScreen(cv::Rect(compressedSubwidth * i, compressedHeight - compressedEdge, compressedCurrentW, compressedEdge));

                cv::Scalar sLeft   = getVibrantMean(leftSlice);
                cv::Scalar sRight  = getVibrantMean(rightSlice);
                cv::Scalar sTop    = getVibrantMean(topSlice);
                cv::Scalar sBottom = getVibrantMean(bottomSlice);

                cv::Vec3b cLeft(  static_cast<uchar>(sLeft[0]),   static_cast<uchar>(sLeft[1]),   static_cast<uchar>(sLeft[2]));
                cv::Vec3b cRight( static_cast<uchar>(sRight[0]),  static_cast<uchar>(sRight[1]),  static_cast<uchar>(sRight[2]));
                cv::Vec3b cTop(    static_cast<uchar>(sTop[0]),    static_cast<uchar>(sTop[1]),    static_cast<uchar>(sTop[2]));
                cv::Vec3b cBottom( static_cast<uchar>(sBottom[0]), static_cast<uchar>(sBottom[1]), static_cast<uchar>(sBottom[2]));

                // Store slice colors for 75-LED mapping
                leftSlices[i]   = cLeft;
                rightSlices[i]  = cRight;
                topSlices[i]    = cTop;
                bottomSlices[i] = cBottom;

                // Place each of the colors onto the lights they belong to

                dashboard(cv::Rect(0, lightSize + (subheight * i), lightSize, currentH)).setTo(cLeft); // Left
                dashboard(cv::Rect(screenWidth - lightSize, lightSize + (subheight * i), lightSize, currentH)).setTo(cRight); // Right
                dashboard(cv::Rect(lightSize + (subwidth * i), 0, currentW, lightSize)).setTo(cTop); // Top
                dashboard(cv::Rect(lightSize + (subwidth * i), screenHeight - lightSize, currentW, lightSize)).setTo(cBottom); // Bottom
            }
            
            // Clear entire array first (ensures skipped LEDs 0-4, 14-18, 38-42, 51-56 stay off)
            std::fill(adalightPayload.begin(), adalightPayload.end(), cv::Vec3b(0, 0, 0));

            // ONLY WORKS ON MY COMPUTER.

            // Segment 0 (Right Edge): LEDs 5 – 13 (9 LEDs, Bottom -> Top)
            for (int k = 0; k < 9; ++k) {
                float rawIdx = (8.0f - k) * 31.0f / 8.0f;
                int idx = min(31, max(0, static_cast<int>(std::round(rawIdx))));
                adalightPayload[5 + k] = rightSlices[idx];
            }

            // Segment 1 (Top Edge): LEDs 19 – 37 (19 LEDs, Right -> Left)
            for (int k = 0; k < 19; ++k) {
                float rawIdx = 31.0f - (k * 31.0f / 18.0f);
                int idx = min(31, max(0, static_cast<int>(std::round(rawIdx))));
                adalightPayload[19 + k] = topSlices[idx];
            }

            // Segment 2 (Left Edge): LEDs 43 – 50 (8 LEDs, Top -> Bottom)
            for (int k = 0; k < 8; ++k) {
                float rawIdx = k * 31.0f / 7.0f;
                int idx = min(31, max(0, static_cast<int>(std::round(rawIdx))));
                adalightPayload[43 + k] = leftSlices[idx];
            }

            // Segment 3 (Bottom Edge): LEDs 57 – 74 (18 LEDs, Left -> Right)
            for (int k = 0; k < 18; ++k) {
                float rawIdx = k * 31.0f / 17.0f;
                int idx = min(31, max(0, static_cast<int>(std::round(rawIdx))));
                adalightPayload[57 + k] = bottomSlices[idx];
            }

            sendAdalightData(hSerial, adalightPayload);

            cv::imshow("Ambilight Command Center", dashboard);
        }
        else{ // No pixels showing
            cv::Mat rawScaled;
            cv::resize(screen, rawScaled, cv::Size(screenWidth, screenHeight));
            cv::imshow("Ambilight Command Center", rawScaled);
        }

        // Press q to escape and escape to toggle the borders

        int waitKey = cv::waitKey(1);

        if (waitKey == 'q' || waitKey == 'Q'){
            break;
        }
        else if(waitKey == 27){
            borders = !borders;

        }

    }

    // When escaped, stop recording, kill windows, and exit program

    inMotion = false;
    if(backgroundProcess.joinable()){ // If task is running, pause it
        backgroundProcess.join();
    }

    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
    }

    capture.release();
    cv::destroyAllWindows();
    
    return 0;

}