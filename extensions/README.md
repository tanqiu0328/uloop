# uloop_timeline

![version](https://img.shields.io/badge/version-1.0-brightgreen.svg)
![build](https://img.shields.io/badge/build-2026.02.10-brightgreen.svg)
![author](https://img.shields.io/badge/author-Aki-brightgreen.svg)

基于 uloop 的时间线调度器扩展

## 特性

- 支持单次/循环执行
- 精确的时间点控制
- 零动态内存分配

## 快速开始

```c
#include "uloop_timeline.h"

// 定义动作函数
void led_on(void *arg) { /* LED 亮 */ }
void led_off(void *arg) { /* LED 灭 */ }
void beep(void *arg) { /* 蜂鸣 */ }

// 定义时间线（循环周期 1000ms）
ULOOP_TIMELINE_DEFINE(
    led_blink,
    1000,  // 循环周期
    {0,   led_on,  NULL},  // 0ms: LED 亮
    {100, beep,    NULL},  // 100ms: 蜂鸣
    {500, led_off, NULL}   // 500ms: LED 灭
);

// 启动时间线
uloop_timeline_start(&led_blink);

// 停止时间线
uloop_timeline_stop(&led_blink);
```

## 使用场景

### 单次序列

```c
// 启动序列：按键按下 → 延时 → LED 亮 → 蜂鸣
ULOOP_TIMELINE_DEFINE(
    boot_sequence,
    0,  // 不循环
    {0,    show_logo,    NULL},
    {500,  play_sound,   NULL},
    {1000, init_system,  NULL}
);

uloop_timeline_start(&boot_sequence);
```

### 呼吸灯效果

```c
ULOOP_TIMELINE_DEFINE(
    breathing_led,
    2000,  // 2 秒循环
    {0,    set_pwm, (void *)0},
    {250,  set_pwm, (void *)64},
    {500,  set_pwm, (void *)128},
    {750,  set_pwm, (void *)255},
    {1000, set_pwm, (void *)128},
    {1250, set_pwm, (void *)64}
);
```

### 状态机转换

```c
ULOOP_TIMELINE_DEFINE(
    motor_control,
    0,
    {0,    motor_start,     NULL},
    {100,  motor_accel,     NULL},
    {500,  motor_steady,    NULL},
    {2000, motor_decel,     NULL},
    {2500, motor_stop,      NULL}
);
```

## API

| 函数                                         | 说明             |
| -------------------------------------------- | ---------------- |
| `ULOOP_TIMELINE_DEFINE(name, cycle_ms, ...)` | 定义时间线（宏） |
| `uloop_timeline_start(timeline)`             | 启动时间线       |
| `uloop_timeline_stop(timeline)`              | 停止时间线       |
| `uloop_timeline_reset(timeline)`             | 重置时间线       |

## 依赖

需要 `uloop.h` 核心库支持
