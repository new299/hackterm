#include "utf.h"
#include "../base64.h"


int main() {
  utf_init();
  begin_test_set("base64");
  test(true);
  test(true);
  test(true);
  test(true);
  test(true);
  test(true);
  test(true);
  test(false);
  end_test_set();

  test_report();
  

  return 0;
}
