// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "ArucoDetector.h"
#include "MotionDetector.h"
#include "RobotController.h"
#include "ArduinoController.h"
#include "ControlPanel.h"
#include "ConfigManager.h"

#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <iomanip>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <fstream>
#include <memory>  // Для shared_ptr
#include <algorithm>

extern std::atomic<int> offsetPixels;

double directionToPointDegrees(double x1, double y1, double x2, double y2) {
    double angleRad = atan2(y2 - y1, x2 - x1);
    return angleRad * 180.0 / 3.141592653589;
}

// Глобальные переменные для тестового режима
std::atomic<int> testMotionPointX{-1};
std::atomic<int> testMotionPointY{-1};
std::atomic<bool> testModeActive{false};
std::mutex testModeMutex;

// Глобальные переменные для арены
std::atomic<int> arenaCenterX{640};  // По умолчанию 640
std::atomic<int> arenaCenterY{512};  // По умолчанию 512
std::atomic<bool> arenaCenterSelectMode{false};

std::atomic<int> arenaSizeX1{-1};
std::atomic<int> arenaSizeY1{-1};
std::atomic<int> arenaSizeX2{-1};
std::atomic<int> arenaSizeY2{-1};
std::atomic<bool> arenaSizeSelectMode{false};
std::vector<cv::Point> arenaSizePoints; // Вектор для хранения точек выбора размера арены

struct SavedMarkerInfo {
    cv::Point position;
    double rotation;
    std::chrono::steady_clock::time_point detectionTime;
    bool hasMarker;
};

std::atomic<bool> savedMarkerHasMarker{false};
std::atomic<int> savedMarkerX{-1};
std::atomic<int> savedMarkerY{-1};
std::atomic<double> savedMarkerRotation{0.0};
std::chrono::steady_clock::time_point lastMarkerDetectionTime;
std::mutex markerPersistenceMutex;
const int MARKER_PERSISTENCE_MS = 3000;  // 3 секунды

// Указатель на ControlPanel для доступа из mouseCallback
static ControlPanel* g_controlPanel = nullptr;

// НОВАЯ ПЕРЕМЕННАЯ: коэффициент масштабирования экрана
std::atomic<float> screenSizeFactor{0.75f};

// Helper функция для получения точки
cv::Point getTestMotionPoint() {
    return cv::Point(testMotionPointX.load(), testMotionPointY.load());
}

// Helper функция для установки точки
void setTestMotionPoint(const cv::Point& point) {
    testMotionPointX.store(point.x);
    testMotionPointY.store(point.y);
}

void updateSavedMarkerInfo(bool hasMarker, cv::Point markerCenter, double rotation) {
    std::lock_guard<std::mutex> lock(markerPersistenceMutex);
    
    if (hasMarker && markerCenter.x >= 0) {
        // Сохраняем текущую информацию
        savedMarkerX.store(markerCenter.x);
        savedMarkerY.store(markerCenter.y);
        savedMarkerRotation.store(rotation);
        savedMarkerHasMarker.store(true);
        lastMarkerDetectionTime = std::chrono::steady_clock::now();
    }
    // Если маркер не обнаружен, не обновляем ничего
    // Проверка на истечение времени будет в getMarkerWithPersistence
}

bool getMarkerWithPersistence(cv::Point& markerCenter, double& rotation) {
    std::lock_guard<std::mutex> lock(markerPersistenceMutex);
    
    if (!savedMarkerHasMarker.load()) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastMarkerDetectionTime).count();
    
    if (elapsed < MARKER_PERSISTENCE_MS) {
        markerCenter.x = savedMarkerX.load();
        markerCenter.y = savedMarkerY.load();
        rotation = savedMarkerRotation.load();
        return true;
    } else {
        // Время истекло, сбрасываем информацию
        savedMarkerX.store(-1);
        savedMarkerY.store(-1);
        savedMarkerRotation.store(0.0);
        savedMarkerHasMarker.store(false);
        return false;
    }
}

// Callback для мыши (ОБНОВЛЕН для учета screenSizeFactor)
void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    float factor = screenSizeFactor.load();
    
    // Если factor не равен 1.0, корректируем координаты мыши
    if (factor != 1.0f) {
        x = static_cast<int>(x / factor);
        y = static_cast<int>(y / factor);
    }
    
    if (event == cv::EVENT_LBUTTONDOWN) {
        if (testModeActive.load()) {
            setTestMotionPoint(cv::Point(x, y));
            std::cout << "Test point set to: (" << x << ", " << y << ")" << std::endl;
        }
        else if (arenaCenterSelectMode.load()) {
            // Корректируем координаты с учетом обрезки
            int x1 = arenaSizeX1.load();
            int y1 = arenaSizeY1.load();
            
            int absoluteX = x + x1;
            int absoluteY = y + y1;
            
            arenaCenterX.store(absoluteX);
            arenaCenterY.store(absoluteY);
            
            if (g_controlPanel != nullptr) {
                g_controlPanel->setArenaXValue(std::to_string(absoluteX));
                g_controlPanel->setArenaYValue(std::to_string(absoluteY));
            }
            std::cout << "Arena center set to: (" << absoluteX << ", " << absoluteY << ")" << std::endl;
            std::cout << "Display coordinates (scaled): (" << x << ", " << y << ")" << std::endl;
        }
        else if (arenaSizeSelectMode.load()) {
            // Режим выбора размера арены
            int x1_val = arenaSizeX1.load();
            int y1_val = arenaSizeY1.load();
            
            if (arenaSizePoints.size() < 2) {
                // Корректируем координаты с учетом обрезки
                int absoluteX = x + x1_val;
                int absoluteY = y + y1_val;
                
                arenaSizePoints.push_back(cv::Point(absoluteX, absoluteY));
                std::cout << "Arena size point " << arenaSizePoints.size() << " set to: (" 
                          << absoluteX << ", " << absoluteY << ")" << std::endl;
                std::cout << "Display coordinates (scaled): (" << x << ", " << y << ")" << std::endl;
                
                if (arenaSizePoints.size() == 2) {
                    // Установка координат размера арены
                    int x1_final = std::min(arenaSizePoints[0].x, arenaSizePoints[1].x);
                    int y1_final = std::min(arenaSizePoints[0].y, arenaSizePoints[1].y);
                    int x2_final = std::max(arenaSizePoints[0].x, arenaSizePoints[1].x);
                    int y2_final = std::max(arenaSizePoints[0].y, arenaSizePoints[1].y);
                    
                    arenaSizeX1.store(x1_final);
                    arenaSizeY1.store(y1_final);
                    arenaSizeX2.store(x2_final);
                    arenaSizeY2.store(y2_final);
                    
                    // Обновление значений в панели
                    if (g_controlPanel != nullptr) {
                        g_controlPanel->setArenaSizeX1Value(std::to_string(x1_final));
                        g_controlPanel->setArenaSizeY1Value(std::to_string(y1_final));
                        g_controlPanel->setArenaSizeX2Value(std::to_string(x2_final));
                        g_controlPanel->setArenaSizeY2Value(std::to_string(y2_final));
                    }
                    
                    std::cout << "Arena size set: (" << x1_final << ", " << y1_final << ") - (" 
                              << x2_final << ", " << y2_final << ")" << std::endl;
                    
                    // Сброс вектора точек
                    arenaSizePoints.clear();
                    
                    // Автоматическое выключение режима выбора
                    arenaSizeSelectMode.store(false);
                }
            }
        }
    }
}

int main() {
    std::cout << "=== ArUco Marker Detector with Performance Profiling ===" << std::endl;
    std::cout << "Using Center of Mass method only" << std::endl;
    std::cout << "Exclusion zone is offset 20px in marker direction" << std::endl;
    std::cout << "Control Panel will open with PID controls" << std::endl;
    std::cout << "\n=== Control Panel Controls ===" << std::endl;
    std::cout << "  Camera Exposure   - Set camera exposure value" << std::endl;
    std::cout << "  Apply             - Apply camera settings" << std::endl;
    std::cout << "\n=== SINGLE THREAD MODE ===" << std::endl;
    std::cout << "All processing happens in linear sequence" << std::endl;
    std::cout << "No multithreading, minimal latency" << std::endl;

    // Создаем менеджер конфигурации
    ConfigManager configManager;
    
    // Создаем ArduinoController с использованием shared_ptr
    auto arduino = std::make_shared<ArduinoController>("COM3", 115200);

    // ===== ТЕСТ СКОРОСТИ ARDUINO =====
    std::cout << "\n=== ARDUINO SPEED TEST ===" << std::endl;
    
    // Тест 1: Измерение времени 100 команд set_motors
    {
        std::cout << "Test 1: Measuring 100 set_motors commands..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100; i++) {
            arduino->set_motors(50 + i % 10, 50 - i % 10, 0);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
        
        std::cout << "100 set_motors commands: " << total_time << " ms" << std::endl;
        std::cout << "Average per command: " << (total_time / 100.0) << " ms" << std::endl;
        std::cout << "Commands per second: " << (1000.0 / (total_time / 100.0)) << " Hz" << std::endl;
    }
    
    // Тест 2: Измерение времени 1000 отдельных команд
    {
        std::cout << "\nTest 2: Measuring 1000 individual commands..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000; i++) {
            arduino->set_motor(1, 50 + i % 10);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
        
        std::cout << "1000 set_motor commands: " << total_time << " ms" << std::endl;
        std::cout << "Average per command: " << (total_time / 1000.0) << " ms" << std::endl;
        std::cout << "Commands per second: " << (1000.0 / (total_time / 1000.0)) << " Hz" << std::endl;
    }

    
    std::cout << "\n=== END OF ARDUINO SPEED TEST ===" << std::endl;
    std::cout << "==================================" << std::endl;
    

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    arduino->set_motors(50, 50, 0);

    // Создаем контроллер робота с передачей ArduinoController
    RobotController robotController(arduino, 0.1f, 0.1f, 0.1f);
    std::cout << "Robot Controller initialized" << std::endl;
    std::cout << robotController.getPIDParameters() << std::endl;
    
    // Запускаем Control Panel
    ControlPanel panel;
    g_controlPanel = &panel;  // Устанавливаем глобальный указатель
    
    // Создаем детекторы
    ArucoDetector detector;
    MotionDetector motionDetector;
    
    // Настройки
    bool showExclusionZone = true;
    bool useOffsetExclusion = true;
    bool fastMode = true;
    std::atomic<int> exclusionRadius{150};
    std::atomic<int> attackRadius{100};
    std::atomic<bool> robotProcessingActive{false};

    // ========== ЗАГРУЗКА КОНФИГА ПЕРЕД ЗАПУСКОМ ПАНЕЛИ ==========
    bool configLoaded = configManager.loadConfig(
        panel,
        exclusionRadius,
        attackRadius,
        robotController,
        arenaCenterX,
        arenaCenterY,
        arenaSizeX1,
        arenaSizeY1,
        arenaSizeX2,
        arenaSizeY2,
        motionDetector,
        detector  // Добавлен ArucoDetector
    );

    
    if (configLoaded) {
        std::cout << "Configuration loaded successfully" << std::endl;
    } else {
        // Устанавливаем значения по умолчанию в панель
        panel.setMotionThresholdValue(std::to_string(motionDetector.getThreshold()));
        panel.setMorphKernelWidthValue(std::to_string(motionDetector.getMorphKernelWidth()));
        panel.setMorphKernelHeightValue(std::to_string(motionDetector.getMorphKernelHeight()));
        panel.setDilationIterationsValue(std::to_string(motionDetector.getDilationIterations()));
        panel.setErosionIterationsValue(std::to_string(motionDetector.getErosionIterations()));
        panel.setBlurKernelWidthValue(std::to_string(motionDetector.getBlurKernelWidth()));
        panel.setBlurKernelHeightValue(std::to_string(motionDetector.getBlurKernelHeight()));
        panel.setBlurSigmaValue(std::to_string(motionDetector.getBlurSigma()));
        
        // Устанавливаем значения по умолчанию для других полей
        panel.setRobot1ExclusionValue(std::to_string(exclusionRadius.load()));
        panel.setKpValue(std::to_string(robotController.getKp()));
        panel.setKiValue(std::to_string(robotController.getKi()));
        panel.setKdValue(std::to_string(robotController.getKd()));
        panel.setKrlValue(std::to_string(robotController.getKrlCoefficient()));
        panel.setKfbValue(std::to_string(robotController.getKfbCoefficient()));

        panel.setCenterAfterAttackValue(std::to_string(robotController.getCenterAfterAttack()));
        panel.setCenterTriggerThresholdValue(std::to_string(robotController.getCenterTriggerThreshold()));
        panel.setEnemyTriggerThresholdValue(std::to_string(robotController.getEnemyTriggerThreshold()));
        panel.setEnemyTriggerTimeoutValue(std::to_string(robotController.getEnemyTriggerTimeout()));
        panel.setReverseAngleThresholdValue(std::to_string(robotController.getReverseAngleThreshold()));
        panel.setUseReverseValue(std::to_string(robotController.getUseReverse()));
        panel.setAngleMinExtraSpeedValue(std::to_string(robotController.getAngleMinExtraSpeed()));
        panel.setAngleMaxExtraSpeedValue(std::to_string(robotController.getAngleMaxExtraSpeed()));

        panel.setMinMarkerPerimeterRateValue(std::to_string(detector.getMinMarkerPerimeterRate()));
        panel.setMaxMarkerPerimeterRateValue(std::to_string(detector.getMaxMarkerPerimeterRate()));
        
        panel.setArenaXValue(std::to_string(arenaCenterX.load()));
        panel.setArenaYValue(std::to_string(arenaCenterY.load()));
        panel.setArenaSizeX1Value(std::to_string(arenaSizeX1.load()));
        panel.setArenaSizeY1Value(std::to_string(arenaSizeY1.load()));
        panel.setArenaSizeX2Value(std::to_string(arenaSizeX2.load()));
        panel.setArenaSizeY2Value(std::to_string(arenaSizeY2.load()));
    }


    // Запускаем панель управления
    std::thread controlPanelThread(&ControlPanel::run, &panel);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Открываем камеру
    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        std::cerr << "ERROR: Cannot open camera!" << std::endl;
        panel.stop();
        if (controlPanelThread.joinable()) controlPanelThread.join();
        return -1;
    }

    int CAP_PROP_FRAME_WIDTH = 1280;
    int CAP_PROP_FRAME_HEIGHT = 1024;
    
    // Если есть сохраненное значение экспозиции, используем его
    std::string savedExposure = panel.getCameraExposureValue();
    if (!savedExposure.empty() && savedExposure != "-1") {
        try {
            int exposure = std::stoi(savedExposure);
            //cap.set(cv::CAP_PROP_EXPOSURE, exposure);
            std::cout << "Using saved camera exposure: " << exposure << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid saved exposure value, using default" << std::endl;
            cap.set(cv::CAP_PROP_EXPOSURE, -9);
        }
    } else {
        cap.set(cv::CAP_PROP_EXPOSURE, -9);
    }

    // Настраиваем камеру с фиксированным разрешением
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CAP_PROP_FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAP_PROP_FRAME_HEIGHT);
    
    
    cap.set(cv::CAP_PROP_FPS, 150);

    // Получаем текущие параметры камеры
    double currentWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double currentHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double currentExposure = cap.get(cv::CAP_PROP_EXPOSURE);
    
    // Устанавливаем начальные значения в панель
    panel.setCameraExposureValue(std::to_string(static_cast<int>(currentExposure)));

    // Получаем параметры
    double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double actualFPS = cap.get(cv::CAP_PROP_FPS);
    double actualExposure = cap.get(cv::CAP_PROP_EXPOSURE);
    
    std::cout << "\nCamera Parameters:" << std::endl;
    std::cout << "  Resolution: " << actualWidth << "x" << actualHeight << std::endl;
    std::cout << "  Exposure: " << actualExposure << std::endl;
    std::cout << "  Target FPS: " << actualFPS << std::endl;
    std::cout << "  Backend: DirectShow" << std::endl;

    
    // Флаг работы
    bool running = true;
    std::atomic<int> currentFPS{0};

    // Создаем окно
    cv::namedWindow("Motion Detection with Exclusion", cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

    // Инициализируем размер окна с учетом screenSizeFactor
    float initialFactor = screenSizeFactor.load();
    if (initialFactor != 1.0f) {
        // ИСПРАВЛЕНО: используем actualWidth и actualHeight
        int windowWidth = static_cast<int>(CAP_PROP_FRAME_WIDTH * initialFactor);
        int windowHeight = static_cast<int>(CAP_PROP_FRAME_HEIGHT * initialFactor);
        cv::resizeWindow("Motion Detection with Exclusion", windowWidth, windowHeight);
        std::cout << "Initial window size scaled to: " << windowWidth << "x" << windowHeight 
                << " (factor: " << initialFactor << ")" << std::endl;
    } else {
        // ИСПРАВЛЕНО: используем actualWidth и actualHeight
        cv::resizeWindow("Motion Detection with Exclusion", CAP_PROP_FRAME_WIDTH, CAP_PROP_FRAME_HEIGHT);
    }
    
    // Устанавливаем callback для мыши
    cv::setMouseCallback("Motion Detection with Exclusion", mouseCallback, nullptr);
    
    std::cout << "Single-thread mode: ENABLED (linear processing)" << std::endl;
    std::cout << "Initial channel values: " << robotController.getChannelsInfo() << std::endl;
    std::cout << "Initial exclusion radius: " << exclusionRadius.load() << std::endl;
    std::cout << "Initial attack radius: " << attackRadius.load() << std::endl;
    std::cout << "Initial arena center: (" << arenaCenterX.load() << ", " << arenaCenterY.load() << ")" << std::endl;
    std::cout << "Initial screen size factor: " << screenSizeFactor.load() << std::endl;
    
    // Статистика
    int capturedFrames = 0;
    auto programStart = std::chrono::steady_clock::now();
    
    // Для измерения FPS
    int frameCounter = 0;
    auto lastFpsTime = std::chrono::steady_clock::now();
    
    // Переменные для сохранения точки движения
    cv::Point savedMotionPoint(-1, -1);
    auto lastMotionTime = std::chrono::steady_clock::now();
    const int MOTION_PERSISTENCE_MS = 5000;  // 5 секунд
    
    int lastX1 = -1, lastY1 = -1, lastX2 = -1, lastY2 = -1;

    lastMarkerDetectionTime = std::chrono::steady_clock::now() - 
                              std::chrono::milliseconds(MARKER_PERSISTENCE_MS + 1000);

    // Callback функции в Control Panel для обработки изменений:
    panel.setOnRobot1ExclusionCallback([&](const std::string& value) {
        try {
            int radius = std::stoi(value);
            if (radius > 0 && radius <= 1000) {
                exclusionRadius.store(radius);
                std::cout << "Robot 1 ExclusionRadius updated to: " << radius << std::endl;
            } else {
                std::cerr << "ExclusionRadius out of range (1-1000): " << radius << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid ExclusionRadius value: " << value << std::endl;
        }
    });
    // Callback функции для настройки PID параметров
    panel.setOnKpCallback([&](const std::string& value) {
        try {
            float kp = std::stof(value);
            if (kp >= 0.0f && kp <= 10.0f) {
                robotController.setKp(kp);
                std::cout << "KP updated to: " << kp << std::endl;
            } else {
                std::cerr << "KP out of range (0.0-10.0): " << kp << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid KP value: " << value << std::endl;
        }
    });

    panel.setOnKiCallback([&](const std::string& value) {
        try {
            float ki = std::stof(value);
            if (ki >= 0.0f && ki <= 10.0f) {
                robotController.setKi(ki);
                std::cout << "KI updated to: " << ki << std::endl;
            } else {
                std::cerr << "KI out of range (0.0-10.0): " << ki << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid KI value: " << value << std::endl;
        }
    });

    panel.setOnKdCallback([&](const std::string& value) {
        try {
            float kd = std::stof(value);
            if (kd >= 0.0f && kd <= 10.0f) {
                robotController.setKd(kd);
                std::cout << "KD updated to: " << kd << std::endl;
            } else {
                std::cerr << "KD out of range (0.0-10.0): " << kd << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid KD value: " << value << std::endl;
        }
    });

    panel.setOnKrlCallback([&](const std::string& value) {
        try {
            float krl = std::stof(value);
            if (krl >= 0.0f && krl <= 10.0f) {
                robotController.setKrlCoefficient(krl);
                std::cout << "KRL coefficient updated to: " << krl << std::endl;
            } else {
                std::cerr << "KRL coefficient out of range (0.0-10.0): " << krl << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid KRL value: " << value << std::endl;
        }
    });

    panel.setOnKfbCallback([&](const std::string& value) {
        try {
            float kfb = std::stof(value);
            if (kfb >= 0.0f && kfb <= 10.0f) {
                robotController.setKfbCoefficient(kfb);
                std::cout << "KFB coefficient updated to: " << kfb << std::endl;
            } else {
                std::cerr << "KFB coefficient out of range (0.0-10.0): " << kfb << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid KFB value: " << value << std::endl;
        }
    });
    
    // Callback функция для переключения тестового режима
    panel.setOnSwitchCallback([&](bool enabled) {
        std::lock_guard<std::mutex> lock(testModeMutex);
        testModeActive.store(enabled);
        
        if (enabled) {
            std::cout << "=== TEST MODE ENABLED ===" << std::endl;
            std::cout << "Click on the video window to set motion point" << std::endl;
            std::cout << "Press 'T' to toggle test mode" << std::endl;
            std::cout << "Switch OFF to return to normal mode" << std::endl;
            
            // Сбрасываем точку при включении тестового режима
            testMotionPointX.store(-1);
            testMotionPointY.store(-1);
        } else {
            std::cout << "=== TEST MODE DISABLED ===" << std::endl;
            std::cout << "Returning to normal motion detection" << std::endl;
        }
    });

    // Callback функции для START и STOP
    panel.setOnStartCallback([&]() {
        robotController.reset();
        
        robotController.startCh3RampUp(100.0f, 3000);
        
        robotProcessingActive.store(true);
        std::cout << "=== ROBOT PROCESSING STARTED ===" << std::endl;
        std::cout << "Robot controller is now active" << std::endl;
        std::cout << "CH3 will ramp up smoothly over 3 seconds" << std::endl;
    });

    panel.setOnStopCallback([&]() {
        robotController.reset();
        robotProcessingActive.store(false);
        std::cout << "=== ROBOT PROCESSING STOPPED ===" << std::endl;
        std::cout << "Robot controller is now inactive" << std::endl;
        std::cout << "Channels reset to neutral position" << std::endl;
        
        // Сбрасываем каналы в нейтральное положение
        robotController.setChannel1(50.0f);
        robotController.setChannel2(50.0f);
        robotController.setChannel3(0.0f);

        robotController.sendToArduino();

        {
            std::lock_guard<std::mutex> lock(markerPersistenceMutex);
            savedMarkerX.store(-1);
            savedMarkerY.store(-1);
            savedMarkerRotation.store(0.0);
            savedMarkerHasMarker.store(false);
        }
    });
    
    // Callback функции для Arena Center
    panel.setOnArenaXCallback([&](const std::string& value) {
        try {
            int x = std::stoi(value);
            arenaCenterX.store(x);
            std::cout << "Arena X center updated to: " << x << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena X value: " << value << std::endl;
        }
    });
    
    panel.setOnArenaYCallback([&](const std::string& value) {
        try {
            int y = std::stoi(value);
            arenaCenterY.store(y);
            std::cout << "Arena Y center updated to: " << y << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena Y value: " << value << std::endl;
        }
    });
    
    panel.setOnArenaSelectCallback([&](bool enabled) {
        arenaCenterSelectMode.store(enabled);
        
        if (enabled) {
            std::cout << "=== ARENA CENTER SELECT MODE ENABLED ===" << std::endl;
            std::cout << "Click on the video window to set arena center" << std::endl;
            std::cout << "Switch OFF to exit select mode" << std::endl;
        } else {
            std::cout << "=== ARENA CENTER SELECT MODE DISABLED ===" << std::endl;
        }
    });

    // Callback функции для Arena Size
    panel.setOnArenaSizeX1Callback([&](const std::string& value) {
        try {
            int x = std::stoi(value);
            arenaSizeX1.store(x);
            std::cout << "Arena Size X1 updated to: " << x << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena Size X1 value: " << value << std::endl;
        }
    });

    panel.setOnArenaSizeY1Callback([&](const std::string& value) {
        try {
            int y = std::stoi(value);
            arenaSizeY1.store(y);
            std::cout << "Arena Size Y1 updated to: " << y << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena Size Y1 value: " << value << std::endl;
        }
    });

    panel.setOnArenaSizeX2Callback([&](const std::string& value) {
        try {
            int x = std::stoi(value);
            arenaSizeX2.store(x);
            std::cout << "Arena Size X2 updated to: " << x << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena Size X2 value: " << value << std::endl;
        }
    });

    panel.setOnArenaSizeY2Callback([&](const std::string& value) {
        try {
            int y = std::stoi(value);
            arenaSizeY2.store(y);
            std::cout << "Arena Size Y2 updated to: " << y << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid Arena Size Y2 value: " << value << std::endl;
        }
    });

    panel.setOnArenaSizeSelectCallback([&]() {
        arenaSizeSelectMode.store(true);
        arenaSizePoints.clear(); // Очищаем предыдущие точки
        
        std::cout << "=== ARENA SIZE SELECT MODE ENABLED ===" << std::endl;
        std::cout << "Click two diagonal corners of the arena" << std::endl;
        std::cout << "After second click, mode will auto-exit" << std::endl;
    });

    panel.setOnArenaSizeResetCallback([&]() {
        arenaSizeX1.store(-1);
        arenaSizeY1.store(-1);
        arenaSizeX2.store(-1);
        arenaSizeY2.store(-1);
        arenaSizePoints.clear();
        
        // Обновляем значения в панели
        panel.setArenaSizeX1Value("-1");
        panel.setArenaSizeY1Value("-1");
        panel.setArenaSizeX2Value("-1");
        panel.setArenaSizeY2Value("-1");
        
        std::cout << "Arena size reset to default values" << std::endl;
    });

    // Добавляем callback для Apply кнопки (только экспозиция)
    panel.setOnCameraApplyCallback([&](const std::string& exposureStr) {
        try {
            int exposure = std::stoi(exposureStr);

            //cap.set(cv::CAP_PROP_FRAME_WIDTH, CAP_PROP_FRAME_WIDTH);
            //cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAP_PROP_FRAME_HEIGHT);
            
            // Устанавливаем новую экспозицию
            cap.set(cv::CAP_PROP_EXPOSURE, exposure);
            
            // Получаем актуальное значение для проверки
            double newExposure = cap.get(cv::CAP_PROP_EXPOSURE);
            
            // Обновляем значение в панели с актуальным значением
            panel.setCameraExposureValue(std::to_string(static_cast<int>(newExposure)));
            
            std::cout << "Camera settings applied:" << std::endl;
            std::cout << "  Exposure: " << newExposure << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Invalid camera exposure value: " << e.what() << std::endl;
            
            // Возвращаем текущее значение экспозиции в поле ввода
            double currentExposure = cap.get(cv::CAP_PROP_EXPOSURE);
            panel.setCameraExposureValue(std::to_string(static_cast<int>(currentExposure)));
        }
    });

    // Callback функции для MotionDetector параметров
    panel.setOnMotionThresholdCallback([&](const std::string& value) {
        try {
            int threshold = std::stoi(value);
            if (threshold >= 0 && threshold <= 255) {
                motionDetector.setThreshold(threshold);
                std::cout << "Motion Threshold updated to: " << threshold << std::endl;
            } else {
                std::cerr << "Motion Threshold out of range (0-255): " << threshold << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Motion Threshold value: " << value << std::endl;
        }
    });

    panel.setOnMorphKernelWidthCallback([&](const std::string& value) {
        try {
            int width = std::stoi(value);
            if (width >= 1 && width <= 31) {
                int currentHeight = motionDetector.getMorphKernelHeight();
                motionDetector.setMorphKernelSize(width, currentHeight);
                std::cout << "Morph Kernel Width updated to: " << width << std::endl;
            } else {
                std::cerr << "Morph Kernel Width out of range (1-31): " << width << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Morph Kernel Width value: " << value << std::endl;
        }
    });

    panel.setOnMorphKernelHeightCallback([&](const std::string& value) {
        try {
            int height = std::stoi(value);
            if (height >= 1 && height <= 31) {
                int currentWidth = motionDetector.getMorphKernelWidth();
                motionDetector.setMorphKernelSize(currentWidth, height);
                std::cout << "Morph Kernel Height updated to: " << height << std::endl;
            } else {
                std::cerr << "Morph Kernel Height out of range (1-31): " << height << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Morph Kernel Height value: " << value << std::endl;
        }
    });

    panel.setOnDilationIterationsCallback([&](const std::string& value) {
        try {
            int iterations = std::stoi(value);
            if (iterations >= 0 && iterations <= 10) {
                motionDetector.setDilationIterations(iterations);
                std::cout << "Dilation Iterations updated to: " << iterations << std::endl;
            } else {
                std::cerr << "Dilation Iterations out of range (0-10): " << iterations << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Dilation Iterations value: " << value << std::endl;
        }
    });

    panel.setOnErosionIterationsCallback([&](const std::string& value) {
        try {
            int iterations = std::stoi(value);
            if (iterations >= 0 && iterations <= 10) {
                motionDetector.setErosionIterations(iterations);
                std::cout << "Erosion Iterations updated to: " << iterations << std::endl;
            } else {
                std::cerr << "Erosion Iterations out of range (0-10): " << iterations << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Erosion Iterations value: " << value << std::endl;
        }
    });

    panel.setOnBlurKernelWidthCallback([&](const std::string& value) {
        try {
            int width = std::stoi(value);
            if (width >= 1 && width <= 31) {
                int currentHeight = motionDetector.getBlurKernelHeight();
                motionDetector.setBlurKernelSize(width, currentHeight);
                std::cout << "Blur Kernel Width updated to: " << width << std::endl;
            } else {
                std::cerr << "Blur Kernel Width out of range (1-31): " << width << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Blur Kernel Width value: " << value << std::endl;
        }
    });

    panel.setOnBlurKernelHeightCallback([&](const std::string& value) {
        try {
            int height = std::stoi(value);
            if (height >= 1 && height <= 31) {
                int currentWidth = motionDetector.getBlurKernelWidth();
                motionDetector.setBlurKernelSize(currentWidth, height);
                std::cout << "Blur Kernel Height updated to: " << height << std::endl;
            } else {
                std::cerr << "Blur Kernel Height out of range (1-31): " << height << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Blur Kernel Height value: " << value << std::endl;
        }
    });

    panel.setOnBlurSigmaCallback([&](const std::string& value) {
        try {
            double sigma = std::stod(value);
            if (sigma >= 0.1 && sigma <= 10.0) {
                motionDetector.setBlurSigma(sigma);
                std::cout << "Blur Sigma updated to: " << sigma << std::endl;
            } else {
                std::cerr << "Blur Sigma out of range (0.1-10.0): " << sigma << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid Blur Sigma value: " << value << std::endl;
        }
    });

    // Добавьте callback для обработки изменений:
    panel.setOnRobot1OffsetCallback([&](const std::string& value) {
        try {
            int offset = std::stoi(value);
            if (offset >= 0 && offset <= 1000) {
                offsetPixels.store(offset);
                std::cout << "Robot 1 OffsetPixels updated to: " << offset << std::endl;
            } else {
                std::cerr << "OffsetPixels out of range (0-1000): " << offset << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid OffsetPixels value: " << value << std::endl;
        }
    });
    
    // Callback функции для новых параметров RobotController
    panel.setOnCenterAfterAttackCallback([&](const std::string& value) {
        try {
            int centerAfterAttack = std::stoi(value);
            if (centerAfterAttack >= 0 && centerAfterAttack <= 1) {
                robotController.setCenterAfterAttack(centerAfterAttack);
                std::cout << "CenterAfterAttack updated to: " << centerAfterAttack << std::endl;
            } else {
                std::cerr << "CenterAfterAttack out of range (0-1): " << centerAfterAttack << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid CenterAfterAttack value: " << value << std::endl;
        }
    });

    panel.setOnCenterTriggerThresholdCallback([&](const std::string& value) {
        try {
            int threshold = std::stoi(value);
            if (threshold >= 0 && threshold <= 1000) {
                robotController.setCenterTriggerThreshold(threshold);
                std::cout << "CenterTriggerThreshold updated to: " << threshold << std::endl;
            } else {
                std::cerr << "CenterTriggerThreshold out of range (0-1000): " << threshold << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid CenterTriggerThreshold value: " << value << std::endl;
        }
    });

    panel.setOnEnemyTriggerThresholdCallback([&](const std::string& value) {
        try {
            int threshold = std::stoi(value);
            if (threshold >= 0 && threshold <= 1000) {
                robotController.setEnemyTriggerThreshold(threshold);
                std::cout << "EnemyTriggerThreshold updated to: " << threshold << std::endl;
            } else {
                std::cerr << "EnemyTriggerThreshold out of range (0-1000): " << threshold << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid EnemyTriggerThreshold value: " << value << std::endl;
        }
    });

    panel.setOnEnemyTriggerTimeoutCallback([&](const std::string& value) {
        try {
            int timeout = std::stoi(value);
            if (timeout >= 0 && timeout <= 10000) {
                robotController.setEnemyTriggerTimeout(timeout);
                std::cout << "EnemyTriggerTimeout updated to: " << timeout << std::endl;
            } else {
                std::cerr << "EnemyTriggerTimeout out of range (0-10000): " << timeout << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid EnemyTriggerTimeout value: " << value << std::endl;
        }
    });

    panel.setOnReverseAngleThresholdCallback([&](const std::string& value) {
        try {
            int threshold = std::stoi(value);
            if (threshold >= 0 && threshold <= 360) {
                robotController.setReverseAngleThreshold(threshold);
                std::cout << "ReverseAngleThreshold updated to: " << threshold << std::endl;
            } else {
                std::cerr << "ReverseAngleThreshold out of range (0-360): " << threshold << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid ReverseAngleThreshold value: " << value << std::endl;
        }
    });

    // Callback для AngleMinExtraSpeed
    panel.setOnAngleMinExtraSpeedCallback([&](const std::string& value) {
        try {
            float angle = std::stof(value);
            if (angle >= 0.0f && angle <= 180.0f) {
                robotController.setAngleMinExtraSpeed(angle);
                std::cout << "AngleMinExtraSpeed updated to: " << angle << std::endl;
            } else {
                std::cerr << "AngleMinExtraSpeed out of range (0.0-180.0): " << angle << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid AngleMinExtraSpeed value: " << value << std::endl;
        }
    });

    // Callback для AngleMaxExtraSpeed
    panel.setOnAngleMaxExtraSpeedCallback([&](const std::string& value) {
        try {
            float angle = std::stof(value);
            if (angle >= 0.0f && angle <= 180.0f) {
                robotController.setAngleMaxExtraSpeed(angle);
                std::cout << "AngleMaxExtraSpeed updated to: " << angle << std::endl;
            } else {
                std::cerr << "AngleMaxExtraSpeed out of range (0.0-180.0): " << angle << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid AngleMaxExtraSpeed value: " << value << std::endl;
        }
    });

    // Callback для InPlaceSpeedRotation
    panel.setOnInPlaceSpeedRotationCallback([&](const std::string& value) {
        float SpeedRotation = std::stof(value);
        robotController.setInPlaceSpeedRotation(SpeedRotation);
        std::cout << "InPlaceSpeedRotation updated to: " << SpeedRotation << std::endl;
    });

    // Callback для InPlaceSpeedWeapon
    panel.setOnInPlaceSpeedWeaponCallback([&](const std::string& value) {
        float SpeedWeapon = std::stof(value);
        robotController.setInPlaceSpeedWeapon(SpeedWeapon);
        std::cout << "InPlaceSpeedWeapon updated to: " << SpeedWeapon << std::endl;
    });
    

    

    panel.setOnUseReverseCallback([&](const std::string& value) {
        try {
            int useReverse = std::stoi(value);
            if (useReverse >= 0 && useReverse <= 1) {
                robotController.setUseReverse(useReverse);
                std::cout << "UseReverse updated to: " << useReverse << std::endl;
            } else {
                std::cerr << "UseReverse out of range (0-1): " << useReverse << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid UseReverse value: " << value << std::endl;
        }
    });

    // Callback функции для Check
    panel.setOnCheckCallback([arduino]() {
        std::cout << "Check START" << std::endl;
        
        if (arduino && arduino->is_connected()) {
            // Запускаем robot_check в отдельном потоке
            std::thread checkThread([arduino]() {
                arduino->robot_check();
            });
            checkThread.detach(); // Отсоединяем, чтобы не ждать завершения
        } else {
            std::cout << "Arduino not connected!" << std::endl;
        }
        
        std::cout << "Check END (async)" << std::endl;
    });

    // Callback для MinMarkerPerimeterRate
    panel.setOnMinMarkerPerimeterRateCallback([&](const std::string& value) {
        try {
            float rate = std::stof(value);
            if (rate > 0.0f) {
                detector.setMinMarkerPerimeterRate(rate);
                std::cout << "MinMarkerPerimeterRate updated to: " << rate << std::endl;
            } else {
                std::cerr << "MinMarkerPerimeterRate out of range (>0.0): " << rate << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid MinMarkerPerimeterRate value: " << value << std::endl;
        }
    });

    // Callback для MaxMarkerPerimeterRate
    panel.setOnMaxMarkerPerimeterRateCallback([&](const std::string& value) {
        try {
            float rate = std::stof(value);
            if (rate > 0.0f) {
                detector.setMaxMarkerPerimeterRate(rate);
                std::cout << "MaxMarkerPerimeterRate updated to: " << rate << std::endl;
            } else {
                std::cerr << "MaxMarkerPerimeterRate out of range (>0.0): " << rate << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid MaxMarkerPerimeterRate value: " << value << std::endl;
        }
    });

    robotController.setChannel1(50.0f);
    robotController.setChannel2(50.0f);
    robotController.setChannel3(0.0f);

    robotController.sendToArduino();

    // Основной цикл (ОДИН ПОТОК)
    while (panel.isRunning() && running) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // ЗАХВАТ КАДРА
        cv::Mat frame;
        if (!cap.read(frame) || frame.empty()) {
            std::cerr << "ERROR: Cannot read frame from camera!" << std::endl;
            break;
        }
        
        capturedFrames++;
        frameCounter++;
        
        // ===== ОБРЕЗКА КАДРА ПО РАЗМЕРУ АРЕНЫ =====
        int x1 = arenaSizeX1.load();
        int y1 = arenaSizeY1.load();
        int x2 = arenaSizeX2.load();
        int y2 = arenaSizeY2.load();
        
        // Проверяем, изменился ли размер обрезки
        bool cropSizeChanged = (x1 != lastX1 || y1 != lastY1 || x2 != lastX2 || y2 != lastY2);
        
        cv::Mat frameToProcess = frame.clone(); // Копируем кадр
        
        if (x1 != -1 && y1 != -1 && x2 != -1 && y2 != -1) {
            // Создаем обрезанный кадр
            cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
            if (roi.x >= 0 && roi.y >= 0 && 
                roi.x + roi.width <= frameToProcess.cols && 
                roi.y + roi.height <= frameToProcess.rows) {
                frameToProcess = frameToProcess(roi).clone();
            }
        }
        
        // Если размер обрезки изменился, сбрасываем MotionDetector
        if (cropSizeChanged) {
            motionDetector.reset();
            std::cout << "Crop size changed, resetting MotionDetector" << std::endl;
            lastX1 = x1;
            lastY1 = y1;
            lastX2 = x2;
            lastY2 = y2;
        }
        
        // ===== ОБРАБОТКА ARUCO =====
        detector.processFrame(frameToProcess);

        bool hasMarker = false;
        cv::Point markerCenterPoint(-1, -1);
        double markerRotation = 0.0;
        int markerId = -1;

        // Информация о маркере (координаты уже относительно обрезанного кадра)
        if (detector.hasMarkers()) {
            // Берем первый найденный маркер (уже отфильтрован по ID 0 и 1)
            cv::Point2f markerCenter = detector.getMarkerCenter(0);
            markerRotation = detector.getMarkerRotation(0);
            markerId = detector.markerIds[0];
            
            markerCenterPoint = cv::Point(
                static_cast<int>(markerCenter.x), 
                static_cast<int>(markerCenter.y));
            hasMarker = true;
            
            // Обновляем сохраненную информацию
            updateSavedMarkerInfo(true, markerCenterPoint, markerRotation);
        } else {
            // Обновляем сохраненную информацию (маркер не обнаружен)
            updateSavedMarkerInfo(false, cv::Point(-1, -1), 0.0);
            
            // Пытаемся получить сохраненную информацию
            if (getMarkerWithPersistence(markerCenterPoint, markerRotation)) {
                hasMarker = true;
                // При использовании сохраненной информации ID неизвестен
                markerId = -1;
                
                // Визуально отмечаем, что используется сохраненная информация
                if (robotProcessingActive.load()) {
                    std::cout << "Using saved marker info (last seen " 
                            << std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - lastMarkerDetectionTime).count() 
                            << " ms ago)" << std::endl;
                }
            }
        }

        robotController.setMarkerDetected(hasMarker);
        
        // ===== EXCLUSION ZONE =====
        cv::Point exclusionZoneCenter(-1, -1);
        if (markerCenterPoint.x >= 0) {
            if (useOffsetExclusion) {
                exclusionZoneCenter = motionDetector.getOffsetExclusionCenter(
                    markerCenterPoint, markerRotation, offsetPixels.load());
            } else {
                exclusionZoneCenter = markerCenterPoint;
            }
        }
        
        // ===== ДЕТЕКЦИЯ ДВИЖЕНИЯ =====
        cv::Point motionCenter(-1, -1);
        bool motionDetected = false;
        
        if (testModeActive.load()) {
            // ТЕСТОВЫЙ РЕЖИМ
            cv::Point testPoint = getTestMotionPoint();
            if (testPoint.x >= 0 && testPoint.y >= 0) {
                motionCenter = testPoint;
                motionDetected = true;
                savedMotionPoint = testPoint;
                lastMotionTime = std::chrono::steady_clock::now();
            }
        } else {
            // НОРМАЛЬНЫЙ РЕЖИМ
            cv::Point detectedPoint = motionDetector.detectMotionWithExclusion(
                frameToProcess, exclusionZoneCenter, exclusionRadius.load());
            
            auto now = std::chrono::steady_clock::now();
            
            if (detectedPoint.x >= 0 && detectedPoint.y >= 0) {
                // Детектировано движение
                motionCenter = detectedPoint;
                motionDetected = true;
                savedMotionPoint = detectedPoint;
                lastMotionTime = now;
            } else {
                // Проверяем сохраненную точку
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastMotionTime).count();
                
                if (elapsed < MOTION_PERSISTENCE_MS && savedMotionPoint.x >= 0) {
                    // Используем сохраненную точку
                    motionCenter = savedMotionPoint;
                    motionDetected = true;
                } else {
                    savedMotionPoint = cv::Point(-1, -1);
                }
            }
        }
        
        // ===== ВЫЗОВ ROBOT CONTROLLER С ID МАРКЕРА =====
        if (robotProcessingActive.load()) {
            // Корректируем координаты центра арены с учетом обрезки
            int arenaX = arenaCenterX.load();
            int arenaY = arenaCenterY.load();
            int correctedArenaCenterX = arenaX;
            int correctedArenaCenterY = arenaY;
            
            if (x1 != -1 && y1 != -1 && x2 != -1 && y2 != -1) {
                correctedArenaCenterX = arenaX - x1;
                correctedArenaCenterY = arenaY - y1;
            }
            
            // КОРРЕКТИРОВАНИЕ ПОЗИЦИИ ДВИЖЕНИЯ С УЧЕТОМ ОБРЕЗКИ
            int motionX = -1;
            int motionY = -1;
            
            if (motionDetected) {
                // Позиция уже находится в координатах обрезанного кадра
                motionX = motionCenter.x;
                motionY = motionCenter.y;
            }
            
            robotController.process(
                markerRotation,
                correctedArenaCenterX, correctedArenaCenterY,  // Центр арены
                markerCenterPoint.x, markerCenterPoint.y,  // Позиция маркера
                motionX, motionY,  // Позиция движения/цели
                markerId  // ID маркера
            );
}
        
        // ===== ОТРИСОВКА И МАСШТАБИРОВАНИЕ =====
        cv::Mat displayFrame = frameToProcess.clone();
        
        // Отрисовка размера арены (если режим выбора активен)
        if (arenaSizeSelectMode.load()) {
            // Отрисовка точек выбора
            for (size_t i = 0; i < arenaSizePoints.size(); i++) {
                // Корректируем координаты точек с учетом обрезки
                int pointX = arenaSizePoints[i].x - x1;
                int pointY = arenaSizePoints[i].y - y1;
                
                if (pointX >= 0 && pointY >= 0 && 
                    pointX < displayFrame.cols && 
                    pointY < displayFrame.rows) {
                    
                    cv::circle(displayFrame, 
                              cv::Point(pointX, pointY), 
                              5, cv::Scalar(0, 255, 255), -1);
                    cv::putText(displayFrame, 
                              std::to_string(i+1), 
                              cv::Point(pointX + 10, pointY - 10),
                              cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                }
            }
            
            // Отрисовка инструкции
            if (arenaSizePoints.size() < 2) {
                std::string instruction = arenaSizePoints.size() == 0 ? 
                    "Click first corner" : "Click second corner";
                cv::putText(displayFrame, instruction, 
                           cv::Point(10, displayFrame.rows - 30),
                           cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
            }
        }

        // Отрисовка результатов ArUco
        detector.drawResults(displayFrame);  // ИСПРАВЛЕНО: используем . вместо ->
            
        if (showExclusionZone && exclusionZoneCenter.x >= 0 && 
            exclusionZoneCenter.y >= 0) {

            motionDetector.drawExclusionZone(displayFrame, 
                                             exclusionZoneCenter, 
                                             exclusionRadius.load());
            
            // В функции displayThread, после отрисовки маркера добавьте:
            if (markerCenterPoint.x >= 0) {
                std::string markerInfo = "Marker: (" + 
                    std::to_string(markerCenterPoint.x) + 
                    ", " + std::to_string(markerCenterPoint.y) + 
                    ") R: " + std::to_string(static_cast<int>(markerRotation)) + "°";
                
                cv::putText(displayFrame, markerInfo, 
                            cv::Point(markerCenterPoint.x - 60, 
                                    markerCenterPoint.y - 30),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1);
                
                if (useOffsetExclusion) {
                    cv::arrowedLine(displayFrame, 
                                    markerCenterPoint, 
                                    exclusionZoneCenter, 
                                    cv::Scalar(255, 255, 0), 2, 8, 0, 0.3);
                }
                
                int offset = 40; // Смещение в пикселях
                
                // Преобразуем угол поворота из градусов в радианы
                double rotationRad = markerRotation * CV_PI / 180.0;
                
                // Получаем значения CH1 и CH2 (в процентах от 0 до 100)
                float ch1_value = robotController.getChannel1();
                float ch2_value = robotController.getChannel2();
                
                if (robotController.getMarkerSide(markerId)==0) {
                    robotController.SwapInvertChannels(ch1_value, ch2_value);
                }

                // ===== НОВАЯ ЛОГИКА ДЛИНЫ СТРЕЛКИ =====
                // 50 = нет стрелки (0 пикселей)
                // 0 = стрелка назад (отрицательная длина, -25 пикселей)
                // 100 = стрелка вперед (положительная длина, +25 пикселей)
                
                // Нормализуем значения от -1.0 до +1.0
                float normalizedCH1 = (std::clamp(ch1_value, 0.0f, 100.0f) - 50.0f) / 50.0f; // 0→-1.0, 50→0.0, 100→+1.0
                float normalizedCH2 = (std::clamp(ch2_value, 0.0f, 100.0f) - 50.0f) / 50.0f; // 0→-1.0, 50→0.0, 100→+1.0
                
                // Конвертируем в длину стрелки (-25 до +25 пикселей)
                int arrowLengthCH1 = static_cast<int>(normalizedCH1 * 25.0f * 3);
                int arrowLengthCH2 = static_cast<int>(normalizedCH2 * 25.0f * 3);

                // ===== ТОЧКА СЛЕВА ОТ МАРКЕРА =====
                // Левая точка: против направления маркера (center - offset * direction)
                cv::Point leftPoint(
                    markerCenterPoint.x - static_cast<int>(offset * cos(rotationRad)),
                    markerCenterPoint.y - static_cast<int>(offset * sin(rotationRad))
                );
                
                // ===== ТОЧКА СПРАВА ОТ МАРКЕРА =====
                // Правая точка: по направлению маркера (center + offset * direction)
                cv::Point rightPoint(
                    markerCenterPoint.x + static_cast<int>(offset * cos(rotationRad)),
                    markerCenterPoint.y + static_cast<int>(offset * sin(rotationRad))
                );
                
                // Левая точка - зеленая
                cv::circle(displayFrame, leftPoint, 6, cv::Scalar(0, 255, 0), -1);
                cv::circle(displayFrame, leftPoint, 7, cv::Scalar(255, 255, 255), 1);
                
                // Правая точка - тоже зеленая
                cv::circle(displayFrame, rightPoint, 6, cv::Scalar(0, 255, 0), -1);
                cv::circle(displayFrame, rightPoint, 7, cv::Scalar(255, 255, 255), 1);

                // ===== СТРЕЛКИ В НАПРАВЛЕНИИ МАРКЕРА ИЗ КАЖДОЙ ТОЧКИ =====

                double arrowRotation = rotationRad - CV_PI / 2.0;

                // Для левой точки - ЗЕЛЕНАЯ стрелка (CH1)
                // Если arrowLengthCH1 = 0, стрелка не рисуется
                if (arrowLengthCH1 != 0) {
                    cv::Point leftArrowEnd(
                        leftPoint.x + static_cast<int>(arrowLengthCH1 * cos(arrowRotation)),
                        leftPoint.y + static_cast<int>(arrowLengthCH1 * sin(arrowRotation))
                    );
                    
                    // Определяем цвет стрелки в зависимости от направления
                    cv::Scalar leftArrowColor = (arrowLengthCH1 > 0) ? 
                        cv::Scalar(0, 255, 0) :  // Зеленый для вперед
                        cv::Scalar(0, 0, 255);   // Красный для назад
                    
                    // Рисуем стрелку от левой точки
                    cv::arrowedLine(displayFrame, 
                                    leftPoint, 
                                    leftArrowEnd, 
                                    leftArrowColor,  // Цвет зависит от направления
                                    2,                       // Толщина линии
                                    cv::LINE_AA,             // Сглаженная линия
                                    0,                       // Смещение
                                    0.3);                    // Длина наконечника
                }
                
                // Для правой точки - ЗЕЛЕНАЯ стрелка (CH2)
                // Если arrowLengthCH2 = 0, стрелка не рисуется
                if (arrowLengthCH2 != 0) {
                    cv::Point rightArrowEnd(
                        rightPoint.x + static_cast<int>(arrowLengthCH2 * cos(arrowRotation)),
                        rightPoint.y + static_cast<int>(arrowLengthCH2 * sin(arrowRotation))
                    );
                    
                    // Определяем цвет стрелки в зависимости от направления
                    cv::Scalar rightArrowColor = (arrowLengthCH2 > 0) ? 
                        cv::Scalar(0, 255, 0) :  // Зеленый для вперед
                        cv::Scalar(0, 0, 255);   // Красный для назад
                    
                    // Рисуем стрелку от правой точки
                    cv::arrowedLine(displayFrame, 
                                    rightPoint, 
                                    rightArrowEnd, 
                                    rightArrowColor,  // Цвет зависит от направления
                                    2,                       // Толщина линии
                                    cv::LINE_AA,             // Сглаженная линия
                                    0,                       // Смещение
                                    0.3);                    // Длина наконечника
                }
            }
        }
        
        // Отрисовка центра движения
        if (testModeActive.load()) {
            // ТЕСТОВЫЙ РЕЖИМ: отрисовываем точку красным крестом
            cv::Point testPoint = getTestMotionPoint();
            if (testPoint.x >= 0 && testPoint.y >= 0) {
                // Рисуем крест для тестовой точки
                cv::line(displayFrame, 
                        cv::Point(testPoint.x - 15, testPoint.y), 
                        cv::Point(testPoint.x + 15, testPoint.y), 
                        cv::Scalar(0, 0, 255), 2);
                cv::line(displayFrame, 
                        cv::Point(testPoint.x, testPoint.y - 15), 
                        cv::Point(testPoint.x, testPoint.y + 15), 
                        cv::Scalar(0, 0, 255), 2);
                
                // Круг вокруг точки
                cv::circle(displayFrame, testPoint, 30, cv::Scalar(0, 0, 255), 2);
                
                // Текст "Test Point"
                cv::putText(displayFrame, "Test Point", 
                           cv::Point(testPoint.x - 30, testPoint.y - 40),
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            }
        } else {
            // НОРМАЛЬНЫЙ РЕЖИМ: отрисовываем обычный центр движения
            motionDetector.drawMotionCenter(displayFrame, motionCenter);
        }
        
        // Отрисовка центра арены
        int arenaX = arenaCenterX.load();
        int arenaY = arenaCenterY.load();
        if (arenaX >= 0 && arenaY >= 0) {
            // Корректируем координаты центра арены с учетом обрезки
            int arenaXAdjusted = arenaX - x1;
            int arenaYAdjusted = arenaY - y1;
            
            // Проверяем, что точка внутри обрезанного кадра
            if (arenaXAdjusted >= 0 && arenaYAdjusted >= 0 && 
                arenaXAdjusted < displayFrame.cols && 
                arenaYAdjusted < displayFrame.rows) {
                
                // Рисуем голубую точку для центра арены
                cv::circle(displayFrame, cv::Point(arenaXAdjusted, arenaYAdjusted), 8, cv::Scalar(255, 255, 255), -1);  // Белая подложка
                cv::circle(displayFrame, cv::Point(arenaXAdjusted, arenaYAdjusted), 6, cv::Scalar(255, 255, 0), -1);  // Голубая точка
                cv::circle(displayFrame, cv::Point(arenaXAdjusted, arenaYAdjusted), 7, cv::Scalar(255, 255, 255), 1);  // Тонкий белый контур
            } else {
                // Если точка вне обрезанного кадра, рисуем ее на границе
                int clampedX = std::clamp(arenaXAdjusted, 0, displayFrame.cols - 1);
                int clampedY = std::clamp(arenaYAdjusted, 0, displayFrame.rows - 1);
                cv::circle(displayFrame, cv::Point(clampedX, clampedY), 8, cv::Scalar(255, 255, 255), -1);
                cv::circle(displayFrame, cv::Point(clampedX, clampedY), 6, cv::Scalar(255, 0, 0), -1);
                cv::circle(displayFrame, cv::Point(clampedX, clampedY), 7, cv::Scalar(255, 255, 255), 1);
            }
        }
        
        // Информация на кадре
        std::string motionText;
        cv::Scalar motionStatusColor;
        
        if (testModeActive.load()) {
            cv::Point testPoint = getTestMotionPoint();
            motionText = testPoint.x >= 0 ? "Test Point: 1" : "Test Point: 0";
            motionStatusColor = testPoint.x >= 0 ? cv::Scalar(0, 165, 255) : cv::Scalar(0, 0, 255);
        } else {
            motionText = motionDetected ? "Enemy found: 1" : "Enemy found: 0";
            motionStatusColor = motionDetected ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        }
        
        // Enemy found/Test point в левом верхнем углу
        cv::putText(displayFrame, motionText, cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.8, motionStatusColor, 2);
        
        // Markers found под Enemy found
        bool markerDetected = robotController.isMarkerDetected();
        std::string markersText = "Markers found: " + std::string(markerDetected ? "1" : "0");
        cv::Scalar markersColor = markerDetected ? cv::Scalar(0, 200, 255) : cv::Scalar(150, 150, 150);
        
        cv::putText(displayFrame, markersText, cv::Point(10, 60),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, markersColor, 2);
        
        // CH1 и CH2 под Markers found
        std::string ch12Text = "CH1: " + std::to_string(static_cast<int>(robotController.getChannel1())) + 
                               " | CH2: " + std::to_string(static_cast<int>(robotController.getChannel2()));
        cv::Scalar ch12Color = cv::Scalar(255, 255, 0); // Желтый цвет
        
        cv::putText(displayFrame, ch12Text, cv::Point(10, 90),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, ch12Color, 2);
        
        // State под CH1 и CH2
        std::string stateText = "State: " + std::to_string(robotController.getState());
        cv::Scalar stateColor;
        
        // Выбираем цвет в зависимости от состояния
        int state = robotController.getState();
        switch(state) {
            case 0: stateColor = cv::Scalar(0, 255, 0); break;
            case 1: stateColor = cv::Scalar(0, 0, 255);     break;
            default: stateColor = cv::Scalar(255, 255, 255);
        }
        
        cv::putText(displayFrame, stateText, cv::Point(10, 120),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, stateColor, 2);

        // CH3 под CH1 и CH2
        std::string ch3Text = "CH3: " + std::to_string(static_cast<int>(robotController.getChannel3()));
        cv::Scalar ch3Color = cv::Scalar(200, 100, 255); // Фиолетовый цвет
        
        cv::putText(displayFrame, ch3Text, cv::Point(10, 150),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, ch3Color, 2);
        
        
        // Статус обработки RobotController - FIGHT ACTIVE вверху посередине (только если активен)
        if (robotProcessingActive.load()) {
            std::string fightStatus = "FIGHT ACTIVE";
            
            // Рассчитываем позицию для центрирования текста
            cv::Size textSize = cv::getTextSize(fightStatus, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, 0);
            int centerX = (displayFrame.cols - textSize.width) / 2;
            int topY = 40; // Отступ сверху
            
            // Рисуем текст вверху посередине
            cv::putText(displayFrame, fightStatus, cv::Point(centerX, topY),
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        }

        
        // FPS в правом верхнем углу
        std::string fpsText = "FPS: " + std::to_string(currentFPS.load());
        cv::Size textSize = cv::getTextSize(fpsText, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, 0);
        int fpsX = displayFrame.cols - textSize.width - 10;
        
        cv::putText(displayFrame, fpsText, cv::Point(fpsX, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
        
        // Offset и PID в нижнем левом углу (более заметные)
        if (markerDetected) {
            // Offset с более жирным шрифтом
            std::string offsetText = "Offset: " + std::to_string(static_cast<int>(robotController.getCurrentOffset())) + "°";
            cv::putText(displayFrame, offsetText, cv::Point(10, displayFrame.rows - 60),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(200, 200, 100), 2);
            
            // PID параметры под Offset
            std::string pidText = robotController.getPIDParameters();
            cv::putText(displayFrame, pidText, cv::Point(10, displayFrame.rows - 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(100, 200, 255), 2);
        }
        
        // Инструкции в правом нижнем углу
        std::string helpText;
        if (testModeActive.load()) {
            helpText = "Click: Set point | ESC: Exit";
        } else if (arenaCenterSelectMode.load()) {
            helpText = "Click: Set arena center | ESC: Exit";
        } else if (arenaSizeSelectMode.load()) {
            helpText = "Click: Select arena size corners | ESC: Exit";
        } else {
            helpText = "ESC: Exit";
        }
        cv::Size helpSize = cv::getTextSize(helpText, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, 0);
        int helpX = displayFrame.cols - helpSize.width - 10;
        
        cv::putText(displayFrame, helpText,
                   cv::Point(helpX, displayFrame.rows - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);
        
        // ===== ПРИМЕНЕНИЕ МАСШТАБИРОВАНИЯ (screenSizeFactor) =====
        cv::Mat finalDisplayFrame = displayFrame;
        float factor = screenSizeFactor.load();

        if (factor != 1.0f) {
            int newWidth = static_cast<int>(displayFrame.cols * factor);
            int newHeight = static_cast<int>(displayFrame.rows * factor);
            
            cv::resize(displayFrame, finalDisplayFrame, 
                    cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);
            
            // Обновляем размер окна, если factor изменился
            static float lastFactor = 1.0f;
            if (factor != lastFactor) {
                // ИСПРАВЛЕНО: используем исходные размеры кадра
                int windowWidth = static_cast<int>(CAP_PROP_FRAME_WIDTH * factor);
                int windowHeight = static_cast<int>(CAP_PROP_FRAME_HEIGHT * factor);
                cv::resizeWindow("Motion Detection with Exclusion", windowWidth, windowHeight);
                std::cout << "Window resized to: " << windowWidth << "x" << windowHeight 
                        << " (factor: " << factor << ")" << std::endl;
                lastFactor = factor;
            }
        }
        
        // ===== ОТОБРАЖЕНИЕ И ОБРАБОТКА КЛАВИШ =====
        cv::imshow("Motion Detection with Exclusion", finalDisplayFrame);
        int key = cv::waitKey(1);
        
        // Обработка клавиш
        if (key == 27) {  // ESC
            running = false;
        } else if (key == 't' || key == 'T') {  // T для быстрого переключения тестового режима
            testModeActive = !testModeActive.load();
            std::cout << "Test mode " << (testModeActive.load() ? "ENABLED" : "DISABLED") << std::endl;
        }
        
        // Обновление FPS
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastFpsTime).count();
        
        if (elapsed >= 100) {
            if (elapsed > 0) {
                currentFPS.store(static_cast<int>(frameCounter * 1000.0f / elapsed));
            }
            frameCounter = 0;
            lastFpsTime = now;
            
            // Обновляем заголовок окна
            std::string windowTitle = "Motion Detection";
            if (robotProcessingActive.load()) {
                windowTitle += " [FIGHT ACTIVE]";
            }
            windowTitle += " - FPS: " + std::to_string(currentFPS.load());
            //windowTitle += " | Scale: " + std::to_string(screenSizeFactor.load());
            cv::setWindowTitle("Motion Detection with Exclusion", windowTitle);
        }
    }
    
    robotController.setChannel1(50.0f);
    robotController.setChannel2(50.0f);
    robotController.setChannel3(0.0f);

    robotController.sendToArduino();

    std::cout << "\nSaving configuration..." << std::endl;
    configManager.saveConfig(
        panel,
        exclusionRadius,
        attackRadius,
        robotController,
        arenaCenterX,
        arenaCenterY,
        arenaSizeX1,
        arenaSizeY1,
        arenaSizeX2,
        arenaSizeY2,
        motionDetector,
        detector  // Добавлен ArucoDetector
    );

    
    // Финализация
    cv::destroyWindow("Motion Detection with Exclusion");
    panel.stop();
    cap.release();
    
    auto programEnd = std::chrono::steady_clock::now();
    auto programDuration = std::chrono::duration_cast<std::chrono::seconds>(
        programEnd - programStart).count();
    
    std::cout << "\n=== Program finished ===" << std::endl;
    std::cout << "Total captured frames: " << capturedFrames << std::endl;
    std::cout << "Average FPS: " << (programDuration > 0 ? capturedFrames / programDuration : 0) << std::endl;
    std::cout << "Program duration: " << programDuration << " seconds" << std::endl;
    std::cout << "Final channel values: " << robotController.getChannelsInfo() << std::endl;
    std::cout << "Final PID parameters: " << robotController.getPIDParameters() << std::endl;
    std::cout << "Final exclusion radius: " << exclusionRadius.load() << std::endl;
    std::cout << "Final attack radius: " << attackRadius.load() << std::endl;
    std::cout << "Final arena center: (" << arenaCenterX.load() << ", " << arenaCenterY.load() << ")" << std::endl;
    std::cout << "Final arena size: (" << arenaSizeX1.load() << ", " << arenaSizeY1.load() << ") - (" 
              << arenaSizeX2.load() << ", " << arenaSizeY2.load() << ")" << std::endl;
    std::cout << "Final screen size factor: " << screenSizeFactor.load() << std::endl;
    
    if (controlPanelThread.joinable()) {
        controlPanelThread.join();
    }
    
    return 0;
}