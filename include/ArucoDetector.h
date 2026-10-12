// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef ARUCODETECTOR_H
#define ARUCODETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <vector>
#include <memory>
#include <string>

class ArucoDetector {
private:
    cv::aruco::ArucoDetector detector;
    cv::aruco::Dictionary dictionary;
    cv::aruco::DetectorParameters params;
    
    // Статистика
    double fps;
    int frameCount;
    std::chrono::high_resolution_clock::time_point lastTime;
    
    // Переменные для детекции
    cv::Mat gray;
    std::vector<std::vector<cv::Point2f>> rejected;
    
public:
    // Публичные переменные для доступа из main
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;
    
    // Конструктор
    ArucoDetector();
    
    // Основные методы
    void processFrame(cv::Mat& frame);
    void drawResults(cv::Mat& frame);
    double getFPS() const;
    int getMarkerCount() const;
    
    // Настройки
    void setDictionary(int dictionaryType = cv::aruco::DICT_4X4_50);
    void setParameters(bool fastMode = true);
    
    // Утилиты
    void generateMarker(int id, int size, const std::string& filename);
    
    // Методы для настройки параметров
    void setMinMarkerPerimeterRate(float value);
    void setMaxMarkerPerimeterRate(float value);
    float getMinMarkerPerimeterRate() const;
    float getMaxMarkerPerimeterRate() const;
    
    // Статические утилиты (не требуют экземпляра класса)
    static void generateMarker(int dictionaryType, int id, int size, const std::string& filename);
    
    // Методы доступа к данным о маркерах
    bool hasMarkers() const { return !markerIds.empty(); }
    const std::vector<int>& getMarkerIds() const { return markerIds; }
    const std::vector<std::vector<cv::Point2f>>& getMarkerCorners() const { return markerCorners; }
    
    // Получение информации о конкретном маркере
    cv::Point2f getMarkerCenter(int index = 0) const;
    double getMarkerRotation(int index = 0) const;
    cv::Point2f getMarkerCenterAndRotation(int index = 0, double* rotation = nullptr) const;
    
private:
    void calculateFPS();
    void drawMarkerInfo(cv::Mat& frame);
};

#endif // ARUCODETECTOR_H