#include <ama-context.h>

AmaCtx *ama_create_context(obs_data_t *settings, obs_encoder_t *enc_handle,
			   int32_t codec)
{
	AmaCtx *ctx = bzalloc(sizeof(AmaCtx));
	ctx->codec = codec;
	ctx->enc_handle = enc_handle;
	ctx->settings = settings;
	EncoderProperties *enc_props = &ctx->enc_props;
	video_t *video = obs_encoder_video(ctx->enc_handle);
	const struct video_output_info *voi = video_output_get_info(video);
	obs_data_t *custom_settings = ctx->settings;
	int control_rate =
		(int)obs_data_get_int(custom_settings, "control_rate");
	/* Initialize the encoder parameters */
	enc_props->device_id = DEFAULT_DEVICE_ID;
	enc_props->codec_id = ctx->codec;
	enc_props->device_type = ENC_DEFAULT_DEVICE_TYPE;
	enc_props->width = voi->width;
	enc_props->height = voi->height;
	enc_props->bitrate =
		(int)obs_data_get_int(custom_settings, "bitrate") != 0 &&
				control_rate != ENC_RC_MODE_CONSTANT_QP &&
				control_rate != ENC_CRF_ENABLE_ALIAS
			? (int)obs_data_get_int(custom_settings, "bitrate")
			: 0;
	enc_props->max_bitrate =
		(int)obs_data_get_int(custom_settings, "max_bitrate") != 0 &&
				control_rate == ENC_RC_MODE_CABR
			? (int)obs_data_get_int(custom_settings, "max_bitrate")
			: ENC_DEFAULT_MAX_BITRATE;
	enc_props->crf = control_rate == ENC_CRF_ENABLE_ALIAS ? ENC_CRF_ENABLE
							      : ENC_CRF_DISABLE;
	enc_props->force_idr = ENC_IDR_ENABLE;
	enc_props->fps = voi->fps_num / voi->fps_den;
	enc_props->gop_size =
		(int)obs_data_get_int(custom_settings, "keyint_sec") > 0
			? enc_props->fps *
				  (int)obs_data_get_int(custom_settings,
							"keyint_sec")
			: enc_props->fps * 2;
	enc_props->min_qp = ENC_DEFAULT_MIN_QP;
	enc_props->max_qp = ctx->codec == ENCODER_ID_AV1
				    ? ENC_SUPPORTED_MAX_AV1_QP
				    : ENC_DEFAULT_MAX_QP;
	enc_props->num_bframes =
		obs_data_get_bool(custom_settings, "lookahead")
			? (int)obs_data_get_int(custom_settings, "b_frames")
			: ENC_MIN_NUM_B_FRAMES;
	enc_props->spat_aq_gain = ENC_AQ_GAIN_NOT_USED;
	enc_props->temp_aq_gain = ENC_AQ_GAIN_NOT_USED;
	enc_props->spatial_aq = ENC_DEFAULT_SPATIAL_AQ;
	enc_props->temporal_aq = ENC_DEFAULT_TEMPORAL_AQ;
	enc_props->slice = DEFAULT_SLICE_ID;
	enc_props->qp = (control_rate == ENC_CRF_ENABLE_ALIAS ||
			 control_rate == ENC_RC_MODE_CONSTANT_QP)
				? (int)obs_data_get_int(custom_settings, "qp")
				: ENC_DEFAULT_MIN_QP;
	enc_props->rc_mode = control_rate != ENC_CRF_ENABLE_ALIAS
				     ? control_rate
				     : ENC_RC_MODE_DEFAULT;
	enc_props->qp_mode = ENC_DEFAULT_QP_MODE;
	enc_props->preset = XMA_ENC_PRESET_DEFAULT;
	enc_props->cores = XMA_ENC_CORES_DEFAULT;
	enc_props->profile = (int)obs_data_get_int(custom_settings, "profile");
	enc_props->level = ENC_DEFAULT_LEVEL;
	enc_props->tier = -1;
	enc_props->lookahead_depth =
		obs_data_get_bool(custom_settings, "lookahead")
			? ENC_DEFAULT_LOOKAHEAD_DEPTH
			: 0;
	enc_props->tune_metrics = 1;
	enc_props->dynamic_gop = -1;
	enc_props->pix_fmt = XMA_YUV420P_FMT_TYPE;
	return ctx;
}

int32_t ama_initialize_sdk(AmaCtx *ctx)
{
	int32_t ret = XMA_SUCCESS;
	ret = xma_log_init(XMA_WARNING_LOG, XMA_LOG_TYPE_CONSOLE,
			   &ctx->filter_log);
	if (ret != XMA_SUCCESS) {
		ctx->filter_log = ctx->log;
	}

	XmaInitParameter xma_init_param;
	memset(&xma_init_param, 0, sizeof(XmaInitParameter));
	strcpy(ctx->app_name, "ma35_encoder_app");
	xma_init_param.app_name = ctx->app_name;
	xma_init_param.device = ctx->enc_props.device_id;

	XmaParameter params[1];
	uint32_t api_version = XMA_API_VERSION_1_1;

	params[0].name = (char *)XMA_API_VERSION;
	params[0].type = XMA_UINT32;
	params[0].length = sizeof(uint32_t);
	params[0].value = &api_version;

	xma_init_param.params = params;
	xma_init_param.param_cnt = 1;

	if ((ret = xma_initialize(ctx->filter_log, &xma_init_param,
				  &ctx->handle)) != XMA_SUCCESS) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "XMA Initialization failed\n");
	}
	return ret;
}

int32_t context_destroy(AmaCtx *ctx)
{
	if (ctx->handle) {
		xma_release(ctx->handle);
		ctx->handle = NULL;
	}

	if (ctx->log != ctx->filter_log) {
		xma_log_release(ctx->filter_log);
		ctx->filter_log = NULL;
	}
	xma_log_release(ctx->log);
	ctx->log = NULL;
	bfree(ctx);
	return 0;
}
