// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include "PIDController.h"
#include "ArduinoController.h"
#include <atomic>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <deque>
#include <mutex>
#include <chrono>
#include <opencv2/opencv.hpp>

class RobotController {
private:
    PIDController pidController;
    std::shared_ptr<ArduinoController> arduinoController;
    std::atomic<float> ch1;
    std::atomic<float> ch2;
    std::atomic<float> ch3;
    float baseValue;
    float minValue;
    float maxValue;
    bool markerDetected;
    bool motionDetected;
    float currentOffset;
    float krlCoefficient;
    float kfbCoefficient;

    std::atomic<int> centerAfterAttack;
    std::atomic<int> centerTriggerThreshold;
    std::atomic<int> enemyTriggerThreshold;
    std::atomic<int> enemyTriggerTimeout;
    std::atomic<int> reverseAngleThreshold;
    std::atomic<int> useReverse;
    
    std::atomic<float> InPlaceSpeedRotation;
    std::atomic<float> InPlaceSpeedWeapon;

    std::atomic<float> angleMinExtraSpeed;
    std::atomic<float> angleMaxExtraSpeed;

    std::atomic<int> robotState_{0};

    /*
    State 0 - идти в центр
    State 1 - ехать в противника
    State 2 - пропал противник, едем в центр и кружимся
    */

    std::atomic<long long> lastTimeStateUpdate_{0};
    std::atomic<bool> reverse_{false};
    std::atomic<int> attack_count{0};
    
    // Флаг для вращения на месте (заменили state 3)
    std::atomic<bool> rotatingInPlace_{false};

    // Для записи истории позиций/поворотов маркера
    struct MarkerPositionRecord {
        cv::Point position;
        double rotation;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::deque<MarkerPositionRecord> positionHistory;
    mutable std::mutex historyMutex; // mutable для const методов
    
    // Настройки записи и сравнения
    std::atomic<int> maxHistorySize{30}; // Максимальное количество записей
    std::atomic<int> checkBackSteps{3}; // Сколько записей назад проверять (3 = 3 секунды при 1 FPS)
    std::atomic<int> checkIntervalMs{1000}; // Интервал проверки в миллисекундах (1000 = 1 FPS)
    
    std::chrono::steady_clock::time_point lastCheckTime;
    
    // Переменная для хранения последнего результата проверки движения
    std::atomic<bool> robotIsMoving_{true};
    
    // Для плавного переключения скорости оружия при застревании
    std::chrono::steady_clock::time_point weaponCycleStartTime;
    std::atomic<int> weaponCycleDurationMs{4000}; // Полный цикл: 2 сек вниз + 2 сек вверх
    
    // Метод для записи и сравнения позиций маркера
    bool checkAndCompareMarker(float markerAngle, int markerX, int markerY);
    
    // Для плавного старта CH3
    std::atomic<bool> ch3StartPending{false};
    std::chrono::steady_clock::time_point ch3StartTime;
    std::atomic<int> ch3RampUpDurationMs{3000}; // 3 секунды по умолчанию
    std::atomic<float> ch3TargetValue{100.0f}; // Целевое значение CH3

public:
    RobotController(std::shared_ptr<ArduinoController> arduino, 
                   float kp = 0.1f, float ki = 0.1f, float kd = 0.1f,
                   float base = 50.0f, float min = 0.0f, float max = 100.0f);
    
    RobotController(float kp = 0.1f, float ki = 0.1f, float kd = 0.1f,
                   float base = 50.0f, float min = 0.0f, float max = 100.0f);
    
    ~RobotController() = default;
    
    // НОВЫЙ МЕТОД: получение side по ID (1 - normal, 0 - ubnormal)
    int getMarkerSide(int markerId) const {
        if (markerId == 0) return 1;
        if (markerId == 1) return 0;
        return -1;
    }
    
    void SwapInvertChannels(float& ch1, float& ch2) const {
        float temp = ch1;
        ch1 = 100.0f - ch2;
        ch2 = 100.0f - temp;
    }

    void process(float markerAngle, int centerX, int centerY,
                int markerX, int markerY, int motionX, int motionY, int markerId = -1);

    void setMarkerDetected(bool detected) { markerDetected = detected; }
    bool isMarkerDetected() const { return markerDetected; }
    
    void setMotionDetected(bool detected) { motionDetected = detected; }
    bool isMotionDetected() const { return motionDetected; }
    
    // Геттер для состояния движения робота
    bool isRobotMoving() const { return robotIsMoving_.load(); }
    
    // Геттер для вращения на месте
    bool isRotatingInPlace() const { return rotatingInPlace_.load(); }
    
    void setArduinoController(std::shared_ptr<ArduinoController> arduino) {
        arduinoController = arduino;
    }
    
    float getKp() const {
        double kp, ki, kd;
        pidController.getParameters(kp, ki, kd);
        return static_cast<float>(kp);
    }
    
    float getKi() const { 
        double kp, ki, kd;
        pidController.getParameters(kp, ki, kd);
        return static_cast<float>(ki);
    }
    
    float getKd() const {
        double kp, ki, kd;
        pidController.getParameters(kp, ki, kd);
        return static_cast<float>(kd);
    }
    
    float getKrlCoefficient() const { return krlCoefficient; }
    float getKfbCoefficient() const { return kfbCoefficient; }
    int getCenterAfterAttack() const { return centerAfterAttack.load(); }
    int getCenterTriggerThreshold() const { return centerTriggerThreshold.load(); }
    int getEnemyTriggerThreshold() const { return enemyTriggerThreshold.load(); }
    int getEnemyTriggerTimeout() const { return enemyTriggerTimeout.load(); }
    int getReverseAngleThreshold() const { return reverseAngleThreshold.load(); }
    int getUseReverse() const { return useReverse.load(); }
    
    float getAngleMinExtraSpeed() const { return angleMinExtraSpeed.load(); }
    float getAngleMaxExtraSpeed() const { return angleMaxExtraSpeed.load(); }

    float getInPlaceSpeedRotation() const { return InPlaceSpeedRotation.load(); }
    float getInPlaceSpeedWeapon() const { return InPlaceSpeedWeapon.load(); }

    int getState() const { return robotState_.load(); }
    
    void setState(int state) {
        robotState_.store(state);
        auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        lastTimeStateUpdate_.store(currentTimeMs);
    }

    long long getlastTimeStateUpdate() const { return lastTimeStateUpdate_.load(); }
    
    void setKrlCoefficient(float krl) { krlCoefficient = krl; }
    void setKfbCoefficient(float kfb) { kfbCoefficient = kfb; }
    void setCenterAfterAttack(int value) { centerAfterAttack.store(value); }
    void setCenterTriggerThreshold(int threshold) { centerTriggerThreshold.store(threshold); }
    void setEnemyTriggerThreshold(int threshold) { enemyTriggerThreshold.store(threshold); }
    void setEnemyTriggerTimeout(int timeout) { enemyTriggerTimeout.store(timeout); }
    void setReverseAngleThreshold(int threshold) { reverseAngleThreshold.store(threshold); }
    void setUseReverse(int value) { useReverse.store(value); }
    
    void setAngleMinExtraSpeed(float angle) { angleMinExtraSpeed.store(angle); }
    void setAngleMaxExtraSpeed(float angle) { angleMaxExtraSpeed.store(angle); }
    
    void setInPlaceSpeedRotation(float speedRotation) { InPlaceSpeedRotation.store(speedRotation); }
    void setInPlaceSpeedWeapon(float speedWeapon) { InPlaceSpeedWeapon.store(speedWeapon); }
    
    void setPIDParameters(float kp, float ki, float kd) { pidController.setParameters(kp, ki, kd); }
    void setKp(float kp) {
        double currentKp, currentKi, currentKd;
        pidController.getParameters(currentKp, currentKi, currentKd);
        pidController.setParameters(kp, currentKi, currentKd);
    }
    
    void setKi(float ki) {
        double currentKp, currentKi, currentKd;
        pidController.getParameters(currentKp, currentKi, currentKd);
        pidController.setParameters(currentKp, ki, currentKd);
    }
    
    void setKd(float kd) {
        double currentKp, currentKi, currentKd;
        pidController.getParameters(currentKp, currentKi, currentKd);
        pidController.setParameters(currentKp, currentKi, kd);
    }
    
    void setChannelLimits(float min, float max) { minValue = min; maxValue = max; }
    void setBaseValue(float base) { baseValue = base; }
    
    // Методы для настройки записи и сравнения позиций
    void setCheckIntervalMs(int intervalMs) { 
        if (intervalMs > 0) checkIntervalMs.store(intervalMs); 
    }
    
    int getCheckIntervalMs() const { return checkIntervalMs.load(); }
    
    void setCheckBackSteps(int steps) { 
        if (steps > 0) checkBackSteps.store(steps); 
    }
    
    int getCheckBackSteps() const { return checkBackSteps.load(); }
    
    void setMaxHistorySize(int size) { 
        if (size > 0) maxHistorySize.store(size); 
    }
    
    int getMaxHistorySize() const { return maxHistorySize.load(); }
    
    // Метод для получения текущей истории
    std::vector<MarkerPositionRecord> getPositionHistory() const {
        std::lock_guard<std::mutex> lock(historyMutex);
        return std::vector<MarkerPositionRecord>(positionHistory.begin(), positionHistory.end());
    }
    
    // Метод для очистки истории
    void clearPositionHistory() {
        std::lock_guard<std::mutex> lock(historyMutex);
        positionHistory.clear();
        lastCheckTime = std::chrono::steady_clock::now();
        robotIsMoving_.store(true); // Сбрасываем состояние движения при очистке истории
        rotatingInPlace_.store(false); // Сбрасываем вращение на месте
        weaponCycleStartTime = std::chrono::steady_clock::now(); // Сбрасываем цикл оружия
    }
    
    void reset() {
        pidController.reset();
        ch1 = baseValue;
        ch2 = baseValue;
        ch3 = 0.0f;
        markerDetected = false;
        motionDetected = false;
        currentOffset = 0.0f;
        reverse_.store(false);
        attack_count.store(0);
        robotState_.store(0);
        rotatingInPlace_.store(false);
        robotIsMoving_.store(true);
        clearPositionHistory();
        
        // Сбрасываем состояние плавного старта
        ch3StartPending = false;
        
        sendToArduino();
    }
    
    float getChannel1() const { return ch1.load(); }
    float getChannel2() const { return ch2.load(); }
    float getChannel3() const { return ch3.load(); }
    
    void setChannel1(float value) {
        value = std::max(minValue, std::min(maxValue, value));
        ch1 = value;
    }
    
    void setChannel2(float value) {
        value = std::max(minValue, std::min(maxValue, value));
        ch2 = value;
    }
    
    void setChannel3(float value) {
        value = std::max(minValue, std::min(maxValue, value));
        ch3 = value;
    }
    
    void sendToArduino() {
        if (arduinoController && arduinoController->is_connected()) {
            arduinoController->set_motors(
                static_cast<int>(ch1.load()),
                static_cast<int>(ch2.load()),
                static_cast<int>(ch3.load())
            );
        }
    }
    
    float getCurrentOffset() const { return currentOffset; }
    
    std::string getPIDParameters() const {
        double kp, ki, kd;
        pidController.getParameters(kp, ki, kd);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "PID: (" << kp << ", " << ki << ", " << kd << ")";
        ss << " KRL: " << krlCoefficient << " KFB: " << kfbCoefficient;
        ss << " CAA: " << centerAfterAttack.load();
        return ss.str();
    }
    
    std::string getChannelsInfo() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "CH1: " << ch1.load() 
           << " | CH2: " << ch2.load() 
           << " | CH3: " << ch3.load();
        return ss.str();
    }
    
    // Метод для получения информации о проверке позиций
    std::string getPositionCheckInfo() const {
        std::stringstream ss;
        ss << "Position checking: Interval=" << checkIntervalMs.load() << "ms, ";
        ss << "Compare with=" << checkBackSteps.load() << " records back, ";
        ss << "Max records=" << maxHistorySize.load();
        return ss.str();
    }
    
    // Метод для получения информации о состоянии движения робота
    std::string getRobotMovementInfo() const {
        std::stringstream ss;
        ss << "Robot is moving: " << (robotIsMoving_.load() ? "YES" : "NO");
        ss << " | Rotating in place: " << (rotatingInPlace_.load() ? "YES" : "NO");
        return ss.str();
    }
    
    // Метод для настройки длительности цикла оружия
    void setWeaponCycleDurationMs(int durationMs) {
        if (durationMs > 0) weaponCycleDurationMs.store(durationMs);
    }
    
    int getWeaponCycleDurationMs() const { return weaponCycleDurationMs.load(); }
    
    // ===== МЕТОДЫ ДЛЯ ПЛАВНОГО СТАРТА CH3 =====
    
    // Запустить плавное включение CH3
    void startCh3RampUp(float targetValue = 100.0f, int durationMs = 3000) {
        ch3TargetValue = targetValue;
        ch3RampUpDurationMs = durationMs;
        ch3StartPending = true;
        ch3StartTime = std::chrono::steady_clock::now();
        std::cout << "CH3 ramp-up started: target=" << targetValue 
                  << "%, duration=" << durationMs << "ms" << std::endl;
    }
    
    // Проверить и выполнить плавный старт CH3
    void updateCh3RampUp() {
        if (!ch3StartPending.load()) return;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - ch3StartTime).count();
        
        int duration = ch3RampUpDurationMs.load();
        
        if (elapsed >= duration) {
            // Достигли целевого значения
            ch3 = ch3TargetValue.load();
            ch3StartPending = false;
            std::cout << "CH3 ramp-up completed, value=" << ch3.load() << std::endl;
        } else {
            // Плавно увеличиваем
            float progress = static_cast<float>(elapsed) / duration;
            ch3 = progress * ch3TargetValue.load();
        }
    }
    
    // Настройка длительности разгона CH3
    void setCh3RampUpDuration(int durationMs) {
        if (durationMs > 0) {
            ch3RampUpDurationMs = durationMs;
            std::cout << "CH3 ramp-up duration set to " << durationMs << "ms" << std::endl;
        }
    }
    
    // Получение длительности разгона CH3
    int getCh3RampUpDuration() const {
        return ch3RampUpDurationMs.load();
    }
    
    // Проверка, выполняется ли сейчас плавный старт
    bool isCh3RampingUp() const {
        return ch3StartPending.load();
    }
};

#endif // ROBOT_CONTROLLER_H