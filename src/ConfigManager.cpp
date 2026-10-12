// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "ConfigManager.h"
#include <iostream>
#include <string>
#include <stdexcept>

std::atomic<int> offsetPixels{20};

void ConfigManager::saveConfig(
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
    const ArucoDetector& arucoDetector) {
    
    std::ofstream file(configFile_);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot save config to " << configFile_ << std::endl;
        return;
    }
    
    std::string kpStr = panel.getKpValue();
    std::string kiStr = panel.getKiValue();
    std::string kdStr = panel.getKdValue();
    std::string krlStr = panel.getKrlValue();
    std::string kfbStr = panel.getKfbValue();
    std::string centerAfterAttackStr = panel.getCenterAfterAttackValue();
    std::string centerTriggerThresholdStr = panel.getCenterTriggerThresholdValue();
    std::string enemyTriggerThresholdStr = panel.getEnemyTriggerThresholdValue();
    std::string enemyTriggerTimeoutStr = panel.getEnemyTriggerTimeoutValue();
    std::string reverseAngleThresholdStr = panel.getReverseAngleThresholdValue();
    std::string useReverseStr = panel.getUseReverseValue();
    std::string angleMinExtraSpeedStr = panel.getAngleMinExtraSpeedValue();
    std::string angleMaxExtraSpeedStr = panel.getAngleMaxExtraSpeedValue();
    std::string InPlaceSpeedRotationStr = panel.getInPlaceSpeedRotationValue();
    std::string InPlaceSpeedWeaponStr = panel.getInPlaceSpeedWeaponValue();
    std::string robot1Str = panel.getRobot1ExclusionValue();
    std::string arenaXStr = panel.getArenaXValue();
    std::string arenaYStr = panel.getArenaYValue();
    std::string sizeX1Str = panel.getArenaSizeX1Value();
    std::string sizeY1Str = panel.getArenaSizeY1Value();
    std::string sizeX2Str = panel.getArenaSizeX2Value();
    std::string sizeY2Str = panel.getArenaSizeY2Value();
    std::string motionThreshStr = panel.getMotionThresholdValue();
    std::string morphWidthStr = panel.getMorphKernelWidthValue();
    std::string morphHeightStr = panel.getMorphKernelHeightValue();
    std::string dilationStr = panel.getDilationIterationsValue();
    std::string erosionStr = panel.getErosionIterationsValue();
    std::string blurWidthStr = panel.getBlurKernelWidthValue();
    std::string blurHeightStr = panel.getBlurKernelHeightValue();
    std::string blurSigmaStr = panel.getBlurSigmaValue();
    std::string exposureStr = panel.getCameraExposureValue();
    std::string testModeStr = panel.getSwitchState() ? "1" : "0";
    std::string robot1OffsetStr = panel.getRobot1OffsetValue();
    
    // ArucoDetector параметры
    std::string minMarkerPerimeterRateStr = panel.getMinMarkerPerimeterRateValue();
    std::string maxMarkerPerimeterRateStr = panel.getMaxMarkerPerimeterRateValue();
    
    if (kpStr.empty()) kpStr = "0.10";
    if (kiStr.empty()) kiStr = "0.10";
    if (kdStr.empty()) kdStr = "0.10";
    if (krlStr.empty()) krlStr = "1.00";
    if (kfbStr.empty()) kfbStr = "1.00";
    if (centerAfterAttackStr.empty()) centerAfterAttackStr = "1";
    if (centerTriggerThresholdStr.empty()) centerTriggerThresholdStr = "150";
    if (enemyTriggerThresholdStr.empty()) enemyTriggerThresholdStr = "150";
    if (enemyTriggerTimeoutStr.empty()) enemyTriggerTimeoutStr = "2000";
    if (reverseAngleThresholdStr.empty()) reverseAngleThresholdStr = "180";
    if (useReverseStr.empty()) useReverseStr = "1";
    if (angleMinExtraSpeedStr.empty()) angleMinExtraSpeedStr = "90.0";
    if (angleMaxExtraSpeedStr.empty()) angleMaxExtraSpeedStr = "0.0";
    if (InPlaceSpeedRotationStr.empty()) InPlaceSpeedRotationStr = "0.5";
    if (InPlaceSpeedWeaponStr.empty()) InPlaceSpeedWeaponStr = "1.0";
    if (robot1Str.empty()) robot1Str = "150";
    if (arenaXStr.empty()) arenaXStr = "640";
    if (arenaYStr.empty()) arenaYStr = "512";
    if (sizeX1Str.empty()) sizeX1Str = "-1";
    if (sizeY1Str.empty()) sizeY1Str = "-1";
    if (sizeX2Str.empty()) sizeX2Str = "-1";
    if (sizeY2Str.empty()) sizeY2Str = "-1";
    if (motionThreshStr.empty()) motionThreshStr = "25";
    if (morphWidthStr.empty()) morphWidthStr = "5";
    if (morphHeightStr.empty()) morphHeightStr = "5";
    if (dilationStr.empty()) dilationStr = "2";
    if (erosionStr.empty()) erosionStr = "1";
    if (blurWidthStr.empty()) blurWidthStr = "5";
    if (blurHeightStr.empty()) blurHeightStr = "5";
    if (blurSigmaStr.empty()) blurSigmaStr = "1.5";
    if (exposureStr.empty()) exposureStr = "-9";
    if (robot1OffsetStr.empty()) robot1OffsetStr = "20";
    if (minMarkerPerimeterRateStr.empty()) minMarkerPerimeterRateStr = "0.05";
    if (maxMarkerPerimeterRateStr.empty()) maxMarkerPerimeterRateStr = "8.0";
 
    file << "[PanelValues]" << std::endl;
    file << "Robot1Exclusion=" << robot1Str << std::endl;
    file << "Robot1Offset=" << robot1OffsetStr << std::endl;
    file << "Kp=" << kpStr << std::endl;
    file << "Ki=" << kiStr << std::endl;
    file << "Kd=" << kdStr << std::endl;
    file << "KrlCoefficient=" << krlStr << std::endl;
    file << "KfbCoefficient=" << kfbStr << std::endl;
    file << "CenterAfterAttack=" << centerAfterAttackStr << std::endl;
    file << "CenterTriggerThreshold=" << centerTriggerThresholdStr << std::endl;
    file << "EnemyTriggerThreshold=" << enemyTriggerThresholdStr << std::endl;
    file << "EnemyTriggerTimeout=" << enemyTriggerTimeoutStr << std::endl;
    file << "ReverseAngleThreshold=" << reverseAngleThresholdStr << std::endl;
    file << "UseReverse=" << useReverseStr << std::endl;
    file << "AngleMinExtraSpeed=" << angleMinExtraSpeedStr << std::endl;
    file << "AngleMaxExtraSpeed=" << angleMaxExtraSpeedStr << std::endl;
    file << "InPlaceSpeedRotation=" << InPlaceSpeedRotationStr << std::endl;
    file << "InPlaceSpeedWeapon=" << InPlaceSpeedWeaponStr << std::endl;
    file << "ArenaX=" << arenaXStr << std::endl;
    file << "ArenaY=" << arenaYStr << std::endl;
    file << "ArenaSizeX1=" << sizeX1Str << std::endl;
    file << "ArenaSizeY1=" << sizeY1Str << std::endl;
    file << "ArenaSizeX2=" << sizeX2Str << std::endl;
    file << "ArenaSizeY2=" << sizeY2Str << std::endl;
    file << "MotionThreshold=" << motionThreshStr << std::endl;
    file << "MorphKernelWidth=" << morphWidthStr << std::endl;
    file << "MorphKernelHeight=" << morphHeightStr << std::endl;
    file << "DilationIterations=" << dilationStr << std::endl;
    file << "ErosionIterations=" << erosionStr << std::endl;
    file << "BlurKernelWidth=" << blurWidthStr << std::endl;
    file << "BlurKernelHeight=" << blurHeightStr << std::endl;
    file << "BlurSigma=" << blurSigmaStr << std::endl;
    file << "CameraExposure=" << exposureStr << std::endl;
    file << "TestMode=" << testModeStr << std::endl;
    
    // ArucoDetector параметры
    file << "MinMarkerPerimeterRate=" << minMarkerPerimeterRateStr << std::endl;
    file << "MaxMarkerPerimeterRate=" << maxMarkerPerimeterRateStr << std::endl;

    file.close();
    std::cout << "Config saved to " << configFile_ << std::endl;
}

bool ConfigManager::loadConfig(
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
    ArucoDetector& arucoDetector) {
    
    std::ifstream file(configFile_);
    if (!file.is_open()) {
        std::cout << "No config file found. Using default values." << std::endl;
        return false;
    }
    
    std::string line;
    bool success = true;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == '#') {
            continue;
        }
        
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            
            if (value.empty()) {
                continue;
            }

            std::cout << "Loading config: " << key << " = " << value << std::endl;

            try {
                if (key == "Robot1Exclusion") {
                    int val = std::stoi(value);
                    if (val > 0 && val <= 1000) {
                        exclusionRadius.store(val);
                        panel.setRobot1ExclusionValue(value);
                    }
                }
                else if (key == "Robot1Offset") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 1000) {
                        offsetPixels.store(val);
                        panel.setRobot1OffsetValue(value);
                    }
                }
                else if (key == "Kp") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKp(val);
                        panel.setKpValue(value);
                    }
                }
                else if (key == "Ki") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKi(val);
                        panel.setKiValue(value);
                    }
                }
                else if (key == "Kd") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKd(val);
                        panel.setKdValue(value);
                    }
                }
                else if (key == "FrRatio") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKrlCoefficient(val);
                        panel.setKrlValue(value);
                        std::cout << "Legacy FrRatio loaded as KrlCoefficient: " << val << std::endl;
                    }
                }
                else if (key == "KrlCoefficient") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKrlCoefficient(val);
                        panel.setKrlValue(value);
                    }
                }
                else if (key == "KfbCoefficient") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 10.0f) {
                        robotController.setKfbCoefficient(val);
                        panel.setKfbValue(value);
                    }
                }
                else if (key == "StaticState") {
                    float val = std::stof(value);
                    if (val >= 0 && val <= 1) {
                        robotController.setCenterAfterAttack(static_cast<int>(val));
                        panel.setCenterAfterAttackValue(value);
                        std::cout << "Legacy StaticState loaded as CenterAfterAttack: " << val << std::endl;
                    }
                }
                else if (key == "CenterAfterAttack") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 1) {
                        robotController.setCenterAfterAttack(val);
                        panel.setCenterAfterAttackValue(value);
                    }
                }
                else if (key == "CenterTriggerThreshold") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 1000) {
                        robotController.setCenterTriggerThreshold(val);
                        panel.setCenterTriggerThresholdValue(value);
                    }
                }
                else if (key == "EnemyTriggerThreshold") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 1000) {
                        robotController.setEnemyTriggerThreshold(val);
                        panel.setEnemyTriggerThresholdValue(value);
                    }
                }
                else if (key == "EnemyTriggerTimeout") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 10000) {
                        robotController.setEnemyTriggerTimeout(val);
                        panel.setEnemyTriggerTimeoutValue(value);
                    }
                }
                else if (key == "ReverseAngleThreshold") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 360) {
                        robotController.setReverseAngleThreshold(val);
                        panel.setReverseAngleThresholdValue(value);
                    }
                }
                else if (key == "UseReverse") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 1) {
                        robotController.setUseReverse(val);
                        panel.setUseReverseValue(value);
                    }
                }
                else if (key == "AngleMinExtraSpeed") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 180.0f) {
                        robotController.setAngleMinExtraSpeed(val);
                        panel.setAngleMinExtraSpeedValue(value);
                    }
                }
                else if (key == "AngleMaxExtraSpeed") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 180.0f) {
                        robotController.setAngleMaxExtraSpeed(val);
                        panel.setAngleMaxExtraSpeedValue(value);
                    }
                }
                else if (key == "InPlaceSpeedRotation") {
                    float val = std::stof(value);
                    robotController.setInPlaceSpeedRotation(val);
                    panel.setInPlaceSpeedRotationValue(value);
                }
                else if (key == "InPlaceSpeedWeapon") {
                    float val = std::stof(value);
                    robotController.setInPlaceSpeedWeapon(val);
                    panel.setInPlaceSpeedWeaponValue(value);
                }
                else if (key == "ArenaX") {
                    int val = std::stoi(value);
                    arenaCenterX.store(val);
                    panel.setArenaXValue(value);
                }
                else if (key == "ArenaY") {
                    int val = std::stoi(value);
                    arenaCenterY.store(val);
                    panel.setArenaYValue(value);
                }
                else if (key == "ArenaSizeX1") {
                    int val = std::stoi(value);
                    arenaSizeX1.store(val);
                    panel.setArenaSizeX1Value(value);
                }
                else if (key == "ArenaSizeY1") {
                    int val = std::stoi(value);
                    arenaSizeY1.store(val);
                    panel.setArenaSizeY1Value(value);
                }
                else if (key == "ArenaSizeX2") {
                    int val = std::stoi(value);
                    arenaSizeX2.store(val);
                    panel.setArenaSizeX2Value(value);
                }
                else if (key == "ArenaSizeY2") {
                    int val = std::stoi(value);
                    arenaSizeY2.store(val);
                    panel.setArenaSizeY2Value(value);
                }
                else if (key == "MotionThreshold") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 255) {
                        motionDetector.setThreshold(val);
                        panel.setMotionThresholdValue(value);
                    }
                }
                else if (key == "MorphKernelWidth") {
                    int val = std::stoi(value);
                    if (val >= 1 && val <= 31) {
                        int currentHeight = motionDetector.getMorphKernelHeight();
                        motionDetector.setMorphKernelSize(val, currentHeight);
                        panel.setMorphKernelWidthValue(value);
                    }
                }
                else if (key == "MorphKernelHeight") {
                    int val = std::stoi(value);
                    if (val >= 1 && val <= 31) {
                        int currentWidth = motionDetector.getMorphKernelWidth();
                        motionDetector.setMorphKernelSize(currentWidth, val);
                        panel.setMorphKernelHeightValue(value);
                    }
                }
                else if (key == "DilationIterations") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 10) {
                        motionDetector.setDilationIterations(val);
                        panel.setDilationIterationsValue(value);
                    }
                }
                else if (key == "ErosionIterations") {
                    int val = std::stoi(value);
                    if (val >= 0 && val <= 10) {
                        motionDetector.setErosionIterations(val);
                        panel.setErosionIterationsValue(value);
                    }
                }
                else if (key == "BlurKernelWidth") {
                    int val = std::stoi(value);
                    if (val >= 1 && val <= 31) {
                        int currentHeight = motionDetector.getBlurKernelHeight();
                        motionDetector.setBlurKernelSize(val, currentHeight);
                        panel.setBlurKernelWidthValue(value);
                    }
                }
                else if (key == "BlurKernelHeight") {
                    int val = std::stoi(value);
                    if (val >= 1 && val <= 31) {
                        int currentWidth = motionDetector.getBlurKernelWidth();
                        motionDetector.setBlurKernelSize(currentWidth, val);
                        panel.setBlurKernelHeightValue(value);
                    }
                }
                else if (key == "BlurSigma") {
                    double val = std::stod(value);
                    if (val >= 0.1 && val <= 10.0) {
                        motionDetector.setBlurSigma(val);
                        panel.setBlurSigmaValue(value);
                    }
                }
                else if (key == "CameraExposure") {
                    int val = std::stoi(value);
                    panel.setCameraExposureValue(value);
                }
                else if (key == "TestMode") {
                    bool val = (value == "1" || value == "true" || value == "TRUE");
                    panel.setSwitchState(val);
                }
                // ArucoDetector параметры
                else if (key == "MinMarkerPerimeterRate") {
                    float val = std::stof(value);
                    if (val >= 0.0f && val <= 1.0f) {
                        arucoDetector.setMinMarkerPerimeterRate(val);
                        panel.setMinMarkerPerimeterRateValue(value);
                    }
                }
                else if (key == "MaxMarkerPerimeterRate") {
                    float val = std::stof(value);
                    if (val >= 1.0f && val <= 10.0f) {
                        arucoDetector.setMaxMarkerPerimeterRate(val);
                        panel.setMaxMarkerPerimeterRateValue(value);
                    }
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error parsing config value for " << key << ": " << e.what() 
                        << " (value: '" << value << "')" << std::endl;
                success = false;
            }
        }
    }
    
    file.close();
    
    if (success) {
        std::cout << "Config loaded from " << configFile_ << std::endl;
    } else {
        std::cout << "Config loaded from " << configFile_ << " with some errors" << std::endl;
    }
    
    return success;
}