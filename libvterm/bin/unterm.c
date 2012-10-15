#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "vterm.h"

static VTerm *vt;
static VTermScreen *vts;

static int cols;
static int rows;

void dump_row(int row)
{
  VTermRect rect = {
    .start_row = row,
    .start_col = 0,
    .end_row   = row+1,
    .end_col   = cols,
  };

  size_t len = vterm_screen_get_text(vts, NULL, 0, rect);
  char *text = malloc(len + 1);
  text[len] = 0;

  vterm_screen_get_text(vts, text, len, rect);

  printf("%s\n", text);

  free(text);
}

static int screen_prescroll(VTermRect rect, void *user)
{
  if(rect.start_row != 0 || rect.start_col != 0 || rect.end_col != cols)
    return 0;

  for(int row = 0; row < rect.end_row; row++)
    dump_row(row);

  return 1;
}

static int screen_resize(int new_rows, int new_cols, void *user)
{
  rows = new_rows;
  cols = new_cols;
  return 1;
}

static VTermScreenCallbacks cb_screen = {
  .prescroll = &screen_prescroll,
  .resize    = &screen_resize,
};

int main(int argc, char *argv[])
{
  const char *file = argv[1];
  int fd = open(file, O_RDONLY);
  if(fd == -1) {
    fprintf(stderr, "Cannot open %s - %s\n", file, strerror(errno));
    exit(1);
  }

  rows = 25;
  cols = 80;

  vt = vterm_new(rows, cols);
  vts = vterm_obtain_screen(vt);
  vterm_screen_set_callbacks(vts, &cb_screen, NULL);

  vterm_screen_reset(vts, 1);

  int len;
  char buffer[1024];
  while((len = read(fd, buffer, sizeof(buffer))) > 0) {
    vterm_push_bytes(vt, buffer, len);
  }

  for(int row = 0; row < rows; row++) {
    dump_row(row);
  }

  close(fd);

  vterm_free(vt);
}
