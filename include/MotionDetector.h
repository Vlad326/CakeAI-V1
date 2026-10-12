// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <opencv2/opencv.hpp>

// Класс для детектирования движения одного объекта с exclusion zone
class MotionDetector {
private:
    cv::Mat prevGray;
    bool firstFrame = true;
    int motionThreshold = 25;
    int minWidth = 20;
    int minHeight = 20;
    cv::Scalar motionColor = cv::Scalar(0, 255, 0); // Зеленый для движения
    
    // Параметры морфологических операций
    int morphKernelWidth = 5;
    int morphKernelHeight = 5;
    int dilationIterations = 2;
    int erosionIterations = 1;
    
    // Параметры Gaussian blur
    int blurKernelWidth = 5;
    int blurKernelHeight = 5;
    double blurSigma = 1.5;
    
public:
    // Обнаружение движения с exclusion zone
    cv::Point detectMotionWithExclusion(const cv::Mat& frame, 
                                       const cv::Point& exclusionCenter, 
                                       int exclusionRadius = 150);
    
    // Функция для вычисления смещенного центра exclusion zone
    cv::Point getOffsetExclusionCenter(const cv::Point& markerCenter, 
                                       double markerRotationDeg, 
                                       int offsetPixels = 20);
    
    // Отрисовка центра движения на кадре
    void drawMotionCenter(cv::Mat& frame, const cv::Point& center);
    void drawRealMotionCenter(cv::Mat& frame, const cv::Point& center);
     
    // Отрисовка exclusion zone
    void drawExclusionZone(cv::Mat& frame, const cv::Point& center, 
                          int radius = 150, const cv::Scalar& color = cv::Scalar(0, 0, 255));
    
    // Установка параметров
    void setThreshold(int thresh) { motionThreshold = thresh; }
    void setMinSize(int width, int height) { 
        minWidth = width; 
        minHeight = height; 
    }
    
    // Установка параметров морфологических операций
    void setMorphKernelSize(int width, int height) { 
        morphKernelWidth = width; 
        morphKernelHeight = height; 
    }
    void setDilationIterations(int iterations) { dilationIterations = iterations; }
    void setErosionIterations(int iterations) { erosionIterations = iterations; }
    
    // Установка параметров Gaussian blur
    void setBlurKernelSize(int width, int height) { 
        blurKernelWidth = width; 
        blurKernelHeight = height; 
    }
    void setBlurSigma(double sigma) { blurSigma = sigma; }
    
    // Получение параметров
    int getThreshold() const { return motionThreshold; }
    int getMorphKernelWidth() const { return morphKernelWidth; }
    int getMorphKernelHeight() const { return morphKernelHeight; }
    int getDilationIterations() const { return dilationIterations; }
    int getErosionIterations() const { return erosionIterations; }
    int getBlurKernelWidth() const { return blurKernelWidth; }
    int getBlurKernelHeight() const { return blurKernelHeight; }
    double getBlurSigma() const { return blurSigma; }
    int getMinWidth() const { return minWidth; }
    int getMinHeight() const { return minHeight; }
    
    // Сброс
    void reset() { 
        firstFrame = true;
        prevGray = cv::Mat();
    }
};

#endif // MOTIONDETECTOR_H