#ifndef _AMA_FILTER_H_
#define _AMA_FILTER_H_

#include <unistd.h>
#include <ama-context.h>

int32_t filter_create(AmaCtx *ctx);

int32_t filter_upload_frame(struct encoder_frame *frame, AmaCtx *ctx);

int32_t filter_destroy(AmaCtx *ctx);

#endif // _AMA_ENCODER_H_
