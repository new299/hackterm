#include "vterm.h"
#include "../src/vterm_internal.h" // We pull in some internal bits too

#include <stdio.h>
#include <string.h>

#define streq(a,b) (!strcmp(a,b))
#define strstartswith(a,b) (!strncmp(a,b,strlen(b)))

static size_t inplace_hex2bytes(char *s)
{
  char *inpos = s, *outpos = s;

  while(*inpos) {
    int ch;
    sscanf(inpos, "%2x", &ch);
    *outpos = ch;
    outpos += 1; inpos += 2;
  }

  return outpos - s;
}

static VTermModifier strpe_modifiers(char **strp)
{
  VTermModifier state = 0;

  while((*strp)[0]) {
    switch(((*strp)++)[0]) {
      case 'S': state |= VTERM_MOD_SHIFT; break;
      case 'C': state |= VTERM_MOD_CTRL;  break;
      case 'A': state |= VTERM_MOD_ALT;   break;
      default: return state;
    }
  }

  return state;
}

static VTermKey strp_key(char *str)
{
  static struct {
    char *name;
    VTermKey key;
  } keys[] = {
    { "Up",    VTERM_KEY_UP },
    { "Tab",   VTERM_KEY_TAB },
    { "Enter", VTERM_KEY_ENTER },
    { NULL,    VTERM_KEY_NONE },
  };

  for(int i = 0; keys[i].name; i++) {
    if(streq(str, keys[i].name))
      return keys[i].key;
  }

  return VTERM_KEY_NONE;
}

static VTerm *vt;
static VTermState *state;
static VTermScreen *screen;

static VTermEncodingInstance encoding;

static int parser_text(const char bytes[], size_t len, void *user)
{
  printf("text ");
  int i;
  for(i = 0; i < len; i++) {
    unsigned char b = bytes[i];
    if(b < 0x20 || (b >= 0x80 && b < 0xa0))
      break;
    printf(i ? ",%x" : "%x", b);
  }
  printf("\n");

  return i;
}

static int parser_control(unsigned char control, void *user)
{
  printf("control %02x\n", control);

  return 1;
}

static int parser_escape(const char bytes[], size_t len, void *user)
{
  if(bytes[0] >= 0x20 && bytes[0] < 0x30) {
    if(len < 2)
      return -1;
    len = 2;
  }
  else {
    len = 1;
  }

  printf("escape ");
  for(int i = 0; i < len; i++)
    printf("%02x", bytes[i]);
  printf("\n");

  return len;
}

static int parser_csi(const char *leader, const long args[], int argcount, const char *intermed, char command, void *user)
{
  printf("csi %02x", command);

  if(leader && leader[0]) {
    printf(" L=");
    for(int i = 0; leader[i]; i++)
      printf("%02x", leader[i]);
  }

  for(int i = 0; i < argcount; i++) {
    char sep = i ? ',' : ' ';

    if(args[i] == CSI_ARG_MISSING)
      printf("%c*", sep);
    else
      printf("%c%ld%s", sep, CSI_ARG(args[i]), CSI_ARG_HAS_MORE(args[i]) ? "+" : "");
  }

  if(intermed && intermed[0]) {
    printf(" I=");
    for(int i = 0; intermed[i]; i++)
      printf("%02x", intermed[i]);
  }

  printf("\n");

  return 1;
}

static int parser_osc(const char *command, size_t cmdlen, void *user)
{
  printf("osc ");
  for(int i = 0; i < cmdlen; i++)
    printf("%02x", command[i]);
  printf("\n");

  return 1;
}

static int parser_dcs(const char *command, size_t cmdlen, void *user)
{
  printf("dcs ");
  for(int i = 0; i < cmdlen; i++)
    printf("%02x", command[i]);
  printf("\n");

  return 1;
}

static VTermParserCallbacks parser_cbs = {
  .text    = parser_text,
  .control = parser_control,
  .escape  = parser_escape,
  .csi     = parser_csi,
  .osc     = parser_osc,
  .dcs     = parser_dcs,
};

/* These callbacks are shared by State and Screen */

static int want_movecursor = 0;
static VTermPos state_pos;
static int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user)
{
  state_pos = pos;

  if(want_movecursor)
    printf("movecursor %d,%d\n", pos.row, pos.col);

  return 1;
}

static int want_scrollrect = 0;
static int scrollrect(VTermRect rect, int downward, int rightward, void *user)
{
  if(!want_scrollrect)
    return 0;

  printf("scrollrect %d..%d,%d..%d => %+d,%+d\n",
      rect.start_row, rect.end_row, rect.start_col, rect.end_col,
      downward, rightward);

  return 1;
}

static int want_moverect = 0;
static int moverect(VTermRect dest, VTermRect src, void *user)
{
  if(!want_moverect)
    return 0;

  printf("moverect %d..%d,%d..%d -> %d..%d,%d..%d\n",
      src.start_row,  src.end_row,  src.start_col,  src.end_col,
      dest.start_row, dest.end_row, dest.start_col, dest.end_col);

  return 1;
}

static int want_settermprop = 0;
static int settermprop(VTermProp prop, VTermValue *val, void *user)
{
  if(!want_settermprop)
    return 1;

  VTermValueType type = vterm_get_prop_type(prop);
  switch(type) {
  case VTERM_VALUETYPE_BOOL:
    printf("settermprop %d %s\n", prop, val->boolean ? "true" : "false");
    return 1;
  case VTERM_VALUETYPE_INT:
    printf("settermprop %d %d\n", prop, val->number);
    return 1;
  case VTERM_VALUETYPE_STRING:
    printf("settermprop %d \"%s\"\n", prop, val->string);
    return 1;
  case VTERM_VALUETYPE_COLOR:
    printf("settermprop %d rgb(%d,%d,%d)\n", prop, val->color.red, val->color.green, val->color.blue);
    return 1;
  }

  return 0;
}

static int want_mouse = 0;
static VTermMouseFunc mousefunc;
static void *mousedata;
static int setmousefunc(VTermMouseFunc func, void *data, void *user)
{
  mousefunc = func;
  mousedata = data;

  if(want_mouse)
    printf("setmousefunc %s\n", func ? "func" : "(null)");

  return 1;
}

/* These callbacks are for State */

static int want_state_putglyph = 0;
static int state_putglyph(const uint32_t chars[], int width, VTermPos pos, void *user)
{
  if(!want_state_putglyph)
    return 1;

  printf("putglyph ");
  for(int i = 0; chars[i]; i++)
    printf(i ? ",%x" : "%x", chars[i]);
  printf(" %d %d,%d\n", width, pos.row, pos.col);

  return 1;
}

static int want_state_erase = 0;
static int state_erase(VTermRect rect, void *user)
{
  if(!want_state_erase)
    return 1;

  printf("erase %d..%d,%d..%d\n",
      rect.start_row, rect.end_row, rect.start_col, rect.end_col);

  return 1;
}

static struct {
  int bold;
  int underline;
  int italic;
  int blink;
  int reverse;
  int strike;
  int font;
  VTermColor foreground;
  VTermColor background;
} state_pen;
static int state_setpenattr(VTermAttr attr, VTermValue *val, void *user)
{
  switch(attr) {
  case VTERM_ATTR_BOLD:
    state_pen.bold = val->boolean;
    break;
  case VTERM_ATTR_UNDERLINE:
    state_pen.underline = val->number;
    break;
  case VTERM_ATTR_ITALIC:
    state_pen.italic = val->boolean;
    break;
  case VTERM_ATTR_BLINK:
    state_pen.blink = val->boolean;
    break;
  case VTERM_ATTR_REVERSE:
    state_pen.reverse = val->boolean;
    break;
  case VTERM_ATTR_STRIKE:
    state_pen.strike = val->boolean;
    break;
  case VTERM_ATTR_FONT:
    state_pen.font = val->number;
    break;
  case VTERM_ATTR_FOREGROUND:
    state_pen.foreground = val->color;
    break;
  case VTERM_ATTR_BACKGROUND:
    state_pen.background = val->color;
    break;
  }

  return 1;
}

VTermStateCallbacks state_cbs = {
  .putglyph     = state_putglyph,
  .movecursor   = movecursor,
  .scrollrect   = scrollrect,
  .moverect     = moverect,
  .erase        = state_erase,
  .setpenattr   = state_setpenattr,
  .settermprop  = settermprop,
  .setmousefunc = setmousefunc,
};

static int want_screen_damage = 0;
static int screen_damage(VTermRect rect, void *user)
{
  if(!want_screen_damage)
    return 1;

  printf("damage %d..%d,%d..%d\n",
      rect.start_row, rect.end_row, rect.start_col, rect.end_col);

  return 1;
}

static int want_screen_prescroll = 0;
static int screen_prescroll(VTermRect rect, void *user)
{
  if(!want_screen_prescroll)
    return 1;

  printf("prescroll %d..%d,%d..%d\n",
      rect.start_row, rect.end_row, rect.start_col, rect.end_col);

  return 1;
}


VTermScreenCallbacks screen_cbs = {
  .damage       = screen_damage,
  .prescroll    = screen_prescroll,
  .moverect     = moverect,
  .movecursor   = movecursor,
  .settermprop  = settermprop,
  .setmousefunc = setmousefunc,
};

int main(int argc, char **argv)
{
  char line[1024];
  int flag;

  int err;

  setvbuf(stdout, NULL, _IONBF, 0);

  while(fgets(line, sizeof line, stdin)) {
    err = 0;

    char *nl;
    if((nl = strchr(line, '\n')))
      *nl = '\0';

    if(streq(line, "INIT")) {
      if(!vt)
        vt = vterm_new(25, 80);
    }

    else if(streq(line, "WANTPARSER")) {
      vterm_set_parser_callbacks(vt, &parser_cbs, NULL);
    }

    else if(strstartswith(line, "WANTSTATE") && (line[9] == '\0' || line[9] == ' ')) {
      if(!state) {
        state = vterm_obtain_state(vt);
        vterm_state_set_callbacks(state, &state_cbs, NULL);
        vterm_state_set_bold_highbright(state, 1);
        vterm_state_reset(state, 1);
      }

      int i = 9;
      int sense = 1;
      while(line[i] == ' ')
        i++;
      for( ; line[i]; i++)
        switch(line[i]) {
        case '+':
          sense = 1;
          break;
        case '-':
          sense = 0;
          break;
        case 'g':
          want_state_putglyph = sense;
          break;
        case 's':
          want_scrollrect = sense;
          break;
        case 'm':
          want_moverect = sense;
          break;
        case 'e':
          want_state_erase = sense;
          break;
        case 'p':
          want_settermprop = sense;
          break;
        case 'M':
          want_mouse = sense;
          break;
        default:
          fprintf(stderr, "Unrecognised WANTSTATE flag '%c'\n", line[i]);
        }
    }

    else if(strstartswith(line, "WANTSCREEN") && (line[10] == '\0' || line[10] == ' ')) {
      if(!screen)
        screen = vterm_obtain_screen(vt);
      vterm_screen_enable_altscreen(screen, 1);
      vterm_screen_set_callbacks(screen, &screen_cbs, NULL);

      int i = 10;
      int sense = 1;
      while(line[i] == ' ')
        i++;
      for( ; line[i]; i++)
        switch(line[i]) {
        case '-':
          sense = 0;
          break;
        case 'd':
          want_screen_damage = sense;
          break;
        case 's':
          want_screen_prescroll = sense;
          break;
        case 'm':
          want_moverect = sense;
          break;
        case 'c':
          want_movecursor = sense;
          break;
        case 'p':
          want_settermprop = 1;
          break;
        case 'M':
          want_mouse = sense;
          break;
        default:
          fprintf(stderr, "Unrecognised WANTSCREEN flag '%c'\n", line[i]);
        }
    }

    else if(sscanf(line, "UTF8 %d", &flag)) {
      vterm_parser_set_utf8(vt, flag);
    }

    else if(streq(line, "RESET")) {
      if(state) {
        vterm_state_reset(state, 1);
        vterm_state_get_cursorpos(state, &state_pos);
      }
      if(screen) {
        vterm_screen_reset(screen, 1);
      }
    }

    else if(strstartswith(line, "RESIZE ")) {
      int rows, cols;
      char *linep = line + 7;
      while(linep[0] == ' ')
        linep++;
      sscanf(linep, "%d, %d", &rows, &cols);
      vterm_set_size(vt, rows, cols);
    }

    else if(strstartswith(line, "PUSH ")) {
      char *bytes = line + 5;
      size_t len = inplace_hex2bytes(bytes);
      vterm_push_bytes(vt, bytes, len);
    }

    else if(streq(line, "WANTENCODING")) {
      /* This isn't really external API but it's hard to get this out any
       * other way
       */
      encoding.enc = vterm_lookup_encoding(ENC_UTF8, 'u');
      if(encoding.enc->init)
        (*encoding.enc->init)(encoding.enc, encoding.data);
    }

    else if(strstartswith(line, "ENCIN ")) {
      char *bytes = line + 6;
      size_t len = inplace_hex2bytes(bytes);

      uint32_t cp[len];
      int cpi = 0;
      size_t pos = 0;

      (*encoding.enc->decode)(encoding.enc, encoding.data,
          cp, &cpi, len, bytes, &pos, len);

      if(cpi > 0) {
        printf("encout ");
        for(int i = 0; i < cpi; i++) {
          printf(i ? ",%x" : "%x", cp[i]);
        }
        printf("\n");
      }
    }

    else if(strstartswith(line, "INCHAR ")) {
      char *linep = line + 7;
      int c = 0;
      while(linep[0] == ' ')
        linep++;
      VTermModifier state = strpe_modifiers(&linep);
      sscanf(linep, " %x", &c);

      vterm_input_push_char(vt, state, c);
    }

    else if(strstartswith(line, "INKEY ")) {
      char *linep = line + 6;
      while(linep[0] == ' ')
        linep++;
      VTermModifier state = strpe_modifiers(&linep);
      while(linep[0] == ' ')
        linep++;
      VTermKey key = strp_key(linep);

      vterm_input_push_key(vt, state, key);
    }

    else if(strstartswith(line, "MOUSE ")) {
      char *linep = line + 6;
      int press, button, row, col;
      while(linep[0] == ' ')
        linep++;
      VTermModifier state = strpe_modifiers(&linep);
      while(linep[0] == ' ')
        linep++;
      sscanf(linep, "%d %d %d,%d", &press, &button, &row, &col);
      if(mousefunc)
        (*mousefunc)(col, row, button, press, state, mousedata);
    }

    else if(strstartswith(line, "DAMAGEMERGE ")) {
      char *linep = line + 12;
      while(linep[0] == ' ')
        linep++;
      if(streq(linep, "CELL"))
        vterm_screen_set_damage_merge(screen, VTERM_DAMAGE_CELL);
      else if(streq(linep, "ROW"))
        vterm_screen_set_damage_merge(screen, VTERM_DAMAGE_ROW);
      else if(streq(linep, "SCREEN"))
        vterm_screen_set_damage_merge(screen, VTERM_DAMAGE_SCREEN);
      else if(streq(linep, "SCROLL"))
        vterm_screen_set_damage_merge(screen, VTERM_DAMAGE_SCROLL);
    }

    else if(strstartswith(line, "DAMAGEFLUSH")) {
      vterm_screen_flush_damage(screen);
    }

    else if(line[0] == '?') {
      if(streq(line, "?cursor")) {
        VTermPos pos;
        vterm_state_get_cursorpos(state, &pos);
        if(pos.row != state_pos.row)
          printf("! row mismatch: state=%d,%d event=%d,%d\n",
              pos.row, pos.col, state_pos.row, state_pos.col);
        else if(pos.col != state_pos.col)
          printf("! col mismatch: state=%d,%d event=%d,%d\n",
              pos.row, pos.col, state_pos.row, state_pos.col);
        else
          printf("%d,%d\n", state_pos.row, state_pos.col);
      }
      else if(strstartswith(line, "?pen ")) {
        char *linep = line + 5;
        while(linep[0] == ' ')
          linep++;

        VTermValue val;
#define BOOLSTR(v) ((v) ? "on" : "off")

        if(streq(linep, "bold")) {
          vterm_state_get_penattr(state, VTERM_ATTR_BOLD, &val);
          if(val.boolean != state_pen.bold)
            printf("! pen bold mismatch; state=%s, event=%s\n",
                BOOLSTR(val.boolean), BOOLSTR(state_pen.bold));
          else
            printf("%s\n", BOOLSTR(state_pen.bold));
        }
        else if(streq(linep, "underline")) {
          vterm_state_get_penattr(state, VTERM_ATTR_UNDERLINE, &val);
          if(val.boolean != state_pen.underline)
            printf("! pen underline mismatch; state=%d, event=%d\n",
                val.boolean, state_pen.underline);
          else
            printf("%d\n", state_pen.underline);
        }
        else if(streq(linep, "italic")) {
          vterm_state_get_penattr(state, VTERM_ATTR_ITALIC, &val);
          if(val.boolean != state_pen.italic)
            printf("! pen italic mismatch; state=%s, event=%s\n",
                BOOLSTR(val.boolean), BOOLSTR(state_pen.italic));
          else
            printf("%s\n", BOOLSTR(state_pen.italic));
        }
        else if(streq(linep, "blink")) {
          vterm_state_get_penattr(state, VTERM_ATTR_BLINK, &val);
          if(val.boolean != state_pen.blink)
            printf("! pen blink mismatch; state=%s, event=%s\n",
                BOOLSTR(val.boolean), BOOLSTR(state_pen.blink));
          else
            printf("%s\n", BOOLSTR(state_pen.blink));
        }
        else if(streq(linep, "reverse")) {
          vterm_state_get_penattr(state, VTERM_ATTR_REVERSE, &val);
          if(val.boolean != state_pen.reverse)
            printf("! pen reverse mismatch; state=%s, event=%s\n",
                BOOLSTR(val.boolean), BOOLSTR(state_pen.reverse));
          else
            printf("%s\n", BOOLSTR(state_pen.reverse));
        }
        else if(streq(linep, "font")) {
          vterm_state_get_penattr(state, VTERM_ATTR_FONT, &val);
          if(val.boolean != state_pen.font)
            printf("! pen font mismatch; state=%d, event=%d\n",
                val.boolean, state_pen.font);
          else
            printf("%d\n", state_pen.font);
        }
        else if(streq(linep, "foreground")) {
          printf("rgb(%d,%d,%d)\n", state_pen.foreground.red, state_pen.foreground.green, state_pen.foreground.blue);
        }
        else if(streq(linep, "background")) {
          printf("rgb(%d,%d,%d)\n", state_pen.background.red, state_pen.background.green, state_pen.background.blue);
        }
        else
          printf("?\n");
      }
      else if(strstartswith(line, "?screen_chars ")) {
        char *linep = line + 13;
        VTermRect rect;
        size_t len;
        while(linep[0] == ' ')
          linep++;
        if(sscanf(linep, "%d,%d,%d,%d", &rect.start_row, &rect.start_col, &rect.end_row, &rect.end_col) < 4) {
          printf("! screen_chars unrecognised input\n");
          goto abort_line;
        }
        len = vterm_screen_get_chars(screen, NULL, 0, rect);
        if(len == (size_t)-1)
          printf("! screen_chars error\n");
        else if(len == 0)
          printf("\n");
        else {
          uint32_t *chars = malloc(sizeof(uint32_t) * len);
          vterm_screen_get_chars(screen, chars, len, rect);
          for(size_t i = 0; i < len; i++) {
            printf("0x%02x%s", chars[i], i < len-1 ? "," : "\n");
          }
          free(chars);
        }
      }
      else if(strstartswith(line, "?screen_text ")) {
        char *linep = line + 12;
        VTermRect rect;
        size_t len;
        while(linep[0] == ' ')
          linep++;
        if(sscanf(linep, "%d,%d,%d,%d", &rect.start_row, &rect.start_col, &rect.end_row, &rect.end_col) < 4) {
          printf("! screen_text unrecognised input\n");
          goto abort_line;
        }
        len = vterm_screen_get_text(screen, NULL, 0, rect);
        if(len == (size_t)-1)
          printf("! screen_text error\n");
        else if(len == 0)
          printf("\n");
        else {
          char *text = malloc(len);
          vterm_screen_get_text(screen, text, len, rect);
          for(size_t i = 0; i < len; i++) {
            printf("0x%02x%s", (unsigned char)text[i], i < len-1 ? "," : "\n");
          }
          free(text);
        }
      }
      else if(strstartswith(line, "?screen_cell ")) {
        char *linep = line + 12;
        VTermPos pos;
        while(linep[0] == ' ')
          linep++;
        if(sscanf(linep, "%d,%d\n", &pos.row, &pos.col) < 2) {
          printf("! screen_cell unrecognised input\n");
          goto abort_line;
        }
        VTermScreenCell cell;
        if(!vterm_screen_get_cell(screen, pos, &cell))
          goto abort_line;
        printf("{");
        for(int i = 0; cell.chars[i] && i < VTERM_MAX_CHARS_PER_CELL; i++) {
          printf("%s0x%x", i ? "," : "", cell.chars[i]);
        }
        printf("} width=%d attrs={", cell.width);
        if(cell.attrs.bold)      printf("B");
        if(cell.attrs.underline) printf("U%d", cell.attrs.underline);
        if(cell.attrs.italic)    printf("I");
        if(cell.attrs.blink)     printf("K");
        if(cell.attrs.reverse)   printf("R");
        if(cell.attrs.font)      printf("F%d", cell.attrs.font);
        printf("} ");
        printf("fg=rgb(%d,%d,%d) ",  cell.fg.red, cell.fg.green, cell.fg.blue);
        printf("bg=rgb(%d,%d,%d)\n", cell.bg.red, cell.bg.green, cell.bg.blue);
      }
      else if(strstartswith(line, "?screen_eol ")) {
        char *linep = line + 12;
        while(linep[0] == ' ')
          linep++;
        VTermPos pos;
        if(sscanf(linep, "%d,%d\n", &pos.row, &pos.col) < 2) {
          printf("! screen_eol unrecognised input\n");
          goto abort_line;
        }
        printf("%d\n", vterm_screen_is_eol(screen, pos));
      }
      else
        printf("?\n");

      continue;
    }

    else
      abort_line: err = 1;

    size_t outlen = vterm_output_bufferlen(vt);
    if(outlen > 0) {
      char outbuff[outlen];
      vterm_output_bufferread(vt, outbuff, outlen);

      printf("output ");
      for(int i = 0; i < outlen; i++)
        printf("%x%s", (unsigned char)outbuff[i], i < outlen-1 ? "," : "\n");
    }

    printf(err ? "?\n" : "DONE\n");
  }

  vterm_free(vt);

  return 0;
}
