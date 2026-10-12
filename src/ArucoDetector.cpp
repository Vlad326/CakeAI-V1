// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "ArucoDetector.h"
#include <chrono>
#include <iostream>
#include <cmath>

ArucoDetector::ArucoDetector() 
    : fps(0), frameCount(0) {
    
    lastTime = std::chrono::high_resolution_clock::now();
    
    // Инициализация словаря по умолчанию
    setDictionary(cv::aruco::DICT_4X4_50);
    
    // Установка параметров по умолчанию (быстрый режим)
    setParameters(true);
}

void ArucoDetector::setDictionary(int dictionaryType) {
    dictionary = cv::aruco::getPredefinedDictionary(dictionaryType);
    detector = cv::aruco::ArucoDetector(dictionary, params);
}

void ArucoDetector::setParameters(bool fastMode) {
    if (fastMode) {
        // БЫСТРЫЙ РЕЖИМ (оптимизирован для FPS)
        params.adaptiveThreshWinSizeMin = 5;
        params.adaptiveThreshWinSizeMax = 21;
        params.adaptiveThreshConstant = 7;
        params.minMarkerPerimeterRate = 0.05f;
        params.maxMarkerPerimeterRate = 0.8f;
        params.polygonalApproxAccuracyRate = 0.05f;
        params.cornerRefinementMethod = cv::aruco::CORNER_REFINE_NONE;
        params.errorCorrectionRate = 0.3f;
        params.minDistanceToBorder = 1;
        params.minOtsuStdDev = 3.0f;
        params.perspectiveRemovePixelPerCell = 6;
    } else {
        // ТОЧНЫЙ РЕЖИМ (лучшее качество, медленнее)
        params.adaptiveThreshWinSizeMin = 3;
        params.adaptiveThreshWinSizeMax = 23;
        params.adaptiveThreshConstant = 7;
        params.minMarkerPerimeterRate = 0.03f;
        params.maxMarkerPerimeterRate = 4.0f;
        params.polygonalApproxAccuracyRate = 0.03f;
        params.cornerRefinementMethod = cv::aruco::CORNER_REFINE_SUBPIX;
        params.cornerRefinementWinSize = 5;
        params.cornerRefinementMaxIterations = 30;
        params.cornerRefinementMinAccuracy = 0.1f;
        params.errorCorrectionRate = 0.6f;
        params.minDistanceToBorder = 3;
        params.minOtsuStdDev = 5.0f;
        params.perspectiveRemovePixelPerCell = 4;
    }
    
    // Обновляем детектор с новыми параметрами
    detector = cv::aruco::ArucoDetector(dictionary, params);
}

void ArucoDetector::setMinMarkerPerimeterRate(float value) {
    params.minMarkerPerimeterRate = value;
    detector = cv::aruco::ArucoDetector(dictionary, params);
    std::cout << "ArucoDetector: minMarkerPerimeterRate set to " << value << std::endl;
}

void ArucoDetector::setMaxMarkerPerimeterRate(float value) {
    params.maxMarkerPerimeterRate = value;
    detector = cv::aruco::ArucoDetector(dictionary, params);
    std::cout << "ArucoDetector: maxMarkerPerimeterRate set to " << value << std::endl;
}

float ArucoDetector::getMinMarkerPerimeterRate() const {
    return params.minMarkerPerimeterRate;
}

float ArucoDetector::getMaxMarkerPerimeterRate() const {
    return params.maxMarkerPerimeterRate;
}

void ArucoDetector::processFrame(cv::Mat& frame) {
    // Конвертация в grayscale
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Временные векторы для всех найденных маркеров
    std::vector<int> allIds;
    std::vector<std::vector<cv::Point2f>> allCorners;
    
    // Детекция всех маркеров
    detector.detectMarkers(gray, allCorners, allIds, rejected);
    
    // Фильтруем только ID 0 и 1
    markerIds.clear();
    markerCorners.clear();
    
    for (size_t i = 0; i < allIds.size(); i++) {
        if (allIds[i] == 0 || allIds[i] == 1) {
            markerIds.push_back(allIds[i]);
            markerCorners.push_back(allCorners[i]);
        }
    }
    
    // Расчет FPS
    calculateFPS();
}

void ArucoDetector::calculateFPS() {
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - lastTime).count();
    
    if (elapsed >= 1000) {
        fps = frameCount * 1000.0 / elapsed;
        frameCount = 0;
        lastTime = currentTime;
    }
}

void ArucoDetector::drawResults(cv::Mat& frame) {
    // Рисуем обнаруженные маркеры
    if (!markerIds.empty()) {
        cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
        drawMarkerInfo(frame);
    }
}

void ArucoDetector::drawMarkerInfo(cv::Mat& frame) {
    for (size_t i = 0; i < markerIds.size(); i++) {
        const auto& corners = markerCorners[i];
        cv::Point2f center = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;
        
        // Рисуем крест в центре маркера
        cv::drawMarker(frame, center, cv::Scalar(0, 255, 255), 
                      cv::MARKER_CROSS, 20, 2);
        
        // Отображаем ID маркера
        cv::putText(frame, std::to_string(markerIds[i]),
                  cv::Point(center.x + 15, center.y - 15),
                  cv::FONT_HERSHEY_SIMPLEX, 0.6,
                  cv::Scalar(255, 0, 255), 2);
    }
}

double ArucoDetector::getFPS() const {
    return fps;
}

int ArucoDetector::getMarkerCount() const {
    return markerIds.size();
}

// Получение центра маркера
cv::Point2f ArucoDetector::getMarkerCenter(int index) const {
    if (markerIds.empty() || index >= static_cast<int>(markerIds.size())) {
        return cv::Point2f(-1, -1);
    }
    
    const auto& corners = markerCorners[index];
    cv::Point2f center = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;
    return center;
}

// Получение угла поворота маркера (в диапазоне [0, 360))
double ArucoDetector::getMarkerRotation(int index) const {
    if (markerIds.empty() || index >= static_cast<int>(markerIds.size())) {
        return 0.0;
    }
    
    const auto& corners = markerCorners[index];
    
    // Углы маркера в порядке: top-left, top-right, bottom-right, bottom-left
    cv::Point2f tl = corners[0];  // top-left
    cv::Point2f tr = corners[1];  // top-right
    
    // Вычисляем вектор между верхними точками
    double dx = tr.x - tl.x;
    double dy = tr.y - tl.y;
    
    // Вычисляем угол в радианах
    double angle_rad = atan2(dy, dx);
    
    // Конвертируем в градусы
    double angle_deg = angle_rad * 180.0 / CV_PI;
    
    // Добавляем +90 градусов (как было в вашем исходном коде)
    angle_deg += 90.0;
    
    // Нормализуем угол к диапазону [0, 360)
    angle_deg = fmod(angle_deg, 360.0);
    if (angle_deg < 0) {
        angle_deg += 360.0;
    }
    
    return angle_deg;
}

// Получение центра и угла поворота одновременно
cv::Point2f ArucoDetector::getMarkerCenterAndRotation(int index, double* rotation) const {
    cv::Point2f center(-1, -1);
    double rot = 0.0;
    
    if (!markerIds.empty() && index < static_cast<int>(markerIds.size())) {
        const auto& corners = markerCorners[index];
        
        // Центр маркера
        center = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;
        
        // Угол поворота
        cv::Point2f tl = corners[0];
        cv::Point2f tr = corners[1];
        
        double dx = tr.x - tl.x;
        double dy = tr.y - tl.y;
        double angle_rad = atan2(dy, dx);
        rot = angle_rad * 180.0 / CV_PI;
        
        // Добавляем +90 градусов
        rot += 90.0;
        
        // Нормализуем угол к диапазону [0, 360)
        rot = fmod(rot, 360.0);
        if (rot < 0) {
            rot += 360.0;
        }
    }
    
    if (rotation) {
        *rotation = rot;
    }
    
    return center;
}

// Метод генерации маркера
void ArucoDetector::generateMarker(int id, int size, const std::string& filename) {
    cv::Mat marker;
    cv::aruco::generateImageMarker(dictionary, id, size, marker, 1);
    cv::imwrite(filename, marker);
    std::cout << "Generated marker: " << filename << std::endl;
}

// Статический метод генерации маркера
void ArucoDetector::generateMarker(int dictionaryType, int id, int size, const std::string& filename) {
    cv::aruco::Dictionary dict = cv::aruco::getPredefinedDictionary(dictionaryType);
    cv::Mat marker;
    cv::aruco::generateImageMarker(dict, id, size, marker, 1);
    cv::imwrite(filename, marker);
    std::cout << "Generated marker: " << filename << " with dictionary type " << dictionaryType << std::endl;
}