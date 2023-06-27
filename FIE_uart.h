#include <stdio.h>
#include "hbird_sdk_soc.h"


#define FIE_FRAME_LEN 14

#define FIE_CMD_EMPTY   0
#define FIE_CMD_RDY     1
#define FIE_CMD_CHECK   2
#define FIE_CMD_FINISH  3
#define FIE_BASE 0x00000000
#define FIE_DATA(offset)        _REG8(0x00000000,  offset)

int uart0_init();
void FIE_cmd_init();
void FIE_cmd_fsm();
void FIE_cmd_exe();
void FIE_cmd_check_and_decode();