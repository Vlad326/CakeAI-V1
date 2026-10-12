# CakeAI-V1

Autonomous combat robot driven by an overhead camera: tracks itself by an
ArUco marker, finds the opponent by motion detection, and controls the
robot through an Arduino — no human at the controls.

This is the version the robot CakeAI (Торт ИИ) competed with at
[Crashilovo](https://crashilovo.ru/) on April 25, 2026. The current version is not public.

https://github.com/user-attachments/assets/5153ef42-87cf-4e12-afa7-24857af2e5f4

## How it works

- **Overhead camera** sees the whole arena.
- **Own robot:** position and heading from an ArUco marker on top.
- **Opponent:** frame-to-frame motion detection, with our own robot
  excluded.
- **Behaviour:** return to the centre → attack → search when the opponent
  is lost; a PID controller steers towards the target.
- **Arduino** receives drive and weapon commands over serial
  (firmware: `src/ArduinoProgram.ino`).
- **Control panel** for live tuning; values are saved to `config.ini`.

**Stack:** C++17, OpenCV 4, CMake, Windows (MSYS2 UCRT64), Arduino.

## Download

In the **MSYS2 UCRT64** terminal:

```bash
pacman -S git
git clone https://github.com/Vlad326/CakeAI-V1.git
cd CakeAI-V1
```

## Build

Install the compiler, CMake and OpenCV (once):

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-opencv
```

Build:

```bash
mkdir -p build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

Run:

```bash
./main.exe
```

`build/config.ini` contains the settings CakeAI used at Crashilovo
(April 25, 2026): tuned for that robot, camera and arena. The program
reads it from the folder it is run in.

**Warning:** on exit the program saves the current panel values back to
`config.ini`, overwriting it.

Before the first run, check:

- **Arduino port** is hard-coded as `COM3` in `src/main.cpp` — change it
  to your port.
- **Camera exposure** from `config.ini` is not applied at startup — set it
  for your lighting in the control panel and press **Apply**. The program
  uses the first camera (index 0).

A camera and an Arduino with the firmware above are needed to drive a
real robot. Parameters are tuned for a specific robot, camera and arena —
expect to tune your own.

## License

[Apache License 2.0](LICENSE). Author: Vlad Kolmogorov ([Vlad326](https://github.com/Vlad326)).
