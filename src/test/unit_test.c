#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <topaz/topaz.h>

START_TEST(cc_errno_size)
{
  ck_assert_msg(sizeof(tp_errno_t) == 4,
    "Expected tp_errno_t to be 4 bytes in size");
}
END_TEST

START_TEST(cc_errno_sign)
{
  ck_assert_msg((tp_errno_t)0 < (tp_errno_t)-1,
    "Expected tp_errno_t to be unsigned");
}
END_TEST

Suite *cc_suite(void)
{
  Suite *s = suite_create("Compiler");
  
  /* Datatype tests */
  TCase *tc_errno = tcase_create("Errno Datatype");
  tcase_add_test(tc_errno, cc_errno_size);
  tcase_add_test(tc_errno, cc_errno_sign);
  suite_add_tcase(s, tc_errno);
  
  return s;
}

int main (void)
{
  int number_failed;
  Suite *s = cc_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
