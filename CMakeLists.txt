cmake_minimum_required(VERSION 3.10)
project(ArUcoDetector)

set(CMAKE_CXX_STANDARD 17)

# Поиск OpenCV
find_package(OpenCV REQUIRED)

# Включаем каталог заголовков
include_directories(${OpenCV_INCLUDE_DIRS} include)

# Создаем исполняемый файл с именем main
add_executable(main
    src/main.cpp
    src/ArucoDetector.cpp
    src/MotionDetector.cpp
    src/ControlPanel.cpp
    src/RobotController.cpp
    src/ConfigManager.cpp
)

# Подключаем OpenCV
target_link_libraries(main ${OpenCV_LIBS})

# Для Windows
if(WIN32)
    target_link_libraries(main
        gdi32
        user32
    )
endif()

# Оптимизации компилятора
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(main PRIVATE
        -O3
        -march=native
        -ffast-math
        -Wall
        -Wextra
    )
endif()