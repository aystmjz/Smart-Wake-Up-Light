#ifndef __DMA_H
#define __DMA_H

#include "sys.h"

void dma_init(DMA_Channel_TypeDef *DMA_CHx, uint32_t cpar, uint32_t cmar, uint16_t cndtr);

#endif