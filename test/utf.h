/* UnitTest - Copyright (c) Nava Whiteford 2012
 *
 * This software is supplied under the zlib license, which is copyfree and GPL compatible.
 * See license.txt for full details.
 */

#ifndef UNITTESTC_H
#define UNITTESTC_H

#include <stdbool.h>


#define test(t1) _test(t1,__LINE__,__FILE__)

utf_init();
void write_tap(char *filename);
void begin_test_set(char *description);
void end_test_set();
void _test(bool t1,int linenumber,const char *current_file);
void test_report();

#endif
