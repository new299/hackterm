#include <string.h>
#include <SDL.h>
#include "nsdl.h"
#include "nunifont.h"
#include "ngui.h"
#include <stdbool.h>

typedef struct {
  char name[100];
  int  outcount;
  bool selected;
  int  connected_to_flowbox;
  int  connected_to_port;
} ports_data;

typedef struct {
  bool valid;
  int x;
  int y;
  int x_padding;
  int y_padding;
  int shine;
  bool selected;
  int input_ports_count;
  ports_data input_ports[5];
  
  int output_ports_count;
  ports_data output_ports[5];
  
  char text[100];
  void (*callback)(const char *);
} ngui_flowbox_data;
    
typedef struct {
  bool valid;
  int input_port_flowbox;
  int input_port_number;
  int output_port_flowbox;
  int output_port_number;
} ngui_flowbox_connection_data;

int ngui_flowboxs_size = 0;
ngui_flowbox_data ngui_flowboxs[50];

int ngui_flowbox_connections_size = 0;
ngui_flowbox_connection_data ngui_flowbox_connections[50];

bool draw_selection_line = false;
int draw_selection_line_start_x = -1;
int draw_selection_line_start_y = -1;
int draw_selection_line_end_x = -1;
int draw_selection_line_end_y = -1;

bool flow_running = false;
int flow_run_count = 0;

int output_port_x(ngui_flowbox_data *d,int port) {
  return d->x+(strlen(d->text)*9)+(d->x_padding)-1;
}

int output_port_y(ngui_flowbox_data *d,int port) {
  return 2+d->y-d->y_padding+(port*15);
}

int output_port_w(ngui_flowbox_data *d,int port) {
  return 5;
}

int output_port_h(ngui_flowbox_data *d,int n) {
  return 10;
}

bool output_port_inside(int cx,int cy,ngui_flowbox_data *d,int port) {

  int x = output_port_x(d,port);
  int y = output_port_y(d,port);
  int w = output_port_w(d,port);
  int h = output_port_h(d,port);

  if((cx >= x) &&
     (cx <= (x+w)) &&
     (cy >= y) &&
     (cy <= (y+h))) {
    return true;
  }

  return false;
}

int input_port_x(ngui_flowbox_data *d,int port) {
  return d->x-d->x_padding-4;
}

int input_port_y(ngui_flowbox_data *d,int port) {
  return 2+d->y-d->y_padding+(port*15);
}

int input_port_w(ngui_flowbox_data *d,int port) {
  return 5;
}

int input_port_h(ngui_flowbox_data *d,int n) {
  return 10;
}

bool input_port_inside(int cx,int cy,ngui_flowbox_data *d,int port) {

  int x = input_port_x(d,port);
  int y = input_port_y(d,port);
  int w = input_port_w(d,port);
  int h = input_port_h(d,port);

  if((cx >= x) &&
     (cx <= (x+w)) &&
     (cy >= y) &&
     (cy <= (y+h))) {
    return true;
  }

  return false;
}

bool no_select=true;
void ngui_receive_event_flowbox(SDL_Event *event, ngui_flowbox_data *d) {

  if(event->type == SDL_MOUSEBUTTONDOWN) {

    int x = event->button.x;
    int y = event->button.y;
    
    if((x > (d->x-d->x_padding)) && (x < ((d->x)+(strlen(d->text)*9)+d->x_padding)) &&
       (y > (d->y-d->y_padding)) && (y < ((d->y)+16+d->y_padding))) {
      //d->callback("press");
      d->shine=20;
      d->selected=true;
      ngui_redraw_required();
    }

    for(int n=0;n<d->output_ports_count;n++) {
      if(output_port_inside(x,y,d,n)) {
        d->output_ports[n].selected=true;
        no_select=false;
      } 
    }

    for(int n=0;n<d->output_ports_count;n++) {
      if(input_port_inside(x,y,d,n)) {
        d->input_ports[n].selected=true;
        no_select=false;
      }
    }
    return;
  }

  if(event->type == SDL_MOUSEBUTTONUP) {
    d->selected=false;
    int x = event->button.x;
    int y = event->button.y;

    for(int n=0;n<d->output_ports_count;n++) {
      if(output_port_inside(x,y,d,n)) {
        d->output_ports[n].selected=true;
        no_select=false;
      } 
    }

    for(int n=0;n<d->output_ports_count;n++) {
      if(input_port_inside(x,y,d,n)) {
        d->input_ports[n].selected=true;
        no_select=false;
      } 
    }
    return;
  }

  no_select=false;

  if(event->type == SDL_MOUSEMOTION) {
    if(d->selected) {
      int x = event->button.x;
      int y = event->button.y;

      d->x = x;
      d->y = y;  
    }
    draw_selection_line_end_x = event->button.x;
    draw_selection_line_end_y = event->button.y;
    ngui_redraw_required();
  }
}

void ngui_move_flowbox(char *name,int nx,int ny) {

  for(int n=0;n<ngui_flowboxs_size;n++) {
    if(strcmp(ngui_flowboxs[n].text,name)==0) {
      ngui_flowboxs[n].x = nx;
      ngui_flowboxs[n].y = ny;
    }
  }

}

void ngui_render_flowbox(ngui_flowbox_data *d) {

  uint16_t text[100];
  for(int n=0;n<100;n++) text[n] = d->text[n];

  SDL_SetRenderDrawColor(ngui_renderer,0xAA,0xAA,0xAA,0xFF);
  
  bool notext=false;
  if(d->shine > 0) {d->shine--; ngui_redraw_required();}

  if(!notext) {
    SDL_Rect rect;
  
    rect.x = d->x-d->x_padding;
    rect.y = d->y-d->y_padding;
    rect.w = (strlen(d->text)*9)+(d->x_padding*2);
    rect.h = 16+(d->y_padding*2);
  
    SDL_RenderDrawRect(ngui_renderer,&rect);
  
    draw_unitext_renderer(ngui_renderer,
                d->x,
                d->y,
                text,
                0,
                65535,0,0,0,0);
  }

  SDL_SetRenderDrawColor(ngui_renderer,0xAA,0xAA,0xAA,0xFF);
  for(int n=0;n<d->output_ports_count;n++) {
    SDL_Rect rect;
  
    rect.x = output_port_x(d,n);
    rect.y = output_port_y(d,n);
    rect.w = output_port_w(d,n);
    rect.h = output_port_h(d,n);
  
    SDL_RenderDrawRect(ngui_renderer,&rect);
  }
  
  for(int n=0;n<d->input_ports_count;n++) {
    SDL_Rect rect;
  
    rect.x = input_port_x(d,n); 
    rect.y = input_port_y(d,n); 
    rect.w = input_port_w(d,n);
    rect.h = input_port_h(d,n); 
  
    SDL_RenderDrawRect(ngui_renderer,&rect);
  }
}

int flowbox_add_connection(int input_port_flowbox,int input_port_number,int output_port_flowbox,int output_port_number) {

  ngui_flowbox_connections[ngui_flowbox_connections_size].input_port_flowbox  = input_port_flowbox;
  ngui_flowbox_connections[ngui_flowbox_connections_size].input_port_number   = input_port_number;
  ngui_flowbox_connections[ngui_flowbox_connections_size].output_port_flowbox = output_port_flowbox;
  ngui_flowbox_connections[ngui_flowbox_connections_size].output_port_number  = output_port_number;
  ngui_flowbox_connections[ngui_flowbox_connections_size].valid = true;
  
  ngui_flowbox_connections_size++;
}

int ngui_add_flowbox(int x,int y,char *text,void *callback) {

  ngui_flowboxs[ngui_flowboxs_size].valid=true;
  ngui_flowboxs[ngui_flowboxs_size].x = x;
  ngui_flowboxs[ngui_flowboxs_size].y = y;
  ngui_flowboxs[ngui_flowboxs_size].x_padding = 15;
  ngui_flowboxs[ngui_flowboxs_size].y_padding = 15;
  ngui_flowboxs[ngui_flowboxs_size].shine = 0;
  ngui_flowboxs[ngui_flowboxs_size].output_ports_count = 3;
  ngui_flowboxs[ngui_flowboxs_size].input_ports_count = 1;
  strcpy(ngui_flowboxs[ngui_flowboxs_size].text,text);
  ngui_flowboxs[ngui_flowboxs_size].callback = (void (*)(const char *)) callback;

  ngui_flowboxs_size++;

  return ngui_flowboxs_size-1;
}

void ngui_delete_flowbox(int id) {
  ngui_flowboxs[id].valid = false;
}

void ngui_receiveall_flowbox(SDL_Event *event) {

  no_select=true;
  for(int n=0;n<ngui_flowboxs_size;n++) {
    ngui_flowbox_data *d = &ngui_flowboxs[n];
    if(d->valid) {
      ngui_receive_event_flowbox(event,d);
    }
  }

  for(int n=0;n<ngui_flowboxs_size;n++) {
    ngui_flowbox_data *d = &ngui_flowboxs[n];
    if(no_select) {
      for(int n=0;n<d->output_ports_count;n++) {d->output_ports[n].selected=false;}
      for(int n=0;n<d->input_ports_count;n++)  {d->input_ports [n].selected=false;}
    }
  }
}
   
void ngui_render_flowbox_connection(ngui_flowbox_connection_data *d) {

  int input_flowbox  = d->input_port_flowbox;
  int input_port     = d->input_port_number;
  int output_flowbox = d->output_port_flowbox;
  int output_port    = d->output_port_number;

  double ix = input_port_x (&ngui_flowboxs[input_flowbox] ,input_port);
  double iy = input_port_y (&ngui_flowboxs[input_flowbox] ,input_port);
  double ox = output_port_x(&ngui_flowboxs[output_flowbox],output_port);
  double oy = output_port_y(&ngui_flowboxs[output_flowbox],output_port);

  SDL_RenderDrawLine(ngui_renderer,
                     ix,
                     iy,
                     ox,
                     oy);

  int delta=0;
  if(ox < ix) delta= 1;
         else delta=-1;

  if(flow_running) {
    double m = (oy-iy) / (ox-ix);

    int n=0;
    for(int x=ox;x!=ix;x+=delta) {
      
      SDL_Rect rect;
  
      rect.x = x;
      rect.y = (m*(x-ox))+oy-4;
      rect.w = 8;//-((n+(flow_run_count/50))%10);
      rect.h = 8;//-((n+(flow_run_count/50))%10);
  
      if(n%15 == ((flow_run_count/50)%15)) SDL_RenderDrawRect(ngui_renderer,&rect);
      n++;
    }
    ngui_redraw_required();
    flow_run_count++;
  }
}

void ngui_renderall_flowbox() {
  for(int n=0;n<ngui_flowboxs_size;n++) {
    ngui_flowbox_data *d = &ngui_flowboxs[n];
    if(d->valid) {
      ngui_render_flowbox(d);
    }
  }

  int selected_output_port_flowbox = -1;
  int selected_output_port_number  = -1;
  for(int n=0;n<ngui_flowboxs_size;n++) {
    ngui_flowbox_data *d = &ngui_flowboxs[n];
    if(d->valid) {
      for(int i=0;i<d->output_ports_count;i++) {
        if(d->output_ports[i].selected == true) {
          selected_output_port_flowbox = n;
          selected_output_port_number  = i;
        }
      }
    }
  }
  
  int selected_input_port_flowbox = -1;
  int selected_input_port_number  = -1;
  for(int n=0;n<ngui_flowboxs_size;n++) {
    ngui_flowbox_data *d = &ngui_flowboxs[n];
    if(d->valid) {
      for(int i=0;i<d->input_ports_count;i++) {
        if(d->input_ports[i].selected == true) {
          selected_input_port_flowbox = n;
          selected_input_port_number  = i;
        }
      }
    }
  }

  if(selected_output_port_flowbox != -1) {
    draw_selection_line = true;
    draw_selection_line_start_x = output_port_x(&ngui_flowboxs[selected_output_port_flowbox],selected_output_port_number)+2;
    draw_selection_line_start_y = output_port_y(&ngui_flowboxs[selected_output_port_flowbox],selected_output_port_number)+2;
  } else
  if(selected_input_port_flowbox != -1) {
    draw_selection_line = true;
    draw_selection_line_start_x = input_port_x(&ngui_flowboxs[selected_input_port_flowbox],selected_input_port_number)+2;
    draw_selection_line_start_y = input_port_y(&ngui_flowboxs[selected_input_port_flowbox],selected_input_port_number)+2;
  } else {
    draw_selection_line = false;
  }

  if(draw_selection_line) {
    SDL_RenderDrawLine(ngui_renderer,
                       draw_selection_line_start_x,
                       draw_selection_line_start_y,
                       draw_selection_line_end_x,
                       draw_selection_line_end_y);
  }

  if((selected_input_port_flowbox != -1) &&
     (selected_output_port_flowbox != -1)) {
    ngui_flowboxs[selected_input_port_flowbox ].input_ports [selected_input_port_number ].selected = false;
    ngui_flowboxs[selected_output_port_flowbox].output_ports[selected_output_port_number].selected = false;

    flowbox_add_connection(selected_input_port_flowbox,selected_input_port_number,selected_output_port_flowbox,selected_output_port_number);
    selected_input_port_flowbox=-1;
    selected_output_port_flowbox=-1;
  }

  // render connections
  for(int n=0;n<ngui_flowbox_connections_size;n++) {
    ngui_flowbox_connection_data *d = &ngui_flowbox_connections[n];
    if(d->valid) {
      ngui_render_flowbox_connection(d);
    }
  }
}

void ngui_flowbox_run() {
  flow_running= !flow_running;
}
