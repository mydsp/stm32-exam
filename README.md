# stm32-exam —— VOFA+ 上位机调试舵机（STM32F401RET6）

基于 STM32F401RET6 的舵机上位机调试工程：上位机经自定义协议下发两路舵机目标角度，
MCU 解析后驱动舵机，并以 20Hz 回传当前角度与 PWM 占空比，在 VOFA+ 中以 JustFloat 曲线实时可视化。
对应《G308 电控组 2026 夏季考核题》软件第二题。

## 功能特性

- 自定义 12 字节下发帧（2×float + JustFloat 帧尾），12 字节滑窗按帧尾收敛对齐，抗字节流错位
- TIM3 双路 50Hz 舵机 PWM（500~2500µs），上电 1500µs 中位
- 20Hz 回传 4 通道 JustFloat：pan/tilt 角度 + PWM 占空比
- 目标角限幅 0~180°；全分辨率换算脉宽，避免整数量化台阶

## 硬件连接

| 引脚 | 连接 | 说明 |
| --- | --- | --- |
| PA9 / PA10 | USB 转串口 RX / TX | USART1，115200-8-N-1 |
| PA6（TIM3_CH1） | 舵机 1 信号线 | 50Hz |
| PA7（TIM3_CH2） | 舵机 2 信号线 | 50Hz |
| 外接 5V / GND | 舵机电源 | 独立 5V≥3A，与 MCU 共地 |

## 构建与烧录

```bat
cmake --preset Debug
cmake --build --preset Debug
pyocd flash -t stm32f401retx build\Debug\stm32-exam.bin
```

## 使用说明（VOFA+）

1. 新建串口连接，波特率 115200，数据引擎选 JustFloat，波形区出现 4 条曲线即为正常回传。
2. 命令区新建 Hex 命令，发送完整 12 字节帧，例如 pan=30°、tilt=90°：
   `00 00 F0 41 00 00 B4 42 00 00 80 7F`
3. 通道定义：ch0=pan 角度(°)，ch1=pan 占空比(%)，ch2=tilt 角度(°)，ch3=tilt 占空比(%)。
   换算：脉宽(µs) = 500 + 角度 × 2000 / 180；占空比(%) = 脉宽 / 200。

## 文档与交付物

| 材料 | 文件 |
| --- | --- |
| 软件一 开发环境与仓库说明 | [docs/01-开发环境与仓库说明.md](docs/01-开发环境与仓库说明.md) |
| 软件二 开发文档 | [docs/02-开发文档.md](docs/02-开发文档.md) |
| 实操视频 | [docs/video/soft2-demo.mp4](docs/video/soft2-demo.mp4) |

## 已知限制

- 回传角度为最近一次下发的指令角：MG996R 为内置闭环模拟舵机，无对外位置反馈接口
- MG996R 实际有效行程略窄于 500~2500µs，应用时可按需收窄限幅范围
