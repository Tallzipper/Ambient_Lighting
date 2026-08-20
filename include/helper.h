#ifndef HELPER_H
#define HELPER_H

#include <opencv2/opencv.hpp>
#include <windows.h>

// Gets the user's screen size
inline void getScreenSize(int& width, int& height) {
    width  = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
}

// Calculates the average color's brightness of an image slice
inline cv::Scalar getVibrantMean(const cv::Mat& slice) {
    if (slice.empty()) return cv::Scalar(0, 0, 0); // All black if no image

    double totalWeight = 0;
    double weightBlue  = 0;
    double weightRed   = 0;
    double weightGreen = 0;

    for (int row = 0; row < slice.rows; row++) {
        const cv::Vec3b* pixel = slice.ptr<cv::Vec3b>(row); // gets each color's value per pixel

        for (int col = 0; col < slice.cols; col++) { // for every color distribute its weight
            unsigned char blue  = pixel[col][0];
            unsigned char green = pixel[col][1];
            unsigned char red   = pixel[col][2];

            // Reason for lights found on official OPENCV doccumentation. Green is most visible/important color
            double brightness = (0.299 * red + 0.587 * green + 0.114 * blue) / 255.0;

            // Doesn't work with std:: will check why later
            unsigned char maxColor = std::max(red, std::max(green, blue));
            unsigned char minColor = std::min(red, std::min(green, blue));

            double saturation = 0;

            if (maxColor > 0) {
                saturation = static_cast<double>(maxColor - minColor) / maxColor;
            }

            // Forces brighter LEDs through saturation getting multiplied
            double weight = brightness * saturation * saturation + 0.01; 

            totalWeight += weight; // denominator to keep everything normalized

            // Numerators of their weight in the image

            weightBlue  += blue * weight;  
            weightGreen += green * weight;
            weightRed   += red * weight;
        }
    }

    if (totalWeight <= 0.01) { // guards division by 0
        return cv::Scalar(0, 0, 0);
    } 
    else{ // Returns a vector of the three pixel's average per BGR color
        return cv::Scalar(weightBlue  / totalWeight, 
                          weightGreen / totalWeight, 
                          weightRed   / totalWeight);
    }
}

// Formats LED colors into the Adalight frame and pushes to serial
void sendAdalightData(HANDLE hComm, const std::vector<cv::Vec3b>& leds) {
    if (hComm == INVALID_HANDLE_VALUE) return; // Ensures your hardware is still connected 

    uint16_t count = static_cast<uint16_t>(leds.size()); // gets LEDs to be read by 'framebuff'
    
    uint8_t topHalf  = (count - 1) >> 8;        // 00000000XXXXXXXX
    uint8_t bottomHalf  = (count - 1) & 0xFF;   // XXXXXXXX00000000
    uint8_t check = topHalf ^ bottomHalf ^ 0x55;  // checks for corruption

    std::vector<uint8_t> frameBuffer;
    frameBuffer.reserve(6 + count * 3);

    // 6-Byte Header framing
    frameBuffer.push_back('A');         //[0] R
    frameBuffer.push_back('d');         //[1] G
    frameBuffer.push_back('a');         //[2] B
    frameBuffer.push_back(topHalf);     //[3] top and bottom are LED count
    frameBuffer.push_back(bottomHalf);  //[4]
    frameBuffer.push_back(check);       //[5] checks the lights validity later, better drop than flash random color

    // Converts OpenCV BGR to Adalight's RGB Order
    for (const auto& color : leds) {
        frameBuffer.push_back(color[2]); // Red
        frameBuffer.push_back(color[1]); // Green
        frameBuffer.push_back(color[0]); // Blue
    }

    DWORD bytesWritten;
    
    WriteFile(hComm, frameBuffer.data(), static_cast<DWORD>(frameBuffer.size()), &bytesWritten, NULL); // Sends the data over
}

// https://aticleworld.com/serial-port-programming-using-win32-api/
// Windows Datatypes to note, for self
    // CreateFileA()- Opens a com port
    // DWORD        - Window's version of unsigned long
    // DCB          - Manipulates the open COM port
    // HANDLE       - For the file to be accessed with copiable attributes
    // Commitimeouts- 


// Initializes the port for the ESP32 which tells the lights how to act. 
HANDLE initSerial(const std::string& portName, DWORD baudRate) { 

    std::string paths = portName;

    // Fixes an error where the program would fail if the COM was above 10 since it 
    //  isn't one of the hardcoded inputs into the system

    if(paths.rfind("\\\\.\\", 0) != 0){ 
        paths = "\\\\.\\" + portName;
    }

    // Opens COM port device to read and write to
    HANDLE hComm = CreateFileA(
        paths.c_str(), 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL,
        OPEN_EXISTING, 
        0, 
        NULL
    );
    
    if (hComm == INVALID_HANDLE_VALUE) { // For when the port isn't able to be opened
        std::cerr << "Could not open " << portName << std::endl;
        return INVALID_HANDLE_VALUE; // Doesn't work with exceptions. This is windows based.
    }

    // Manipulates UART settings
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hComm, &dcbSerialParams)) { // Opened but unusable not responding
        CloseHandle(hComm);
        return INVALID_HANDLE_VALUE; 
    }


    dcbSerialParams.BaudRate = baudRate;

    dcbSerialParams.ByteSize = 8;           // Payload used by Uart
    dcbSerialParams.StopBits = ONESTOPBIT;  // End of byte uses 1 to signal finish 
    dcbSerialParams.Parity   = NOPARITY;    // Removes parity system to increase speed + better system in place

    if (!SetCommState(hComm, &dcbSerialParams)) { // If baudrate/byte size failed
        CloseHandle(hComm);
        return INVALID_HANDLE_VALUE;
    }

    // Prioritizes speed by returning immediately any available bytes
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = MAXDWORD; 
    timeouts.ReadTotalTimeoutConstant    = 0;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.WriteTotalTimeoutConstant   = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(hComm, &timeouts)) { // In case hardware fails during initialization
        CloseHandle(hComm);
        return INVALID_HANDLE_VALUE;
    }    

    // Checks to see if opened or not

    if (hComm == INVALID_HANDLE_VALUE) {
        std::cerr << "Continuing without physical LED output." << std::endl;
    } else {
        std::cout << "Connected to ESP32" << std::endl;
    }

    return hComm;
}

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

#endif // HELPER_H