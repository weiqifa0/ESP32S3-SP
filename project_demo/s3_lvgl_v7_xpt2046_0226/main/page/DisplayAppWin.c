#include "DisplayPrivate.h"
#include "page_menu.h"


typedef struct{
    lv_obj_t * cont;
    lv_style_t style;

}AppWindow_TypeDef;

static AppWindow_TypeDef appWindow_Grp[Disp_Max_Page];

lv_obj_t * AppWindow_GetCont(uint8_t pageID)
{
    return (pageID < Disp_Max_Page) ? appWindow_Grp[pageID].cont : NULL;
}

lv_coord_t AppWindow_GetHeight()
{
    return (lv_obj_get_height(lv_scr_act()));// - BarStatus_GetHeight() - BarNavigation_GetHeight());
}

lv_coord_t AppWindow_GetWidth()
{
    return (lv_obj_get_width(lv_scr_act()));
}

void AppWindow_Create()
{

}
