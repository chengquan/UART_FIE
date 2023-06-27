#include "FPGA_DDR.h"
#include "basic.h"

unsigned int cnn_map_base;
unsigned int mem_map_base;


///// function for software//////////
void Map_Feature_Soft(short* in, struct Mapped_Feature* feature)
{
	for (int i = 0; i < feature->height; i++)
		for (int j = 0; j < feature->width; j++)
			for (int k = 0; k < feature->channel; k = k + Tout)
			{
				unsigned int dst_base = (k / Tout) * feature->surface_stride / 2 + i * feature->line_stride / 2 + j * Tout;
				unsigned int src_base = i * feature->width * feature->channel + j * feature->channel;
				for (int kk = k; kk < k + Tout; kk++)
				{
					if (kk < feature->channel)
						feature->payload[dst_base + (kk - k)] = in[src_base + kk];
					else
						feature->payload[dst_base + (kk - k)] = 0;
				}
			}
}

void Map_Weight_Soft(short* kernel, struct Mapped_Weight* weight)
{
	int addr;
	addr = 0;

	for (int k = 0; k < weight->out_ch; k += Tout)
		for (int l = 0; l < weight->in_ch; l += Tin)
			for (int i = 0; i < weight->Ky; i++)
				for (int j = 0; j < weight->Kx; j++)
					for (int kk = k; kk < k + Tout; kk++)
					{
						if (kk < weight->out_ch)
						{
							for (int ll = l; ll < l + Tin; ll += Tout)
							{
								short tp[Tout];
								unsigned int src_base = i * weight->Kx * weight->in_ch * weight->out_ch + j * weight->in_ch * weight->out_ch + kk;
								for (int lll = ll; lll < ll + Tout; lll++)
								{
									if (lll < weight->in_ch)
										tp[lll - ll] = kernel[src_base + lll * weight->out_ch];
									else
										tp[lll - ll] = 0;
								}
								for (int cp = 0; cp < Tout; cp++)
									weight->payload[addr + cp] = tp[cp];
								addr = addr + Tout;
							}
						}
					}
}

struct Mapped_Feature* Malloc_Feature_Soft(int height, int width, int ch, int scale, int conv_out_scale, int dat_bit)
{
	struct Mapped_Feature* ret = (struct Mapped_Feature*)malloc(sizeof(struct Mapped_Feature));//Ϊ�ṹ������ڴ�ռ䣬����ֵΪָ������ڴ���׵�ַ
	if (ret == 0)
	{
		printf("failed to malloc structure\n");
		return 0;
	}

	ret->scale = scale;
	ret->conv_out_scale = conv_out_scale;
	ret->height = height;
	ret->width = width;
	ret->channel = ch;
	ret->dat_bit = dat_bit;

	ret->line_stride = width * 2 * Tout;
	ret->surface_stride = ret->line_stride * height;


	unsigned int require_bytes = ret->surface_stride * ((ch + Tout - 1) / Tout);
	ret->payload_size = require_bytes;

	ret->payload = (short*)malloc(require_bytes);
	if (ret->payload == 0)
	{
		free(ret);
		printf("failed to malloc payload:%d\n", require_bytes);
		return 0;
	}

	return ret;
}

struct Mapped_Weight* Malloc_Weight_Soft(int Ky, int Kx, int in_ch, int out_ch, int scale, int wt_bit)
{
	struct Mapped_Weight* ret = (struct Mapped_Weight*)malloc(sizeof(struct Mapped_Weight));

	if (ret == 0)
		return 0;

	ret->scale = scale;
	ret->Ky = Ky;
	ret->Kx = Kx;
	ret->in_ch = in_ch;
	ret->out_ch = out_ch;
	ret->wt_bit = wt_bit;

	unsigned int require_bytes = (Tin * 2 * Kx * Ky * out_ch * ((in_ch + Tin - 1) / Tin));
	ret->payload_size = require_bytes;

	ret->payload = (short*)malloc(require_bytes);
	if (ret->payload == 0)
	{
		free(ret);
		return 0;
	}

	return ret;
}

short* Get_Element_Soft(struct Mapped_Feature* feature, int row, int col, int ch)//ֱ�Ӵ��ļ��н�bin������ȡ�����Ա��ӡ
{
	return (short*)(((unsigned int)feature->payload) + (ch / Tout) * feature->surface_stride + row * feature->line_stride + col * Tout * 2 + (ch % Tout) * 2);
}

short* Get_Weight_Soft(struct Mapped_Weight* weight, int n_h, int n_w, int n_cin, int n_cout)
{
	int in_ch_slice = weight->in_ch;
	unsigned int ch_group_now = ((n_cout / Tout) == ((weight->out_ch - 1) / Tout)) ? (((weight->out_ch - 1) % Tout) + 1) : Tout;
	unsigned int m = (n_cin / in_ch_slice);
	unsigned int n_cin_now = n_cin % in_ch_slice;
	unsigned int ch_in_now = (m == ((weight->in_ch - 1) / in_ch_slice)) ? (((weight->in_ch - 1) % in_ch_slice) + 1) : in_ch_slice;

	unsigned int addr_bias1 = m * in_ch_slice * weight->out_ch * weight->Kx * weight->Ky * 2;
	unsigned int addr_bias2 = ((n_cout / Tout) * Tout * ((ch_in_now + Tin - 1) / Tin) * weight->Kx * weight->Ky + (n_cin_now / Tin) * ch_group_now * weight->Kx * weight->Ky + (n_h * weight->Kx + n_w) * ch_group_now + (n_cout % Tout)) * Tin * 2 + (n_cin_now % Tin) * 2;
	return (short*)(((unsigned int)weight->payload) + addr_bias1 + addr_bias2);
}


///// function for hardware//////////
#ifdef Run_on_FPGA


void FPGA_Init()
{
	cnn_map_base = 0x10041000;
	if (cnn_map_base == 0)
		printf("Error: vpu_base mmap fail\n");

	mem_map_base = 0x40000000;
	if (mem_map_base == 0)
		printf("Error: mem_base mmap fail\n");
	
	//printf("**** cnn_map_base=%d\n", cnn_map_base);
	//printf("**** mem_map_base=%d\n", mem_map_base);
	
	printf("FPGA Init Done\n");
}
void CSB_Write(unsigned int addr, unsigned int data)
{
	*(uint32_t*)(((unsigned int)cnn_map_base) + (addr << 2)) = data;
}

unsigned int CSB_Read(unsigned int addr)
{
	unsigned int rt = *(uint32_t*)(((unsigned int)cnn_map_base) + (addr << 2));
	return rt;
}

void Map_Feature(short* in, struct Mapped_Feature* feature)
{
	for (int i = 0; i < feature->height; i++)
		for (int j = 0; j < feature->width; j++)
			for (int k = 0; k < feature->channel; k = k + Tout)
			{
				unsigned int dst_base = (k / Tout) * feature->surface_stride / (MAX_DAT_DW / 8) + i * feature->line_stride / (MAX_DAT_DW / 8) + j * Tout;
				unsigned int src_base = i * feature->width * feature->channel + j * feature->channel;
				for (int kk = k; kk < k + Tout; kk++)
				{
					if (kk < feature->channel)
						*(short*)(((unsigned int)mem_map_base) + ((unsigned int)(feature->payload + dst_base + (kk - k)))) = in[src_base + kk];
					else
						*(short*)(((unsigned int)mem_map_base) + ((unsigned int)(feature->payload + dst_base + (kk - k)))) = 0;
				}
			}
}

struct Mapped_Feature* Malloc_Feature(int height, int width, int ch, int scale, int conv_out_scale, int dat_bit)
{
	struct Mapped_Feature* ret = (struct Mapped_Feature*)malloc(sizeof(struct Mapped_Feature));
	if (ret == 0)
	{
		return 0;
	}

	ret->scale = scale;
	ret->height = height;
	ret->width = width;
	ret->channel = ch;
	ret->dat_bit = dat_bit;
	ret->conv_out_scale = conv_out_scale;

	ret->line_stride = width * (MAX_DAT_DW / 8) * Tout;
	ret->surface_stride = ret->line_stride * height;

	int require_bytes = ret->surface_stride * ((ch + Tout - 1) / Tout);
	ret->payload_size = require_bytes;


	ret->payload = (short*)FPGA_DDR_malloc(require_bytes);
	printf("feature payload addr is %x\n", ret->payload);
	printf("feature require_bytes is %d\n", require_bytes);
	if (ret->payload == 0)
	{
		free(ret);
		return 0;
	}
	
	//printf("**** feature->payload=%d\n", ret->payload);
	return ret;
}
/*
void DeMap_Feature(struct Mapped_Feature *feature,short *out)
{
	for(int i=0;i<feature->height;i++)
		for(int j=0;j<feature->width;j++)
			for(int k=0;k<feature->channel;k=k+Tout)
			{
				unsigned int dst_base=i*feature->width*feature->channel+j*feature->channel;
				unsigned int src_base=(k/Tout)*feature->surface_stride/2+i*feature->line_stride/2+j*Tout;
				for(int kk=k;kk<k+Tout;kk++)
				{
					if(kk<feature->channel)
						out[dst_base+kk]=*(short *)(((unsigned int)mem_map_base)+((unsigned int)(feature->payload+src_base+(kk-k))));
				}
			}
}

def demap_feature(feature_in, feature_out, Height, Width, Channel, Tout):
    for h in range(Height):
        for w in range(Width):
            for ch in range(0, Channel, Tout):
                dst_addr = h * Width * Channel + w * Channel
                src_addr = (ch // Tout) * Height * Width * Tout + h * Width * Tout + w * Tout
                for s in range(ch, ch + Tout, 1):
                    if s < Channel:
                        feature_out[dst_addr + s] = feature_in[src_addr + s - ch]
    return feature_out
*/
void DeMap_Feature(struct Mapped_Feature *feature,uint8_t *out)
{
	for(int i=0;i<feature->height;i++)
		for(int j=0;j<feature->width;j++)
			for(int k=0;k<feature->channel;k=k+Tout)
			{
				unsigned int dst_base=i*(feature->width)*(feature->channel)+j*(feature->channel);
				unsigned int src_base=((unsigned int)(k/Tout))*(feature->height)*(feature->width)*Tout + i*feature->width*Tout + j*Tout;
				for(int kk=k;kk<k+Tout;kk++)
				{
					if(kk<feature->channel)
						out[dst_base+kk]=*(uint8_t*)(((unsigned int)mem_map_base)+(((unsigned int)(feature->payload))+src_base+(kk-k)));
				}
			}
}

void Rearray_Feature(struct Mapped_Feature *feature,uint8_t *input)
{
   for(int k=0;k< feature->channel;k++)
    {
            for(int i=0;i<feature->height;i++)//28*28*6
            {
                for(int j=0;j<feature->width;j++)//width
                {
                    printf("%02x ",input[i*feature->width*feature->channel + j*feature->channel + k]);
                }
                printf("\n");
            }
            printf("#############\n");
    }

}


struct Mapped_Weight* Malloc_Weight(int Ky, int Kx, int in_ch, int out_ch, int scale, int wt_bit)
{
	struct Mapped_Weight* ret = (struct Mapped_Weight*)malloc(sizeof(struct Mapped_Weight));
	if (ret == 0)
		return 0;

	ret->scale = scale;
	ret->Ky = Ky;
	ret->Kx = Kx;
	ret->in_ch = in_ch;
	ret->out_ch = out_ch;
	ret->wt_bit = wt_bit;

	int Tin_L = Tin * MAX_DAT_DW / wt_bit;
	int require_bytes = ((Tin_L * wt_bit) / 8) * Kx * Ky * out_ch * ((in_ch + Tin_L - 1) / Tin_L);
	ret->payload_size = require_bytes;

	ret->payload = (short*)FPGA_DDR_malloc(require_bytes);
	printf("weight payload addr is %x\n",(uint32_t)ret->payload);
	printf("weight require_bytes is %d\n",(uint32_t)require_bytes);
	if (ret->payload == 0)
	{
		free(ret);
		return 0;
	}	
	//printf("**** weight->payload=%d\n", ret->payload);
	return ret;
}

signed char* Get_Element(struct Mapped_Feature* feature, int row, int col, int ch)
{
	return (signed char*)(((unsigned int)mem_map_base) + ((unsigned int)feature->payload) + (ch / Tout) * feature->surface_stride + row * feature->line_stride + col * Tout * (MAX_DAT_DW / 8) + (ch % Tout) * (MAX_DAT_DW / 8));
}



#endif