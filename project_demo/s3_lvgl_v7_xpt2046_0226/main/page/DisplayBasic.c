
#include "DisplayPrivate.h"
#include "page_menu.h"


/*页面注册器*/
#define PAGE_REG(name)                               \
  do                                                 \
  {                                                  \
    extern void PageRegister_##name(uint8_t pageID); \
    PageRegister_##name(Disp_##name);                \
  } while (0)

/**
  * @brief  页面初始化
  * @param  无
  * @retval 无
  */
static void Pages_Init()
{
  page.PageManager(Disp_Max_Page, Disp_Max_Page);

  PAGE_REG(Home);
  PAGE_REG(Menu);
  PAGE_REG(Cam);
  PAGE_REG(Color);
  PAGE_REG(Face);
  PAGE_REG(Baidu);
  PAGE_REG(IMU);
  PAGE_REG(FFT);
  PAGE_REG(Daily);
  PAGE_REG(Game_2048);
  PAGE_REG(Game_Snake);
  PAGE_REG(Music);
  PAGE_REG(Calendar);
  PAGE_REG(Setting);
  PAGE_REG(About);

  page.PagePush(Disp_Home); //打开表盘
}

/**
  * @brief  显示初始化
  * @param  无
  * @retval 无
  */
void Display_Init()
{

  /*APP窗口初始化*/
  // AppWindow_Create();

  /*页面初始化*/
  Pages_Init();
}

/**
  * @brief  显示更新
  * @param  无
  * @retval 无
  */
void Display_Update()
{
  lv_task_handler();
  page.Running();
}

/**
  * @brief  页面阻塞延时，保持lvgl更新
  * @param  无
  * @retval 无
  */
void PageDelay(uint32_t ms)
{
  uint32_t lastTime = xTaskGetTickCount();
  while (xTaskGetTickCount() - lastTime <= ms)
  {
    lv_task_handler();
  }
}
