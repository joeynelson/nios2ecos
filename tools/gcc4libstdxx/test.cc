#include <stdio.h>
#include <cyg/kernel/kapi.h>      // Kernel API.
#include <cyg/infra/diag.h>      // For diagnostic printing.

void test(void)
{
  diag_printf("Testing\n");
try
  {
    diag_printf("Testing throw\n");
    throw 1;
  } catch (int a)
  {
    diag_printf("Caught one exception\n");
  }

}


int main(int argc, char **argv)
{
  printf("Hello world\n");
  test();
  return 0;
}

