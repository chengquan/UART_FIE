#ifndef __FPGA_DDR__
#define __FPGA_DDR__

#include <stdio.h>
#include <stdint.h>
#include "basic.h"


#define FPGA_DDR_BASE_ADDRESS 0x40000000
#define FPGA_DDR_SIZE         0x10000000
#define MIN_BLOCK_SIZE        (Tout*(MAX_DAT_DW/8))


#define FPGA_NULL ((void *)0xFFFFFFFF)

void Debug_mcb();
unsigned int FPGA_DDR_malloc(unsigned int numbytes);
void FPGA_DDR_free();

#endif
