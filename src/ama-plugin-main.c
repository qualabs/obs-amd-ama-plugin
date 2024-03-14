/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <obs-avc.h>
#include <plugin-support.h>
#include <xma.h>
#include <xrm.h>
#include <xrm_enc_interface.h>
#include "ama-encoder.h"
#include "ama-scaler.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

const char *ama_get_name(void *type_data)
{
	(void)type_data;
	return "AMA H264";
}

void *ama_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_create\n");
	EncoderCtx *enc_ctx = bzalloc(sizeof(EncoderCtx));
	encoder_create(settings, encoder, enc_ctx);

	return enc_ctx;
}

void ama_destroy(void *data)
{
	obs_log(LOG_INFO, "ama_destroy\n");
	encoder_destroy(data);
	// int32_t scaler_destroy(scl_ctx);
}

bool ama_encode(void *enc_ctx, struct encoder_frame *frame,
		struct encoder_packet *packet, bool *received_packet)
{
	bool res = encoder_process_frame(frame, packet, received_packet,
					 enc_ctx) == XMA_SUCCESS;
	return res;
}

bool ama_update(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "ama update \n");
	(void)data;
	(void)settings;
	return false;
}

bool ama_get_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
	obs_log(LOG_INFO, "ama_get_extra_data \n");
	EncoderCtx *enc_ctx = (EncoderCtx *)data;
	*size = enc_ctx->header_size;
	*extra_data = enc_ctx->header_data;
	return true;
}

bool ama_get_sei_data(void *data, uint8_t **sei_data, size_t *size)
{
	obs_log(LOG_INFO, "ama_get_sei_data \n");
	EncoderCtx *enc_ctx = (EncoderCtx *)data;
	*size = enc_ctx->sei_size;
	*sei_data = enc_ctx->sei_data;
	return true;
}

void ama_get_video_info(void *data, struct video_scale_info *info)
{
	obs_log(LOG_INFO, "ama_get_video_info \n");
	(void)data;
	info->format = VIDEO_FORMAT_I420;
}

static struct obs_encoder_info ama_encoder = {
	.id = "ama_encoder",
	.type = OBS_ENCODER_VIDEO,
	.codec = "h264",
	.get_name = ama_get_name,
	.create = ama_create,
	.destroy = ama_destroy,
	.encode = ama_encode,
	.update = ama_update,
	.get_extra_data = ama_get_extra_data,
	.get_sei_data = ama_get_sei_data,
	.get_video_info = ama_get_video_info};

bool obs_module_load(void)
{
	obs_register_encoder(&ama_encoder);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
