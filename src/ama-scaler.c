#include "ama-scaler.h"

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      ScalerCtx scl_ctx)
{
	(void)encoder;
	(void)settings;
	(void)scl_ctx;
	return 0;
}

int32_t scaler_process_frame()
{
	return 0;
}

int32_t scaler_destroy()
{
	return 0;
}
