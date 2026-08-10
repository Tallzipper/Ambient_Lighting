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
            unsigned char maxColor = max(red, max(green, blue));
            unsigned char minColor = min(red, min(green, blue));

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

#endif // HELPER_H