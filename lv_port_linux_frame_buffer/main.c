#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "ui/ui.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 320
#define DISP_BUF_LINES 80
#define DISP_BUF_SIZE (DISP_HOR_RES * DISP_BUF_LINES)

/* Tick thread to increment LVGL time base every 1 ms */
static void * tick_thread(void * data)
{
    (void)data;
    while(1) {
        lv_tick_inc(1);     // Tell LVGL that 1 ms has passed
        usleep(1000);       // Sleep for 1 ms
    }
    return NULL;
}

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*Touch input device init*/
    evdev_init();

    /*Drawing buffer*/
    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    /*Display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    lv_disp_drv_register(&disp_drv);

    /*Input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

    /* Start LVGL tick thread */
    pthread_t tick_t;
    pthread_create(&tick_t, NULL, tick_thread, NULL);

    /* Initialize your UI */
    ui_init();

    /* Create a Demo */
#if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
#endif
#if LV_USE_DEMO_BENCHMARK
    lv_demo_benchmark();
#endif

    /* Handle LVGL tasks */
    while(1) {
        lv_timer_handler(); // Process LVGL tasks
        usleep(5000);       // Run every 5 ms
    }

    return 0;
}
