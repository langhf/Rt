#include <rtthread.h>
#include "stm32f10x.h"
#include "key.h"




#define KEY_RCC RCC_APB2Periph_GPIOA
#define KEY_PORT GPIOA
#define KEY_PIN GPIO_Pin_0

#define KEY_ALL_NSET		(0x0000|KEY_PIN)

extern struct rt_messagequeue mq;
static void key_scan(void); /* 防抖动 */
static void rt_hw_key_init(void);

void key_thread_entry(void* parameter)
{
  rt_hw_key_init();

  while(1)
  {
      key_scan();
      rt_thread_delay(5);
  }
}

void rt_hw_key_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(KEY_RCC|RCC_APB2Periph_AFIO, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = KEY_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

static char key_state = -1;
void key_scan(void)
{
  char buf[128];
  rt_memset(&buf[0], 0, sizeof(buf));
  static vu16 s_KeyCode;//内部检查按键使用
  static vu8 s_key_debounce_count, s_key_long_count;
  vu16	t_key_code;

  t_key_code = GPIO_ReadInputData(KEY_PORT) & KEY_ALL_NSET;

  // 按键按下为 0 ，不按为 1; 开始状况进入if ，按下一次则进入 else
  if((t_key_code == KEY_ALL_NSET)||(t_key_code != s_KeyCode))
  {
      s_key_debounce_count = 0;	//第一次
      s_key_long_count = 0;
      if(key_state){
        if (rt_mq_recv(&mq, &buf[0], sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("[Receiver:] %s\n", buf);
        }
      }
  }
  else
  {
    //按下按键则停止接收
    if(++s_key_debounce_count == DEBOUNCE_SHORT_TIME)
    {   //短按键
        key_state = ~key_state;
        rt_kprintf("key_state: %d\n", key_state);
    }
    /* 从消息队列中接收消息 */
    if(key_state){
      if (rt_mq_recv(&mq, &buf[0], sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
      {
          rt_kprintf("[Receiver:] %s\n", buf);
      }
    }

    if(s_key_debounce_count == DEBOUNCE_COUT_FIRST + DEBOUNCE_COUT_INTERVAL)
    {   //连按键
        s_key_debounce_count = DEBOUNCE_COUT_FIRST;
        ++s_key_long_count;
    }

    if(s_key_long_count == DEBOUNCE_LONG_TIME)
    {   //长按键

        s_key_long_count = DEBOUNCE_LONG_TIME;
    }
  }
  s_KeyCode = t_key_code;				// 保存本次键值
}
