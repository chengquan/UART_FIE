

//structure of LeNet5
//change VGG16 into LeNet5 (only the first 5 layers are used)
//structure of VGG16:      0,   1,       2,   3,       4,     5,   6,          7,   8,     9,          10,  11,  12,           13,  14,   15
int conv_Hin[]        = {  32, 14,      5,    1,      1,    56,  56,         28,  28,    28,         14,  14,  14,           7,    1,   1 };
int conv_Win[]        = {  32, 14,      5,    1,      1,    56,  56,         28,  28,    28,         14,  14,  14,           7,    1,   1 };
int conv_CHin[]       = {  1,  6,      16,  120,     84,   256, 256,        256, 512,   512,        512, 512, 512,         512, 4096,4096 };
int conv_CHout[]      = {  6,  16,    120,   84,     10,   256, 256,        512, 512,   512,        512, 512, 512,        4096, 4096,1000 };
int conv_Ky[]         = { 5,   5,       5,   1,       1,     3,   3,          3,   3,     3,          3,   3,   3,           7,    1,   1 };
int conv_Kx[]         = { 5,   5,       5,   1,       1,     3,   3,          3,   3,     3,          3,   3,   3,           7,    1,   1 };
int Sx[]              = { 1,   1,       1,   1,       1,     1,   1,          1,   1,     1,          1,   1,   1,           1,    1,   1 };
int Sy[]              = { 1,   1,       1,   1,       1,     1,   1,          1,   1,     1,          1,   1,   1,           1,    1,   1 };
int pad_left[]        = { 0,   0,       0,   0,       0,     1,   1,          1,   1,     1,          1,   1,   1,           0,    0,   0 };
int pad_right[]       = { 0,   0,       0,   0,       0,     1,   1,          1,   1,     1,          1,   1,   1,           0,    0,   0 };
int pad_up[]          = { 0,   0,       0,   0,       0,     1,   1,          1,   1,     1,          1,   1,   1,           0,    0,   0 };
int pad_down[]        = { 0,   0,       0,   0,       0,     1,   1,          1,   1,     1,          1,   1,   1,           0,    0,   0 };
int conv_Hout[]       = { 28,  10,       1,  1,       1,    56,  56,         28,  28,    28,         14,  14,  14,           1,    1,   1 };
int conv_Wout[]       = { 28,  10,       1,  1,       1,    56,  56,         28,  28,    28,         14,  14,  14,           1,    1,   1 };

int conv_bias_scale[] = { 5,   6,       7,   7,       7,     7,   7,          7,   7,     7,          7,   7,   3,           7,    6,   7 };


//structure of VGG16:     0,   1,       2,   3,       4,     5,   6,          7,   8,    9,          10,  11,  12,           13,   14,  15
int conv_wt_scale[]   = { 8,   5,       8,   8,  	  8,     6,   6,     	  7,   7,    5,           5,   7,   7,            6,   8,   12 };
int conv_in_scale[]   = { 7,  -1,      -3,  -4,      -3,     1,   0,         -1,  -1,   -4,          -4,  -2,  -2,           -4,   0,    6 };
///////////////////////// 1
///////////////////////// 3
int conv_out_scale[]  = { -1, -3,      -4,   -3,      -1,     0,  -1,         -1,  -4,   -4,          -2,  -2,  -4,            0,   6,    4 };

int conv_wt_bit[]     = { 8,   4,       4,   4,       4,     4,   4,          8,   4,    2,           2,   4,   4,           2,    4,   8 };
int conv_in_bit[]     = { 8,   4,       4,   4,       4,     4,   4,          8,   4,    2,           2,   4,   4,           2,    4,   8 };
///////////////////////// 4  *2^(-1)  x.xxx_xxxx      8   *2^(-1)  0000_x.xxx_xxxx_xxxx  *2^(-1)  0000_0x.xx_xxxx_xxxx
///////////////////////// 4  *2^(-3)                  8   *2^(-7)                        *2^(-6)  image>>1
int conv_out_bit[]    = { 4,   4,       4,   4,       4,     4,   4,          4,   2,    2,           4,   4,   2,           4,    8,   8 };

int pool_in_bit[]     = {          4,              4,                       8,                     2,                   2 };
int pool_out_bit[]    = {          4,              4,                       8,                     2,                   2 };
int pool_in_scale[]   = {         -1,             -3,                      -1,                    -4,                  -4 };
int pool_out_scale[]  = {         -1,             -3,                      -1,                    -4,                  -4 };

int pool_Hin[]        = {          28,            10,                     56,                    28,                  14 };
int pool_Win[]        = {          28,            10,                     56,                    28,                  14 };
int pool_CHin[]       = {          6,             16,                     256,                   512,                 512 };
int pool_CHout[]      = {          6,             16,                     256,                   512,                 512 };
int pool_Ky[]         = {          2,              2,                       2,                     2,                   2 };
int pool_Kx[]         = {          2,              2,                       2,                     2,                   2 };
int pool_Sx[]         = {          2,              2,                       2,                     2,                   2 };
int pool_Sy[]         = {          2,              2,                       2,                     2,                   2 };
int pool_pad_left[]   = {          0,              0,                       0,                     0,                   0 };
int pool_pad_right[]  = {          0,              0,                       0,                     0,                   0 };
int pool_pad_up[]     = {          0,              0,                       0,                     0,                   0 };
int pool_pad_down[]   = {          0,              0,                       0,                     0,                   0 };
int pool_Hout[]       = {          14,             5,                      28,                    14,                  7 };
int pool_Wout[]       = {          14,             5,                      28,                    14,                  7 };

