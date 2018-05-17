#ifndef __KEY_H__
#define __KEY_H__

#define DEBOUNCE_SHORT_TIME     2   		// 轻触按键消抖时间5（单位：50毫秒）
#define DEBOUNCE_LONG_TIME      3//10 // 长按键时间DEBOUNCE_COUT_FIRST+DEBOUNCE_COUT_INTERVAL*DEBOUNCE_LONG_TIME（单位：50毫秒）
#define DEBOUNCE_COUT_FIRST     20 	// 连按键间隔时间100（单位：50毫秒）
#define DEBOUNCE_COUT_INTERVAL  3   // 连按键间隔时间20（单位：50毫秒）

void key_thread_entry(void* parameter);

#endif
