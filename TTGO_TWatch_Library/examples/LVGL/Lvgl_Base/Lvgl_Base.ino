//载入库
#include "config.h"

//定义ttgo类
TTGOClass *ttgo;


void setup()
{
    //ttgo初始化
    ttgo = TTGOClass::getWatch();
    ttgo->begin();

    //打开背光
    ttgo->openBL();

    //载入图形库
    ttgo->lvgl_begin();

    //创建标签
    lv_obj_t *text = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(text, "T-Watch");
    lv_obj_align(text, NULL, LV_ALIGN_CENTER, 0, 0);
}

void loop()
{
    //事件处理机制
    lv_task_handler();
    delay(5);
}
