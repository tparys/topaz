#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <topaz/topaz.h>
#include <topaz/buffer.h>

/* Unit Tests for errno data type */

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

/* Unit Tests for fixed length data buffers */

START_TEST(cc_buf_null)
{
  /* set up buffer */
  char buf[2];
  tp_buffer_t my_buf;
  my_buf.ptr = buf;
  my_buf.cur_len = 0;
  my_buf.total_len = 1; /* exclude NULL */
  memset(buf, 0, sizeof(buf));
  
  /* check null source pointer */
  tp_buf_add(&my_buf, NULL, 0);
  ck_assert_int_eq(tp_errno, TP_ERR_NULL);
  ck_assert_int_eq(my_buf.cur_len, 0);

  /* check null source buffer */
  tp_buf_add_buf(&my_buf, NULL);
  ck_assert_int_eq(tp_errno, TP_ERR_NULL);
  ck_assert_int_eq(my_buf.cur_len, 0);

  /* check null target */
  tp_buf_add_byte(NULL, 'x');
  ck_assert_int_eq(tp_errno, TP_ERR_NULL);
  
  /* check null target pointer */
  my_buf.ptr = NULL;
  tp_buf_add_byte(&my_buf, 'x');
  ck_assert_int_eq(tp_errno, TP_ERR_NULL);
  ck_assert_int_eq(my_buf.cur_len, 0);
}
END_TEST

START_TEST(cc_buf_bounds)
{
  /* set up buffer */
  char buf[2];
  tp_buffer_t my_buf;
  my_buf.ptr = buf;
  my_buf.cur_len = 0;
  my_buf.total_len = 1; /* exclude NULL */
  memset(buf, 0, sizeof(buf));
  
  /* add one byte */
  tp_buf_add_byte(&my_buf, 'x');
  
  /* checks */
  ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
  ck_assert_str_eq(buf, "x");
  
  /* add one more byte */
  tp_buf_add_byte(&my_buf, 'y');
  
  /* checks */
  ck_assert_int_eq(tp_errno, TP_ERR_SPACE);
  ck_assert_str_eq(buf, "x");
  
}
END_TEST

/* Unit Test Automation */

Suite *cc_suite(void)
{
  Suite *s = suite_create("Topaz");
  
  /* Datatype tests */
  TCase *tc_errno = tcase_create("Errno Datatype");
  tcase_add_test(tc_errno, cc_errno_size);
  tcase_add_test(tc_errno, cc_errno_sign);
  suite_add_tcase(s, tc_errno);
  
  /* Datatype tests */
  TCase *tc_buf = tcase_create("Data Buffers");
  tcase_add_test(tc_errno, cc_buf_null);
  tcase_add_test(tc_errno, cc_buf_bounds);
  suite_add_tcase(s, tc_buf);
  
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
