/**
 * @file uloop_timeline.c
 * @brief 基于uloop的时间线调度器
 * @author Aki
 * @version 1.0
 * @date 2026-02-10
 */

#include "uloop_timeline.h"

static void _timeline_process_callback(void *arg);

/**
 * @brief 内部处理回调，计算下一次调度
 *
 * @param arg 传入的timeline对象
 */
static void _timeline_process_callback(void *arg)
{
    uloop_timeline_t *timeline = (uloop_timeline_t *)arg;

    if (!timeline || !timeline->is_running)
    {
        return;
    }

    if (timeline->current_index >= timeline->step_count)
    {
        timeline->is_running = false;
        return;
    }

    const uloop_timeline_step_t *current_step = &timeline->steps[timeline->current_index];

    if (current_step->handler)
    {
        current_step->handler(current_step->arg);
    }

    timeline->current_index++;
    ULOOP_TICK_TYPE next_delay = 0;
    bool need_post = false;

    if (timeline->current_index < timeline->step_count)
    {
        const uloop_timeline_step_t *next_step = &timeline->steps[timeline->current_index];
        if (next_step->time_ms >= current_step->time_ms)
        {
            next_delay = next_step->time_ms - current_step->time_ms;
        }
        need_post = true;
    }
    else if (timeline->cycle_ms > 0)
    {
        if (timeline->cycle_ms > current_step->time_ms)
        {
            next_delay = timeline->cycle_ms - current_step->time_ms;
        }

        if (timeline->steps[0].time_ms > 0)
        {
            next_delay += timeline->steps[0].time_ms;
        }

        timeline->current_index = 0;
        need_post = true;
    }
    else
    {
        timeline->is_running = false;
        timeline->current_index = 0;
        need_post = false;
    }

    if (need_post && timeline->is_running)
    {
        uloop_post_delayed(_timeline_process_callback, timeline, next_delay);
    }
}

/**
 * @brief 启动时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_start(uloop_timeline_t *timeline)
{
    if (!timeline)
    {
        return;
    }

    if (timeline->is_running)
    {
        return;
    }

    timeline->is_running = true;
    timeline->current_index = 0;

    ULOOP_TICK_TYPE start_delay = 0;
    if (timeline->step_count > 0)
    {
        start_delay = timeline->steps[0].time_ms;
        uloop_post_delayed(_timeline_process_callback, timeline, start_delay);
    }
    else
    {
        timeline->is_running = false;
    }
}

/**
 * @brief 停止时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_stop(uloop_timeline_t *timeline)
{
    if (timeline)
    {
        timeline->is_running = false;
    }
}

/**
 * @brief 重置时间线
 *
 * @param timeline 时间线句柄
 */
void uloop_timeline_reset(uloop_timeline_t *timeline)
{
    if (timeline)
    {
        timeline->is_running = false;
        timeline->current_index = 0;
    }
}
