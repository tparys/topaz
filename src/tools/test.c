#include <stdio.h>
#include <string.h>
#include <topaz/transport_ata.h>
#include <topaz/errno.h>

int main()
{
  printf("Topaz errno %s\n", tp_errno_lookup(-1)); //TP_ERR_LIBATA));
  return 0;
  
  struct TP_ATA_DRIVE *handle = tp_ata_open("/dev/sdc");
  //int i;
  char buf[512];
  memset(buf, 1, 512);
  
  printf("Topaz errno is %x (%s)\n", tp_errno, tp_errno_lookup_cur());
  
  if (handle == NULL)
  {
    return 1;
  }
  
  if (tp_ata_get_identify(handle, buf))
  {
    printf("ATA fail\n");
  }
  
  tp_ata_close(handle);
  handle = NULL;
  printf("Topaz errno is %x\n", tp_errno);
  
  return 0;
  
}
