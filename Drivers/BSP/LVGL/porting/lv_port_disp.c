/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "lvgl.h"
#include "lcd.h"
/*********************
 *      DEFINES
 *********************/
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
/**********************
 *      TYPEDEFS
 **********************/
#define  test_h  200
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
// static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//         const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/
//f407外部SRAM只有1MB无DMA2D所以-----双缓冲去容量太大所以使用单缓冲  把绘图缓冲区放到外部SRAM上（内部控件匮乏时可以取）
 static lv_color_t buf_2_1[MY_DISP_HOR_RES * test_h] __attribute__((at(0x68000000)));   /* 设置缓冲区的大小为屏幕的全尺寸大小 */
  static lv_color_t buf_2_2[MY_DISP_HOR_RES * test_h] __attribute__((at((0x68000000)+MY_DISP_HOR_RES*test_h*2)));
 //static lv_color_t buf_2_2[MY_DISP_HOR_RES * test_h] __attribute__((at(0x68100000))); // 使用更大的间隔

 // 使用了GCC编译器的** __attribute__((at())) 属性**，它可以将变量强制放置在指定的内存地址上，将这个数组直接放置在外部SRAM的基地址0x68000000处
 // static lv_color_t buf_1[320 * 100] __attribute__((at(0x68000000))); /* 设置缓冲区的大小为屏幕的全尺寸大小 */
 /**********************
  *      MACROS
  **********************/

 /**********************
  *   GLOBAL FUNCTIONS
  **********************/

 void lv_port_disp_init(void)
 {
     /*-------------------------
      * Initialize your display
      * -----------------------*/
     disp_init();

     /*-----------------------------
      * Create a buffer for drawing创建一个绘图缓冲区
      *----------------------------*/

     /**
      * LVGL requires a buffer where it internally draws the widgets.
      * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
      * The buffer has to be greater than 1 display row
      *
      * There are 3 buffering configurations:
      * 1. Create ONE buffer:1. 单缓冲区:
      *      LVGL will draw the display's content here and writes it to your display LVGL 会将显示设备的内容绘制到这里，并将他写入显示设备。
      *
      * 2. Create TWO buffer:2.双缓冲区:
      *      LVGL will draw the display's content to a buffer and writes it your display.  LVGL 会将显示设备的内容绘制到其中一个缓冲区，并将他写入显示设备。
      *      You should use DMA to write the buffer's content to the display.
      *      It will enable LVGL to draw the next part of the screen to the other buffer while
      *      the data is being sent form the first buffer. It makes rendering and flushing parallel. 这样使得渲染和刷新可以并行执行。
      *
      * 3. Double buffering 全尺寸双缓冲区
      *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1. 设置两个屏幕大小的全尺寸缓冲区，并且设置 disp_drv.full_refresh = 1。
      *      This way LVGL will always provide the whole rendered screen in `flush_cb`
      *      and you only need to change the frame buffer's address. 这样，LVGL将始终以 'flush_cb' 的形式提供整个渲染屏幕，您只需更改帧缓冲区的地址。
      */

     /* Example for 1) */
     //    static lv_disp_draw_buf_t draw_buf_dsc_1;
     //    // static lv_color_t buf_1[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
     //    // lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, 320 * 10);   /*Initialize the display buffer*/
     //    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, 320 * 100); /*用户添加*/

     /* Example for 2) */
     static lv_disp_draw_buf_t draw_buf_dsc_2;
     // static lv_color_t buf_2_1[MY_DISP_HOR_RES * 240];                        /*A buffer for 10 rows*/
     // static lv_color_t buf_2_2[MY_DISP_HOR_RES * 240];                        /*An other buffer for 10 rows*/
     lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * test_h); /*Initialize the display buffer*/

     /* Example for 3) also set disp_drv.full_refresh = 1 below*/
     //	static lv_disp_draw_buf_t draw_buf_dsc_3;
     //	//static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
     //	//static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*Another screen sized buffer*/
     //	lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_VER_RES * MY_DISP_HOR_RES);   /*Initialize the display buffer*/

     /*-----------------------------------
      * Register the display in LVGL
      *----------------------------------*/

     static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
     lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

     /*Set up the functions to access to your display*/

     /*Set the resolution of the display*/
     disp_drv.hor_res = MY_DISP_HOR_RES;
     disp_drv.ver_res = MY_DISP_VER_RES;

     /*Used to copy the buffer's content to the display*/
     disp_drv.flush_cb = disp_flush;

     /*Set a display buffer*/
     disp_drv.draw_buf = &draw_buf_dsc_2;

     /*Required for Example 3)*/
      disp_drv.full_refresh = 1;

     /* Fill a memory array with a color if you have GPU.
      * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
      * But if you have a different GPU you can use with this callback.*/
     // disp_drv.gpu_fill_cb = gpu_fill;

     /*Finally register the driver*/
     lv_disp_drv_register(&disp_drv);
 }

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}
void LVGL_LCD_FSMC_DMA_pCallback(DMA_HandleTypeDef *_hdma)
{

    lv_disp_flush_ready(lv_disp_get_default()->driver); // 通知发送完成
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    //    int32_t x;
    //    int32_t y;
    //    for(y = area->y1; y <= area->y2; y++) {
    //        for(x = area->x1; x <= area->x2; x++) {
    //            /*Put a pixel to the display. For example:*/
    //            /*put_px(x, y, *color_p)*/
    //            color_p++;
    //        }
    //    }
    //	lcd_color_fill(area->x1,area->y1,area->x2,area->y2,(uint16_t *)color_p); //自己添加修改，上面的注释掉了
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    // 通知图形库，已经刷新完毕了
    // lv_disp_flush_ready(disp_drv);//在下面回调函数里面定义

    // lcd_fill(area->x1,area->y1,area->x2,area->y2,(uint16_t *)color_p);//这个是原来使用的填充函数
    uint16_t height, width;
    width = area->x2 - area->x1 + 1;                                                                           // 得到填充的宽度
    height = area->y2 - area->y1 + 1;                                                                          // 高度
    lcd_set_window(area->x1, area->y1, width, height);                                                         // 将填充函数改为块填充函数，这里使用正点原子lcd驱动的窗口函数，将在下面展示出来
    lcd_write_ram_prepare();                                                                                   // 准备发送（这个有的驱动有有的没有，根据自己驱动修改）
    HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, (uint32_t)color_p, (uint32_t)&LCD->LCD_RAM, height * width); // 启动dma传输
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
// static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                     const lv_area_t * fill_area, lv_color_t color)
//{
//     /*It's an example code which should be done by your GPU*/
//     int32_t x, y;
//     dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//     for(y = fill_area->y1; y <= fill_area->y2; y++) {
//         for(x = fill_area->x1; x <= fill_area->x2; x++) {
//             dest_buf[x] = color;
//         }
//         dest_buf+=dest_width;    /*Go to the next line*/
//     }
// }

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
