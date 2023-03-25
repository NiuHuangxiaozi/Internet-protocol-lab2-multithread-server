#include "server_show.h"

void analyze(union Client_Buffet *cb, union Server_Buffet *sb)
{
  int operation_higher_bit = cb->content.operation_number[0];
  int operation_lower_bit = cb->content.operation_number[1];
  switch (operation_higher_bit)
  {
  case 0:
    switch (operation_lower_bit)
    {
    case LOGIN_EXAMINATION:
      break;
    }
    break;
  }
}