# stm32-exam —— 软2：VOFA+ 调舵机（STM32F401RET6）

> 一句话：VOFA+ 发目标角度 → 舵机转到 → MCU 20Hz 回传"指令角 + PWM 占空比"四通道 JustFloat 波形。
> 对应《G308 电控组 2026 夏季考核题》软件第二题。

## 交付物

| 题面提交要求 | 位置 |
| --- | --- |
| （1）项目文件 | 本仓库源码 |
| （2）开发文档 | [`docs/02-开发文档.md`](docs/02-开发文档.md) |
| （3）实操视频 | [`docs/video/soft2-demo.mp4`](docs/video/soft2-demo.mp4) |

软件第一题（现代开发环境与仓库说明）也放在本仓库：[`docs/01-开发环境与仓库说明.md`](docs/01-开发环境与仓库说明.md)，内含四个考核仓库的索引。

## 当前状态

- 编译通过：arm-gcc 14.3 + CMake + Ninja，`text 11.6KB`
- **占空比回传修正已完成实测**：VOFA+ 实测读数 ch0=30.000 / ch1=4.165 / ch2=90.000 / ch3=7.500，与手算值一致（2026-09-11 取证）


## 接线（引脚级）

| F401 | 去向 | 备注 |
| --- | --- | --- |
| PA9 / PA10 | USB 转串口 / DAPLink VCP | USART1，115200 |
| PA6 (TIM3_CH1) | 舵机1 信号（橙） | 50Hz PWM |
| PA7 (TIM3_CH2) | 舵机2 信号（橙） | 同上 |
| — | 舵机红线 | **外接 5V≥3A**，与板共地；996R 堵转 2.5A，别吃 USB 电 |

## 协议（下发自定义二进制，回传 JustFloat）

| 方向 | 帧 | 含义 |
| --- | --- | --- |
| PC → MCU | 2×float + 尾 `00 00 80 7F`（12B） | pan/tilt 目标角 0~180；VOFA+ 命令区用 Hex 发送 |
| MCU → PC | 4×float + 尾（20B）@20Hz | ch0 pan 指令角（deg）、ch1 pan PWM 占空比（%）、ch2 tilt 指令角（deg）、ch3 tilt PWM 占空比（%） |

VOFA+ 1.3.10 的命令编辑器只有 `Hex` 和 `ABC` 两种格式。本工程下发端使用 `Hex`，接收端数据引擎使用 `JustFloat`。测试 pan=30°、tilt=90° 时，命令内容为：

```text
00 00 F0 41 00 00 B4 42 00 00 80 7F
```

其中 `00 00 F0 41` 是小端 float `30.0`，`00 00 B4 42` 是小端 float `90.0`。不要把 `30 90` 当作 Hex 字节发送，也不要把命令区误认为支持 JustFloat 命令类型。

996R 为内部闭环舵机，回传的是**指令角**（command_angle_deg），不是实测角——不装外部角度传感器就不许说"实测"。

## 构建与烧录

```powershell
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM=E:\STM32CubeCLT_1.21.0\Ninja\bin\ninja.exe -DCMAKE_TOOLCHAIN_FILE=cmake\gcc-arm-none-eabi.cmake
cmake --build build
pyocd flash -t stm32f401retx build\stm32-exam.bin
```

## 目录

```
Core/Inc  main.h(引脚) pwm.h  hal_conf(裁剪)
Core/Src  main.c(JustFloat 收发) pwm.c(TIM3 50Hz) can.c(遗留,已被移出构建)
```

## 已知限制

- `CMakeLists.txt` 的 `FW_DIR` 指向本机 CubeFW F4 路径，换机编译改这一行
- 旧版 `can.c`（按 F405 寄存器写的实验代码）已 `list(REMOVE_ITEM)` 移出构建，保留作学习记录；CAN 正解在 [can-exam 仓库](https://github.com/mydsp/can-exam)
- 996R 实际行程以实测为准，软件限幅 0~180
