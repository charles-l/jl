/* Taken and adapated from https://github.com/bmateusz/tst/blob/master/test.h
 *
 * ORIGINAL LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2017 Börcsök Máté
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TST_H
#define TST_H

#include <stdio.h>
#include <string.h>

static int RETVAL;

#define BEGIN \
static int run_tc_or_not(int argc, char *argv[], char *name) {\
  int i, skip = 0, ret = 0;\
  if (argc == 1) return 1;\
  for (i = 1; i < argc; ++i) {\
    if (strcmp(argv[i], "-skip") == 0) { skip = 1; if (i == 1) ret = 1; }\
    if (strstr(name, argv[i]) != (void*)0) { if (skip) ret = 0; else ret = 1; }\
  }\
  return ret;\
}\
int main(int argc, char *argv[]) {\
  int success = 0, failed = 0;\
  printf("test %s\n", __FILE__);

#define TEST(x) \
  if (run_tc_or_not(argc, argv, #x)) {\
    printf("%s ", #x);\
    RETVAL = 0;\
    x();\
    if (RETVAL) ++failed;\
    else { printf("success\n"); ++success; }\
  }

#define END \
  printf("  success: %d\n  failed: %d\n", success, failed);\
  return failed;\
}

#define fail printf("failed at %s:%d\n", __FILE__, __LINE__); RETVAL = 1; return;
#define fail_v(x) printf(x " at %s:%d\n", __FILE__, __LINE__); RETVAL = 1; return;
#define assert(x) if(!(x)) { printf("failed %s assertion at %s:%d\n", #x, __FILE__, __LINE__); RETVAL = 1; return; }

#define streq(x, y) if (strcmp(x, y) != 0) { printf("failed \"%s\" == \"%s\" assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define strne(x, y) if (strcmp(x, y) == 0) { printf("failed \"%s\" != \"%s\" assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define strin(x, y) if (strstr(x, y) == (void*)0) { printf("failed \"%s\" contains \"%s\" assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define liste(x, y) for(long *a = x, b = y; a != NIL || b != NIL; a = cdr_(a), b = cdr_(b)) eq(car_(a), car_(b));

#define eq(x, y) if (x != y) { printf("failed %d == %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define ne(x, y) if (x == y) { printf("failed %d != %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define lt(x, y) if (x >= y) { printf("failed %d < %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define gt(x, y) if (x <= y) { printf("failed %d > %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define le(x, y) if (x > y) { printf("failed %d <= %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define ge(x, y) if (x < y) { printf("failed %d >= %d assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }

#define eq_t(x, y, t) if (x != y) { printf("failed " t " == " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define ne_t(x, y, t) if (x == y) { printf("failed " t " != " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define lt_t(x, y, t) if (x >= y) { printf("failed " t " < " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define gt_t(x, y, t) if (x <= y) { printf("failed " t " > " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define le_t(x, y, t) if (x > y) { printf("failed " t " <= " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }
#define ge_t(x, y, t) if (x < y) { printf("failed " t " >= " t " assertion at %s:%d\n", x, y, __FILE__, __LINE__); RETVAL = 1; return; }

#endif
