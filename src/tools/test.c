#include <stdio.h>
#include <string.h>
#include <topaz/debug.h>
#include <topaz/errno.h>
#include <topaz/topaz.h>

int main()
{
  tp_handle_t *handle = NULL;
  char buf[512] = {0};
  
  tp_debug = 3;
  handle = tp_open("/dev/sdc");
  if (!handle)
  {
    printf("Failure reported: %s\n", tp_errno_lookup_cur());
    return 1;
  }
  
  
  tp_close(handle);
  handle = NULL;

  return 0;
  
}
