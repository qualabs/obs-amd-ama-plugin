/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "ama-filter.h"
#include "ama-context.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

#define do_log_enc(level, encoder, format, ...)    \
	blog(level, "[ama encoder: '%s'] " format, \
	     obs_encoder_get_name(encoder), ##__VA_ARGS__)
#define do_log(level, format, ...) \
	do_log_enc(level, obs_ama->encoder, format, ##__VA_ARGS__)
#define warn_enc(encoder, format, ...) \
	do_log_enc(LOG_WARNING, encoder, format, ##__VA_ARGS__)

#define TEXT_RATE_CONTROL obs_module_text("Rate Control")
#define TEXT_KEYINT_SEC obs_module_text("Key Frame Interval (0 = auto)")
#define TEXT_BITRATE obs_module_text("Bitrate")
#define TEXT_MAX_BITRATE obs_module_text("Max Bitrate")
#define TEXT_QP obs_module_text("QP")
#define TEXT_B_FRAMES obs_module_text("B Frames")
#define TEXT_PROFILE obs_module_text("Profile")
#define TEXT_LEVEL obs_module_text("Level")
#define TEXT_LOOK_AHEAD obs_module_text("Look Ahead")
#define TEXT_SCALE_OUTPUT obs_module_text("Scale Output")
#define TEXT_SCALE_RESOLUTION obs_module_text("Resolution")
#define TEXT_PRESET obs_module_text("Preset")
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

AmaCtx *ama_create(obs_data_t *settings, obs_encoder_t *encoder, int32_t codec)
{
	AmaCtx *ctx = ama_create_context(settings, encoder, codec);
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	obs_encoder_set_preferred_video_format(encoder, VIDEO_FORMAT_I420);
	encoder_reserve(ctx);
	if (is_scaling) {
		scaler_reserve(ctx);
	}
	ama_initialize_sdk(ctx);
	filter_create(ctx);
	if (is_scaling) {
		scaler_create(ctx);
	}
	encoder_create(ctx);
	return ctx;
}

bool check_level_and_set_error(int width, int height, int fps,
			       int current_level, double required_level,
			       obs_encoder_t *encoder)
{
	if (current_level >= required_level) {
		return true;
	}

	char message[256];
	snprintf(
		message, sizeof(message),
		"Wrong encoding level for resolution selected, %dx%d@%d needs at least level %.1f",
		width, height, fps, required_level);
	obs_encoder_set_last_error(encoder, obs_module_text(message));
	return false;
}

bool ama_validate_encoding_level(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_validate_encoding_level\n");
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	int width = voi->width;
	int height = voi->height;
	int fps = voi->fps_num;
	int level = (int)obs_data_get_int(settings, "level");

	if (width <= 696) {
		if (fps <= 15)
			return check_level_and_set_error(width, height, fps,
							 level, ENC_LEVEL_22,
							 encoder);
		if (fps <= 30)
			return check_level_and_set_error(width, height, fps,
							 level, ENC_LEVEL_30,
							 encoder);
		return check_level_and_set_error(width, height, fps, level,
						 ENC_LEVEL_31, encoder);
	}

	if (width <= 1280) {
		if (fps <= 30)
			return check_level_and_set_error(width, height, fps,
							 level, ENC_LEVEL_31,
							 encoder);
		return check_level_and_set_error(width, height, fps, level,
						 ENC_LEVEL_32, encoder);
	}

	if (width <= 1920) {
		if (fps <= 30)
			return check_level_and_set_error(width, height, fps,
							 level, ENC_LEVEL_40,
							 encoder);
		return check_level_and_set_error(width, height, fps, level,
						 ENC_LEVEL_42, encoder);
	}

	if (fps <= 30)
		return check_level_and_set_error(width, height, fps, level,
						 ENC_LEVEL_50, encoder);
	return check_level_and_set_error(width, height, fps, level,
					 ENC_LEVEL_52, encoder);
}

void *ama_create_h264(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_create_h264\n");
	if (!ama_validate_encoding_level(settings, encoder)) {
		return NULL;
	}
	return ama_create(settings, encoder, ENCODER_ID_H264);
}

void *ama_create_hevc(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_create_hevc\n");
	if (!ama_validate_encoding_level(settings, encoder)) {
		return NULL;
	}
	return ama_create(settings, encoder, ENCODER_ID_HEVC);
}

void *ama_create_av1(obs_data_t *settings, obs_encoder_t *encoder)
{
	obs_log(LOG_INFO, "ama_create_av1");
	return ama_create(settings, encoder, ENCODER_ID_AV1);
}

void ama_destroy(void *data)
{
	obs_log(LOG_INFO, "ama_destroy");
	AmaCtx *ctx = data;
	filter_destroy(ctx);
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	if (is_scaling) {
		scaler_destroy(ctx);
	}
	encoder_destroy(ctx);
	context_destroy(ctx);
}

bool ama_encode(void *data, struct encoder_frame *frame,
		struct encoder_packet *packet, bool *received_packet)
{
	bool res = true;
	AmaCtx *ctx = (AmaCtx *)data;
	res &= filter_upload_frame(frame, ctx) != XMA_ERROR;
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	if (is_scaling) {
		res &= scaler_process_frame(ctx) != XMA_ERROR;
	}
	res &= encoder_process_frame(packet, received_packet, ctx) != XMA_ERROR;
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
	int rc = (int)obs_data_get_int(settings, "control_rate");
	bool cabr = rc == ENC_RC_MODE_CABR;
	bool cabr_or_cbr = rc == ENC_RC_MODE_CABR || rc == ENC_RC_MODE_CBR;
	bool cqp_or_crf = rc == ENC_RC_MODE_CONSTANT_QP ||
			  rc == ENC_CRF_ENABLE_ALIAS;

	p = obs_properties_get(ppts, "bitrate");
	obs_property_set_visible(p, cabr_or_cbr);
	p = obs_properties_get(ppts, "max_bitrate");
	obs_property_set_visible(p, cabr);
	p = obs_properties_get(ppts, "qp");
	obs_property_set_visible(p, cqp_or_crf);

	return true;
}

static bool look_ahead_modified(obs_properties_t *ppts, obs_property_t *p,
				obs_data_t *settings)
{
	bool la = obs_data_get_bool(settings, "lookahead");
	p = obs_properties_get(ppts, "b_frames");
	obs_property_set_visible(p, la);
	return true;
}

static bool enable_scaling_modified(obs_properties_t *ppts, obs_property_t *p,
				    obs_data_t *settings)
{
	bool scaling_enabled = obs_data_get_bool(settings, "enable_scaling");
	p = obs_properties_get(ppts, "scaler_resolution");
	obs_property_set_visible(p, scaling_enabled);
	return true;
}

static void add_scaler_resolutions(obs_properties_t *props)
{
	obs_property_t *list;
	list = obs_properties_add_list(props, "scaler_resolution",
				       TEXT_SCALE_RESOLUTION,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "1920x1080", SCALER_RES_1920_1080);
	obs_property_list_add_int(list, "1664x936", SCALER_RES_1664_936);
	obs_property_list_add_int(list, "1536x864", SCALER_RES_1536_864);
	obs_property_list_add_int(list, "1280x720", SCALER_RES_1280_720);
	obs_property_list_add_int(list, "1152x548", SCALER_RES_1152_648);
	obs_property_list_add_int(list, "1096x616", SCALER_RES_1096_616);
	obs_property_list_add_int(list, "960x540", SCALER_RES_960_540);
	obs_property_list_add_int(list, "852x480", SCALER_RES_852_480);
	obs_property_list_add_int(list, "768x432", SCALER_RES_768_432);
	obs_property_list_add_int(list, "698x392", SCALER_RES_698_392);
	obs_property_list_add_int(list, "640x360", SCALER_RES_640_360);
}

static obs_properties_t *obs_ama_props_h264(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;
	obs_property_t *list;
	obs_property_t *headers;

	list = obs_properties_add_list(props, "control_rate", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
	obs_property_list_add_int(list, "CRF", ENC_CRF_ENABLE_ALIAS);

	obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_bool(props, "lookahead", TEXT_LOOK_AHEAD);
	obs_property_set_modified_callback(p, look_ahead_modified);

	p = obs_properties_add_int(props, "b_frames", TEXT_B_FRAMES,
				   ENC_MIN_NUM_B_FRAMES, ENC_MAX_NUM_B_FRAMES,
				   1);

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

	list = obs_properties_add_list(props, "", TEXT_PRESET,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "slow", XMA_ENC_PRESET_SLOW);
	obs_property_list_add_int(list, "medium", XMA_ENC_PRESET_MEDIUM);
	obs_property_list_add_int(list, "fast", XMA_ENC_PRESET_FAST);

	p = obs_properties_add_bool(props, "enable_scaling", TEXT_SCALE_OUTPUT);
	obs_property_set_modified_callback(p, enable_scaling_modified);

	add_scaler_resolutions(props);

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

	list = obs_properties_add_list(props, "control_rate", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
	obs_property_list_add_int(list, "CRF", ENC_CRF_ENABLE_ALIAS);

	obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_bool(props, "lookahead", TEXT_LOOK_AHEAD);
	obs_property_set_modified_callback(p, look_ahead_modified);

	p = obs_properties_add_int(props, "b_frames", TEXT_B_FRAMES,
				   ENC_MIN_NUM_B_FRAMES, ENC_MAX_NUM_B_FRAMES,
				   1);

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

	list = obs_properties_add_list(props, "", TEXT_PRESET,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "slow", XMA_ENC_PRESET_SLOW);
	obs_property_list_add_int(list, "medium", XMA_ENC_PRESET_MEDIUM);
	obs_property_list_add_int(list, "fast", XMA_ENC_PRESET_FAST);

	p = obs_properties_add_bool(props, "enable_scaling", TEXT_SCALE_OUTPUT);
	obs_property_set_modified_callback(p, enable_scaling_modified);

	add_scaler_resolutions(props);

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

	list = obs_properties_add_list(props, "control_rate", TEXT_RATE_CONTROL,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "CQP", ENC_RC_MODE_CONSTANT_QP);
	obs_property_list_add_int(list, "CBR", ENC_RC_MODE_CBR);
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
				   ENC_SUPPORTED_MAX_AV1_QP, 1);

	p = obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20,
				   1);
	obs_property_int_set_suffix(p, " s");

	list = obs_properties_add_list(props, "", TEXT_PRESET,
				       OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "slow", XMA_ENC_PRESET_SLOW);
	obs_property_list_add_int(list, "medium", XMA_ENC_PRESET_MEDIUM);
	obs_property_list_add_int(list, "fast", XMA_ENC_PRESET_DEFAULT);

	p = obs_properties_add_bool(props, "enable_scaling", TEXT_SCALE_OUTPUT);
	obs_property_set_modified_callback(p, enable_scaling_modified);

	add_scaler_resolutions(props);

	headers = obs_properties_add_bool(props, "repeat_headers",
					  "repeat_headers");
	obs_property_set_visible(headers, false);

	return props;
}

static void obs_ama_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "keyint_sec", 0);
	obs_data_set_default_bool(settings, "lookahead", false);
	obs_data_set_default_int(settings, "b_frames", 0);
	obs_data_set_default_int(settings, "bitrate", ENC_DEFAULT_BITRATE);
	obs_data_set_default_int(settings, "max_bitrate",
				 ENC_DEFAULT_MAX_BITRATE);
	obs_data_set_default_int(settings, "control_rate", ENC_RC_MODE_CBR);
	obs_data_set_default_int(settings, "level", ENC_DEFAULT_LEVEL);
	obs_data_set_default_int(settings, "qp", ENC_DEFAULT_QP);
	obs_data_set_default_int(settings, "profile", ENC_PROFILE_DEFAULT);
	obs_data_set_default_bool(settings, "enable_scaling", false);
	obs_data_set_default_int(settings, "scaler_resolution",
				 SCALER_RES_1920_1080);
	obs_data_set_default_int(settings, "preset", XMA_ENC_PRESET_DEFAULT);
}

bool ama_get_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
	obs_log(LOG_INFO, "ama_get_extra_data \n");
	AmaCtx *ctx = data;
	*size = ctx->header_size;
	*extra_data = ctx->header_data;
	return true;
}

bool ama_get_sei_data(void *data, uint8_t **sei_data, size_t *size)
{
	obs_log(LOG_INFO, "ama_get_sei_data \n");
	AmaCtx *ctx = data;
	*size = ctx->sei_size;
	*sei_data = ctx->sei_data;
	return ctx->codec != ENCODER_ID_AV1;
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
