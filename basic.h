#pragma once

#ifndef SRC_BASIC_H_
#define SRC_BASIC_H_

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "hbird_sdk_soc.h"
#include "FPGA_DDR.h"

#define FPGA_NULL ((void *)0xFFFFFFFF)
#define CNN_IP_BASE_ADDR 0x10041000
#define Run_on_FPGA

#define Tin 16					//Parallel factor of CH_in
#define Tout 8					//Parallel factor of CH_out
#define MAX_DAT_DW 8
#define MAX_WT_DW 8

#define BRAM_DEPTH 2048
#define BRAM_NUM 16

#define POOL_1D_DAT_IN_DEP 16
#define POOL_2D_DAT_IN_DEP (POOL_1D_DAT_IN_DEP*2)

#define bn_reg_bias 64
#define pool_reg_bias 128



struct Mapped_Feature
{
	short* payload;
	int payload_size;
	int surface_stride;
	int line_stride;
	int scale;
	int conv_out_scale;
	int height;
	int width;
	int channel;
	int dat_bit;
};

struct Mapped_Weight
{
	short* payload;
	int payload_size;
	int scale;
	int Ky;
	int Kx;
	int in_ch;
	int out_ch;
	int wt_bit;
};

/////////////////////////////////

///// function for software//////////
void Map_Feature_Soft(short* in, struct Mapped_Feature* feature);
struct Mapped_Feature* Malloc_Feature_Soft(int height, int width, int ch, int scale, int conv_out_scale, int dat_bit);

struct Mapped_Weight* Malloc_Weight_Soft(int Ky, int Kx, int in_ch, int out_ch, int scale, int wt_bit);
void Map_Weight_Soft(short* kernel, struct Mapped_Weight* weight);

short* Get_Element_Soft(struct Mapped_Feature* feature, int row, int col, int ch);
short* Get_Weight_Soft(struct Mapped_Weight* weight, int n_h, int n_w, int n_cin, int n_cout);

///// function for hardware//////////
#ifdef Run_on_FPGA

void FPGA_Init();
void Map_Feature(short* in, struct Mapped_Feature* feature);
void Map_Weight(short* kernel, struct Mapped_Weight* weight);
struct Mapped_Feature* Malloc_Feature(int height, int width, int ch, int scale, int conv_out_scale, int dat_bit);
struct Mapped_Weight* Malloc_Weight(int Ky, int Kx, int in_ch, int out_ch, int scale, int wt_bit);
void DeMap_Feature(struct Mapped_Feature *feature,uint8_t *out);
void Rearray_Feature(struct Mapped_Feature *feature,uint8_t *input);
void CSB_Write(unsigned int addr,unsigned int data);
unsigned int CSB_Read(unsigned int addr);
signed char* Get_Element(struct Mapped_Feature* feature, int row, int col, int ch);
#endif

#endif /* SRC_BASIC_H_ */
