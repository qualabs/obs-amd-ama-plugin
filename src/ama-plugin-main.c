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
#include <util/dstr.h>
#include <xma.h>
#include <xrm.h>
#include <xrm_enc_interface.h>
#include "ama-encoder.h"
#include "ama-scaler.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

#define TEXT_RATE_CONTROL obs_module_text("Rate Control")
#define TEXT_KEYINT_SEC obs_module_text("Key Frame Interval (0 = auto)")
#define TEXT_BITRATE obs_module_text("Bitrate")
#define TEXT_MAX_BITRATE obs_module_text("Max Bitrate")
#define TEXT_QP obs_module_text("QP")
#define TEXT_PROFILE obs_module_text("Profile")
#define TEXT_LEVEL obs_module_text("Level")
#define TEXT_NONE obs_module_text("None")

struct encoder_type_data {
	const char *disp_name;
};

static inline void add_strings(obs_property_t *list, const char *const *strings)
{
	while (*strings) {
		obs_property_list_add_string(list, *strings, *strings);
		strings++;
	}
}

const char *ama_get_name_h264(void *type_data)
{
	(void)type_data;
	return "AMD AMA H264";
}

const char *ama_get_name_hevc(void *type_data)
{
	(void)type_data;
	return "AMD AMA HEVC";
}

const char *ama_get_name_av1(void *type_data)
{
	(void)type_data;
	return "AMD AMA AV1";
}

void *ama_create_h264(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_create_h264\n");
	EncoderCtx *enc_ctx = bzalloc(sizeof(EncoderCtx));
	enc_ctx->codec = ENCODER_ID_H264;
	encoder_create(settings, encoder, enc_ctx);

	return enc_ctx;
}

void *ama_create_hevc(obs_data_t *settings, obs_encoder_t *encoder)
{
	(void)settings;
	(void)encoder;
	obs_log(LOG_INFO,
		"ama_create_hevc, create function still to be implemented\n");
	EncoderCtx *enc_ctx = bzalloc(sizeof(EncoderCtx));
	enc_ctx->codec = ENCODER_ID_HEVC;
	encoder_create(settings, encoder, enc_ctx);

	return enc_ctx;
}

void *ama_create_av1(obs_data_t *settings, obs_encoder_t *encoder)
{
	(void)settings;
	(void)encoder;
	obs_log(LOG_INFO,
		"ama_create_av1, create function still to be implemented\n");
	EncoderCtx *enc_ctx = bzalloc(sizeof(EncoderCtx));

	return enc_ctx;
}

void ama_destroy(void *data)
{
	obs_log(LOG_INFO, "ama_destroy\n");
	encoder_destroy(data);
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

static bool rate_control_modified(obs_properties_t *ppts, obs_property_t *p,
				  obs_data_t *settings)
{
	int rc = (int)obs_data_get_int(settings, "rate_control");
	bool cvbr_or_vbr = rc == ENC_RC_MODE_VBR || rc == ENC_RC_MODE_CVBR;
	bool cvbr_or_vbr_or_cbr = rc == ENC_RC_MODE_VBR ||
				  rc == ENC_RC_MODE_CVBR ||
				  rc == ENC_RC_MODE_CBR;
	bool cqp_or_crf = rc == ENC_RC_MODE_CONSTANT_QP ||
			  rc == ENC_CRF_ENABLE_ALIAS;

	p = obs_properties_get(ppts, "bitrate");
	obs_property_set_visible(p, cvbr_or_vbr_or_cbr);
	p = obs_properties_get(ppts, "max_bitrate");
	obs_property_set_visible(p, cvbr_or_vbr);
	p = obs_properties_get(ppts, "qp");
	obs_property_set_visible(p, cqp_or_crf);

	return true;
}

static obs_properties_t *obs_ama_props_h264(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;
	obs_property_t *list;
	obs_property_t *headers;

	list = obs_properties_add_list(props, "rate_control", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
	obs_property_list_add_int(list, "CVBR", ENC_RC_MODE_CVBR);
	obs_property_list_add_int(list, "VBR", ENC_RC_MODE_VBR);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CRF", ENC_CRF_ENABLE_ALIAS);

	obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_int(props, "bitrate", TEXT_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "max_bitrate", TEXT_MAX_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "qp", TEXT_QP, ENC_SUPPORTED_MIN_QP,
				   ENC_SUPPORTED_MAX_QP, 1);

	list = obs_properties_add_list(props, "profile", TEXT_PROFILE,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "main", ENC_H264_MAIN);
	obs_property_list_add_int(list, "baseline", ENC_H264_BASELINE);
	obs_property_list_add_int(list, "high", ENC_H264_HIGH);
	obs_property_list_add_int(list, "high10", ENC_H264_HIGH_10);
	obs_property_list_add_int(list, "high10 intra", ENC_H264_HIGH_10_INTRA);

	list = obs_properties_add_list(props, "level", TEXT_LEVEL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "1", ENC_LEVEL_10);
	obs_property_list_add_int(list, "1.1", ENC_LEVEL_11);
	obs_property_list_add_int(list, "1.2", ENC_LEVEL_12);
	obs_property_list_add_int(list, "1.3", ENC_LEVEL_13);
	obs_property_list_add_int(list, "2", ENC_LEVEL_20);
	obs_property_list_add_int(list, "2.1", ENC_LEVEL_21);
	obs_property_list_add_int(list, "2.2", ENC_LEVEL_22);
	obs_property_list_add_int(list, "3", ENC_LEVEL_30);
	obs_property_list_add_int(list, "3.1", ENC_LEVEL_31);
	obs_property_list_add_int(list, "3.2", ENC_LEVEL_32);
	obs_property_list_add_int(list, "4", ENC_LEVEL_40);
	obs_property_list_add_int(list, "4.1", ENC_LEVEL_41);
	obs_property_list_add_int(list, "4.2", ENC_LEVEL_42);
	obs_property_list_add_int(list, "5", ENC_LEVEL_50);
	obs_property_list_add_int(list, "5.1", ENC_LEVEL_51);
	obs_property_list_add_int(list, "5.2", ENC_LEVEL_52);

	p = obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20,
				   1);
	obs_property_int_set_suffix(p, " s");

	headers = obs_properties_add_bool(props, "repeat_headers",
					  "repeat_headers");
	obs_property_set_visible(headers, false);

	return props;
}

static obs_properties_t *obs_ama_props_hevc(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;
	obs_property_t *list;
	obs_property_t *headers;

	list = obs_properties_add_list(props, "rate_control", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
	obs_property_list_add_int(list, "CVBR", ENC_RC_MODE_CVBR);
	obs_property_list_add_int(list, "VBR", ENC_RC_MODE_VBR);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CRF", ENC_CRF_ENABLE_ALIAS);

	obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_int(props, "bitrate", TEXT_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "max_bitrate", TEXT_MAX_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "qp", TEXT_QP, ENC_SUPPORTED_MIN_QP,
				   ENC_SUPPORTED_MAX_QP, 1);

	list = obs_properties_add_list(props, "profile", TEXT_PROFILE,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "main", ENC_HEVC_MAIN);
	obs_property_list_add_int(list, "main intra", ENC_HEVC_MAIN_INTRA);
	obs_property_list_add_int(list, "main10", ENC_HEVC_MAIN_10);
	obs_property_list_add_int(list, "main10 intra", ENC_HEVC_MAIN10_INTRA);

	list = obs_properties_add_list(props, "level", TEXT_LEVEL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "1", ENC_LEVEL_10);
	obs_property_list_add_int(list, "1.1", ENC_LEVEL_11);
	obs_property_list_add_int(list, "2", ENC_LEVEL_20);
	obs_property_list_add_int(list, "2.1", ENC_LEVEL_21);
	obs_property_list_add_int(list, "3", ENC_LEVEL_30);
	obs_property_list_add_int(list, "3.1", ENC_LEVEL_31);
	obs_property_list_add_int(list, "4", ENC_LEVEL_40);
	obs_property_list_add_int(list, "4.1", ENC_LEVEL_41);
	obs_property_list_add_int(list, "5", ENC_LEVEL_50);
	obs_property_list_add_int(list, "5.1", ENC_LEVEL_51);

	p = obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20,
				   1);
	obs_property_int_set_suffix(p, " s");

	headers = obs_properties_add_bool(props, "repeat_headers",
					  "repeat_headers");
	obs_property_set_visible(headers, false);

	return props;
}

static obs_properties_t *obs_ama_props_av1(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;
	obs_property_t *list;
	obs_property_t *headers;

	list = obs_properties_add_list(props, "rate_control", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
	obs_property_list_add_int(list, "CVBR", ENC_RC_MODE_CVBR);
	obs_property_list_add_int(list, "VBR", ENC_RC_MODE_VBR);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CRF", ENC_CRF_ENABLE_ALIAS);

	obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_int(props, "bitrate", TEXT_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "max_bitrate", TEXT_MAX_BITRATE,
				   ENC_SUPPORTED_MIN_BITRATE,
				   ENC_SUPPORTED_MAX_BITRATE, 50);
	obs_property_int_set_suffix(p, " Kbps");

	p = obs_properties_add_int(props, "qp", TEXT_QP, ENC_SUPPORTED_MIN_QP,
				   ENC_SUPPORTED_MAX_QP, 1);

	p = obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20,
				   1);
	obs_property_int_set_suffix(p, " s");

	headers = obs_properties_add_bool(props, "repeat_headers",
					  "repeat_headers");
	obs_property_set_visible(headers, false);

	return props;
}

static void obs_ama_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "keyint_sec", 0);
	obs_data_set_default_int(settings, "bitrate", ENC_DEFAULT_BITRATE);
	obs_data_set_default_int(settings, "max_bitrate",
				 ENC_DEFAULT_MAX_BITRATE);
	obs_data_set_default_int(settings, "qp", ENC_DEFAULT_QP_MODE);
	obs_data_set_default_int(settings, "profile", ENC_PROFILE_DEFAULT);
	obs_data_set_default_int(settings, "level", ENC_DEFAULT_LEVEL);
	obs_data_set_default_int(settings, "rate_control", ENC_RC_MODE_CBR);
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

static struct obs_encoder_info ama_encoder_h264 = {
	.id = "ama_encoder_h264",
	.type = OBS_ENCODER_VIDEO,
	.codec = "h264",
	.get_name = ama_get_name_h264,
	.create = ama_create_h264,
	.destroy = ama_destroy,
	.encode = ama_encode,
	.update = ama_update,
	.get_properties = obs_ama_props_h264,
	.get_defaults = obs_ama_defaults,
	.get_extra_data = ama_get_extra_data,
	.get_sei_data = ama_get_sei_data,
	.get_video_info = ama_get_video_info};

static struct obs_encoder_info ama_encoder_hevc = {
	.id = "ama_encoder_hevc",
	.type = OBS_ENCODER_VIDEO,
	.codec = "hevc",
	.get_name = ama_get_name_hevc,
	.create = ama_create_hevc,
	.destroy = ama_destroy,
	.encode = ama_encode,
	.update = ama_update,
	.get_properties = obs_ama_props_hevc,
	.get_defaults = obs_ama_defaults,
	.get_extra_data = ama_get_extra_data,
	.get_sei_data = ama_get_sei_data,
	.get_video_info = ama_get_video_info};

static struct obs_encoder_info ama_encoder_av1 = {
	.id = "ama_encoder_av1",
	.type = OBS_ENCODER_VIDEO,
	.codec = "av1",
	.get_name = ama_get_name_av1,
	.create = ama_create_av1,
	.destroy = ama_destroy,
	.encode = ama_encode,
	.update = ama_update,
	.get_properties = obs_ama_props_av1,
	.get_defaults = obs_ama_defaults,
	.get_extra_data = ama_get_extra_data,
	.get_sei_data = ama_get_sei_data,
	.get_video_info = ama_get_video_info};

bool obs_module_load(void)
{
	obs_register_encoder(&ama_encoder_h264);
	obs_register_encoder(&ama_encoder_hevc);
	obs_register_encoder(&ama_encoder_av1);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
