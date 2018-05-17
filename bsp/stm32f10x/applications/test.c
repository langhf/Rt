#include <rtthread.h>
#include <stm32f10x.h>
#include "test.h"
#include "key.h"

#define KEY_DEBUG

#ifdef KEY_DEBUG
#define printf               rt_kprintf /*使用rt_kprintf来输出*/
#else
#define printf(...)                     /*无输出*/
#endif

ALIGN(RT_ALIGN_SIZE)

/* 静态线程的 线程堆栈 */
static rt_uint8_t key_stack[512];
static rt_uint8_t sndmsg_stack[512];

/* 静态线程的 线程控制块 */
static struct rt_thread key_thread;
static struct rt_thread sndmsg_thread;
/* 消息队列控制块 */
struct rt_messagequeue mq;
/* 消息队列中用到的放置消息的内存池 */
static char msg_pool[2048];

static char key_info[]= "The char is x";
static char i = 41;
static char state;
void sndmsg_thread_entry(void* parameter)
{
  rt_err_t result;
  /* 发送消息到消息队列中 */
  while(1){
    if(i>126) i = 41;
    key_info[12] = i++;
    result = rt_mq_send(&mq, &key_info[0], sizeof(key_info));
    if ( result == -RT_EFULL){
      if(state){
        printf("[Sender:] message queue is full!\n");
        state = ~state;
      }
    }else{
      state = -1;
      #ifdef KEY_DEBUG
          printf("[Sender:] %s\n", key_info);
      #endif
    }

    rt_thread_delay(50);
  }
}

rt_err_t demo_thread_create(void)
{
  rt_err_t result;
  result = rt_mq_init(&mq, "mqt",
                      &msg_pool[0], /*内存池指向msg_pool*/
                      128- sizeof(void*), /*每个消息的大小是 128 - void*  void* 是消息头的大小 */
                      sizeof(msg_pool), /*内存池的大小是msg_pool的大小*/
                      RT_IPC_FLAG_FIFO);/*如果有多个线程等待，按照先来先到的方法分配消息*/
  if(result != RT_EOK)
  {
    rt_kprintf("init message queue failed.\n");
    return -1;
  }

  /* 创建发送线程 */
  result = rt_thread_init(&key_thread,
                          "key",
                          key_thread_entry, RT_NULL,
                          (rt_uint8_t*)&key_stack[0], sizeof(key_stack), 20, 5);
  if(result == RT_EOK)
  {
    rt_thread_startup(&key_thread);
  }

  /* 创建接收线程 */
  result = rt_thread_init(&sndmsg_thread,
                          "sndmsg",
                          sndmsg_thread_entry, RT_NULL,
                          (rt_uint8_t*)&sndmsg_stack[0], sizeof(sndmsg_stack), 20, 5);
  if(result == RT_EOK)
  {
    rt_thread_startup(&sndmsg_thread);
  }

  return 0;
}
