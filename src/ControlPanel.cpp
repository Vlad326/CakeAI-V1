// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include "ControlPanel.h"

static ControlPanel* g_instance = nullptr;

ControlPanel::ControlPanel() {
    g_instance = this;
}

ControlPanel::~ControlPanel() {
    stop();
    g_instance = nullptr;
}

LRESULT CALLBACK ControlPanel::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_instance) {
        return g_instance->handleMessage(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT ControlPanel::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            createControls(hwnd);
            return 0;
            
        case WM_COMMAND:
            {
                int id = LOWORD(wParam);
                int notification = HIWORD(wParam);
                
                if (notification == BN_CLICKED) {
                    switch (id) {
                        case 1: // Check
                            if (onCheckCallback_) onCheckCallback_();
                            else std::cout << "Check button pressed!" << std::endl;
                            break;
                            
                        case 2: // Start
                            if (onStartCallback_) onStartCallback_();
                            else std::cout << "Start button pressed!" << std::endl;
                            break;
                            
                        case 6: // Switch (point_test)
                            {
                                bool checked = SendMessage(hwndSwitch, BM_GETCHECK, 0, 0) == BST_CHECKED;
                                switchState_ = checked;
                                SetWindowText(hwndSwitch, checked ? "ON" : "OFF");
                                if (onSwitchCallback_) onSwitchCallback_(checked);
                                else std::cout << "Switch toggled: " << (checked ? "ON" : "OFF") << std::endl;
                            }
                            break;
                            
                        case 7: // Stop
                            if (onStopCallback_) onStopCallback_();
                            else std::cout << "Stop button pressed!" << std::endl;
                            break;
                        
                        case 14: // Arena Select Switch
                            {
                                bool checked = SendMessage(hwndArenaSelectSwitch, BM_GETCHECK, 0, 0) == BST_CHECKED;
                                SetWindowText(hwndArenaSelectSwitch, checked ? "ON" : "OFF");
                                
                                if (onArenaSelectCallback_) {
                                    onArenaSelectCallback_(checked);
                                } else {
                                    std::cout << "Arena Select switch toggled: " << (checked ? "ON" : "OFF") << std::endl;
                                }
                            }
                            break;
                            
                        case 18: // Camera Apply button
                            {
                                char exposureBuffer[32];
                                GetWindowText(hwndCameraExposureEdit, exposureBuffer, sizeof(exposureBuffer));
                                
                                strncpy(cameraExposureBuffer, exposureBuffer, sizeof(cameraExposureBuffer));
                                
                                if (onCameraApplyCallback_) {
                                    onCameraApplyCallback_(exposureBuffer);
                                } else {
                                    std::cout << "Camera Apply pressed! Exposure: " << exposureBuffer << std::endl;
                                }
                            }
                            break;
                            
                        case 34: // Arena Size Select button
                            if (onArenaSizeResetCallback_) {
                                onArenaSizeResetCallback_();
                            }
                            if (onArenaSizeSelectCallback_) {
                                onArenaSizeSelectCallback_();
                            }
                            break;
                            
                        case 35: // Arena Size Reset button
                            if (onArenaSizeResetCallback_) {
                                onArenaSizeResetCallback_();
                            }
                            break;
                    }
                }
                else if (notification == EN_CHANGE) {
                    
                    char buffer[32];
                    switch (id) {
                        case 3: // KP
                            GetWindowText(hwndKpEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(kpBuffer, buffer, sizeof(kpBuffer));
                            if (onKpCallback_) onKpCallback_(buffer);
                            else std::cout << "KP changed to: " << buffer << std::endl;
                            break;
                            
                        case 4: // KI
                            GetWindowText(hwndKiEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(kiBuffer, buffer, sizeof(kiBuffer));
                            if (onKiCallback_) onKiCallback_(buffer);
                            else std::cout << "KI changed to: " << buffer << std::endl;
                            break;
                            
                        case 5: // KD
                            GetWindowText(hwndKdEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(kdBuffer, buffer, sizeof(kdBuffer));
                            if (onKdCallback_) onKdCallback_(buffer);
                            else std::cout << "KD changed to: " << buffer << std::endl;
                            break;
                            
                        case 8: // KRL
                            GetWindowText(hwndKrlEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(krlBuffer, buffer, sizeof(krlBuffer));
                            if (onKrlCallback_) onKrlCallback_(buffer);
                            else std::cout << "KRL changed to: " << buffer << std::endl;
                            break;
                            
                        case 36: // KFB
                            GetWindowText(hwndKfbEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(kfbBuffer, buffer, sizeof(kfbBuffer));
                            if (onKfbCallback_) onKfbCallback_(buffer);
                            else std::cout << "KFB changed to: " << buffer << std::endl;
                            break;

                        case 37: // CenterAfterAttack
                            GetWindowText(hwndCenterAfterAttackEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(centerAfterAttackBuffer, buffer, sizeof(centerAfterAttackBuffer));
                            if (onCenterAfterAttackCallback_) onCenterAfterAttackCallback_(buffer);
                            else std::cout << "CenterAfterAttack changed to: " << buffer << std::endl;
                            break;
                        
                        case 38: // CenterTriggerThreshold
                            GetWindowText(hwndCenterTriggerThresholdEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(centerTriggerThresholdBuffer, buffer, sizeof(centerTriggerThresholdBuffer));
                            if (onCenterTriggerThresholdCallback_) onCenterTriggerThresholdCallback_(buffer);
                            else std::cout << "CenterTriggerThreshold changed to: " << buffer << std::endl;
                            break;
                        
                        case 39: // EnemyTriggerThreshold
                            GetWindowText(hwndEnemyTriggerThresholdEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(enemyTriggerThresholdBuffer, buffer, sizeof(enemyTriggerThresholdBuffer));
                            if (onEnemyTriggerThresholdCallback_) onEnemyTriggerThresholdCallback_(buffer);
                            else std::cout << "EnemyTriggerThreshold changed to: " << buffer << std::endl;
                            break;

                        case 40: // EnemyTriggerTimeout
                            GetWindowText(hwndEnemyTriggerTimeoutEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(enemyTriggerTimeoutBuffer, buffer, sizeof(enemyTriggerTimeoutBuffer));
                            if (onEnemyTriggerTimeoutCallback_) onEnemyTriggerTimeoutCallback_(buffer);
                            else std::cout << "EnemyTriggerTimeout changed to: " << buffer << std::endl;
                            break;
                        
                        case 41: // ReverseAngleThreshold
                            GetWindowText(hwndReverseAngleThresholdEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(reverseAngleThresholdBuffer, buffer, sizeof(reverseAngleThresholdBuffer));
                            if (onReverseAngleThresholdCallback_) onReverseAngleThresholdCallback_(buffer);
                            else std::cout << "ReverseAngleThreshold changed to: " << buffer << std::endl;
                            break;
                            
                        case 42: // UseReverse
                            GetWindowText(hwndUseReverseEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(useReverseBuffer, buffer, sizeof(useReverseBuffer));
                            if (onUseReverseCallback_) onUseReverseCallback_(buffer);
                            else std::cout << "UseReverse changed to: " << buffer << std::endl;
                            break;
                            
                        case 43: // AngleMinExtraSpeed
                            GetWindowText(hwndAngleMinExtraSpeedEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(angleMinExtraSpeedBuffer, buffer, sizeof(angleMinExtraSpeedBuffer));
                            if (onAngleMinExtraSpeedCallback_) onAngleMinExtraSpeedCallback_(buffer);
                            else std::cout << "AngleMinExtraSpeed changed to: " << buffer << std::endl;
                            break;
                            
                        case 44: // AngleMaxExtraSpeed
                            GetWindowText(hwndAngleMaxExtraSpeedEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(angleMaxExtraSpeedBuffer, buffer, sizeof(angleMaxExtraSpeedBuffer));
                            if (onAngleMaxExtraSpeedCallback_) onAngleMaxExtraSpeedCallback_(buffer);
                            else std::cout << "AngleMaxExtraSpeed changed to: " << buffer << std::endl;
                            break;
                        case 45: // InPlaceSpeedRotation
                            GetWindowText(hwndInPlaceSpeedRotationEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(InPlaceSpeedRotationBuffer, buffer, sizeof(InPlaceSpeedRotationBuffer));
                            if (onInPlaceSpeedRotationCallback_) onInPlaceSpeedRotationCallback_(buffer);
                            else std::cout << "InPlaceSpeedRotation changed to: " << buffer << std::endl;
                            break;
                        case 46: // InPlaceSpeedWeapon
                            GetWindowText(hwndInPlaceSpeedWeaponEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(InPlaceSpeedWeaponBuffer, buffer, sizeof(InPlaceSpeedWeaponBuffer));
                            if (onInPlaceSpeedWeaponCallback_) onInPlaceSpeedWeaponCallback_(buffer);
                            else std::cout << "InPlaceSpeedWeapon changed to: " << buffer << std::endl;
                            break;
                        
                        case 9: // Robot 1 ExclusionRadius
                            GetWindowText(hwndRobot1ExclusionEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(robot1ExclusionBuffer, buffer, sizeof(robot1ExclusionBuffer));
                            if (onRobot1ExclusionCallback_) onRobot1ExclusionCallback_(buffer);
                            else std::cout << "Robot 1 ExclusionRadius changed to: " << buffer << std::endl;
                            break;
                        case 11: // Robot 1 OffsetPixels
                            GetWindowText(hwndRobot1OffsetEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(robot1OffsetBuffer, buffer, sizeof(robot1OffsetBuffer));
                            if (onRobot1OffsetCallback_) onRobot1OffsetCallback_(buffer);
                            else std::cout << "Robot 1 OffsetPixels changed to: " << buffer << std::endl;
                            break;
                        case 12: // Arena X
                            GetWindowText(hwndArenaXEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaXBuffer, buffer, sizeof(arenaXBuffer));
                            if (onArenaXCallback_) onArenaXCallback_(buffer);
                            else std::cout << "Arena X changed to: " << buffer << std::endl;
                            break;
                            
                        case 13: // Arena Y
                            GetWindowText(hwndArenaYEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaYBuffer, buffer, sizeof(arenaYBuffer));
                            if (onArenaYCallback_) onArenaYCallback_(buffer);
                            else std::cout << "Arena Y changed to: " << buffer << std::endl;
                            break;
                            
                        case 17: // Camera Exposure
                            GetWindowText(hwndCameraExposureEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(cameraExposureBuffer, buffer, sizeof(cameraExposureBuffer));
                            break;
                            
                        // ArucoDetector parameters
                        case 47: // MinMarkerPerimeterRate
                            GetWindowText(hwndMinMarkerPerimeterRateEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(minMarkerPerimeterRateBuffer, buffer, sizeof(minMarkerPerimeterRateBuffer));
                            if (onMinMarkerPerimeterRateCallback_) onMinMarkerPerimeterRateCallback_(buffer);
                            else std::cout << "MinMarkerPerimeterRate changed to: " << buffer << std::endl;
                            break;
                            
                        case 48: // MaxMarkerPerimeterRate
                            GetWindowText(hwndMaxMarkerPerimeterRateEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(maxMarkerPerimeterRateBuffer, buffer, sizeof(maxMarkerPerimeterRateBuffer));
                            if (onMaxMarkerPerimeterRateCallback_) onMaxMarkerPerimeterRateCallback_(buffer);
                            else std::cout << "MaxMarkerPerimeterRate changed to: " << buffer << std::endl;
                            break;
                            
                        case 20: // Motion Threshold
                            GetWindowText(hwndMotionThresholdEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(motionThresholdBuffer, buffer, sizeof(motionThresholdBuffer));
                            if (onMotionThresholdCallback_) onMotionThresholdCallback_(buffer);
                            else std::cout << "Motion Threshold changed to: " << buffer << std::endl;
                            break;
                            
                        case 21: // Morph Kernel Width
                            GetWindowText(hwndMorphKernelWidthEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(morphKernelWidthBuffer, buffer, sizeof(morphKernelWidthBuffer));
                            if (onMorphKernelWidthCallback_) onMorphKernelWidthCallback_(buffer);
                            else std::cout << "Morph Kernel Width changed to: " << buffer << std::endl;
                            break;
                            
                        case 22: // Morph Kernel Height
                            GetWindowText(hwndMorphKernelHeightEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(morphKernelHeightBuffer, buffer, sizeof(morphKernelHeightBuffer));
                            if (onMorphKernelHeightCallback_) onMorphKernelHeightCallback_(buffer);
                            else std::cout << "Morph Kernel Height changed to: " << buffer << std::endl;
                            break;
                            
                        case 23: // Dilation Iterations
                            GetWindowText(hwndDilationIterationsEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(dilationIterationsBuffer, buffer, sizeof(dilationIterationsBuffer));
                            if (onDilationIterationsCallback_) onDilationIterationsCallback_(buffer);
                            else std::cout << "Dilation Iterations changed to: " << buffer << std::endl;
                            break;
                            
                        case 24: // Erosion Iterations
                            GetWindowText(hwndErosionIterationsEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(erosionIterationsBuffer, buffer, sizeof(erosionIterationsBuffer));
                            if (onErosionIterationsCallback_) onErosionIterationsCallback_(buffer);
                            else std::cout << "Erosion Iterations changed to: " << buffer << std::endl;
                            break;
                            
                        case 25: // Blur Kernel Width
                            GetWindowText(hwndBlurKernelWidthEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(blurKernelWidthBuffer, buffer, sizeof(blurKernelWidthBuffer));
                            if (onBlurKernelWidthCallback_) onBlurKernelWidthCallback_(buffer);
                            else std::cout << "Blur Kernel Width changed to: " << buffer << std::endl;
                            break;
                            
                        case 26: // Blur Kernel Height
                            GetWindowText(hwndBlurKernelHeightEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(blurKernelHeightBuffer, buffer, sizeof(blurKernelHeightBuffer));
                            if (onBlurKernelHeightCallback_) onBlurKernelHeightCallback_(buffer);
                            else std::cout << "Blur Kernel Height changed to: " << buffer << std::endl;
                            break;
                            
                        case 27: // Blur Sigma
                            GetWindowText(hwndBlurSigmaEdit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(blurSigmaBuffer, buffer, sizeof(blurSigmaBuffer));
                            if (onBlurSigmaCallback_) onBlurSigmaCallback_(buffer);
                            else std::cout << "Blur Sigma changed to: " << buffer << std::endl;
                            break;
                            
                        case 30: // Arena Size X1
                            GetWindowText(hwndArenaSizeX1Edit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaSizeX1Buffer, buffer, sizeof(arenaSizeX1Buffer));
                            if (onArenaSizeX1Callback_) onArenaSizeX1Callback_(buffer);
                            else std::cout << "Arena Size X1 changed to: " << buffer << std::endl;
                            break;
                            
                        case 31: // Arena Size Y1
                            GetWindowText(hwndArenaSizeY1Edit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaSizeY1Buffer, buffer, sizeof(arenaSizeY1Buffer));
                            if (onArenaSizeY1Callback_) onArenaSizeY1Callback_(buffer);
                            else std::cout << "Arena Size Y1 changed to: " << buffer << std::endl;
                            break;
                            
                        case 32: // Arena Size X2
                            GetWindowText(hwndArenaSizeX2Edit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaSizeX2Buffer, buffer, sizeof(arenaSizeX2Buffer));
                            if (onArenaSizeX2Callback_) onArenaSizeX2Callback_(buffer);
                            else std::cout << "Arena Size X2 changed to: " << buffer << std::endl;
                            break;
                            
                        case 33: // Arena Size Y2
                            GetWindowText(hwndArenaSizeY2Edit, buffer, sizeof(buffer));
                            if (strlen(buffer) == 0) {
                                return 0;
                            }
                            strncpy(arenaSizeY2Buffer, buffer, sizeof(arenaSizeY2Buffer));
                            if (onArenaSizeY2Callback_) onArenaSizeY2Callback_(buffer);
                            else std::cout << "Arena Size Y2 changed to: " << buffer << std::endl;
                            break;
                    }
                }
            }
            return 0;
            
        case WM_CLOSE:
            DestroyWindow(hwnd);
            isRunning_ = false;
            return 0;
            
        case WM_DESTROY:
            hwndMain = NULL;
            return 0;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                paintWindow(hwnd, hdc);
                EndPaint(hwnd, &ps);
            }
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ControlPanel::createControls(HWND hwnd) {
    int yPos = 50;
    int buttonWidth = 140;
    int buttonHeight = 40;
    int editWidth = 80;
    int smallEditWidth = 60;
    int verySmallEditWidth = 45;
    int labelWidth = 50;
    int colWidth = 300;
    int currentCol = 0;
    
    int xStart = 30;
    
    hwndCheckBtn = CreateWindow(
        "BUTTON", "Check",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth, yPos, buttonWidth, buttonHeight,
        hwnd, (HMENU)1, NULL, NULL);
    yPos += buttonHeight + 20;
    
    hwndStartBtn = CreateWindow(
        "BUTTON", "Start",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth, yPos, buttonWidth, buttonHeight,
        hwnd, (HMENU)2, NULL, NULL);
    yPos += buttonHeight + 30;
    
    CreateWindow("STATIC", "PID Settings:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 150, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "KP:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, labelWidth, 25,
        hwnd, NULL, NULL, NULL);
    hwndKpEdit = CreateWindow("EDIT", kpBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + labelWidth + 20, yPos, editWidth, 25,
        hwnd, (HMENU)3, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "KI:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, labelWidth, 25,
        hwnd, NULL, NULL, NULL);
    hwndKiEdit = CreateWindow("EDIT", kiBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + labelWidth + 20, yPos, editWidth, 25,
        hwnd, (HMENU)4, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "KD:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, labelWidth, 25,
        hwnd, NULL, NULL, NULL);
    hwndKdEdit = CreateWindow("EDIT", kdBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + labelWidth + 20, yPos, editWidth, 25,
        hwnd, (HMENU)5, NULL, NULL);
    yPos += 40;
    
    CreateWindow("STATIC", "point_test",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 100, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndSwitch = CreateWindow("BUTTON", "OFF",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        xStart + currentCol * colWidth + 110, yPos, 80, 30,
        hwnd, (HMENU)6, NULL, NULL);
    yPos += 50;
    
    hwndStopBtn = CreateWindow(
        "BUTTON", "Stop",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth, yPos, buttonWidth, buttonHeight,
        hwnd, (HMENU)7, NULL, NULL);
    
    currentCol++;
    yPos = 50;
    
    CreateWindow("STATIC", "Robot 1:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 150, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 25;
    
    CreateWindow("STATIC", "ExclusionRadius:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 120, 25,
        hwnd, NULL, NULL, NULL);
    hwndRobot1ExclusionEdit = CreateWindow("EDIT", robot1ExclusionBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 140, yPos, editWidth, 25,
        hwnd, (HMENU)9, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "OffsetPixels:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 120, 25,
        hwnd, NULL, NULL, NULL);
    hwndRobot1OffsetEdit = CreateWindow("EDIT", robot1OffsetBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 140, yPos, editWidth, 25,
        hwnd, (HMENU)11, NULL, NULL);
    
    yPos += 40;
    
    CreateWindow("STATIC", "RobotController:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 200, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "KRL:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 50, 25,
        hwnd, NULL, NULL, NULL);
    hwndKrlEdit = CreateWindow("EDIT", krlBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 70, yPos, smallEditWidth, 25,
        hwnd, (HMENU)8, NULL, NULL);
    
    CreateWindow("STATIC", "KFB:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 140, yPos, 50, 25,
        hwnd, NULL, NULL, NULL);
    hwndKfbEdit = CreateWindow("EDIT", kfbBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 200, yPos, smallEditWidth, 25,
        hwnd, (HMENU)36, NULL, NULL);

    yPos += 30;
    
    CreateWindow("STATIC", "CenterAfterAttack:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndCenterAfterAttackEdit = CreateWindow("EDIT", centerAfterAttackBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)37, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "CenterTriggerThreshold:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndCenterTriggerThresholdEdit = CreateWindow("EDIT", centerTriggerThresholdBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)38, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "EnemyTriggerThreshold:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndEnemyTriggerThresholdEdit = CreateWindow("EDIT", enemyTriggerThresholdBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)39, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "EnemyTriggerTimeout:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndEnemyTriggerTimeoutEdit = CreateWindow("EDIT", enemyTriggerTimeoutBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)40, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "ReverseAngleThreshold:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndReverseAngleThresholdEdit = CreateWindow("EDIT", reverseAngleThresholdBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)41, NULL, NULL);

    yPos += 30;
    
    CreateWindow("STATIC", "UseReverse:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 100, 25,
        hwnd, NULL, NULL, NULL);
    hwndUseReverseEdit = CreateWindow("EDIT", useReverseBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 120, yPos, smallEditWidth, 25,
        hwnd, (HMENU)42, NULL, NULL);
    
    yPos += 30;
    
    CreateWindow("STATIC", "AngleMinExtraSpeed:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndAngleMinExtraSpeedEdit = CreateWindow("EDIT", angleMinExtraSpeedBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)43, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "AngleMaxExtraSpeed:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndAngleMaxExtraSpeedEdit = CreateWindow("EDIT", angleMaxExtraSpeedBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)44, NULL, NULL);
    
    yPos += 30;

    CreateWindow("STATIC", "InPlaceSpeedRotation:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndInPlaceSpeedRotationEdit = CreateWindow("EDIT", InPlaceSpeedRotationBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)45, NULL, NULL);

    yPos += 30;

    CreateWindow("STATIC", "InPlaceSpeedWeapon:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndInPlaceSpeedWeaponEdit = CreateWindow("EDIT", InPlaceSpeedWeaponBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)46, NULL, NULL);
    
    yPos += 40;
    
    CreateWindow("STATIC", "Arena:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 200, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "Center:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 60, 25,
        hwnd, NULL, NULL, NULL);
    
    CreateWindow("STATIC", "x",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 75, yPos, 15, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndArenaXEdit = CreateWindow("EDIT", arenaXBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 95, yPos, smallEditWidth, 25,
        hwnd, (HMENU)12, NULL, NULL);
    
    CreateWindow("STATIC", "y",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 165, yPos, 15, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndArenaYEdit = CreateWindow("EDIT", arenaYBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 185, yPos, smallEditWidth, 25,
        hwnd, (HMENU)13, NULL, NULL);
    
    yPos += 35;
    
    CreateWindow("STATIC", "Select Center:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 100, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndArenaSelectSwitch = CreateWindow("BUTTON", "OFF",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        xStart + currentCol * colWidth + 120, yPos, 80, 30,
        hwnd, (HMENU)14, NULL, NULL);
    
    yPos += 40;
    
    CreateWindow("STATIC", "Size:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 60, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 25;
    
    CreateWindow("STATIC", "x1",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 25, 25,
        hwnd, NULL, NULL, NULL);
    hwndArenaSizeX1Edit = CreateWindow("EDIT", arenaSizeX1Buffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 50, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)30, NULL, NULL);
    
    CreateWindow("STATIC", "y1",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 110, yPos, 25, 25,
        hwnd, NULL, NULL, NULL);
    hwndArenaSizeY1Edit = CreateWindow("EDIT", arenaSizeY1Buffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 140, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)31, NULL, NULL);
    
    yPos += 35;
    
    CreateWindow("STATIC", "x2",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 25, 25,
        hwnd, NULL, NULL, NULL);
    hwndArenaSizeX2Edit = CreateWindow("EDIT", arenaSizeX2Buffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 50, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)32, NULL, NULL);
    
    CreateWindow("STATIC", "y2",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 110, yPos, 25, 25,
        hwnd, NULL, NULL, NULL);
    hwndArenaSizeY2Edit = CreateWindow("EDIT", arenaSizeY2Buffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 140, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)33, NULL, NULL);
    
    yPos += 40;
    
    hwndArenaSizeSelectBtn = CreateWindow(
        "BUTTON", "Select",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth + 20, yPos, 80, 30,
        hwnd, (HMENU)34, NULL, NULL);
    
    hwndArenaSizeResetBtn = CreateWindow(
        "BUTTON", "Reset",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth + 110, yPos, 80, 30,
        hwnd, (HMENU)35, NULL, NULL);
    
    currentCol++;
    yPos = 50;
    
    CreateWindow("STATIC", "Camera:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 150, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "EXPOSURE:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 80, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndCameraExposureEdit = CreateWindow("EDIT", cameraExposureBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 100, yPos, smallEditWidth, 25,
        hwnd, (HMENU)17, NULL, NULL);
    
    yPos += 35;
    
    hwndCameraApplyBtn = CreateWindow(
        "BUTTON", "Apply",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        xStart + currentCol * colWidth + 10, yPos, 80, 30,
        hwnd, (HMENU)18, NULL, NULL);
    
    yPos += 50;
    
    // ArucoDetector section
    CreateWindow("STATIC", "ArucoDetector:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 200, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "MinMarkerPerimeterRate:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndMinMarkerPerimeterRateEdit = CreateWindow("EDIT", minMarkerPerimeterRateBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)47, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "MaxMarkerPerimeterRate:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 170, 25,
        hwnd, NULL, NULL, NULL);
    hwndMaxMarkerPerimeterRateEdit = CreateWindow("EDIT", maxMarkerPerimeterRateBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 190, yPos, smallEditWidth, 25,
        hwnd, (HMENU)48, NULL, NULL);
    yPos += 40;
    
    CreateWindow("STATIC", "MotionDetector:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth, yPos, 200, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "Threshold:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 80, 25,
        hwnd, NULL, NULL, NULL);
    hwndMotionThresholdEdit = CreateWindow("EDIT", motionThresholdBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 100, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)20, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "Morphological ops:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 120, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "Kernel:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 60, 25,
        hwnd, NULL, NULL, NULL);
    hwndMorphKernelWidthEdit = CreateWindow("EDIT", morphKernelWidthBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 90, yPos, 30, 25,
        hwnd, (HMENU)21, NULL, NULL);
    
    CreateWindow("STATIC", "x",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 125, yPos, 15, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndMorphKernelHeightEdit = CreateWindow("EDIT", morphKernelHeightBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 145, yPos, 30, 25,
        hwnd, (HMENU)22, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "Dilation:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 70, 25,
        hwnd, NULL, NULL, NULL);
    hwndDilationIterationsEdit = CreateWindow("EDIT", dilationIterationsBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 100, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)23, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "Erosion:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 70, 25,
        hwnd, NULL, NULL, NULL);
    hwndErosionIterationsEdit = CreateWindow("EDIT", erosionIterationsBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 100, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)24, NULL, NULL);
    yPos += 40;
    
    CreateWindow("STATIC", "Gaussian blur:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 10, yPos, 120, 25,
        hwnd, NULL, NULL, NULL);
    yPos += 30;
    
    CreateWindow("STATIC", "Kernel:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 60, 25,
        hwnd, NULL, NULL, NULL);
    hwndBlurKernelWidthEdit = CreateWindow("EDIT", blurKernelWidthBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 90, yPos, 30, 25,
        hwnd, (HMENU)25, NULL, NULL);
    
    CreateWindow("STATIC", "x",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 125, yPos, 15, 25,
        hwnd, NULL, NULL, NULL);
    
    hwndBlurKernelHeightEdit = CreateWindow("EDIT", blurKernelHeightBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 145, yPos, 30, 25,
        hwnd, (HMENU)26, NULL, NULL);
    yPos += 35;
    
    CreateWindow("STATIC", "Sigma:",
        WS_VISIBLE | WS_CHILD,
        xStart + currentCol * colWidth + 20, yPos, 60, 25,
        hwnd, NULL, NULL, NULL);
    hwndBlurSigmaEdit = CreateWindow("EDIT", blurSigmaBuffer,
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
        xStart + currentCol * colWidth + 100, yPos, verySmallEditWidth, 25,
        hwnd, (HMENU)27, NULL, NULL);
    
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT hBoldFont = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    
    if (hFont && hBoldFont) {
        SendMessage(hwndCheckBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndStartBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndStopBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndCameraApplyBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndKpEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndKiEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndKdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndKrlEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndKfbEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndSwitch, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndArenaSelectSwitch, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndRobot1ExclusionEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaXEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaYEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndCameraExposureEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        // ArucoDetector fonts
        SendMessage(hwndMinMarkerPerimeterRateEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndMaxMarkerPerimeterRateEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        SendMessage(hwndMotionThresholdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndMorphKernelWidthEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndMorphKernelHeightEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndDilationIterationsEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndErosionIterationsEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndBlurKernelWidthEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndBlurKernelHeightEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndBlurSigmaEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        SendMessage(hwndArenaSizeX1Edit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaSizeY1Edit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaSizeX2Edit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaSizeY2Edit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndArenaSizeSelectBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndArenaSizeResetBtn, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SendMessage(hwndRobot1OffsetEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        SendMessage(hwndCenterAfterAttackEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndCenterTriggerThresholdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndEnemyTriggerThresholdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndEnemyTriggerTimeoutEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndReverseAngleThresholdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndUseReverseEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        SendMessage(hwndAngleMinExtraSpeedEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndAngleMaxExtraSpeedEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        EnumChildWindows(hwnd, [](HWND hwndChild, LPARAM lParam) -> BOOL {
            char className[256];
            GetClassName(hwndChild, className, sizeof(className));
            if (strcmp(className, "Static") == 0) {
                SendMessage(hwndChild, WM_SETFONT, lParam, TRUE);
            }
            return TRUE;
        }, (LPARAM)hBoldFont);
        
        DeleteObject(hFont);
        DeleteObject(hBoldFont);
    }
}

void ControlPanel::paintWindow(HWND hwnd, HDC hdc) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    HBRUSH hBrush = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    
    HFONT hTitleFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HGDIOBJ hOldFont = SelectObject(hdc, hTitleFont);
    SetTextColor(hdc, RGB(40, 40, 40));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, 25, 15, "CakeAI Control Panel", 20);
    
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    MoveToEx(hdc, 20, 45, NULL);
    LineTo(hdc, 280, 45);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hTitleFont);
}

void ControlPanel::run() {
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "CakeAIControlPanel";
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "Failed to register window class!" << std::endl;
        return;
    }
    
    hwndMain = CreateWindowEx(
        0,
        "CakeAIControlPanel",
        "CakeAI Control Panel",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 850,
        NULL, NULL, wc.hInstance, NULL);
    
    if (!hwndMain) {
        std::cerr << "Failed to create window!" << std::endl;
        return;
    }
    
    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);
    
    MSG msg = {0};
    while (isRunning_ && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void ControlPanel::stop() {
    isRunning_ = false;
    if (hwndMain) {
        PostMessage(hwndMain, WM_CLOSE, 0, 0);
    }
}

void ControlPanel::setKpValue(const std::string& value) {
    strncpy(kpBuffer, value.c_str(), sizeof(kpBuffer));
    if (hwndKpEdit) {
        SetWindowText(hwndKpEdit, kpBuffer);
    }
}

void ControlPanel::setKiValue(const std::string& value) {
    strncpy(kiBuffer, value.c_str(), sizeof(kiBuffer));
    if (hwndKiEdit) {
        SetWindowText(hwndKiEdit, kiBuffer);
    }
}

void ControlPanel::setKdValue(const std::string& value) {
    strncpy(kdBuffer, value.c_str(), sizeof(kdBuffer));
    if (hwndKdEdit) {
        SetWindowText(hwndKdEdit, kdBuffer);
    }
}

void ControlPanel::setKrlValue(const std::string& value) {
    strncpy(krlBuffer, value.c_str(), sizeof(krlBuffer));
    if (hwndKrlEdit) {
        SetWindowText(hwndKrlEdit, krlBuffer);
    }
}

void ControlPanel::setKfbValue(const std::string& value) {
    strncpy(kfbBuffer, value.c_str(), sizeof(kfbBuffer));
    if (hwndKfbEdit) {
        SetWindowText(hwndKfbEdit, kfbBuffer);
    }
}

void ControlPanel::setCenterAfterAttackValue(const std::string& value) {
    strncpy(centerAfterAttackBuffer, value.c_str(), sizeof(centerAfterAttackBuffer));
    if (hwndCenterAfterAttackEdit) {
        SetWindowText(hwndCenterAfterAttackEdit, centerAfterAttackBuffer);
    }
}

void ControlPanel::setCenterTriggerThresholdValue(const std::string& value) {
    strncpy(centerTriggerThresholdBuffer, value.c_str(), sizeof(centerTriggerThresholdBuffer));
    if (hwndCenterTriggerThresholdEdit) {
        SetWindowText(hwndCenterTriggerThresholdEdit, centerTriggerThresholdBuffer);
    }
}
void ControlPanel::setEnemyTriggerThresholdValue(const std::string& value) {
    strncpy(enemyTriggerThresholdBuffer, value.c_str(), sizeof(enemyTriggerThresholdBuffer));
    if (hwndEnemyTriggerThresholdEdit) {
        SetWindowText(hwndEnemyTriggerThresholdEdit, enemyTriggerThresholdBuffer);
    }
}
void ControlPanel::setEnemyTriggerTimeoutValue(const std::string& value) {
    strncpy(enemyTriggerTimeoutBuffer, value.c_str(), sizeof(enemyTriggerTimeoutBuffer));
    if (hwndEnemyTriggerTimeoutEdit) {
        SetWindowText(hwndEnemyTriggerTimeoutEdit, enemyTriggerTimeoutBuffer);
    }
}
void ControlPanel::setReverseAngleThresholdValue(const std::string& value) {
    strncpy(reverseAngleThresholdBuffer, value.c_str(), sizeof(reverseAngleThresholdBuffer));
    if (hwndReverseAngleThresholdEdit) {
        SetWindowText(hwndReverseAngleThresholdEdit, reverseAngleThresholdBuffer);
    }
}
void ControlPanel::setUseReverseValue(const std::string& value) {
    strncpy(useReverseBuffer, value.c_str(), sizeof(useReverseBuffer));
    if (hwndUseReverseEdit) {
        SetWindowText(hwndUseReverseEdit, useReverseBuffer);
    }
}

void ControlPanel::setAngleMinExtraSpeedValue(const std::string& value) {
    strncpy(angleMinExtraSpeedBuffer, value.c_str(), sizeof(angleMinExtraSpeedBuffer));
    if (hwndAngleMinExtraSpeedEdit) {
        SetWindowText(hwndAngleMinExtraSpeedEdit, angleMinExtraSpeedBuffer);
    }
}

void ControlPanel::setAngleMaxExtraSpeedValue(const std::string& value) {
    strncpy(angleMaxExtraSpeedBuffer, value.c_str(), sizeof(angleMaxExtraSpeedBuffer));
    if (hwndAngleMaxExtraSpeedEdit) {
        SetWindowText(hwndAngleMaxExtraSpeedEdit, angleMaxExtraSpeedBuffer);
    }
}

void ControlPanel::setInPlaceSpeedRotationValue(const std::string& value) {
    strncpy(InPlaceSpeedRotationBuffer, value.c_str(), sizeof(InPlaceSpeedRotationBuffer));
    if (hwndInPlaceSpeedRotationEdit) {
        SetWindowText(hwndInPlaceSpeedRotationEdit, InPlaceSpeedRotationBuffer);
    }
}

void ControlPanel::setInPlaceSpeedWeaponValue(const std::string& value) {
    strncpy(InPlaceSpeedWeaponBuffer, value.c_str(), sizeof(InPlaceSpeedWeaponBuffer));
    if (hwndInPlaceSpeedWeaponEdit) {
        SetWindowText(hwndInPlaceSpeedWeaponEdit, InPlaceSpeedWeaponBuffer);
    }
}

void ControlPanel::setRobot1ExclusionValue(const std::string& value) {
    strncpy(robot1ExclusionBuffer, value.c_str(), sizeof(robot1ExclusionBuffer));
    if (hwndRobot1ExclusionEdit) {
        SetWindowText(hwndRobot1ExclusionEdit, robot1ExclusionBuffer);
    }
}

void ControlPanel::setArenaXValue(const std::string& value) {
    strncpy(arenaXBuffer, value.c_str(), sizeof(arenaXBuffer));
    if (hwndArenaXEdit) {
        SetWindowText(hwndArenaXEdit, arenaXBuffer);
    }
}

void ControlPanel::setArenaYValue(const std::string& value) {
    strncpy(arenaYBuffer, value.c_str(), sizeof(arenaYBuffer));
    if (hwndArenaYEdit) {
        SetWindowText(hwndArenaYEdit, arenaYBuffer);
    }
}

void ControlPanel::setArenaSizeX1Value(const std::string& value) {
    strncpy(arenaSizeX1Buffer, value.c_str(), sizeof(arenaSizeX1Buffer));
    if (hwndArenaSizeX1Edit) {
        SetWindowText(hwndArenaSizeX1Edit, arenaSizeX1Buffer);
    }
}

void ControlPanel::setArenaSizeY1Value(const std::string& value) {
    strncpy(arenaSizeY1Buffer, value.c_str(), sizeof(arenaSizeY1Buffer));
    if (hwndArenaSizeY1Edit) {
        SetWindowText(hwndArenaSizeY1Edit, arenaSizeY1Buffer);
    }
}

void ControlPanel::setArenaSizeX2Value(const std::string& value) {
    strncpy(arenaSizeX2Buffer, value.c_str(), sizeof(arenaSizeX2Buffer));
    if (hwndArenaSizeX2Edit) {
        SetWindowText(hwndArenaSizeX2Edit, arenaSizeX2Buffer);
    }
}

void ControlPanel::setArenaSizeY2Value(const std::string& value) {
    strncpy(arenaSizeY2Buffer, value.c_str(), sizeof(arenaSizeY2Buffer));
    if (hwndArenaSizeY2Edit) {
        SetWindowText(hwndArenaSizeY2Edit, arenaSizeY2Buffer);
    }
}

void ControlPanel::setCameraExposureValue(const std::string& value) {
    strncpy(cameraExposureBuffer, value.c_str(), sizeof(cameraExposureBuffer));
    if (hwndCameraExposureEdit) {
        SetWindowText(hwndCameraExposureEdit, cameraExposureBuffer);
    }
}

void ControlPanel::setSwitchState(bool state) {
    switchState_ = state;
    if (hwndSwitch) {
        SendMessage(hwndSwitch, BM_SETCHECK, state ? BST_CHECKED : BST_UNCHECKED, 0);
        SetWindowText(hwndSwitch, state ? "ON" : "OFF");
    }
}

void ControlPanel::setArenaSelectState(bool state) {
    if (hwndArenaSelectSwitch) {
        SendMessage(hwndArenaSelectSwitch, BM_SETCHECK, state ? BST_CHECKED : BST_UNCHECKED, 0);
        SetWindowText(hwndArenaSelectSwitch, state ? "ON" : "OFF");
    }
}

// ArucoDetector setters
void ControlPanel::setMinMarkerPerimeterRateValue(const std::string& value) {
    strncpy(minMarkerPerimeterRateBuffer, value.c_str(), sizeof(minMarkerPerimeterRateBuffer));
    if (hwndMinMarkerPerimeterRateEdit) {
        SetWindowText(hwndMinMarkerPerimeterRateEdit, minMarkerPerimeterRateBuffer);
    }
}

void ControlPanel::setMaxMarkerPerimeterRateValue(const std::string& value) {
    strncpy(maxMarkerPerimeterRateBuffer, value.c_str(), sizeof(maxMarkerPerimeterRateBuffer));
    if (hwndMaxMarkerPerimeterRateEdit) {
        SetWindowText(hwndMaxMarkerPerimeterRateEdit, maxMarkerPerimeterRateBuffer);
    }
}

void ControlPanel::setMotionThresholdValue(const std::string& value) {
    strncpy(motionThresholdBuffer, value.c_str(), sizeof(motionThresholdBuffer));
    if (hwndMotionThresholdEdit) {
        SetWindowText(hwndMotionThresholdEdit, motionThresholdBuffer);
    }
}

void ControlPanel::setMorphKernelWidthValue(const std::string& value) {
    strncpy(morphKernelWidthBuffer, value.c_str(), sizeof(morphKernelWidthBuffer));
    if (hwndMorphKernelWidthEdit) {
        SetWindowText(hwndMorphKernelWidthEdit, morphKernelWidthBuffer);
    }
}

void ControlPanel::setMorphKernelHeightValue(const std::string& value) {
    strncpy(morphKernelHeightBuffer, value.c_str(), sizeof(morphKernelHeightBuffer));
    if (hwndMorphKernelHeightEdit) {
        SetWindowText(hwndMorphKernelHeightEdit, morphKernelHeightBuffer);
    }
}

void ControlPanel::setDilationIterationsValue(const std::string& value) {
    strncpy(dilationIterationsBuffer, value.c_str(), sizeof(dilationIterationsBuffer));
    if (hwndDilationIterationsEdit) {
        SetWindowText(hwndDilationIterationsEdit, dilationIterationsBuffer);
    }
}

void ControlPanel::setErosionIterationsValue(const std::string& value) {
    strncpy(erosionIterationsBuffer, value.c_str(), sizeof(erosionIterationsBuffer));
    if (hwndErosionIterationsEdit) {
        SetWindowText(hwndErosionIterationsEdit, erosionIterationsBuffer);
    }
}

void ControlPanel::setBlurKernelWidthValue(const std::string& value) {
    strncpy(blurKernelWidthBuffer, value.c_str(), sizeof(blurKernelWidthBuffer));
    if (hwndBlurKernelWidthEdit) {
        SetWindowText(hwndBlurKernelWidthEdit, blurKernelWidthBuffer);
    }
}

void ControlPanel::setBlurKernelHeightValue(const std::string& value) {
    strncpy(blurKernelHeightBuffer, value.c_str(), sizeof(blurKernelHeightBuffer));
    if (hwndBlurKernelHeightEdit) {
        SetWindowText(hwndBlurKernelHeightEdit, blurKernelHeightBuffer);
    }
}

void ControlPanel::setBlurSigmaValue(const std::string& value) {
    strncpy(blurSigmaBuffer, value.c_str(), sizeof(blurSigmaBuffer));
    if (hwndBlurSigmaEdit) {
        SetWindowText(hwndBlurSigmaEdit, blurSigmaBuffer);
    }
}

void ControlPanel::setRobot1OffsetValue(const std::string& value) {
    strncpy(robot1OffsetBuffer, value.c_str(), sizeof(robot1OffsetBuffer));
    if (hwndRobot1OffsetEdit) {
        SetWindowText(hwndRobot1OffsetEdit, robot1OffsetBuffer);
    }
}