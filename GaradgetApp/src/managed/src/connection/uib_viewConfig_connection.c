
/*******************************************************************************
* This file was generated by UI Builder.
* This file will be auto-generated each and every time you save your project.
* Do not hand edit this file.
********************************************************************************/
#include "app_main.h"
#include "uib_views_inc.h"

void connection_viewConfig_OK_onclicked(uib_viewConfig_view_context *vc, Evas_Object *obj, void *event_info){
	viewConfig_OK_onclicked(vc, obj, event_info);
	Elm_Object_Item* navi_item = uib_util_push_view("main");
	viewConfig_OK_onclicked_post(navi_item, vc, obj, event_info);
}
