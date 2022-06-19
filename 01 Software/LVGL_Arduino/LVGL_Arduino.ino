#include <lvgl.h>
#include <TFT_eSPI.h>
/*If you want to use the LVGL examples,
  make sure to install the lv_examples Arduino library
  and uncomment the following line.
#include <lv_examples.h>
*/

#include <lv_demo.h>

#include <FT6336U.h>

#define RST_N_PIN 4
#define INT_N_PIN 15

FT6336U ft6336u(RST_N_PIN, INT_N_PIN);

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {15, 0, false};

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
    s->pressed = true;
    //Serial.printf("isr...\n");
}

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}



/*Initialize your touchpad*/
static void touchpad_init(void)
{
  Serial.printf("touch init...");
    /*Your code comes here*/
    ft6336u.begin();
}



/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    if (button1.pressed) {
        Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
        button1.pressed = false;
        return true;
    }  

    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(uint16_t * x, uint16_t * y)
{
    /*Your code comes here*/

    (*x) = ft6336u.read_touch1_x_ext();
    (*y) = ft6336u.read_touch1_y_ext();
    //Serial.printf("touch x:%d  y:%d\n", *x, *y);
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    //bool touched = tft.getTouch( &touchX, &touchY, 600 );
    //Serial.printf("read...\n");
    //if( !touched )
    if(touchpad_is_pressed() == false)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        touchpad_get_xy(&touchX, &touchY);
        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
#if 0
        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
#endif        
    }
}

void lv_slider_test()
{
    lv_obj_t * slider = lv_slider_create(lv_scr_act()); // 创建滑块对象
    if (slider != NULL)
    {
        lv_obj_set_width(slider, 200); // 设置slider的宽度
        lv_obj_center(slider);  // 对样显示在屏幕中央
        lv_slider_set_range(slider, 10, 100); // 设置滑块值的变化范围10-200
        //lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_MID, 0, 120);
 
        lv_obj_t* label = lv_label_create(lv_scr_act()); // 创建一个标签，用于显示滑块的滑动值
        if (label != NULL)
        {
            lv_label_set_text_fmt(label, "%d", lv_slider_get_min_value(slider)); // 标签默认显示滑块的最小值
            //lv_obj_align(label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
            lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);  // 标签对象放在slider对象的上方中间位置
  
            // 添加滑块值变化事件和事件回调函数，并将label对象最为事件的user_data
            //lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, (void *)label);
        }
    }
}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 1 ); /* Landscape orientation, flipped */

    /*Set the touchscreen calibration data,
     the actual data for your display can be acquired using
     the Generic -> Touch_calibrate example from the TFT_eSPI library*/
    //uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    //tft.setTouch( calData );

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    touchpad_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

#if 0
    /* Create simple label */
    lv_obj_t *label = lv_label_create( lv_scr_act() );
    lv_label_set_text( label, LVGL_Arduino.c_str() );
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    lv_slider_test();
#else
    /* Try an example from the lv_examples Arduino library
       make sure to include it as written above.
    lv_example_btn_1();
   */

    // uncomment one of these demos
    lv_demo_widgets();            // OK
     //lv_demo_benchmark();          // OK
    // lv_demo_keypad_encoder();     // works, but I haven't an encoder
     //lv_demo_music();              // NOK
    // lv_demo_printer();
    // lv_demo_stress();             // seems to be OK
#endif

#if 1
  ft6336u.setRotation(Rotation_1);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterruptArg(button1.PIN, isr, &button1, FALLING);
#endif
    Serial.println( "Setup done" );
}

void loop()
{
    //lv_timer_handler(); /* let the GUI do its work */
    lv_task_handler();
    lv_tick_inc(1);
    delay( 1 );
}
