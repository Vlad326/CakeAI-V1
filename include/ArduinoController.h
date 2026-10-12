// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef ARDUINO_CONTROLLER_H
#define ARDUINO_CONTROLLER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <termios.h>
    #include <unistd.h>
#endif

class ArduinoController {
private:
#ifdef _WIN32
    HANDLE hSerial;
#else
    int serial_fd;
#endif
    std::string port;
    int baudrate;

    // Check if port is open
    bool is_open() {
#ifdef _WIN32
        return hSerial != INVALID_HANDLE_VALUE;
#else
        return serial_fd != -1;
#endif
    }

public:
    // Constructor
    ArduinoController(const std::string& port_name = "COM3", int baud = 115200) 
        : port(port_name), baudrate(baud) {
#ifdef _WIN32
        hSerial = INVALID_HANDLE_VALUE;
#else
        serial_fd = -1;
#endif
        open_serial();
    }

    // Destructor
    ~ArduinoController() {
        close();
    }

    // Open serial port
    bool open_serial() {
#ifdef _WIN32
        // Windows
        hSerial = CreateFileA(port.c_str(),
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             0,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             0);
        
        if (hSerial == INVALID_HANDLE_VALUE) {
            std::cerr << "Error opening port " << port << std::endl;
            return false;
        }

        // Configure port parameters
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            std::cerr << "Error getting port state" << std::endl;
            return false;
        }
        
        dcbSerialParams.BaudRate = baudrate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            std::cerr << "Error configuring port" << std::endl;
            return false;
        }

        // Timeouts
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;
        
        if (!SetCommTimeouts(hSerial, &timeouts)) {
            std::cerr << "Error setting timeouts" << std::endl;
            return false;
        }
#else
        // Linux/macOS
        serial_fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        
        if (serial_fd == -1) {
            std::cerr << "Error opening port " << port << std::endl;
            return false;
        }

        // Configure port parameters
        termios tty;
        memset(&tty, 0, sizeof(tty));
        
        if (tcgetattr(serial_fd, &tty) != 0) {
            std::cerr << "Error getting port parameters" << std::endl;
            return false;
        }

        cfsetospeed(&tty, baudrate);
        cfsetispeed(&tty, baudrate);

        tty.c_cflag &= ~PARENB;        // No parity bit
        tty.c_cflag &= ~CSTOPB;        // 1 stop bit
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;            // 8 data bits
        tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
        tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore line control
        
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
        tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Non-canonical mode
        
        tty.c_oflag &= ~OPOST; // Raw output

        // Timeouts
        tty.c_cc[VMIN] = 0;    // Minimum characters to read
        tty.c_cc[VTIME] = 5;   // Timeout in tenths of seconds

        if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
            std::cerr << "Error setting port parameters" << std::endl;
            return false;
        }
#endif
        
        std::cout << "Port " << port << " opened successfully" << std::endl;
        return true;
    }

    // Send command (БЕЗ ЗАДЕРЖЕК!)
    void send_command(const std::string& cmd) {
        if (!is_open()) {
            std::cerr << "Port is not open!" << std::endl;
            return;
        }

        std::string full_cmd = cmd + "\n";
        
#ifdef _WIN32
        DWORD bytes_written;
        WriteFile(hSerial, full_cmd.c_str(), full_cmd.length(), &bytes_written, NULL);
#else
        write(serial_fd, full_cmd.c_str(), full_cmd.length());
#endif
        // НИКАКИХ ЗАДЕРЖЕК!
    }

    void robot_check() {
        std::cout << "Starting robot check..." << std::endl;
        
        // Используем существующий метод set_motors
        set_motors(50, 50, 0);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        set_motors(70, 70, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        set_motors(30, 30, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        set_motors(50, 50, 0);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        const int ON_STEPS = 40;
        for (int i = 0; i <= ON_STEPS; i++) {
            float progress = static_cast<float>(i) / ON_STEPS;
            int ch3_value = static_cast<int>(40 * progress);
            set_motors(50, 50, ch3_value);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        const int OFF_STEPS = 40;
        for (int i = OFF_STEPS; i >= 0; i--) {
            float progress = static_cast<float>(i) / OFF_STEPS;
            int ch3_value = static_cast<int>(40 * progress);
            set_motors(50, 50, ch3_value);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "Robot check completed" << std::endl;
    }
    
    // Set Y value (0-1023) for DAC
    void set_dac_raw(int channel, int value) {
        if (channel < 1 || channel > 3) {
            std::cerr << "Error: channel must be 1, 2, or 3" << std::endl;
            return;
        }
        
        value = std::max(0, std::min(1023, value));
        std::string cmd = std::to_string(channel) + ":Y:" + std::to_string(value);
        send_command(cmd);
    }

    // Set motor power in percent (0-100%)
    void set_motor(int channel, int percent) {
        if (channel < 1 || channel > 3) {
            std::cerr << "Error: channel must be 1, 2, or 3" << std::endl;
            return;
        }
        
        percent = std::max(0, std::min(100, percent));
        std::string cmd = std::to_string(channel) + ":P:" + std::to_string(percent);
        
        send_command(cmd);
    }
    // Set different power for all motors (3 ОТДЕЛЬНЫЕ КОМАНДЫ)
    void set_motors(int percent_1, int percent_2, int percent_3, bool reverse_direction = false) {
        if (reverse_direction) {
            std::swap(percent_1, percent_2);
            percent_1 = 100 - percent_1;
            percent_2 = 100 - percent_2;
        }
        
        // 3 простые команды, без всяких пакетов
        set_motor(1, percent_1);
        set_motor(2, percent_2); 
        set_motor(3, percent_3);
    }

    // Set same RAW value for all DAC channels
    void set_all_dac_raw(int value) {
        value = std::max(0, std::min(1023, value));
        std::string cmd = "A:Y:" + std::to_string(value);
        send_command(cmd);
    }

    // Set same power for all motors
    void set_all_motors(int percent) {
        percent = std::max(0, std::min(100, percent));
        std::string cmd = "A:P:" + std::to_string(percent);
        send_command(cmd);
    }

    // Stop all motors
    void stop_all() {
        set_all_motors(50); // 50% = neutral position for most motors
        std::cout << "All motors stopped" << std::endl;
    }

    // Interactive mode
    void interactive_mode() {
        std::cout << "\n=== Arduino Controller Interactive Mode ===" << std::endl;
        std::cout << "Available commands:" << std::endl;
        std::cout << "  1Y<value> - Channel 1, RAW DAC (0-1023)" << std::endl;
        std::cout << "  1P<value> - Channel 1, % power (0-100)" << std::endl;
        std::cout << "  AY<value> - All channels, RAW DAC (0-1023)" << std::endl;
        std::cout << "  AP<value> - All channels, % power (0-100)" << std::endl;
        std::cout << "  STOP      - Stop all motors" << std::endl;
        std::cout << "  CHECK     - Run robot check" << std::endl;
        std::cout << "  Q         - Exit" << std::endl;
        std::cout << "==========================================" << std::endl;
        
        std::string cmd;
        while (true) {
            std::cout << "Enter command: ";
            std::getline(std::cin, cmd);
            
            // Convert to uppercase
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
            
            if (cmd == "Q" || cmd == "EXIT" || cmd == "QUIT") {
                std::cout << "Exiting interactive mode" << std::endl;
                break;
            }
            
            if (cmd == "STOP") {
                stop_all();
                continue;
            }
            
            if (cmd == "CHECK") {
                robot_check();
                continue;
            }
            
            if (cmd.length() < 3) {
                std::cerr << "Error: command too short" << std::endl;
                continue;
            }
            
            char dac_sel = cmd[0];
            char mode = cmd[1];
            
            if ((dac_sel == '1' || dac_sel == '2' || dac_sel == '3' || dac_sel == 'A') &&
                (mode == 'Y' || mode == 'P')) {
                
                try {
                    int value = std::stoi(cmd.substr(2));
                    
                    if (dac_sel == 'A') {
                        if (mode == 'Y') {
                            set_all_dac_raw(value);
                        } else {
                            set_all_motors(value);
                        }
                    } else {
                        int channel = dac_sel - '0';
                        if (mode == 'Y') {
                            set_dac_raw(channel, value);
                        } else {
                            set_motor(channel, value);
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: invalid value" << std::endl;
                }
            } else {
                std::cerr << "Error: invalid command format" << std::endl;
            }
        }
    }

    // Close connection
    void close() {
        if (is_open()) {
            stop_all(); // Stop motors before closing
            
#ifdef _WIN32
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
#else
            ::close(serial_fd);
            serial_fd = -1;
#endif
            std::cout << "Connection closed" << std::endl;
        }
    }

    // Check connection
    bool is_connected() const {
#ifdef _WIN32
        return hSerial != INVALID_HANDLE_VALUE;
#else
        return serial_fd != -1;
#endif
    }

    // Get port name
    std::string get_port() const {
        return port;
    }

    // Get baudrate
    int get_baudrate() const {
        return baudrate;
    }
};

#endif // ARDUINO_CONTROLLER_H