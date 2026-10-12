// Copyright 2026 Vlad Kolmogorov
// SPDX-License-Identifier: Apache-2.0

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <chrono>
#include <limits>

class PIDController {
private:
    // Коэффициенты
    double Kp, Ki, Kd;
    
    // Состояние регулятора
    double integral;
    double previous_error;
    
    // Ограничения
    double output_min;
    double output_max;
    
    // Для расчета времени
    std::chrono::steady_clock::time_point last_time;
    bool first_run;
    
public:
    /**
     * Конструктор ПИД-регулятора
     * @param kp Пропорциональный коэффициент
     * @param ki Интегральный коэффициент
     * @param kd Дифференциальный коэффициент
     * @param min Минимальное выходное значение (по умолчанию -макс)
     * @param max Максимальное выходное значение (по умолчанию +макс)
     */
    PIDController(double kp = 1.0, double ki = 0.0, double kd = 0.0, 
                  double min = -std::numeric_limits<double>::max(),
                  double max = std::numeric_limits<double>::max())
        : Kp(kp), Ki(ki), Kd(kd),
          integral(0), previous_error(0),
          output_min(min), output_max(max),
          first_run(true) {
        // Инициализируем время
        last_time = std::chrono::steady_clock::now();
    }
    
    /**
     * Основная функция расчета управления
     * @param target_angle Целевой угол (куда хотим повернуться)
     * @param current_angle Текущий угол (где сейчас находимся)
     * @return Управляющее воздействие (сигнал для мотора)
     */
    double calculate(double target_angle, double current_angle) {
        // 1. Рассчитываем ошибку
        double error = target_angle - current_angle;
        
        // 2. Рассчитываем dt (время с предыдущего вызова)
        auto now = std::chrono::steady_clock::now();
        double dt = 0.01; // значение по умолчанию на первый запуск
        
        if (!first_run) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
            dt = duration.count() / 1000.0; // преобразуем в секунды
            if (dt <= 0) dt = 0.001; // защита от нулевого или отрицательного dt
        }
        
        // 3. Пропорциональная составляющая
        double P = Kp * error;
        
        // 4. Интегральная составляющая
        integral += error * dt;
        double I = Ki * integral;
        
        // 5. Дифференциальная составляющая
        double D = 0;
        if (dt > 0 && !first_run) {
            D = Kd * (error - previous_error) / dt;
        }
        
        // 6. Суммируем все составляющие
        double output = P + I + D;
        
        // 7. Ограничиваем выходное значение
        if (output > output_max) {
            output = output_max;
            // Простой антивиндовп (замораживаем интеграл при насыщении)
            if (error * output > 0) { // если знаки совпадают
                integral -= error * dt; // откатываем интегрирование
            }
        } else if (output < output_min) {
            output = output_min;
            // Антивиндовп для нижнего предела
            if (error * output > 0) {
                integral -= error * dt;
            }
        }
        
        // 8. Сохраняем состояние для следующего вызова
        previous_error = error;
        last_time = now;
        first_run = false;
        
        return output;
    }
    
    /**
     * Функция для вашей задачи с тележкой
     * Рассчитывает скорости для левого и правого колес
     * @param target_angle Целевой угол (0 - вертикальное положение)
     * @param current_angle Текущий угол с камеры
     * @param left_speed [out] Скорость левого мотора
     * @param right_speed [out] Скорость правого мотора
     */
    void calculateWheelSpeeds(double target_angle, double current_angle,
                              double& left_speed, double& right_speed) {
        // Получаем управляющий сигнал от ПИД
        double pid_output = calculate(target_angle, current_angle);
        
        // Для балансировки: V_command = 0, поэтому
        // правое колесо = +pid_output, левое = -pid_output
        right_speed = pid_output;
        left_speed = -pid_output;
    }
    
    /**
     * Сброс состояния регулятора (использовать при старте/перезапуске)
     */
    void reset() {
        integral = 0;
        previous_error = 0;
        first_run = true;
        last_time = std::chrono::steady_clock::now();
    }
    
    /**
     * Установка новых коэффициентов
     */
    void setParameters(double kp, double ki, double kd) {
        Kp = kp;
        Ki = ki;
        Kd = kd;
        reset(); // сбрасываем состояние при изменении коэффициентов
    }
    
    /**
     * Установка ограничений выходного сигнала
     */
    void setOutputLimits(double min, double max) {
        output_min = min;
        output_max = max;
    }
    
    /**
     * Получение текущих коэффициентов
     */
    void getParameters(double& kp, double& ki, double& kd) const {
        kp = Kp;
        ki = Ki;
        kd = Kd;
    }
    
    /**
     * Получение текущего значения интеграла (для отладки)
     */
    double getIntegral() const {
        return integral;
    }
    
    /**
     * Получение предыдущей ошибки (для отладки)
     */
    double getPreviousError() const {
        return previous_error;
    }
};

#endif // PID_CONTROLLER_H