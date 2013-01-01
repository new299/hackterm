#include <string.h>

void ngui_render_scrollbar(int widget_id) {
}


int ngui_create_scrollbar(int x,int y,int width,int height,int docsize,int data_start,int data_end) {
}

void ngui_add_scrollbar_callback(int widget_id,void *callback) {
}


void ngui_receive_mouse() {
}

void ngui_render() {
}

void ngui_info_prompt(const char *p1    ,const char *p2    ,const char *p3,
                      int         p1_opt,int         p2_opt,int         p3_opt,
                      char       *o1    ,char       *o2    ,char       *o3) {

  strcpy(o1,"127.0.0.1");
  strcpy(o2,"user");
  strcpy(o3,"password");

}
