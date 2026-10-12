// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#include <Wire.h>
#include <Adafruit_MCP4725.h>

#define TCA9548A_ADDRESS 0x70
Adafruit_MCP4725 dac;

const int DAC_MIN = 170;
const int DAC_MAX = 4000;

#define DAC1_CHANNEL 5
#define DAC2_CHANNEL 6  
#define DAC3_CHANNEL 7

void selectMUXChannel(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
  delay(1);
}

int interpolate(int input) {
  input = constrain(input, 0, 1023);
  
  if (input == 0) return DAC_MIN;
  
  if (input < 50) {
    float t = input / 50.0;
    return DAC_MIN + t * (300 - DAC_MIN);
  }
  
  return map(input, 50, 1023, 300, DAC_MAX);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  uint8_t dacAddress = 0x60;
  uint8_t dacChannels[] = {DAC1_CHANNEL, DAC2_CHANNEL, DAC3_CHANNEL};
  
  for (int i = 0; i < 3; i++) {
    selectMUXChannel(dacChannels[i]);
    if (dac.begin(dacAddress)) {
      dac.setVoltage(DAC_MIN, false);
    }
  }
  
  selectMUXChannel(DAC1_CHANNEL);
  Serial.println("READY");
}

void setDAC(int dacNumber, int value, bool isPercent) {
  uint8_t channel;
  switch(dacNumber) {
    case 1: channel = DAC1_CHANNEL; break;
    case 2: channel = DAC2_CHANNEL; break;
    case 3: channel = DAC3_CHANNEL; break;
    default: return;
  }
  
  int dacValue;
  
  if (isPercent) {
    int percent = constrain(value, 0, 100);
    dacValue = map(percent, 0, 100, DAC_MIN, DAC_MAX);
  } else {
    dacValue = interpolate(value);
  }
  
  dacValue = constrain(dacValue, DAC_MIN, DAC_MAX);
  selectMUXChannel(channel);
  dac.setVoltage(dacValue, false);
  
  Serial.print("OK ");
  Serial.print(dacNumber);
  Serial.print(":");
  Serial.print(isPercent ? "P:" : "Y:");
  Serial.print(value);
  Serial.print("->");
  Serial.print(dacValue);
  Serial.println();
}

void setAllDAC(int value, bool isPercent) {
  int dacValue;
  
  if (isPercent) {
    int percent = constrain(value, 0, 100);
    dacValue = map(percent, 0, 100, DAC_MIN, DAC_MAX);
  } else {
    dacValue = interpolate(value);
  }
  
  dacValue = constrain(dacValue, DAC_MIN, DAC_MAX);
  
  selectMUXChannel(DAC1_CHANNEL);
  dac.setVoltage(dacValue, false);
  selectMUXChannel(DAC2_CHANNEL);
  dac.setVoltage(dacValue, false);
  selectMUXChannel(DAC3_CHANNEL);
  dac.setVoltage(dacValue, false);
  selectMUXChannel(DAC1_CHANNEL);
  
  Serial.print("OK ALL:");
  Serial.print(isPercent ? "P:" : "Y:");
  Serial.print(value);
  Serial.print("->");
  Serial.print(dacValue);
  Serial.println();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command.length() < 4) return;
    
    char dacSelector = command.charAt(0);
    if (command.charAt(1) != ':') return;
    
    int colonIndex = command.indexOf(':', 2);
    if (colonIndex == -1) return;
    
    String valueStr = command.substring(colonIndex + 1);
    int value = valueStr.toInt();
    
    if (dacSelector >= '1' && dacSelector <= '3') {
      int dacNumber = dacSelector - '0';
      
      if (command.charAt(2) == 'Y') {
        setDAC(dacNumber, value, false);
      } else if (command.charAt(2) == 'P') {
        setDAC(dacNumber, value, true);
      }
    } else if (dacSelector == 'A') {
      if (command.charAt(2) == 'Y') {
        setAllDAC(value, false);
      } else if (command.charAt(2) == 'P') {
        setAllDAC(value, true);
      }
    }
  }
}