#include "config.h"

//ttgo主功能类、tft屏幕驱动、s7xg lora及gps相关、btnTicker按钮调度定时器
TTGOClass *ttgo;
TFT_eSPI *tft ;
S7XG_Class *s7xg;
Ticker btnTicker;

//state状态、prev_state前状态、isInit是否初始化、定义一堆按钮与图形容器
uint32_t state = 0, prev_state = 0;
bool isInit = false;
lv_obj_t *btn2, *btn1, *btn3, *ta1, *gContainer;

void createWin();
void stop_prev();

//控件事件处理程序
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    //若非点击事件则直接返回
    if (event != LV_EVENT_CLICKED) return;

    //判定对应控件设置不同状态
    if (obj == btn1) {
        state = 1;
    } else if (obj == btn2) {
        state = 2;
    } else if (obj == btn3) {
        state = 3;
    }

    //设初始化为假
    isInit = false;

    //若接受数据非空则绘制消息窗口
    if (!ta1) {
        createWin();
    }
}

static void  createGui()
{
    //清空容器中所有控件
    lv_obj_clean(gContainer);

    //设tal指向空，创建3个按钮
    ta1 = NULL;
    lv_obj_t *label ;
    /*Create a normal button*/
    btn1 = lv_btn_create(gContainer, NULL);
    lv_obj_set_size(btn1, 120, 65);
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_obj_set_event_cb(btn1, event_handler);

    /*Add a label to the button*/
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Sender");

    btn2 = lv_btn_create(gContainer, btn1);
    lv_obj_align(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn2, event_handler);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Receiver");

    btn3 = lv_btn_create(gContainer, btn1);
    lv_obj_align(btn3, btn2, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn3, event_handler);

    label = lv_label_create(btn3, NULL);
    lv_label_set_text(label, "GPS");

}

void createWin()
{
    //创建了一个用以显示信息的界面
    lv_obj_clean(gContainer);
    ta1 = lv_textarea_create(gContainer, NULL);
    lv_obj_set_size(ta1, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_textarea_set_text(ta1, "");    /*Set an initial text*/
    lv_textarea_set_max_length(ta1, 128);
}

void add_message(const char *txt)
{
    //若新接收的文本为空或已接收文本为空，则直接返回
    if (!txt || !ta1)return;

    //若未超出最大长度
    if (strlen(lv_textarea_get_text(ta1)) >= lv_textarea_get_max_length(ta1)) {
        lv_textarea_set_text(ta1, "");
    }
    String str = txt;

    //删除两端空格并换行
    str.trim();
    str += "\n";

    //添加文本
    lv_textarea_add_text(ta1, str.c_str());
}

void stop_prev()
{
    //切换不同状态
    switch (prev_state) {
    case 1:
    case 2:
        //停止lora收发监控
        Serial.println("Stop Lora");
        s7xg->loraPingPongStop();
        break;
    case 3:
        //停止gps
        Serial.println("Stop GPS");
        s7xg->gpsStop();
        break;
    default:
        break;
    }

    //将当前状态变为上一状态
    prev_state = state;
}

void lora_sender()
{
    //若初始化为假，则开启lora发送监测
    if (!isInit) {
        isInit = true;

        //更换状态
        stop_prev();
        //打开lora发送
        s7xg->loraSetPingPongSender();
        Serial.println("Start  lora_sender");
    }

    //Get data directly from the serial port
    String str = s7xg->loraGetPingPongMessage();
    if (str != "") {
        add_message(str.c_str());
    }
}

void lora_receiver()
{
    //若初始状态为假，则开启lora接收监测
    if (!isInit) {
        isInit = true;
        //切换状态
        stop_prev();
        //打开lora接收
        s7xg->loraSetPingPongReceiver();
        Serial.println("Start  lora_receiver");
    }
    //Get data directly from the serial port
    String str = s7xg->loraGetPingPongMessage();
    if (str != "") {
        add_message(str.c_str());
    }
}

void s7xg_gps()
{
    //若初始状态为假，则开启gps功能
    if (!isInit) {
        isInit = true;
        //切换状态
        stop_prev();

        //开启gps
        s7xg->gpsReset();
        s7xg->gpsSetLevelShift(true);
        s7xg->gpsSetStart();
        s7xg->gpsSetSystem(0);
        s7xg->gpsSetPositioningCycle(1000);
        s7xg->gpsSetPortUplink(20);
        s7xg->gpsSetFormatUplink(1);
        s7xg->gpsSetMode(1);
        Serial.println("Start  s7xg_gps");
    }
    static uint32_t timestamp = 0;
    // Send gps data command every 1 second interval
    if (millis() - timestamp > 1000) {
        timestamp = millis();
        ttgo->hwSerial->print("gps get_data dms");
    }
    //Get data directly from the serial port
    String str = s7xg->loraGetPingPongMessage();
    if (str != "") {
        add_message(str.c_str());
    }
}

void pressed()
{
    isInit = false;
    state = 0;

    //切换按钮
    stop_prev();

    //重绘界面
    createGui();
}

void setup(void)
{
    //打开串口，波特率为115200
    Serial.begin(115200);

    //创建ttgo对象及一些初始化工作
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->openBL();

    //初始化lvgl图形界面库对象
    ttgo->lvgl_begin();

    //! Open s7xg chip power
    ttgo->enableLDO4();
    //! Open s7xg gps reset power
    ttgo->enableLDO3();

    //开启s7xg模块
    ttgo->s7xg_begin();
    s7xg = ttgo->s7xg;

    //设置按钮按下处理事件
    ttgo->button->setPressedHandler(pressed);

    //创建图形容器
    gContainer = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(gContainer,  LV_HOR_RES, LV_VER_RES);
    // lv_obj_set_style(gContainer, &lv_style_transp_fit);

    lv_obj_t *label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "Begin S7xG");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    int len = 0;
    int retry = 0;

    //取硬件内存容量判定模块是否正常，尝试第五次失败后重启模块，直至成功(感觉写的有bug)
    do {
        lv_task_handler();
        len = s7xg->getHardWareModel().length();
        if (len == 0 && retry++ == 5) {
            s7xg->reset();
            retry = 0;
            Serial.println("Reset s7xg chip");
        }
        if (len == 0)
            delay(1000);
    } while (len == 0);

    //通过串口反馈成功
    Serial.println("Found s7xg module,Start gps module");

    //创建用户界面
    createGui();

    //设置按钮调度(注意lambda表达式)
    btnTicker.attach_ms(30, []() {
        ttgo->button->loop();
    });
}

void loop(void)
{
    //根据状态不同进行相应处理
    switch (state) {
    case 1:
        lora_sender();
        break;
    case 2:
        lora_receiver();
        break;
    case 3:
        s7xg_gps();
        break;
    default:
        break;
    }

    //将控制权交给lvgl的处理机制
    lv_task_handler();
    delay(5);
}