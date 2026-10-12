// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "MotionDetector.h"
#include <cmath>
#include <iostream>  // Для отладки

cv::Point MotionDetector::detectMotionWithExclusion(const cv::Mat& frame, 
                                                   const cv::Point& exclusionCenter, 
                                                   int exclusionRadius) {
    cv::Point center(-1, -1);
    
    if (frame.empty()) return center;
    
    // Преобразуем в оттенки серого
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Применяем размытие с настраиваемыми параметрами
    cv::GaussianBlur(gray, gray, cv::Size(blurKernelWidth, blurKernelHeight), blurSigma);
    
    // Инициализация предыдущего кадра
    if (firstFrame || prevGray.size() != gray.size()) {
        // Если это первый кадр ИЛИ размер кадра изменился, сбрасываем предыдущий кадр
        gray.copyTo(prevGray);
        firstFrame = false;
        return center;
    }
    
    // Вычисление абсолютной разницы
    cv::Mat frameDiff;
    try {
        cv::absdiff(prevGray, gray, frameDiff);
    } catch (const cv::Exception& e) {
        // Если возникает ошибка, сбрасываем предыдущий кадр
        std::cerr << "Error in absdiff: " << e.what() << std::endl;
        std::cerr << "prevGray size: " << prevGray.size() << ", gray size: " << gray.size() << std::endl;
        gray.copyTo(prevGray);
        return center;
    }
    
    // Применение порогового значения
    cv::Mat thresh;
    cv::threshold(frameDiff, thresh, motionThreshold, 255, cv::THRESH_BINARY);
    
    // Морфологические операции для улучшения маски с настраиваемыми параметрами
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, 
                                              cv::Size(morphKernelWidth, morphKernelHeight));
    cv::dilate(thresh, thresh, kernel, cv::Point(-1, -1), dilationIterations);
    cv::erode(thresh, thresh, kernel, cv::Point(-1, -1), erosionIterations);
    
    // Применяем exclusion zone если задан центр
    if (exclusionCenter.x >= 0 && exclusionCenter.y >= 0) {
        cv::Mat exclusionMask = cv::Mat::zeros(thresh.size(), thresh.type());
        cv::circle(exclusionMask, exclusionCenter, exclusionRadius, cv::Scalar(255), -1);
        // Обнуляем пиксели в exclusion zone
        thresh.setTo(0, exclusionMask);
    }
    
    // Находим контуры
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Создаем маску для больших объектов
    cv::Mat largeObjectsMask = cv::Mat::zeros(thresh.size(), thresh.type());
    
    // Фильтрация контуров по размеру
    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        if (rect.width >= minWidth && rect.height >= minHeight) {
            cv::drawContours(largeObjectsMask, std::vector<std::vector<cv::Point>>{contour}, 
                            -1, cv::Scalar(255), -1);
        }
    }
    
    // Находим координаты больших движущихся пикселей
    cv::Mat locations;
    cv::findNonZero(largeObjectsMask, locations);
    
    // Если есть большие движущиеся объекты, вычисляем их центр масс
    if (locations.total() > 0) {
        cv::Scalar mean = cv::mean(locations);
        center = cv::Point(static_cast<int>(mean[0]), static_cast<int>(mean[1]));
    }
    
    // Обновляем предыдущий кадр
    gray.copyTo(prevGray);
    
    return center;
}

cv::Point MotionDetector::getOffsetExclusionCenter(const cv::Point& markerCenter, 
                                                  double markerRotationDeg, 
                                                  int offsetPixels) {
    // Преобразуем градусы в радианы
    double rotationRad = markerRotationDeg * CV_PI / 180.0;
    
    // Вычисляем смещение по осям X и Y
    int offsetX = static_cast<int>(offsetPixels * sin(rotationRad));
    int offsetY = static_cast<int>(-offsetPixels * cos(rotationRad)); // минус потому что Y направлен вниз
    
    // Вычисляем новый центр
    cv::Point offsetCenter(
        markerCenter.x + offsetX,
        markerCenter.y + offsetY
    );
    
    return offsetCenter;
}

void MotionDetector::drawMotionCenter(cv::Mat& frame, const cv::Point& center) {
    if (center.x < 0 || center.y < 0) return;
    
    // Рисуем крест в центре движения
    int crossSize = 20;
    cv::line(frame, 
             cv::Point(center.x - crossSize, center.y), 
             cv::Point(center.x + crossSize, center.y), 
             motionColor, 3);
    cv::line(frame, 
             cv::Point(center.x, center.y - crossSize), 
             cv::Point(center.x, center.y + crossSize), 
             motionColor, 3);
    
    // Рисуем окружность
    cv::circle(frame, center, 15, motionColor, 2);
    
    // Отображаем координаты центра
    std::string coordText = "(" + std::to_string(center.x) + ", " + 
                            std::to_string(center.y) + ")";
    cv::putText(frame, coordText, 
               cv::Point(center.x + 20, center.y - 20),
               cv::FONT_HERSHEY_SIMPLEX, 0.6, motionColor, 2);
}

void MotionDetector::drawRealMotionCenter(cv::Mat& frame, const cv::Point& center) {
    if (center.x < 0 || center.y < 0) return;
    
    // Рисуем крест в центре движения
    int crossSize = 20;
    cv::line(frame, 
             cv::Point(center.x - crossSize, center.y), 
             cv::Point(center.x + crossSize, center.y), 
             motionColor, 3);
    cv::line(frame, 
             cv::Point(center.x, center.y - crossSize), 
             cv::Point(center.x, center.y + crossSize), 
             motionColor, 3);
    
    // Рисуем окружность
    cv::circle(frame, center, 15, motionColor, 2);
    
    // Отображаем координаты центра
    std::string coordText = "(" + std::to_string(center.x) + ", " + 
                            std::to_string(center.y) + ")";
    cv::putText(frame, coordText, 
               cv::Point(center.x + 20, center.y - 20),
               cv::FONT_HERSHEY_SIMPLEX, 0.6, motionColor, 2);
}


void MotionDetector::drawExclusionZone(cv::Mat& frame, const cv::Point& center, 
                                      int radius, const cv::Scalar& color) {
    if (center.x < 0 || center.y < 0) return;
    
    // Рисуем окружность exclusion zone
    cv::circle(frame, center, radius, color, 2);
    
    // Рисуем крест в центре exclusion zone
    int crossSize = 10;
    cv::line(frame, 
             cv::Point(center.x - crossSize, center.y), 
             cv::Point(center.x + crossSize, center.y), 
             color, 2);
    cv::line(frame, 
             cv::Point(center.x, center.y - crossSize), 
             cv::Point(center.x, center.y + crossSize), 
             color, 2);
    
    // Отображаем текст
    std::string text = "Exclusion Zone";
    cv::putText(frame, text, 
               cv::Point(center.x - 40, center.y - radius - 10),
               cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
}