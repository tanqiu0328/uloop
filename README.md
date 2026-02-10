# uloop

![version](https://img.shields.io/badge/version-1.2-brightgreen.svg)
![build](https://img.shields.io/badge/build-2026.02.10-brightgreen.svg)
![author](https://img.shields.io/badge/author-Aki-brightgreen.svg)

嵌入式事件循环库

## 特性

- 静态内存池，无动态分配
- 支持延时任务调度
- 基于Linker的零开销事件订阅

## 快速开始

```c
#include "uloop.h"

// 初始化
uloop_init();

// 发布立即任务
uloop_post(my_handler, NULL);

// 发布延时任务（100 ticks）
uloop_post_delayed(delayed_handler, NULL, 100);

// 主循环
while (1) {
    ULOOP_TICK_TYPE sleep_time = uloop_run();
    // 可选：根据 sleep_time 进入低功耗模式
}
```

## 事件订阅

```c
// 定义事件 ID
#define EVENT_BUTTON_PRESS 1

// 订阅事件
void on_button(void *arg) {
    // 处理按钮事件
}
ULOOP_ON_EVENT(EVENT_BUTTON_PRESS, on_button);

// 发出事件
uloop_emit(EVENT_BUTTON_PRESS, NULL);
```

## 配置

在 `uloop.h` 中调整：

```c
#define ULOOP_POOL_SIZE 32       // 任务池大小
#define ULOOP_TICK_TYPE uint32_t // Tick 计数器类型
```

## API

| 函数                                      | 说明                           |
| ----------------------------------------- | ------------------------------ |
| `uloop_init()`                            | 初始化事件循环                 |
| `uloop_tick()`                            | 心跳注入（SysTick中断调用）    |
| `uloop_run()`                             | 执行事件循环，返回建议休眠时间 |
| `uloop_post(handler, arg)`                | 发布立即任务                   |
| `uloop_post_delayed(handler, arg, ticks)` | 发布延时任务                   |
| `uloop_emit(event_id, arg)`               | 发出事件                       |
| `ULOOP_ON_EVENT(id, func)`                | 订阅事件宏                     |
