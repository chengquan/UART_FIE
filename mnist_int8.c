#include "mnist_int8.h"
#include "mnist_mp_Tin16Tout8_cfg.h"
#include "mnist_mp_Tin16Tout8_scale.h"
#include "conv.h"
#include "basic.h"
#include "pool.h"
#include "FPGA_DDR.h"

struct Mapped_Feature* image ;
struct Mapped_Weight* conv0_W ;
struct Mapped_Feature* conv0_relu_out;
struct Mapped_Feature* pool0_out ;
    
struct Mapped_Weight* conv1_W ;
struct Mapped_Feature* conv1_relu_out;
struct Mapped_Feature* pool1_out;
struct Mapped_Weight* fc0_W ;
struct Mapped_Feature* fc0_out ;
    
struct Mapped_Weight* fc1_W ;
struct Mapped_Feature* fc1_out ;
    
struct Mapped_Weight* fc2_W ;
struct Mapped_Feature* final_relu_out;




void mnist_init()
{
    uint32_t qspi_base_addr = 0x400;
    FPGA_Init();
    FPGA_DDR_free();
    printf("MNIST INT4&8 Memory Allocation\n");
    image = Malloc_Feature(conv_Hin[0], conv_Win[0], conv_CHin[0], conv_in_scale[0], conv_in_scale[0], conv_in_bit[0]);
	conv0_W = Malloc_Weight(conv_Ky[0], conv_Kx[0], conv_CHin[0], conv_CHout[0], conv_wt_scale[0],conv_wt_bit[0]);
    conv0_relu_out = Malloc_Feature(conv_Hout[0], conv_Wout[0], conv_CHout[0], conv_out_scale[0], conv_out_scale[0], conv_out_bit[0]);
	pool0_out = Malloc_Feature(pool_Hout[0], pool_Wout[0], pool_CHout[0], pool_out_scale[0], pool_out_scale[0], pool_out_bit[0]);
    conv1_W = Malloc_Weight(conv_Ky[1], conv_Kx[1], conv_CHin[1], conv_CHout[1], conv_wt_scale[1],conv_wt_bit[1]);
    conv1_relu_out = Malloc_Feature(conv_Hout[1], conv_Wout[1], conv_CHout[1], conv_out_scale[1], conv_out_scale[1], conv_out_bit[1]);
	pool1_out = Malloc_Feature(pool_Hout[1], pool_Wout[1], pool_CHout[1], pool_out_scale[1], pool_out_scale[1], pool_out_bit[1]);

    fc0_W = Malloc_Weight(conv_Ky[2], conv_Kx[2], conv_CHin[2], conv_CHout[2], conv_wt_scale[2],conv_wt_bit[2]);
    fc0_out = Malloc_Feature(conv_Hout[2], conv_Wout[2], conv_CHout[2], conv_out_scale[2], conv_out_scale[2], conv_out_bit[2]);
    fc1_W = Malloc_Weight(conv_Ky[3], conv_Kx[3], conv_CHin[3], conv_CHout[3], conv_wt_scale[3],conv_wt_bit[3]);
    fc1_out = Malloc_Feature(conv_Hout[3], conv_Wout[3], conv_CHout[3], conv_out_scale[3], conv_out_scale[3], conv_out_bit[3]);  
    fc2_W = Malloc_Weight(conv_Ky[4], conv_Kx[4], conv_CHin[4], conv_CHout[4], conv_wt_scale[4],conv_wt_bit[4]);
    final_relu_out = Malloc_Feature(conv_Hout[4], conv_Wout[4], conv_CHout[4], conv_out_scale[4], conv_out_scale[4], conv_out_bit[4]);  

    printf("MNIST INT4&8 loading weights data images\n");
 
    printf("loading image data, the size is %d...\n",image->payload_size);
    for(int i = 0 ; i< (image->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(image->payload) + i*4) = QSPI_REG((unsigned int)(qspi_base_addr)+(i*4));//<<1;
    }
    qspi_base_addr += image->payload_size;
    printf("Loading image finish!\n");

    printf("loading conv0 wdata, the size is %d...\n",conv0_W->payload_size);
    for(int i = 0 ; i< (conv0_W->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(conv0_W->payload) + (i*4)) = QSPI_REG(qspi_base_addr+i*4);
    }
    qspi_base_addr += conv0_W->payload_size;
    printf("Loading conv0 finish!\n");

    printf("loading conv1 data, the size is %d...\n",conv1_W->payload_size);
    for(int i = 0 ; i< (conv1_W->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(conv1_W->payload) + (i*4)) = QSPI_REG(qspi_base_addr+(i*4));
    }
    qspi_base_addr += conv1_W->payload_size;
    printf("Loading conv1 finish!\n");

    printf("loading fc0 data, the size is %d...\n",fc0_W->payload_size);
    for(int i = 0 ; i< (fc0_W->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(fc0_W->payload) + (i*4)) = QSPI_REG(qspi_base_addr+(i*4));
    }

    qspi_base_addr += fc0_W->payload_size;
    printf("Loading fc0 finish!\n");

    printf("loading fc1 data, the size is %d...\n",fc1_W->payload_size);
    for(int i = 0 ; i< (fc1_W->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(fc1_W->payload) + (i*4)) = QSPI_REG(qspi_base_addr+(i*4));
    } 
    qspi_base_addr += fc1_W->payload_size;
    printf("Loading fc1 finish!\n");

    printf("loading fc2 data, the size is %d...\n",fc2_W->payload_size);
    for(int i = 0 ; i< (fc2_W->payload_size)/4 ; i++)
    {
        DDR3_REG((unsigned int)(fc2_W->payload) + (i*4)) = QSPI_REG(qspi_base_addr+(i*4));
    }
    qspi_base_addr += fc2_W->payload_size;
    printf("Loading fc2 finish!\n");

    printf("MNIST INT4&8 Ready\n");
}

#define image_baseaddr 0x156c8  //image addr = 0x156b8 + 0x10
#define lable_baseaddr 0x12fa8  //label addr = 0x12fa0 + 0x08;
//uint8_t imgaemap[1024]={0};
void mnist_imageprocess(uint16_t imageNo)
{
    //clean the image datat to avoid problems
    uint32_t current_image_addr = image_baseaddr + (784 * imageNo);
    for(int i =0;i<image->payload_size/4; i++)
    {
        DDR3_REG( (unsigned int)(image->payload)+ 4*i )=0x0;
    }
    uint32_t ddraddr = 0;
    uint32_t qspiaddr = 0;
    for(int h = 0; h<28 ;h++)
    {
       for(int w = 0; w<28; w++) 
       {
            ddraddr = (unsigned int)(image->payload) + (66 + w + h*32)*8 ;
            qspiaddr = current_image_addr+h*28+w;
            //printf("ddraddr is %x\n",ddraddr);
            //printf("qspiaddr is %x\n",qspiaddr);
            DDR3_REG8(ddraddr) = QSPI_REG8(qspiaddr)>>1;
       }
    }

    //DeMap_Feature(image,imgaemap);
    //Rearray_Feature(image,imgaemap);
    /* shown original images from dataset
    for(int h = 0; h<28 ;h++)
    {
       for(int w = 0; w<28; w++) 
       {
            qspiaddr = current_image_addr+h*28+w;
            printf("%4x " ,QSPI_REG8(qspiaddr)>>1);
       }
       printf("\n");
    }
    */
}

void mnist_run()
{
        //CONV0
		FPGA_Conv(conv0_cfg, 1, image, conv0_W, conv0_relu_out);
        //POOLING0
        FPGA_Pool(pool0_cfg, 1, conv0_relu_out, pool0_out);
        //CONV1
        FPGA_Conv(conv1_cfg, 1, pool0_out, conv1_W, conv1_relu_out);
        //POOLING1
        FPGA_Pool(pool1_cfg, 1, conv1_relu_out, pool1_out);
        //FC fc0_out
        FPGA_Conv(fc0_cfg,   1, pool1_out, fc0_W, fc0_out);
        //FC fc1_out
        FPGA_Conv(fc1_cfg,   1, fc0_out, fc1_W, fc1_out);
        //FC fc2_out
        FPGA_Conv(fc2_cfg,   1, fc1_out, fc2_W, final_relu_out);        
}

uint8_t all_data[5000] = {0};
void mnist_print_alldata()
{   
    printf("image_feature\n");
    DeMap_Feature(image,all_data);
    Rearray_Feature(image,all_data);

    printf("conv0_feature\n");
    DeMap_Feature(conv0_relu_out,all_data);
    Rearray_Feature(conv0_relu_out,all_data);  

    printf("pool0_feature\n");
    DeMap_Feature(pool0_out,all_data);
    Rearray_Feature(pool0_out,all_data);  

    printf("conv1_feature\n");
    DeMap_Feature(conv1_relu_out,all_data);
    Rearray_Feature(conv1_relu_out,all_data);

    printf("pool1_feature\n");
    DeMap_Feature(pool1_out,all_data);
    Rearray_Feature(pool1_out,all_data);  

    printf("fc0_feature\n");
    DeMap_Feature(fc0_out,all_data);
    Rearray_Feature(fc0_out,all_data);  

    printf("fc1_feature\n");
    DeMap_Feature(fc1_out,all_data);
    Rearray_Feature(fc1_out,all_data); 

    printf("fc2_feature\n");
    DeMap_Feature(final_relu_out,all_data);
    Rearray_Feature(final_relu_out,all_data); 
    
    printf("feature finished\n");
}


int8_t result =0;

//#define filter_mode  1
void mnist_predictresult()
{
    uint8_t output[10] = {0};
    DeMap_Feature(final_relu_out,output);
    int8_t max = -128;
    for (int i = 0; i < 10; i++)
    {
        if (((int8_t)(output[i])) > max )
        {
            max = output[i];
            result = i;
        }
    }
    #ifndef filter_mode
    printf("The result is %d\n", result);
    #endif
}

unsigned int tested_sample_number;
unsigned int correct_sample_number;
void mnist_statistic_clear()
{
    tested_sample_number = 0;
    correct_sample_number = 0;
}


//check_mode->  0 : no record data    1:record error data  2:record all data
void mnist_result_check(uint16_t imageNo, uint8_t check_mode)
{
    float accuracy = 0.0;
    uint32_t current_label_addr = lable_baseaddr +  imageNo;
    if(result == QSPI_REG8(current_label_addr))
    {
        correct_sample_number+=1;
    }else
    {
        #ifndef filter_mode
        printf("the imageNo is %d, your result is %d, the expected result should be %d\n", imageNo,result, QSPI_REG8(current_label_addr));
        #else
        printf("%d,\n",imageNo);
        #endif

        if(check_mode==1)
        {
            mnist_print_alldata();
            //printf("print all the error data here\n");
        }
    }

    if(check_mode==2)
    {
        printf("the imageNo is %d\n", imageNo);
        mnist_print_alldata();
        //printf("print all the data here\n");
    }
    tested_sample_number += 1;
    //printf("acc is %f\n",accuracy);
    accuracy = ((float)(correct_sample_number)) * 100.0 / ((float)tested_sample_number) ;
    #ifndef filter_mode
    printf("Accuracy %d / %d is %f\n", correct_sample_number, tested_sample_number, accuracy);
    #endif
}
