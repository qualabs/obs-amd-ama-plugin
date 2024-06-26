#include <ama-context.h>

ScalerResolution getResolution(int scalerConstant)
{
	ScalerResolution resolution;

	switch (scalerConstant) {
	case 0: // SCALER_RES_1920_1080
		resolution.height = 1080;
		resolution.width = 1920;
		break;
	case 1: // SCALER_RES_1664_936
		resolution.height = 936;
		resolution.width = 1664;
		break;
	case 2: // SCALER_RES_1536_864
		resolution.height = 864;
		resolution.width = 1536;
		break;
	case 3: // SCALER_RES_1280_720
		resolution.height = 720;
		resolution.width = 1280;
		break;
	case 4: // SCALER_RES_1152_648
		resolution.height = 648;
		resolution.width = 1152;
		break;
	case 5: // SCALER_RES_1096_616
		resolution.height = 616;
		resolution.width = 1096;
		break;
	case 6: // SCALER_RES_960_540
		resolution.height = 540;
		resolution.width = 960;
		break;
	case 7: // SCALER_RES_852_480
		resolution.height = 480;
		resolution.width = 852;
		break;
	case 8: // SCALER_RES_768_432
		resolution.height = 432;
		resolution.width = 768;
		break;
	case 9: // SCALER_RES_698_392
		resolution.height = 392;
		resolution.width = 698;
		break;
	case 10: // SCALER_RES_640_360
		resolution.height = 360;
		resolution.width = 640;
		break;
	default:
		resolution.height = 0;
		resolution.width = 0;
		break;
	}

	return resolution;
}

AmaCtx *ama_create_context(obs_data_t *settings, obs_encoder_t *enc_handle,
			   int32_t codec)
{
	AmaCtx *ctx = bzalloc(sizeof(AmaCtx));
	ctx->codec = codec;
	ctx->enc_handle = enc_handle;
	ctx->settings = settings;
	EncoderProperties *enc_props = &ctx->enc_props;
	ScalerProps *scale_props = &ctx->abr_params;
	video_t *video = obs_encoder_video(ctx->enc_handle);
	const struct video_output_info *voi = video_output_get_info(video);
	obs_data_t *custom_settings = ctx->settings;
	ScalerResolution scaler_res;
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
	enc_props->crf = control_rate == ENC_CRF_ENABLE_ALIAS
				 ? (int)obs_data_get_int(custom_settings, "crf")
				 : ENC_CRF_DEFAULT;
	enc_props->force_idr = ENC_IDR_ENABLE;
	enc_props->fps = voi->fps_num / voi->fps_den;
	enc_props->gop_size =
		(int)obs_data_get_int(custom_settings, "keyint_sec") > 0
			? enc_props->fps *
				  (int)obs_data_get_int(custom_settings,
							"keyint_sec")
			: enc_props->fps * 2;
	enc_props->min_qp = ENC_DEFAULT_MIN_QP;
	enc_props->max_qp = ctx->codec == ENCODER_ID_AV1 ? ENC_SUPPORTED_MAX_QP
							 : ENC_DEFAULT_MAX_QP;
	enc_props->num_bframes =
		obs_data_get_bool(custom_settings, "lookahead")
			? (int)obs_data_get_int(custom_settings, "b_frames")
			: ENC_MIN_NUM_B_FRAMES;
	enc_props->spat_aq_gain = ENC_AQ_GAIN_NOT_USED;
	enc_props->temp_aq_gain = ENC_AQ_GAIN_NOT_USED;
	enc_props->latency_ms = ENC_DEFAULT_LATENCY_MS;
	enc_props->bufsize = ENC_DEFAULT_BUFSIZE;
	enc_props->spatial_aq = ENC_DEFAULT_SPATIAL_AQ;
	enc_props->temporal_aq = ENC_DEFAULT_TEMPORAL_AQ;
	enc_props->slice = DEFAULT_SLICE_ID;
	enc_props->qp = (control_rate == ENC_RC_MODE_CONSTANT_QP)
				? (int)obs_data_get_int(custom_settings, "qp")
				: ENC_DEFAULT_MIN_QP;
	enc_props->rc_mode = control_rate != ENC_CRF_ENABLE_ALIAS
				     ? control_rate
				     : ENC_RC_MODE_DEFAULT;
	enc_props->qp_mode = ENC_DEFAULT_QP_MODE;
	enc_props->preset = XMA_ENC_PRESET_DEFAULT;
	enc_props->cores = XMA_ENC_CORES_DEFAULT;
	enc_props->profile =
		ctx->codec != ENCODER_ID_AV1
			? (int)obs_data_get_int(custom_settings, "profile")
			: ENC_PROFILE_DEFAULT;
	enc_props->level = ENC_DEFAULT_LEVEL;
	enc_props->tier = -1;
	enc_props->lookahead_depth = -1;
	enc_props->tune_metrics = 1;
	enc_props->dynamic_gop = -1;
	enc_props->pix_fmt = XMA_YUV420P_FMT_TYPE;
	bool is_scaling = obs_data_get_bool(custom_settings, "enable_scaling");
	if (is_scaling) {
		scaler_res = getResolution((int)obs_data_get_int(
			custom_settings, "scaler_resolution"));
		ctx->scaler_input_fprops.width = voi->width;
		ctx->scaler_input_fprops.height = voi->height;
		ctx->scaler_input_fprops.format = XMA_VPE_FMT_TYPE;
		ctx->scaler_input_fprops.sw_format = XMA_YUV420P_FMT_TYPE;
		int32_t planes = xma_frame_planes_get(
			ctx->handle, &ctx->scaler_input_fprops);
		int32_t plane;
		for (plane = 0; plane < planes; plane++) {
			ctx->scaler_input_fprops.linesize[plane] =
				xma_frame_get_plane_stride(
					ctx->handle, &ctx->scaler_input_fprops,
					plane);
		}
		enc_props->width = scaler_res.width;
		enc_props->height = scaler_res.height;
		scale_props->width = voi->width;
		scale_props->height = voi->height;
		scale_props->pix_fmt = XMA_YUV420P_FMT_TYPE;
	}
	return ctx;
}

int get_valid_line_size(AmaCtx *ctx, int plane)
{
	XmaFrameProperties *fprops;
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	if (!is_scaling) {
		fprops = &ctx->enc_frame_props;
	} else {
		fprops = &ctx->scaler_input_fprops;
	}
	switch (fprops->sw_format) {
	case XMA_YUV420P_FMT_TYPE:
		if (plane > 0) {
			return fprops->width / 2;
		} else {
			return fprops->width;
		}
	case XMA_YUV420P10LE_FMT_TYPE:
		if (plane == 0) {
			return fprops->width * 2;
		} else {
			return fprops->width;
		}
	case XMA_NV12_FMT_TYPE:
	case XMA_RGB24_P_FMT_TYPE:
		return fprops->width;
	case XMA_P010LE_FMT_TYPE:
		return fprops->width * 2;
	case XMA_PACKED10_FMT_TYPE:
		return ((fprops->width + 2) / 3) * 4;
	default:
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Pixel format not supported!\n");
		return XMA_ERROR;
	}
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
