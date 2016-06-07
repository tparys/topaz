#include <stdio.h>
#include <string.h>
#include <topaz/debug.h>
#include <topaz/errno.h>
#include <topaz/topaz.h>
#include <topaz/syntax.h>
#include <topaz/buffer.h>
#include <topaz/uid_swg.h>
#include <topaz/swg_core.h>

int main()
{
  tp_handle_t *handle = NULL;
  char raw[512] = {0};
  tp_buffer_t buf, ret, pin;
  memset(&buf, 0, sizeof(buf));
  buf.ptr = raw;
  buf.max_len = sizeof(raw);
  
  tp_debug = 3;
  handle = tp_open("/dev/sdc");
  if (!handle)
  {
    printf("Failure reported: %s\n", tp_errno_lookup_cur());
    return 1;
  }
  
  if ((tp_swg_session_start(handle, TP_SWG_SP_ADMIN)) ||
      (tp_swg_get_by_num(&ret, handle, TP_SWG_C_PIN_MSID, 3)) ||
      (tp_syn_dec_bin(&pin, &ret)))
  {
    printf("Failure reported: %s\n", tp_errno_lookup_cur());
  }
  else
  {
    printf("Got a PIN - %u bytes\n", (unsigned int)pin.cur_len);
    fwrite(pin.ptr, 1, pin.cur_len, stdout);
    printf("\n");
  }
  
  tp_close(handle);
  handle = NULL;
  
  return 0;
  
}
