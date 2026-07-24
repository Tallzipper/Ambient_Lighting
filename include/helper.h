#include <opencv2/opencv.hpp>
#include <iostream>

cv::Scalar getVibrantMean(const cv::Mat& slice){

    double totalWeight = 0;
    double weightBlue = 0;
    double weightRed = 0;
    double weightGreen = 0;

    // This will grab every relevant pixel for the calculation of the lights
    for(int row = 0; row < slice.rows; row++){
        const cv::Vec3b* pixel = slice.ptr<cv::Vec3b>(row); // Grabs a pixel

        for(int col = 0; col < slice.cols; col++){ // Values in each pixel
            unsigned char blue   = pixel[col][0];
            unsigned char green  = pixel[col][1];
            unsigned char red    = pixel[col][2];

            double brightness = (0.2 * red + .7 * green + 0.1 * blue) / 255.0; // Lower the multipliers to get duller colors

            unsigned char maxColor = std::max({red, green, blue});
            unsigned char minColor = std::min({red, green, blue});

            double saturation = 0;

            if(maxColor > 0){
                (maxColor - minColor)/maxColor;
            }

            double weight = brightness * saturation * saturation * saturation + 0.01;

            totalWeight = (weight) + totalWeight;

            weightBlue  = (blue * weight) + weightBlue;
            weightGreen = (green * weight) + weightGreen;
            weightRed   = (red * weight) + weightRed;

        }
    }
    if(totalWeight <= 0.01){
        return cv::Scalar(0,0,0);
    }
    else{
        return cv::Scalar(  weightBlue / totalWeight, 
                            weightGreen/ totalWeight, 
                            weightRed / totalWeight);
    }

}