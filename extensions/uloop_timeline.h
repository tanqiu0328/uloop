/**
 * @file uloop_timeline.h
 * @brief 基于uloop的时间线调度器
 * @author Aki
 * @version 1.0
 * @date 2026-02-10
 */

#ifndef _ULOOP_TIMELINE_H_
#define _ULOOP_TIMELINE_H_

#include "uloop.h"

/**
 * @brief 时间线动作节点
 */
typedef struct
{
    ULOOP_TICK_TYPE time_ms; // 触发时间点
    uloop_handler_t handler; // 执行回调函数
    void *arg;               // 用户参数
} uloop_timeline_step_t;

/**
 * @brief 时间线控制块
 */
typedef struct
{
    const uloop_timeline_step_t *steps; // 动作表数组
    uint16_t step_count;                // 动作总数
    uint16_t current_index;             // 当前执行到的索引
    ULOOP_TICK_TYPE cycle_ms;           // 循环周期(0表示不循环，>0表示循环周期)
    bool is_running;                    // 运行状态标志
} uloop_timeline_t;

/**
 * @brief 定义并初始化一个静态时间线
 *
 * @param name      变量名
 * @param _cycle_ms 循环周期(ms)，0为单次运行
 * @param ...       动作列表，格式：{时间, 函数, 参数}
 */
#define ULOOP_TIMELINE_DEFINE(name, _cycle_ms, ...)                         \
    static const uloop_timeline_step_t name##_steps[] = {__VA_ARGS__};      \
    static uloop_timeline_t name = {                                        \
        .steps = name##_steps,                                              \
        .step_count = sizeof(name##_steps) / sizeof(uloop_timeline_step_t), \
        .current_index = 0,                                                 \
        .cycle_ms = _cycle_ms,                                              \
        .is_running = false}

/**
 * @brief 启动时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_start(uloop_timeline_t *timeline);

/**
 * @brief 停止时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_stop(uloop_timeline_t *timeline);

/**
 * @brief 重置时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_reset(uloop_timeline_t *timeline);

#endif // _ULOOP_TIMELINE_H_
