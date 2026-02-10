/**
 * @file uloop.h
 * @brief 事件循环库
 * @author Aki
 * @version 1.1
 * @date 2026-02-10
 */

#ifndef _ULOOP_H_
#define _ULOOP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ULOOP_POOL_SIZE 32       // 内存池大小
#define ULOOP_TICK_TYPE uint32_t // Tick类型

#define ULOOP_ENTER_CRITICAL() __disable_irq()
#define ULOOP_EXIT_CRITICAL() __enable_irq()

/**
 * @brief 任务回调函数类型
 */
typedef void (*uloop_handler_t)(void *arg);

/**
 * @brief 初始化uloop系统
 */
void uloop_init(void);

/**
 * @brief 系统心跳
 * @note  通常在SysTick中断中调用
 */
void uloop_tick(void);

/**
 * @brief 运行事件循环
 *
 * @return ULOOP_TICK_TYPE 距离下一个最近定时任务的时间(Tick数)，可用于低功耗休眠
 */
ULOOP_TICK_TYPE uloop_run(void);

/**
 * @brief 发布一个立即执行的任务
 *
 * @param handler 回调函数
 * @param arg     回调参数
 * @return int    0：成功, -1：失败(内存池满)
 */
int uloop_post(uloop_handler_t handler, void *arg);

/**
 * @brief 发布一个延时任务
 *
 * @param handler 回调函数
 * @param arg     回调参数
 * @param ticks   延时时间
 * @return int    0：成功, -1：失败(内存池满)
 */
int uloop_post_delayed(uloop_handler_t handler, void *arg, ULOOP_TICK_TYPE ticks);

/**
 * @brief 发出事件
 *
 * @param event_id 事件ID
 * @param arg      事件参数
 */
void uloop_emit(uint16_t event_id, void *arg);

/* Linker */
typedef struct
{
    uint16_t event_id;
    uloop_handler_t handler;
} uloop_event_entry_t;

#define ULOOP_SECTION __attribute__((section("uloop_events"), used))

/**
 * @brief 订阅事件宏
 * @param id   事件ID(uint16_t)
 * @param func 处理函数(uloop_handler_t)
 */
#define ULOOP_ON_EVENT(id, func) \
    const uloop_event_entry_t uloop_event_##id##_##func ULOOP_SECTION = {.event_id = id, .handler = func}

#endif // _ULOOP_H_
