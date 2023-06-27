#include <stdio.h>
#include "hbird_sdk_soc.h"



void mnist_init();
void mnist_run();
void mnist_predictresult();

void mnist_imageprocess(uint16_t imageNo);
void mnist_statistic_clear();
void mnist_result_check(uint16_t imageNo, uint8_t check_mode);
