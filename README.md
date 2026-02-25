# uloop

![version](https://img.shields.io/badge/version-1.3-brightgreen.svg)
![build](https://img.shields.io/badge/build-2026.02.25-brightgreen.svg)
![author](https://img.shields.io/badge/author-Aki-brightgreen.svg)

嵌入式事件循环库

## 用前必看！！！

EVENT回调中的arg指针仅在当前函数作用域内有效。任何需要异步或延期处理该数据的订阅者，必须

自行申请一块新内存，将数据memcpy过去，并自行管理那块新内存的销毁。禁止直接缓存arg指针！

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
    // 可选：根据 sleep_time 进入低功耗休眠模式
}
```

## 事件订阅

```c
// 定义事件ID
#define EVENT_BUTTON_PRESS 1

// 订阅事件
void on_button(void *arg) {
    // 处理按钮事件
}
ULOOP_ON_EVENT(EVENT_BUTTON_PRESS, on_button);

// 发出事件
uloop_emit(EVENT_BUTTON_PRESS, NULL);
```

uloop支持多订阅者机制。为了解决多订阅者下的内存释放隐患，新增uloop_emit_managed函数

```c
// 定义事件ID
#define EVENT_NETWORK_DATA 2

// 订阅者1
void on_network_data_logger(void *arg) {
    // 记录日志
}
ULOOP_ON_EVENT(EVENT_NETWORK_DATA, on_network_data_logger);

// 订阅者2
void on_network_data_parser(void *arg) {
    // 解析数据
}
ULOOP_ON_EVENT(EVENT_NETWORK_DATA, on_network_data_parser);

// 发出事件
void receive_data() {
    void* my_data = malloc(128); 
    // 当所有订阅者同步执行完成后，系统会自动调用free(my_data)
    uloop_emit_managed(EVENT_NETWORK_DATA, my_data, free);
}
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
| `uloop_emit_managed(event_id, arg, dtor)` | 发出受管事件                   |
| `ULOOP_ON_EVENT(id, func)`                | 订阅事件宏                     |