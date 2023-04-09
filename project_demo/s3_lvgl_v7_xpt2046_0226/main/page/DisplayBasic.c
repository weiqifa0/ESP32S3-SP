
#include "DisplayPrivate.h"
#include "page_menu.h"


/*ҳ��ע����*/
#define PAGE_REG(name)                               \
  do                                                 \
  {                                                  \
    extern void PageRegister_##name(uint8_t pageID); \
    PageRegister_##name(Disp_##name);                \
  } while (0)

/**
  * @brief  ҳ���ʼ��
  * @param  ��
  * @retval ��
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

  page.PagePush(Disp_Home); //�򿪱���
}

/**
  * @brief  ��ʾ��ʼ��
  * @param  ��
  * @retval ��
  */
void Display_Init()
{

  /*APP���ڳ�ʼ��*/
  // AppWindow_Create();

  /*ҳ���ʼ��*/
  Pages_Init();
}

/**
  * @brief  ��ʾ����
  * @param  ��
  * @retval ��
  */
void Display_Update()
{
  lv_task_handler();
  page.Running();
}

/**
  * @brief  ҳ��������ʱ������lvgl����
  * @param  ��
  * @retval ��
  */
void PageDelay(uint32_t ms)
{
  uint32_t lastTime = xTaskGetTickCount();
  while (xTaskGetTickCount() - lastTime <= ms)
  {
    lv_task_handler();
  }
}
