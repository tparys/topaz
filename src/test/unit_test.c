#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <check.h>
#include <topaz/topaz.h>
#include <topaz/buffer.h>
#include <topaz/syntax.h>

/* Helper macro for tests */
#define CHECKME(x) if (x)                  \
{                                          \
  printf("FAIL(%s)\n", tp_errno_lookup_cur());	\
  return 1;                                \
} printf("OK\n")

/* Prototypes */

void dump_buf(tp_buffer_t *buf);
int run_uint(uint64_t value, size_t enc_size);
int run_sint(int64_t value, size_t enc_size);
int run_bin(size_t bin_size, size_t enc_size);

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
  char buf[2];
  tp_buffer_t my_buf;

  /* set up buffer */
  memset(&buf, 0, sizeof(buf));
  memset(&my_buf, 0, sizeof(my_buf));
  my_buf.ptr = buf;
  my_buf.max_len = 1; /* exclude NULL */
  
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
  char buf[2];
  tp_buffer_t my_buf;

  /* set up buffer */
  memset(&buf, 0, sizeof(buf));
  memset(&my_buf, 0, sizeof(my_buf));
  my_buf.ptr = buf;
  my_buf.max_len = 1; /* exclude NULL */

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
  
  /* Single byte encodings (tiny atom) */
  ck_assert_int_eq(0, run_uint(0x00, 1));
  ck_assert_int_eq(0, run_uint(0x3f, 1));
  
  /* Smallest two byte encoding (small atom) */
  ck_assert_int_eq(0, run_uint(0x40, 2));
  
  /* small atom boundaries */
  for (i = 1; i < 8; i++)
  {
    uint64_t num = 1UL << (8 * i);
    
    ck_assert_int_eq(0, run_uint(num - 1, i + 1));
    ck_assert_int_eq(0, run_uint(num, i + 2));
  }
  
  /* biggest 8 byte unsigned int (still a small atom) */
  ck_assert_int_eq(0, run_uint(0xffffffffffffffff, 9));
}
END_TEST

START_TEST(t_syn_sint)
{
  int i;
  
  char raw[64];
  tp_buffer_t buf;

  /* set up buffer */
  memset(raw, 0, sizeof(raw));
  memset(&buf, 0, sizeof(buf));
  buf.ptr = raw;
  buf.max_len = sizeof(raw);
  
  /* Single byte encodings (tiny atom) */
  ck_assert_int_eq(0, run_sint(0x00, 1));
  ck_assert_int_eq(0, run_sint(0x1f, 1));
  ck_assert_int_eq(0, run_sint(-0x20, 1));
  
  /* Smallest two byte encodings (small atom) */
  ck_assert_int_eq(0, run_sint(0x20, 2));
  ck_assert_int_eq(0, run_sint(-0x21, 2));
  
  /* small atom boundaries */
  for (i = 1; i < 8; i++)
  {
    int64_t num = 1UL << ((8 * i) - 1);
    
    ck_assert_int_eq(0, run_sint(num - 1, i + 1));
    ck_assert_int_eq(0, run_sint(-num, i + 1));
    ck_assert_int_eq(0, run_sint(num, i + 2));
    ck_assert_int_eq(0, run_sint(-num - 1, i + 2));
  }

  /* biggest 8 byte signed ints (still a small atom) */
  ck_assert_int_eq(0, run_sint(0x7fffffffffffffff, 9));
  ck_assert_int_eq(0, run_sint(0x8000000000000000, 9));
}
END_TEST

START_TEST(t_syn_bin)
{
  /* zero length byte vector */
  ck_assert_int_eq(0, run_bin(0, 1));
  
  /* small atom (15 bytes) */
  ck_assert_int_eq(0, run_bin(15, 16));
  
  /* medium atom (16 bytes) */
  ck_assert_int_eq(0, run_bin(16, 18));
  
  /* medium atom (2047 bytes) */
  ck_assert_int_eq(0, run_bin(2047, 2049));
  
  /* long atom (2048 bytes) */
  ck_assert_int_eq(0, run_bin(2048, 2052));
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
  tcase_add_test(tc_syn, t_syn_sint);
  tcase_add_test(tc_syn, t_syn_bin);
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

/* Utility Functions */

void dump_buf(tp_buffer_t *buf)
{
  uint8_t *raw = (uint8_t*)buf->ptr;
  int i;
  
  printf("Data Bytes:\n");
  for (i = 0; (i < buf->cur_len) && (i < 16); i++)
  {
    printf(" %02x", raw[i]);
  }
  if (buf->cur_len > 16)
  {
    printf("  ..");
  }
  printf("\n");
}

int run_uint(uint64_t value, size_t enc_size)
{
  uint8_t raw[64];
  tp_buffer_t buf;
  uint64_t dec_val;
  
  /* set up buffer */
  memset(raw, 0, sizeof(raw));
  memset(&buf, 0, sizeof(buf));
  buf.ptr = raw;
  buf.max_len = sizeof(raw);
  
  printf("\nTesting unsigned int: %" PRIu64 " (%zu bytes)\n", value, enc_size);
  printf("Raw hex: 0x%016" PRIx64 "\n", value);
  
  /* encode data */
  printf("Encoding data .. ");
  CHECKME(tp_syn_enc_uint(&buf, value));
  
  /* check encoding */
  dump_buf(&buf);
  printf("Encoding: %zu bytes .. ", buf.cur_len);
  CHECKME(buf.cur_len != enc_size);
  
  /* decode data */
  printf("Decoding data .. ");
  CHECKME(tp_syn_dec_uint(&dec_val, &buf));
  
  /* check decoding */
  printf("Decoded value: %" PRIu64 " .. ", dec_val);
  CHECKME(dec_val != value);
  
  return 0;
}

int run_sint(int64_t value, size_t enc_size)
{
  uint8_t raw[64];
  tp_buffer_t buf;
  int64_t dec_val;
  
  /* set up buffer */
  memset(raw, 0, sizeof(raw));
  memset(&buf, 0, sizeof(buf));
  buf.ptr = raw;
  buf.max_len = sizeof(raw);
  
  printf("\nTesting signed int: %" PRId64 " (%zu bytes)\n", value, enc_size);
  
  /* encode data */
  printf("Encoding data .. ");
  CHECKME(tp_syn_enc_sint(&buf, value));
  
  /* check encoding */
  dump_buf(&buf);
  printf("Encoding: %zu bytes .. ", buf.cur_len);
  CHECKME(buf.cur_len != enc_size);
  
  /* decode data */
  printf("Decoding data .. ");
  CHECKME(tp_syn_dec_sint(&dec_val, &buf));
  
  /* check decoding */
  printf("Decoded value: %" PRId64 " .. ", dec_val);
  CHECKME(dec_val != value);
  
  return 0;
}

int run_bin(size_t bin_size, size_t enc_size)
{
  uint8_t raw[2052], raw2[2048];
  tp_buffer_t buf, buf2;
  unsigned int i;
  
  /* set up buffer */
  memset(raw, 0, sizeof(raw));
  memset(raw2, 0, sizeof(raw2));
  memset(&buf, 0, sizeof(buf));
  buf.ptr = raw;
  buf.max_len = sizeof(raw);
  
  /* something to see in the output */
  for (i = 0; i < 16; i++)
  {
    raw2[i] = i + 1;
  }
  
  printf("\nTesting binary blob (%zu bytes)\n", bin_size);
  
  /* encode data */
  printf("Encoding data .. ");
  CHECKME(tp_syn_enc_bin(&buf, raw2, bin_size));
  
  /* check encoding */
  dump_buf(&buf);
  printf("Encoding: %zu bytes .. ", buf.cur_len);
  CHECKME(buf.cur_len != enc_size);
  
  /* decode data */
  printf("Decoding data .. ");
  CHECKME(tp_syn_dec_bin(&buf2, &buf));
  
  /* check decoding */
  printf("Decoded size: %zu .. ", buf2.cur_len);
  CHECKME(bin_size != buf2.cur_len);
  
  /* verify data */
  printf("Verifying decoding: .. ");
  CHECKME(memcmp(raw2, buf2.ptr, bin_size));
  
  return 0;
}
