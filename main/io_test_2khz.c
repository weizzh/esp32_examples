#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"

#define IO_TEST 5 //
#define TIMER_DIVIDER 16 
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) //TIMER_BASE_CLK is 80,000,000
#define FRYQUENCY 2048
#define TIMER_INTERVAL 1.0/FRYQUENCY 
#define WITHOUT_RELOAD 0
#define WITH_RELOAD 1
static bool intr_times = 0 ;//interr times
static void test_gpio_init(int io_test)
{
	gpio_pad_select_gpio(io_test);
	gpio_set_direction(io_test, GPIO_MODE_OUTPUT);
}

void IRAM_ATTR timer_group0_isr(void *para)
{
	timer_spinlock_take(TIMER_GROUP_0);
	int timer_idx = (int) para;
	intr_times = !intr_times;
	timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, timer_idx);
	gpio_set_level(IO_TEST, intr_times); // to-do
	timer_group_enable_alarm_in_isr(TIMER_GROUP_0, timer_idx);
	timer_spinlock_give(TIMER_GROUP_0);

}

static void test_timer_init(int timer_idx, bool auto_reload, double timer_interval_sec)
{
	timer_config_t config = 
	{
		.divider = TIMER_DIVIDER,
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = auto_reload,
	};
	timer_init(TIMER_GROUP_0, timer_idx, &config);
	timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);
	//alarm_value is uint64_t
	timer_set_alarm_value(TIMER_GROUP_0, timer_idx, (int)(timer_interval_sec * TIMER_SCALE));
	timer_enable_intr(TIMER_GROUP_0, timer_idx);
	timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
						(void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);
	timer_start(TIMER_GROUP_0, timer_idx);
}

void app_main(void)
{
	test_gpio_init(IO_TEST);
	printf("gpio inited\n");
	test_timer_init(TIMER_0, WITH_RELOAD, TIMER_INTERVAL);
	printf("timer inited\n");
	while(1)
	{

	}
}