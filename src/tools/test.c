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
  tp_buffer_t buf, ret;
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
  
  if (tp_swg_invoke(handle, &buf, TP_SWG_SMUID, TP_SWG_PROPERTIES, NULL))
  {
    printf("I/O fail : %s\n", tp_errno_lookup_cur());
  }
  
  tp_close(handle);
  handle = NULL;
  
  return 0;
  
}
