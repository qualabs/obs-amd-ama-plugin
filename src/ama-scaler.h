#ifndef _AMA_SCALER_H_
#define _AMA_SCALER_H_

#include <obs-module.h>
#include <ama-context.h>

#define MAX_SCALER_WIDTH 3840
#define MAX_SCALER_HEIGHT 2160
#define MIN_SCALER_WIDTH 284
#define MIN_SCALER_HEIGHT 160
#define MAX_SCALER_PARAMS 2

typedef enum { PLANE_Y = 0, PLANE_U, PLANE_V } PlaneId;

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      AmaCtx *ctx);

int32_t scaler_process_frame(struct encoder_frame *frame, AmaCtx *ctx);

int32_t scaler_destroy(AmaCtx *ctx);

#endif // _AMA_scaler_H_
