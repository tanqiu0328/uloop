/**
 * @file uloop.c
 * @brief 事件循环库
 * @author Aki
 * @version 1.1
 * @date 2026-02-10
 */

#include "uloop.h"

typedef int32_t s_tick_t;

typedef struct task_node
{
    struct task_node *next;
    uloop_handler_t handler;
    void *arg;
    ULOOP_TICK_TYPE expiration; // 截止时间
    bool is_delayed;            // 是否为延时任务
} task_node_t;

// 内存池管理块
static struct
{
    task_node_t pool[ULOOP_POOL_SIZE];
    task_node_t *free_head;
} s_mem;

// 调度器管理块
static struct
{
    task_node_t *ready_head; // 就绪队列头
    task_node_t *ready_tail; // 就绪队列尾
    task_node_t *timer_head; // 定时器链表头
    volatile ULOOP_TICK_TYPE tick_count;
} s_sched;

// Linker
extern const uloop_event_entry_t uloop_events$$Base;
extern const uloop_event_entry_t uloop_events$$Limit;

#define EVENT_START &uloop_events$$Base
#define EVENT_END &uloop_events$$Limit

static void _mem_init(void);
static task_node_t *_mem_alloc(void);
static void _mem_free(task_node_t *node);

/**
 * @brief 初始化内存池链表
 */
static void _mem_init(void)
{
    for (int i = 0; i < ULOOP_POOL_SIZE - 1; i++)
    {
        s_mem.pool[i].next = &s_mem.pool[i + 1];
    }
    s_mem.pool[ULOOP_POOL_SIZE - 1].next = NULL;
    s_mem.free_head = &s_mem.pool[0];
}

/**
 * @brief 从池中申请节点
 * @return task_node_t* 节点指针或NULL
 */
static task_node_t *_mem_alloc(void)
{
    task_node_t *node = NULL;
    ULOOP_ENTER_CRITICAL();
    if (s_mem.free_head != NULL)
    {
        node = s_mem.free_head;
        s_mem.free_head = node->next;
        node->next = NULL;
    }
    ULOOP_EXIT_CRITICAL();
    return node;
}

/**
 * @brief 释放节点回池
 * @param node 节点指针
 */
static void _mem_free(task_node_t *node)
{
    if (!node || node < s_mem.pool || node >= &s_mem.pool[ULOOP_POOL_SIZE])
    {
        return;
    }
    ULOOP_ENTER_CRITICAL();
    node->next = s_mem.free_head;
    s_mem.free_head = node;
    ULOOP_EXIT_CRITICAL();
}

/**
 * @brief 初始化uloop系统
 */
void uloop_init(void)
{
    _mem_init();
    s_sched.ready_head = NULL;
    s_sched.ready_tail = NULL;
    s_sched.timer_head = NULL;
    s_sched.tick_count = 0;
}

/**
 * @brief 系统心跳注入
 * @note  通常在SysTick中断中调用
 */
void uloop_tick(void)
{
    s_sched.tick_count++;
}

/**
 * @brief 发布一个立即执行的任务
 *
 * @param handler 回调函数
 * @param arg     回调参数
 * @return int    0：成功, -1：失败(内存池满)
 */
int uloop_post(uloop_handler_t handler, void *arg)
{
    if (!handler)
    {
        return -1;
    }

    task_node_t *node = _mem_alloc();
    if (!node)
    {
        return -1;
    }

    node->handler = handler;
    node->arg = arg;
    node->is_delayed = false;

    ULOOP_ENTER_CRITICAL();
    if (s_sched.ready_tail)
    {
        s_sched.ready_tail->next = node;
    }
    else
    {
        s_sched.ready_head = node;
    }
    s_sched.ready_tail = node;
    ULOOP_EXIT_CRITICAL();

    return 0;
}

/**
 * @brief 发布一个延时任务
 *
 * @param handler 回调函数
 * @param arg     回调参数
 * @param ticks   延时时间
 * @return int    0：成功, -1：失败(内存池满)
 */
int uloop_post_delayed(uloop_handler_t handler, void *arg, ULOOP_TICK_TYPE ticks)
{
    if (!handler)
    {
        return -1;
    }

    task_node_t *node = _mem_alloc();
    if (!node)
    {
        return -1;
    }

    node->handler = handler;
    node->arg = arg;
    node->is_delayed = true;

    ULOOP_ENTER_CRITICAL();
    node->expiration = s_sched.tick_count + ticks;

    task_node_t **curr = &s_sched.timer_head;
    while (*curr)
    {
        s_tick_t diff = (s_tick_t)((*curr)->expiration - node->expiration);
        if (diff > 0)
        {
            break;
        }
        curr = &(*curr)->next;
    }

    node->next = *curr;
    *curr = node;
    ULOOP_EXIT_CRITICAL();

    return 0;
}

/**
 * @brief 发出事件
 *
 * @param event_id 事件ID
 * @param arg      事件参数
 */
void uloop_emit(uint16_t event_id, void *arg)
{
    const uloop_event_entry_t *entry = EVENT_START;
    while (entry < EVENT_END)
    {
        if (entry->event_id == event_id)
        {
            if (uloop_post(entry->handler, arg) != 0)
            {
                // LOG_WARN("Event pool full");
            }
        }
        entry++;
    }
}

/**
 * @brief 运行事件循环
 *
 * @return ULOOP_TICK_TYPE 距离下一个最近定时任务的时间(Tick数)，可用于低功耗休眠
 */
ULOOP_TICK_TYPE uloop_run(void)
{
    // 检查定时器，将到期任务移入就绪队列
    ULOOP_ENTER_CRITICAL();
    ULOOP_TICK_TYPE now = s_sched.tick_count;

    while (s_sched.timer_head)
    {
        s_tick_t diff = (s_tick_t)(now - s_sched.timer_head->expiration);
        if (diff >= 0)
        {
            task_node_t *timer_node = s_sched.timer_head;
            s_sched.timer_head = timer_node->next;

            timer_node->next = NULL;
            timer_node->is_delayed = false;

            if (s_sched.ready_tail)
            {
                s_sched.ready_tail->next = timer_node;
            }
            else
            {
                s_sched.ready_head = timer_node;
            }
            s_sched.ready_tail = timer_node;
        }
        else
        {
            break;
        }
    }

    task_node_t *task_to_run = s_sched.ready_head;
    s_sched.ready_head = NULL;
    s_sched.ready_tail = NULL;

    ULOOP_EXIT_CRITICAL();

    // 执行就绪队列中的任务
    while (task_to_run)
    {
        task_node_t *curr_node = task_to_run;
        task_to_run = curr_node->next;

        if (curr_node->handler)
        {
            curr_node->handler(curr_node->arg);
        }
        _mem_free(curr_node);
    }

    // 计算建议休眠时间
    ULOOP_TICK_TYPE sleep_ticks = (ULOOP_TICK_TYPE)-1;

    ULOOP_ENTER_CRITICAL();
    if (s_sched.ready_head != NULL)
    {
        sleep_ticks = 0;
    }
    else if (s_sched.timer_head)
    {
        s_tick_t diff = (s_tick_t)(s_sched.timer_head->expiration - s_sched.tick_count);
        if (diff > 0)
        {
            sleep_ticks = (ULOOP_TICK_TYPE)diff;
        }
        else
        {
            sleep_ticks = 0;
        }
    }
    ULOOP_EXIT_CRITICAL();

    return sleep_ticks;
}
