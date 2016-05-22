#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <topaz/topaz.h>
#include <topaz/buffer.h>
#include <topaz/syntax.h>

/* Unit Tests for errno data type */

START_TEST(t_cc_errno)
{
  /* compiler *should* match these definitions ... */
  
  ck_assert_int_eq(sizeof(tp_errno_t), 4);
  ck_assert_msg((tp_errno_t)0 < (tp_errno_t)-1,
    "Expected tp_errno_t to be unsigned");
}
END_TEST

/* Unit Tests for fixed length data buffers */

START_TEST(t_buf_null)
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

START_TEST(t_buf_bounds)
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

/* Unit Tests for binary syntax */

START_TEST(t_syn_uint)
{
  int i;
  
  /* set up buffer */
  char raw[64];
  tp_buffer_t buf;
  buf.ptr = raw;
  buf.cur_len = 0;
  buf.total_len = sizeof(raw);
  
  /* smallest tiny atom */
  buf.cur_len = 0;
  memset(raw, 0, sizeof(raw));
  tp_syn_enc_uint(&buf, 0);
  ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
  ck_assert_int_eq(buf.cur_len, 1);
  ck_assert_int_eq(raw[0], 0);

  /* biggest tiny atom */
  buf.cur_len = 0;
  memset(raw, 0, sizeof(raw));
  tp_syn_enc_uint(&buf, 0x3f);
  ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
  ck_assert_int_eq(buf.cur_len, 1);
  ck_assert_str_eq(raw, "\x3f");

  /* smallest small atom */
  buf.cur_len = 0;
  memset(raw, 0, sizeof(raw));
  tp_syn_enc_uint(&buf, 0x40);
  ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
  ck_assert_int_eq(buf.cur_len, 2);
  ck_assert_str_eq(raw, "\x81\x40");
  
  /* small atom boundaries */
  for (i = 1; i < 8; i++)
  {
    uint64_t num = 1UL << (8 * i);
    
    /* before boundary */   
    buf.cur_len = 0;
    memset(raw, 0, sizeof(raw));
    tp_syn_enc_uint(&buf, num - 1);
    ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
    ck_assert_int_eq(buf.cur_len, i + 1);
    
    /* after boundary */   
    buf.cur_len = 0;
    memset(raw, 0, sizeof(raw));
    tp_syn_enc_uint(&buf, num);
    ck_assert_int_eq(tp_errno, TP_ERR_SUCCESS);
    ck_assert_int_eq(buf.cur_len, i + 2);
  }
}
END_TEST

/* Unit Test Automation */

Suite *cc_suite(void)
{
  Suite *s = suite_create("Topaz");
  
  /* Compiler */
  TCase *tc_errno = tcase_create("Compiler");
  tcase_add_test(tc_errno, t_cc_errno);
  suite_add_tcase(s, tc_errno);
  
  /* Static Buffers */
  TCase *tc_buf = tcase_create("Data Buffers");
  tcase_add_test(tc_errno, t_buf_null);
  tcase_add_test(tc_errno, t_buf_bounds);
  suite_add_tcase(s, tc_buf);
  
  /* Binary Syntax */
  TCase *tc_syn = tcase_create("Syntax");
  tcase_add_test(tc_syn, t_syn_uint);
  suite_add_tcase(s, tc_syn);
  
  return s;
}

int main (void)
{
  int number_failed;
  Suite *s = cc_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
