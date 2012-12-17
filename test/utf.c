/* UnitTest - Copyright (c) Nava Whiteford 2012
 *
 * This software is supplied under the zlib license, which is copyfree and GPL compatible.
 * See license.txt for full details.
 */

#ifndef UNITTESTC_H
#define UNITTESTC_H

#include "utf.h"
#include <stdbool.h>
#include <stdio.h>
#define test(t1) _test(t1,__LINE__,__FILE__)

size_t tests_failed;
size_t tests_passed;
size_t total_tests_failed;
size_t total_tests_passed;
size_t test_id;
char * current_description;
char * test_set_name;

FILE * tap_file;

utf_init() {
  tests_failed = 0;
  tests_passed = 0;
  total_tests_failed = 0;
  total_tests_passed = 0;
  test_id = 0;
  current_description = 0;
  test_set_name = 0;
  tap_file = 0;
}

void write_tap(char *filename) {

  tap_file = fopen(filename,"r");

}

void begin_test_set(char *description) {

  if(current_description != 0) free(current_description);

  current_description = malloc(sizeof(description));

  strcpy(current_description,description);

  tests_passed=0;
  tests_failed=0;
  printf("****** Testing: %s\n",current_description); 
}

void end_test_set() {
  printf("****** Test   : %s complete, ",current_description);
  printf("passed %d, failed %d\n",tests_passed,tests_failed);
}

void _test(bool t1,int linenumber,const char *current_file) {
  if(!t1) {
    printf("****** FAILED : %s,%d\n",current_file,linenumber);
    if(tap_file != 0) {
      fprintf(tap_file,"no ok %d at: %s,%d\n",test_id,current_file,linenumber);
    }
    total_tests_failed++;
    tests_failed++;
  } else { 
    tests_passed++;
    total_tests_passed++; 
    if(tap_file != 0) {
      fprintf(tap_file,"ok %d at: %s,%d\n",test_id,current_file,linenumber);
    }
  }
  test_id++;
}

void test_report() {
  printf("*** Tests complete: passed %d, failed %d\n",test_set_name,total_tests_passed,total_tests_failed);
  if(total_tests_failed != 0) printf("*** TEST FAILED!\n");
  if(tap_file != 0) fclose(tap_file);
}

#endif
