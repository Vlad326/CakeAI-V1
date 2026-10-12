// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <atomic>
#include <fstream>
#include "ControlPanel.h"
#include "RobotController.h"
#include "MotionDetector.h"
#include "ArucoDetector.h"

extern std::atomic<int> offsetPixels;

class ConfigManager {
private:
    std::string configFile_;
    
public:
    ConfigManager(const std::string& filename = "config.ini") 
        : configFile_(filename) {}
    
    void saveConfig(
        ControlPanel& panel,
        const std::atomic<int>& exclusionRadius,
        const std::atomic<int>& attackRadius,
        const RobotController& robotController,
        const std::atomic<int>& arenaCenterX,
        const std::atomic<int>& arenaCenterY,
        const std::atomic<int>& arenaSizeX1,
        const std::atomic<int>& arenaSizeY1,
        const std::atomic<int>& arenaSizeX2,
        const std::atomic<int>& arenaSizeY2,
        const MotionDetector& motionDetector,
        const ArucoDetector& arucoDetector);
    
    bool loadConfig(
        ControlPanel& panel,
        std::atomic<int>& exclusionRadius,
        std::atomic<int>& attackRadius,
        RobotController& robotController,
        std::atomic<int>& arenaCenterX,
        std::atomic<int>& arenaCenterY,
        std::atomic<int>& arenaSizeX1,
        std::atomic<int>& arenaSizeY1,
        std::atomic<int>& arenaSizeX2,
        std::atomic<int>& arenaSizeY2,
        MotionDetector& motionDetector,
        ArucoDetector& arucoDetector);
};

#endif // CONFIG_MANAGER_H