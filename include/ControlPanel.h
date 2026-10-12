// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <windows.h>
#include <iostream>
#include <atomic>
#include <string>
#include <functional>

class ControlPanel {
private:
    HWND hwndMain = NULL;
    HWND hwndCheckBtn = NULL;
    HWND hwndStartBtn = NULL;
    HWND hwndStopBtn = NULL;
    HWND hwndKpEdit = NULL;
    HWND hwndKiEdit = NULL;
    HWND hwndKdEdit = NULL;
    HWND hwndSwitch = NULL;
    HWND hwndRobot1ExclusionEdit = NULL;
    HWND hwndRobot1OffsetEdit = NULL;
    HWND hwndArenaXEdit = NULL;
    HWND hwndArenaYEdit = NULL;
    HWND hwndArenaSelectSwitch = NULL;
    
    HWND hwndKrlEdit = NULL;
    HWND hwndKfbEdit = NULL;
    
    HWND hwndCameraExposureEdit = NULL;
    HWND hwndCameraApplyBtn = NULL;
    
    // ArucoDetector controls
    HWND hwndMinMarkerPerimeterRateEdit = NULL;
    HWND hwndMaxMarkerPerimeterRateEdit = NULL;
    
    HWND hwndMotionThresholdEdit = NULL;
    HWND hwndMorphKernelWidthEdit = NULL;
    HWND hwndMorphKernelHeightEdit = NULL;
    HWND hwndDilationIterationsEdit = NULL;
    HWND hwndErosionIterationsEdit = NULL;
    HWND hwndBlurKernelWidthEdit = NULL;
    HWND hwndBlurKernelHeightEdit = NULL;
    HWND hwndBlurSigmaEdit = NULL;
    
    HWND hwndArenaSizeX1Edit = NULL;
    HWND hwndArenaSizeY1Edit = NULL;
    HWND hwndArenaSizeX2Edit = NULL;
    HWND hwndArenaSizeY2Edit = NULL;
    HWND hwndArenaSizeSelectBtn = NULL;
    HWND hwndArenaSizeResetBtn = NULL;
    
    HWND hwndCenterAfterAttackEdit = NULL;
    HWND hwndCenterTriggerThresholdEdit = NULL;
    HWND hwndEnemyTriggerThresholdEdit = NULL;
    HWND hwndEnemyTriggerTimeoutEdit = NULL;
    HWND hwndReverseAngleThresholdEdit = NULL;
    HWND hwndUseReverseEdit = NULL;
    
    HWND hwndAngleMinExtraSpeedEdit = NULL;
    HWND hwndAngleMaxExtraSpeedEdit = NULL;

    HWND hwndInPlaceSpeedRotationEdit = NULL;
    HWND hwndInPlaceSpeedWeaponEdit = NULL;

    std::atomic<bool> isRunning_{true};
    std::atomic<bool> switchState_{false};
    
    char kpBuffer[32] = "0.10";
    char kiBuffer[32] = "0.10";
    char kdBuffer[32] = "0.10";
    char krlBuffer[32] = "1.00";
    char kfbBuffer[32] = "1.00";
    char robot1ExclusionBuffer[32] = "150";
    char arenaXBuffer[32] = "640";
    char arenaYBuffer[32] = "512";
    char arenaSizeX1Buffer[32] = "-1";
    char arenaSizeY1Buffer[32] = "-1";
    char arenaSizeX2Buffer[32] = "-1";
    char arenaSizeY2Buffer[32] = "-1";
    
    char cameraExposureBuffer[32] = "-9";
    
    // ArucoDetector buffers
    char minMarkerPerimeterRateBuffer[32] = "0.05";
    char maxMarkerPerimeterRateBuffer[32] = "8.0";
    
    char motionThresholdBuffer[32] = "25";
    char morphKernelWidthBuffer[32] = "5";
    char morphKernelHeightBuffer[32] = "5";
    char dilationIterationsBuffer[32] = "2";
    char erosionIterationsBuffer[32] = "1";
    char blurKernelWidthBuffer[32] = "5";
    char blurKernelHeightBuffer[32] = "5";
    char blurSigmaBuffer[32] = "1.5";

    char robot1OffsetBuffer[32] = "20";

    char centerAfterAttackBuffer[32] = "1";
    char centerTriggerThresholdBuffer[32] = "150";
    char enemyTriggerThresholdBuffer[32] = "150";
    char enemyTriggerTimeoutBuffer[32] = "2000";
    char reverseAngleThresholdBuffer[32] = "180";
    char useReverseBuffer[32] = "1";
    
    char angleMinExtraSpeedBuffer[32] = "90.0";
    char angleMaxExtraSpeedBuffer[32] = "0.0";

    char InPlaceSpeedRotationBuffer[32] = "0.5";
    char InPlaceSpeedWeaponBuffer[32] = "1.0";

    std::function<void()> onCheckCallback_ = nullptr;
    std::function<void()> onStartCallback_ = nullptr;
    std::function<void()> onStopCallback_ = nullptr;
    std::function<void(bool)> onSwitchCallback_ = nullptr;
    std::function<void(const std::string&)> onKpCallback_ = nullptr;
    std::function<void(const std::string&)> onKiCallback_ = nullptr;
    std::function<void(const std::string&)> onKdCallback_ = nullptr;
    std::function<void(const std::string&)> onKrlCallback_ = nullptr;
    std::function<void(const std::string&)> onKfbCallback_ = nullptr;
    
    std::function<void(const std::string&)> onCenterAfterAttackCallback_ = nullptr;
    std::function<void(const std::string&)> onCenterTriggerThresholdCallback_ = nullptr;
    std::function<void(const std::string&)> onEnemyTriggerThresholdCallback_ = nullptr;
    std::function<void(const std::string&)> onEnemyTriggerTimeoutCallback_ = nullptr;
    std::function<void(const std::string&)> onReverseAngleThresholdCallback_ = nullptr;
    std::function<void(const std::string&)> onUseReverseCallback_ = nullptr;
    
    std::function<void(const std::string&)> onAngleMinExtraSpeedCallback_ = nullptr;
    std::function<void(const std::string&)> onAngleMaxExtraSpeedCallback_ = nullptr;

    std::function<void(const std::string&)> onInPlaceSpeedRotationCallback_ = nullptr;
    std::function<void(const std::string&)> onInPlaceSpeedWeaponCallback_ = nullptr;

    std::function<void(const std::string&)> onRobot1ExclusionCallback_ = nullptr;
    std::function<void(const std::string&)> onRobot1OffsetCallback_ = nullptr;
    std::function<void(const std::string&)> onArenaXCallback_ = nullptr;
    std::function<void(const std::string&)> onArenaYCallback_ = nullptr;
    std::function<void(bool)> onArenaSelectCallback_ = nullptr;
    
    std::function<void(const std::string&)> onArenaSizeX1Callback_ = nullptr;
    std::function<void(const std::string&)> onArenaSizeY1Callback_ = nullptr;
    std::function<void(const std::string&)> onArenaSizeX2Callback_ = nullptr;
    std::function<void(const std::string&)> onArenaSizeY2Callback_ = nullptr;
    std::function<void()> onArenaSizeSelectCallback_ = nullptr;
    std::function<void()> onArenaSizeResetCallback_ = nullptr;
    
    std::function<void(const std::string&)> onCameraApplyCallback_ = nullptr;
    
    // ArucoDetector callbacks
    std::function<void(const std::string&)> onMinMarkerPerimeterRateCallback_ = nullptr;
    std::function<void(const std::string&)> onMaxMarkerPerimeterRateCallback_ = nullptr;
    
    std::function<void(const std::string&)> onMotionThresholdCallback_ = nullptr;
    std::function<void(const std::string&)> onMorphKernelWidthCallback_ = nullptr;
    std::function<void(const std::string&)> onMorphKernelHeightCallback_ = nullptr;
    std::function<void(const std::string&)> onDilationIterationsCallback_ = nullptr;
    std::function<void(const std::string&)> onErosionIterationsCallback_ = nullptr;
    std::function<void(const std::string&)> onBlurKernelWidthCallback_ = nullptr;
    std::function<void(const std::string&)> onBlurKernelHeightCallback_ = nullptr;
    std::function<void(const std::string&)> onBlurSigmaCallback_ = nullptr;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void createControls(HWND hwnd);
    void paintWindow(HWND hwnd, HDC hdc);
public:
    ControlPanel();
    ~ControlPanel();
    
    void run();
    void stop();
    bool isRunning() const { return isRunning_; }
    
    void setOnCheckCallback(std::function<void()> callback) { onCheckCallback_ = callback; }
    void setOnStartCallback(std::function<void()> callback) { onStartCallback_ = callback; }
    void setOnStopCallback(std::function<void()> callback) { onStopCallback_ = callback; }
    void setOnSwitchCallback(std::function<void(bool)> callback) { onSwitchCallback_ = callback; }
    void setOnKpCallback(std::function<void(const std::string&)> callback) { onKpCallback_ = callback; }
    void setOnKiCallback(std::function<void(const std::string&)> callback) { onKiCallback_ = callback; }
    void setOnKdCallback(std::function<void(const std::string&)> callback) { onKdCallback_ = callback; }
    void setOnKrlCallback(std::function<void(const std::string&)> callback) { onKrlCallback_ = callback; }
    void setOnKfbCallback(std::function<void(const std::string&)> callback) { onKfbCallback_ = callback; }

    void setOnCenterAfterAttackCallback(std::function<void(const std::string&)> callback) { onCenterAfterAttackCallback_ = callback; }
    void setOnCenterTriggerThresholdCallback(std::function<void(const std::string&)> callback) { onCenterTriggerThresholdCallback_ = callback; }
    void setOnEnemyTriggerThresholdCallback(std::function<void(const std::string&)> callback) { onEnemyTriggerThresholdCallback_ = callback; }
    void setOnEnemyTriggerTimeoutCallback(std::function<void(const std::string&)> callback) { onEnemyTriggerTimeoutCallback_ = callback; }
    void setOnReverseAngleThresholdCallback(std::function<void(const std::string&)> callback) { onReverseAngleThresholdCallback_ = callback; }
    void setOnUseReverseCallback(std::function<void(const std::string&)> callback) { onUseReverseCallback_ = callback; }
    
    void setOnAngleMinExtraSpeedCallback(std::function<void(const std::string&)> callback) { onAngleMinExtraSpeedCallback_ = callback; }
    void setOnAngleMaxExtraSpeedCallback(std::function<void(const std::string&)> callback) { onAngleMaxExtraSpeedCallback_ = callback; }
    
    void setOnInPlaceSpeedRotationCallback(std::function<void(const std::string&)> callback) { onInPlaceSpeedRotationCallback_ = callback; }
    void setOnInPlaceSpeedWeaponCallback(std::function<void(const std::string&)> callback) { onInPlaceSpeedWeaponCallback_ = callback; }
    
    void setOnRobot1ExclusionCallback(std::function<void(const std::string&)> callback) { onRobot1ExclusionCallback_ = callback; }
    void setOnRobot1OffsetCallback(std::function<void(const std::string&)> callback) { onRobot1OffsetCallback_ = callback; }
    void setOnArenaXCallback(std::function<void(const std::string&)> callback) { onArenaXCallback_ = callback; }
    void setOnArenaYCallback(std::function<void(const std::string&)> callback) { onArenaYCallback_ = callback; }
    void setOnArenaSelectCallback(std::function<void(bool)> callback) { onArenaSelectCallback_ = callback; }
    
    void setOnArenaSizeX1Callback(std::function<void(const std::string&)> callback) { onArenaSizeX1Callback_ = callback; }
    void setOnArenaSizeY1Callback(std::function<void(const std::string&)> callback) { onArenaSizeY1Callback_ = callback; }
    void setOnArenaSizeX2Callback(std::function<void(const std::string&)> callback) { onArenaSizeX2Callback_ = callback; }
    void setOnArenaSizeY2Callback(std::function<void(const std::string&)> callback) { onArenaSizeY2Callback_ = callback; }
    void setOnArenaSizeSelectCallback(std::function<void()> callback) { onArenaSizeSelectCallback_ = callback; }
    void setOnArenaSizeResetCallback(std::function<void()> callback) { onArenaSizeResetCallback_ = callback; }
    
    void setOnCameraApplyCallback(std::function<void(const std::string&)> callback) {
        onCameraApplyCallback_ = callback;
    }
    
    // ArucoDetector callbacks
    void setOnMinMarkerPerimeterRateCallback(std::function<void(const std::string&)> callback) { 
        onMinMarkerPerimeterRateCallback_ = callback; 
    }
    void setOnMaxMarkerPerimeterRateCallback(std::function<void(const std::string&)> callback) { 
        onMaxMarkerPerimeterRateCallback_ = callback; 
    }
    
    void setOnMotionThresholdCallback(std::function<void(const std::string&)> callback) { 
        onMotionThresholdCallback_ = callback; 
    }
    void setOnMorphKernelWidthCallback(std::function<void(const std::string&)> callback) { 
        onMorphKernelWidthCallback_ = callback; 
    }
    void setOnMorphKernelHeightCallback(std::function<void(const std::string&)> callback) { 
        onMorphKernelHeightCallback_ = callback; 
    }
    void setOnDilationIterationsCallback(std::function<void(const std::string&)> callback) { 
        onDilationIterationsCallback_ = callback; 
    }
    void setOnErosionIterationsCallback(std::function<void(const std::string&)> callback) { 
        onErosionIterationsCallback_ = callback; 
    }
    void setOnBlurKernelWidthCallback(std::function<void(const std::string&)> callback) { 
        onBlurKernelWidthCallback_ = callback; 
    }
    void setOnBlurKernelHeightCallback(std::function<void(const std::string&)> callback) { 
        onBlurKernelHeightCallback_ = callback; 
    }
    void setOnBlurSigmaCallback(std::function<void(const std::string&)> callback) { 
        onBlurSigmaCallback_ = callback; 
    }
    
    std::string getKpValue() const { return kpBuffer; }
    std::string getKiValue() const { return kiBuffer; }
    std::string getKdValue() const { return kdBuffer; }
    std::string getKrlValue() const { return krlBuffer; }
    std::string getKfbValue() const { return kfbBuffer; }
    std::string getCenterAfterAttackValue() const { return centerAfterAttackBuffer; }
    std::string getCenterTriggerThresholdValue() const { return centerTriggerThresholdBuffer; }
    std::string getEnemyTriggerThresholdValue() const { return enemyTriggerThresholdBuffer; }
    std::string getEnemyTriggerTimeoutValue() const { return enemyTriggerTimeoutBuffer; }
    std::string getReverseAngleThresholdValue() const { return reverseAngleThresholdBuffer; }
    std::string getUseReverseValue() const { return useReverseBuffer; }
    
    std::string getAngleMinExtraSpeedValue() const { return angleMinExtraSpeedBuffer; }
    std::string getAngleMaxExtraSpeedValue() const { return angleMaxExtraSpeedBuffer; }
    
    std::string getInPlaceSpeedRotationValue() const { return InPlaceSpeedRotationBuffer; }
    std::string getInPlaceSpeedWeaponValue() const { return InPlaceSpeedWeaponBuffer; }
    
    std::string getRobot1ExclusionValue() const { return robot1ExclusionBuffer; }
    std::string getRobot1OffsetValue() const { return robot1OffsetBuffer; }
    std::string getArenaXValue() const { return arenaXBuffer; }
    std::string getArenaYValue() const { return arenaYBuffer; }
    std::string getArenaSizeX1Value() const { return arenaSizeX1Buffer; }
    std::string getArenaSizeY1Value() const { return arenaSizeY1Buffer; }
    std::string getArenaSizeX2Value() const { return arenaSizeX2Buffer; }
    std::string getArenaSizeY2Value() const { return arenaSizeY2Buffer; }
    std::string getCameraExposureValue() const { return cameraExposureBuffer; }
    
    // ArucoDetector getters
    std::string getMinMarkerPerimeterRateValue() const { return minMarkerPerimeterRateBuffer; }
    std::string getMaxMarkerPerimeterRateValue() const { return maxMarkerPerimeterRateBuffer; }
    
    std::string getMotionThresholdValue() const { return motionThresholdBuffer; }
    std::string getMorphKernelWidthValue() const { return morphKernelWidthBuffer; }
    std::string getMorphKernelHeightValue() const { return morphKernelHeightBuffer; }
    std::string getDilationIterationsValue() const { return dilationIterationsBuffer; }
    std::string getErosionIterationsValue() const { return erosionIterationsBuffer; }
    std::string getBlurKernelWidthValue() const { return blurKernelWidthBuffer; }
    std::string getBlurKernelHeightValue() const { return blurKernelHeightBuffer; }
    std::string getBlurSigmaValue() const { return blurSigmaBuffer; }
    bool getSwitchState() const { return switchState_; }

    void setKpValue(const std::string& value);
    void setKiValue(const std::string& value);
    void setKdValue(const std::string& value);
    void setKrlValue(const std::string& value);
    void setKfbValue(const std::string& value);
    
    void setCenterAfterAttackValue(const std::string& value);
    void setCenterTriggerThresholdValue(const std::string& value);
    void setEnemyTriggerThresholdValue(const std::string& value);
    void setEnemyTriggerTimeoutValue(const std::string& value);
    void setReverseAngleThresholdValue(const std::string& value);
    void setUseReverseValue(const std::string& value);
    
    void setAngleMinExtraSpeedValue(const std::string& value);
    void setAngleMaxExtraSpeedValue(const std::string& value);

    void setInPlaceSpeedRotationValue(const std::string& value);
    void setInPlaceSpeedWeaponValue(const std::string& value);

    void setRobot1ExclusionValue(const std::string& value);
    void setRobot1OffsetValue(const std::string& value);
    void setArenaXValue(const std::string& value);
    void setArenaYValue(const std::string& value);
    void setArenaSizeX1Value(const std::string& value);
    void setArenaSizeY1Value(const std::string& value);
    void setArenaSizeX2Value(const std::string& value);
    void setArenaSizeY2Value(const std::string& value);
    void setCameraExposureValue(const std::string& value);
    void setSwitchState(bool state);
    void setArenaSelectState(bool state);
    
    // ArucoDetector setters
    void setMinMarkerPerimeterRateValue(const std::string& value);
    void setMaxMarkerPerimeterRateValue(const std::string& value);
    
    void setMotionThresholdValue(const std::string& value);
    void setMorphKernelWidthValue(const std::string& value);
    void setMorphKernelHeightValue(const std::string& value);
    void setDilationIterationsValue(const std::string& value);
    void setErosionIterationsValue(const std::string& value);
    void setBlurKernelWidthValue(const std::string& value);
    void setBlurKernelHeightValue(const std::string& value);
    void setBlurSigmaValue(const std::string& value);
};

#endif // CONTROL_PANEL_H