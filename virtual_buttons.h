#ifndef VIRTUALKB_H
#define VIRTUALKB_H

void virtual_kb_up(char *c);
void virtual_kb_down(char *c);
void virtual_kb_left(char *c);
void virtual_kb_right(char *c);
void virtual_kb_esc(char *c);
void virtual_kb_ctrl(char *c);
void virtual_kb_alt(char *c);
void virtual_kb_tab(char *c);
void virtual_kb_paste(char *c);
void virtual_kb_kbshow(char *c);
void virtual_kb_close(char *c);
void virtual_buttons_add();
void virtual_buttons_disable();
void virtual_buttons_reposition();

#endif