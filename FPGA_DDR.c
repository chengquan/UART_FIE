#include "FPGA_DDR.h"


unsigned int has_initialized=0;
unsigned int current_location = 0x00001000;
unsigned int FPGA_DDR_malloc(unsigned int numbytes)
{
	unsigned int cur_location = 0;
    cur_location =  current_location;
    current_location += numbytes;
    return cur_location;
}

void FPGA_DDR_free() 
{
    current_location = 0x00001000;
}

//void Debug_mcb()
//{
//	printf("\n===========================\n");
//	for(int i=0;i<mcb.size();i++)
//	{
//		printf("%d:[%8lx,%8lx]\n",mcb[i].available,mcb[i].board_DDR_address,mcb[i].board_DDR_address+mcb[i].blocksize-1);
//	}
//	printf("===========================\n");
//}
