// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "RobotController.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <deque>
#include <mutex>
#include <chrono>
#include <iostream>

RobotController::RobotController(std::shared_ptr<ArduinoController> arduino,
                               float kp, float ki, float kd,
                               float base, float min, float max)
    : pidController(kp, ki, kd)
    , arduinoController(arduino)
    , ch1(base)
    , ch2(base)
    , ch3(0.0f)
    , baseValue(base)
    , minValue(min)
    , maxValue(max)
    , markerDetected(false)
    , motionDetected(false)
    , currentOffset(0.0f)
    , krlCoefficient(1.0f)
    , kfbCoefficient(1.0f)
    , centerAfterAttack(1)
    , centerTriggerThreshold(150)
    , enemyTriggerThreshold(150)
    , enemyTriggerTimeout(2000)
    , reverseAngleThreshold(180)
    , useReverse(1)
    , angleMinExtraSpeed(90.0f)
    , angleMaxExtraSpeed(0.0f)
    , lastCheckTime(std::chrono::steady_clock::now())
    , robotIsMoving_(true)
    , rotatingInPlace_(false)
    , weaponCycleStartTime(std::chrono::steady_clock::now())
    , ch3StartPending(false)
    , ch3RampUpDurationMs(3000)
    , ch3TargetValue(100.0f)
{
}

RobotController::RobotController(float kp, float ki, float kd,
                               float base, float min, float max)
    : pidController(kp, ki, kd)
    , arduinoController(nullptr)
    , ch1(base)
    , ch2(base)
    , ch3(0.0f)
    , baseValue(base)
    , minValue(min)
    , maxValue(max)
    , markerDetected(false)
    , motionDetected(false)
    , currentOffset(0.0f)
    , krlCoefficient(1.0f)
    , kfbCoefficient(1.0f)
    , centerAfterAttack(1)
    , centerTriggerThreshold(150)
    , enemyTriggerThreshold(150)
    , enemyTriggerTimeout(2000)
    , reverseAngleThreshold(180)
    , useReverse(1)
    , angleMinExtraSpeed(90.0f)
    , angleMaxExtraSpeed(0.0f)
    , lastCheckTime(std::chrono::steady_clock::now())
    , robotIsMoving_(true)
    , rotatingInPlace_(false)
    , weaponCycleStartTime(std::chrono::steady_clock::now())
    , ch3StartPending(false)
    , ch3RampUpDurationMs(3000)
    , ch3TargetValue(100.0f)
{
}

bool RobotController::checkAndCompareMarker(float markerAngle, int markerX, int markerY) {
    auto now = std::chrono::steady_clock::now();
    
    // Проверяем, прошло ли достаточно времени с последней проверки
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastCheckTime).count();
    
    if (elapsed >= checkIntervalMs.load()) {
        std::lock_guard<std::mutex> lock(historyMutex);
        
        // Добавляем текущую запись
        MarkerPositionRecord currentRecord;
        currentRecord.position = cv::Point(markerX, markerY);
        currentRecord.rotation = markerAngle;
        currentRecord.timestamp = now;
        
        positionHistory.push_back(currentRecord);
        
        // Удаляем старые записи, если превысили максимальный размер
        while (positionHistory.size() > static_cast<size_t>(maxHistorySize.load())) {
            positionHistory.pop_front();
        }
        
        // Проверяем, есть ли достаточно записей для сравнения
        int stepsBack = checkBackSteps.load();
        
        // Если еще нету 3 секунд назад (или заданного количества записей), возвращаем true
        if (positionHistory.size() <= static_cast<size_t>(stepsBack)) {
            lastCheckTime = now;
            robotIsMoving_.store(true); // Нет достаточно истории - считаем что робот движется
            rotatingInPlace_.store(false); // Не вращаемся на месте
            return true;
        }
        
        // Есть достаточно записей - выполняем сравнение
        size_t currentIdx = positionHistory.size() - 1;
        size_t compareIdx = positionHistory.size() - 1 - stepsBack;
        
        if (compareIdx < positionHistory.size()) {
            const MarkerPositionRecord& current = positionHistory[currentIdx];
            const MarkerPositionRecord& compare = positionHistory[compareIdx];
            
            // Вычисляем разницу в позиции
            double positionDifference = std::sqrt(
                std::pow(current.position.x - compare.position.x, 2) +
                std::pow(current.position.y - compare.position.y, 2)
            );
            
            // Вычисляем разницу в угле
            double rotationDifference = std::abs(current.rotation - compare.rotation);
            // Нормализуем разницу углов в диапазоне [0, 180]
            while (rotationDifference > 180.0) {
                rotationDifference = std::abs(rotationDifference - 360.0);
            }
            
            // Вычисляем временную разницу
            auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(
                current.timestamp - compare.timestamp).count() / 1000.0;
            
            // Проверяем условие: если изменения маленькие, возвращаем false
            if (rotationDifference < 10 && positionDifference < 30) {
                // Малые изменения - возвращаем false
                robotIsMoving_.store(false);
                rotatingInPlace_.store(false); // Пока не вращаемся на месте
                lastCheckTime = now;
                return false;
            }
            
            // Выводим в консоль только при значительных изменениях
            std::cout << "\n=== MARKER POSITION CHECK ===" << std::endl;
            std::cout << "Time difference: " << std::fixed << std::setprecision(1) 
                    << timeDifference << " seconds" << std::endl;
            std::cout << "Current position: (" << current.position.x << ", " 
                    << current.position.y << ")" << std::endl;
            std::cout << "Position " << stepsBack << " records back: (" << compare.position.x << ", " 
                    << compare.position.y << ")" << std::endl;
            std::cout << "Position difference: " << std::fixed << std::setprecision(2) 
                    << positionDifference << " pixels" << std::endl;
            std::cout << "Current rotation: " << std::fixed << std::setprecision(1) 
                    << current.rotation << "°" << std::endl;
            std::cout << "Rotation " << stepsBack << " records back: " << std::fixed << std::setprecision(1) 
                    << compare.rotation << "°" << std::endl;
            std::cout << "Rotation difference: " << std::fixed << std::setprecision(1) 
                    << rotationDifference << "°" << std::endl;
            std::cout << "Total records in history: " << positionHistory.size() 
                    << " of " << maxHistorySize.load() << std::endl;
            std::cout << "Robot is moving: YES" << std::endl;
            std::cout << "===================================" << std::endl;
            
            robotIsMoving_.store(true);
            rotatingInPlace_.store(false); // Не вращаемся на месте
        }
        
        lastCheckTime = now;
        return true;
    }
    
    // Если проверка не выполнялась (не прошел интервал), используем текущее значение
    return robotIsMoving_.load();
}

void RobotController::process(float markerAngle, int centerX, int centerY,
                            int markerX, int markerY, int motionX, int motionY, int markerId) {
    
    // Обновляем плавный старт CH3 если нужно
    updateCh3RampUp();
    
    markerDetected = (markerX >= 0 && markerY >= 0);
    motionDetected = (motionX >= 0 && motionY >= 0);
    
    bool robotIsMoving = true; // По умолчанию считаем что движется

    // Записываем и сравниваем позиции маркера (только если маркер обнаружен)
    if (markerDetected) {
        robotIsMoving = checkAndCompareMarker(markerAngle, markerX, markerY);
    }
    
    if (motionDetected) {
        if (getState() == 2) {
            setState(1);
        }
    } else {
        setState(2);
    }

    std::cerr << "Robot moving: " << (robotIsMoving ? "YES" : "NO") << std::endl;
    std::cerr << "State: " << getState() << std::endl;
    std::cerr << "Rotating in place: " << (rotatingInPlace_.load() ? "YES" : "NO") << std::endl;

    // Логика вращения на месте (отдельная переменная вместо state 3)
    bool shouldRotateInPlace = false;
    
    // Условие 1: Робот не движется и маркер обнаружен
    if (!robotIsMoving && markerDetected) {
        shouldRotateInPlace = true;
    }
    
    // Условие 2: Маркер не обнаружен
    if (!markerDetected) {
        shouldRotateInPlace = true;
    }
    
    // Условие 3: State 2 и мы рядом с центром
    float distance_to_center = sqrt(pow(centerX - markerX, 2) + pow(centerY - markerY, 2));
    if (getState() == 2 && distance_to_center < centerTriggerThreshold.load()) {
        shouldRotateInPlace = true;
        std::cerr << "KRYTIMSA (near center, rotating in place)" << std::endl;
    }
    
    // Устанавливаем флаг вращения на месте
    rotatingInPlace_.store(shouldRotateInPlace);
    
    // Если нужно вращаться на месте
    if (shouldRotateInPlace) {
        // Расчет скоростей для вращения на месте
        float rotationSpeed = getInPlaceSpeedRotation();
        float weaponSpeed = getInPlaceSpeedWeapon();
        
        // Получаем базовую скорость оружия
        float baseWeaponSpeed = weaponSpeed;
        
        // Если робот НЕ движется, применяем плавное циклическое переключение скорости оружия
        if (!robotIsMoving) {
            auto now = std::chrono::steady_clock::now();
            auto cycleDuration = weaponCycleDurationMs.load();
            
            // Вычисляем прогресс в текущем цикле (0.0 - 1.0)
            auto timeInCycle = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - weaponCycleStartTime).count() % cycleDuration;
            
            float cycleProgress = static_cast<float>(timeInCycle) / cycleDuration;
            
            // Полный цикл: 0.0-0.5: от 100% до 50%, 0.5-1.0: от 50% до 100%
            // Используем треугольную волну для плавного перехода
            float speedMultiplier;
            if (cycleProgress < 0.5f) {
                // Первая половина цикла: плавно уменьшаем от 1.0 до 0.5
                speedMultiplier = 1.0f - (cycleProgress * 1.0f); // 1.0 -> 0.5
            } else {
                // Вторая половина цикла: плавно увеличиваем от 0.5 до 1.0
                speedMultiplier = (cycleProgress - 0.5f) * 1.0f + 0.5f; // 0.5 -> 1.0
            }
            
            // Применяем множитель к скорости оружия
            weaponSpeed = baseWeaponSpeed * speedMultiplier;
            
            // Для отладки
            std::cerr << "Weapon cycle: progress=" << cycleProgress 
                     << ", multiplier=" << speedMultiplier 
                     << ", final speed=" << weaponSpeed * 100 << "%" << std::endl;
        } else {
            // Если робот движется, сбрасываем цикл оружия и используем полную скорость
            weaponCycleStartTime = std::chrono::steady_clock::now();
            weaponSpeed = baseWeaponSpeed; // Полная скорость
        }
        
        // Устанавливаем значения каналов для вращения
        ch1 = 50 - (rotationSpeed / 2 * 100);
        ch2 = 50 + (rotationSpeed / 2 * 100);
        
        // Для CH3 используем плавный старт если он активен, иначе обычное значение
        if (!ch3StartPending.load()) {
            ch3 = weaponSpeed * 100;
        }
        
        currentOffset = 0.0f;
        
        sendToArduino();
        return; // Прерываем дальнейшую обработку
    }
    
    // Если робот движется (не застрял) - сбрасываем цикл оружия
    if (robotIsMoving) {
        weaponCycleStartTime = std::chrono::steady_clock::now();
    }
    
    // Остальная логика если робот движется и не вращается на месте
    float distance_to_enemy = sqrt(pow(motionX - markerX, 2) + pow(motionY - markerY, 2));
    
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    long long timeDiff = currentTimeMs - getlastTimeStateUpdate();
    
    if (centerAfterAttack.load() == 1) {
        if (distance_to_center < centerTriggerThreshold.load() && getState() == 0) {
            setState(1);
            reverse_.store(false);
        }
        
        if (distance_to_enemy < enemyTriggerThreshold.load() && getState() == 1) {
            setState(0);
            attack_count.store(attack_count.load() + 1);
            std::cerr << "setState(0)" << std::endl;
        }
    }
    
    float angle_to_target;
    if (getState() == 0 && timeDiff > enemyTriggerTimeout.load()) {
        angle_to_target = atan2(centerY - markerY, centerX - markerX) * 180.0f / 3.141592653589f;
        if (attack_count > 0) reverse_.store(true);
        std::cerr << "Angle_to_target=center" << std::endl;
    } else if (getState() == 2) {
        angle_to_target = atan2(centerY - markerY, centerX - markerX) * 180.0f / 3.141592653589f;
    } else {
        angle_to_target = atan2(motionY - markerY, motionX - markerX) * 180.0f / 3.141592653589f;
    }
    
    float corrected_marker_angle = markerAngle - 90.0f;
    if (corrected_marker_angle > 180) corrected_marker_angle -= 360;
    if (corrected_marker_angle < -180) corrected_marker_angle += 360;
    
    currentOffset = angle_to_target - corrected_marker_angle;
    if (currentOffset > 180) currentOffset -= 360;
    if (currentOffset < -180) currentOffset += 360;
    
    if (std::abs(currentOffset) < reverseAngleThreshold.load() / 2 && reverse_.load() == 1) {
        reverse_.store(false);
    }
    
    if (reverse_.load() == 1) {
        currentOffset += 180;
        if (currentOffset > 180) currentOffset -= 360;
    }
    
    double pid_output = pidController.calculate(0.0, currentOffset);
    pid_output *= krlCoefficient;
    
    float new_ch1 = baseValue - static_cast<float>(pid_output);
    float new_ch2 = baseValue + static_cast<float>(pid_output);
    float new_ch3 = 100.0f;
    
    float angleMin = angleMinExtraSpeed.load();
    float angleMax = angleMaxExtraSpeed.load();
    float absOffset = std::abs(currentOffset);
    
    if (angleMin > angleMax) {
        if (absOffset <= angleMin) {
            float extraSpeed;
            if (absOffset <= angleMax) {
                extraSpeed = 100.0f;
            } else {
                float range = angleMin - angleMax;
                if (range > 0) {
                    extraSpeed = 100.0f - ((absOffset - angleMax) * 100.0f / range);
                } else {
                    extraSpeed = 0.0f;
                }
            }
            
            extraSpeed *= kfbCoefficient;
            new_ch1 += extraSpeed;
            new_ch2 += extraSpeed;
        }
    } else {
        if (absOffset < 90) {
            float extraSpeed = 100.0f - (absOffset * 100.0f / 90.0f);
            extraSpeed *= kfbCoefficient;
            new_ch1 += extraSpeed;
            new_ch2 += extraSpeed;
        }
    }
    
    new_ch1 = std::max(minValue, std::min(maxValue, new_ch1));
    new_ch2 = std::max(minValue, std::min(maxValue, new_ch2));
    
    if (reverse_.load() == 1 && useReverse.load() == 1) {
        float temp = new_ch1;
        new_ch1 = 100.0f - new_ch2;
        new_ch2 = 100.0f - temp;
    }
    
    // Реверс движения, если робот верх-ногами
    if (markerDetected && markerId >= 0) {
        std::cout << "RobotController: Marker ID=" << markerId 
                  << " Side=" << getMarkerSide(markerId) << std::endl;
        if (getMarkerSide(markerId) == 0) { // Верх-ногами
            SwapInvertChannels(new_ch1, new_ch2);
        }
    }

    ch1 = new_ch1;
    ch2 = new_ch2;
    
    // Устанавливаем CH3 только если нет активного плавного старта
    if (!ch3StartPending.load()) {
        ch3 = new_ch3;
    }

    sendToArduino();
}