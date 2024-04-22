#ifndef _AMA_ENCODER_H_
#define _AMA_ENCODER_H_

#include <obs-module.h>
#include <xma.h>
#include <ama-context.h>

int32_t encoder_reserve(AmaCtx *ctx);

int32_t encoder_create(AmaCtx *ctx);

int32_t encoder_process_frame(struct encoder_packet *packet,
			      bool *received_packet, AmaCtx *ctx);

int32_t encoder_destroy(AmaCtx *ctx);

#endif // _AMA_ENCODER_H_
