#include <stdio.h>
#include <unistd.h>
#include <obs-avc.h>
#include <obs-hevc.h>
#include <obs-av1.h>
#include "ama-encoder.h"

static void enc_xma_params_update(EncoderProperties *enc_props,
				  XmaParameter *custom_xma_params,
				  uint32_t *param_cnt)
{

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_SLICE;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->slice);
	custom_xma_params[*param_cnt].value = &(enc_props->slice);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_SPATIAL_AQ;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->spatial_aq);
	custom_xma_params[*param_cnt].value = &(enc_props->spatial_aq);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_TEMPORAL_AQ;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->temporal_aq);
	custom_xma_params[*param_cnt].value = &(enc_props->temporal_aq);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_LATENCY_LOGGING;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length =
		sizeof(enc_props->latency_logging);
	custom_xma_params[*param_cnt].value = &(enc_props->latency_logging);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_TUNE_METRICS;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->tune_metrics);
	custom_xma_params[*param_cnt].value = &(enc_props->tune_metrics);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_QP_MODE;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->qp_mode);
	custom_xma_params[*param_cnt].value = &(enc_props->qp_mode);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_FORCED_IDR;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->force_idr);
	custom_xma_params[*param_cnt].value = &(enc_props->force_idr);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_CRF;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->crf);
	custom_xma_params[*param_cnt].value = &(enc_props->crf);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_MAX_BITRATE;
	custom_xma_params[*param_cnt].type = XMA_INT64;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->max_bitrate);
	custom_xma_params[*param_cnt].value = &(enc_props->max_bitrate);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_BF;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->num_bframes);
	custom_xma_params[*param_cnt].value = &(enc_props->num_bframes);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_DYNAMIC_GOP;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->dynamic_gop);
	custom_xma_params[*param_cnt].value = &(enc_props->dynamic_gop);
	(*param_cnt)++;

	custom_xma_params[*param_cnt].name = XMA_ENC_PARAM_CORES;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length = sizeof(enc_props->cores);
	custom_xma_params[*param_cnt].value = &(enc_props->cores);
	(*param_cnt)++;

	return;
}

int32_t enc_get_xma_props(XmaHandle handle, EncoderProperties *enc_props,
			  XmaFilterProperties *xma_upload_props,
			  XmaEncoderProperties *xma_enc_props)
{

	/* Initialize upload properties */
	xma_upload_props->hwfilter_type = XMA_UPLOAD_FILTER_TYPE;
	xma_upload_props->param_cnt = 0;
	xma_upload_props->params = NULL;
	xma_upload_props->input.format = enc_props->pix_fmt;
	xma_upload_props->input.sw_format = enc_props->pix_fmt;
	xma_upload_props->input.width = enc_props->width;
	xma_upload_props->input.height = enc_props->height;
	xma_upload_props->input.framerate.numerator = enc_props->fps;
	xma_upload_props->input.framerate.denominator = 1;
	xma_upload_props->output.format = XMA_VPE_FMT_TYPE;
	xma_upload_props->output.sw_format = enc_props->pix_fmt;
	xma_upload_props->output.width = enc_props->width;
	xma_upload_props->output.height = enc_props->height;
	xma_upload_props->output.framerate.numerator = enc_props->fps;
	xma_upload_props->output.framerate.denominator = 1;

	xma_enc_props->handle = handle;
	/* Initialize encoder properties */
	xma_enc_props->hwencoder_type = enc_props->codec_id;
	xma_enc_props->param_cnt = 0;
	xma_enc_props->params = (XmaParameter *)calloc(
		1, ENC_MAX_PARAMS * sizeof(XmaParameter));
	xma_enc_props->format = XMA_VPE_FMT_TYPE;
	xma_enc_props->sw_format = enc_props->pix_fmt;
	xma_enc_props->width = enc_props->width;
	xma_enc_props->height = enc_props->height;
	xma_enc_props->bitrate = enc_props->bitrate * 1000;
	xma_enc_props->lookahead_depth = enc_props->lookahead_depth;
	xma_enc_props->framerate.numerator = enc_props->fps;
	xma_enc_props->framerate.denominator = 1;
	xma_enc_props->preset = enc_props->preset;
	xma_enc_props->profile = enc_props->profile;
	xma_enc_props->level = enc_props->level;
	xma_enc_props->gop_size = enc_props->gop_size;
	xma_enc_props->qp = enc_props->qp;
	xma_enc_props->rc_mode = enc_props->rc_mode;
	xma_enc_props->minQP = enc_props->min_qp;
	xma_enc_props->maxQP = enc_props->max_qp;
	xma_enc_props->temp_aq_gain = enc_props->temp_aq_gain;
	xma_enc_props->spat_aq_gain = enc_props->spat_aq_gain;

	xma_enc_props->framerate.numerator = enc_props->fps;
	xma_enc_props->framerate.denominator = 1;

	if (enc_props->max_bitrate > 0) {
		enc_props->max_bitrate = enc_props->max_bitrate * 1000;
	}

	enc_xma_params_update(enc_props, xma_enc_props->params,
			      &xma_enc_props->param_cnt);

	return XMA_SUCCESS;
}

void initialize_encoder_context(EncoderCtx *enc_ctx)
{
	EncoderProperties *enc_props = &enc_ctx->enc_props;
	video_t *video = obs_encoder_video(enc_ctx->enc_handle);
	const struct video_output_info *voi = video_output_get_info(video);
	obs_data_t *custom_settings = enc_ctx->settings;
	int control_rate =
		(int)obs_data_get_int(custom_settings, "control_rate");
	/* Initialize the encoder parameters */
	enc_props->device_id = DEFAULT_DEVICE_ID;
	enc_props->codec_id = enc_ctx->codec;
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
	enc_props->force_idr = enc_ctx->codec == ENC_IDR_ENABLE;
	enc_props->fps = voi->fps_num / voi->fps_den;
	enc_props->gop_size =
		(int)obs_data_get_int(custom_settings, "keyint_sec") > 0
			? enc_props->fps *
				  (int)obs_data_get_int(custom_settings,
							"keyint_sec")
			: enc_props->fps * 2;
	enc_props->min_qp = ENC_DEFAULT_MIN_QP;
	enc_props->max_qp = enc_ctx->codec == ENCODER_ID_AV1
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
	enc_props->preset = (int)obs_data_get_int(custom_settings, "preset");
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
}

void enc_free_xma_props(XmaEncoderProperties *xma_enc_props)
{
	if (xma_enc_props->params) {
		free(xma_enc_props->params);
	}

	return;
}

int get_valid_line_size(EncoderCtx *enc_ctx, int plane)
{
	XmaFrameProperties *fprops = &enc_ctx->enc_frame_props;
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
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "Pixel format not supported!\n");
		return XMA_ERROR;
	}
}

int get_valid_lines(EncoderCtx *enc_ctx, int plane)
{
	XmaFrameProperties *fprops = &enc_ctx->enc_frame_props;
	switch (fprops->sw_format) {
	case XMA_YUV420P_FMT_TYPE:
	case XMA_YUV420P10LE_FMT_TYPE:
	case XMA_NV12_FMT_TYPE:
	case XMA_P010LE_FMT_TYPE:
		if (plane == 0) {
			return fprops->height;
		} else {
			return fprops->height / 2;
		}
	case XMA_RGB24_P_FMT_TYPE:
	case XMA_PACKED10_FMT_TYPE:
		return fprops->height;
	default:
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "Pixel format not supported!\n");
		return XMA_ERROR;
	}
}

int32_t encoder_create(obs_data_t *settings, obs_encoder_t *encoder,
		       EncoderCtx *enc_ctx)
{
	int ret = XMA_SUCCESS;

	da_init(enc_ctx->dts_array);

	enc_ctx->settings = settings;
	enc_ctx->enc_handle = encoder;
	initialize_encoder_context(enc_ctx);

	XrmInterfaceProperties xrm_props;
	memset(&xrm_props, 0, sizeof(xrm_props));

	xrm_props.width = enc_ctx->enc_props.width;
	xrm_props.height = enc_ctx->enc_props.height;
	xrm_props.fps_num = enc_ctx->enc_props.fps;
	xrm_props.fps_den = 1;
	xrm_props.enc_cores = enc_ctx->enc_props.cores;
	switch (enc_ctx->enc_props.preset) {
	case XMA_ENC_PRESET_SLOW:
		strcpy(xrm_props.preset, "slow");
	case XMA_ENC_PRESET_MEDIUM:
		strcpy(xrm_props.preset, "medium");
	case XMA_ENC_PRESET_FAST:
		strcpy(xrm_props.preset, "fast");
	default:
		strcpy(xrm_props.preset, "medium");
	}
	xrm_props.is_la_enabled = enc_ctx->enc_props.lookahead_depth != 0;
	bool isAV1 = enc_ctx->codec == ENCODER_ID_AV1;
	enc_ctx->xrm_enc_ctx.slice_id = enc_ctx->enc_props.slice;

	ret = xrm_enc_reserve(&enc_ctx->xrm_enc_ctx,
			      enc_ctx->enc_props.device_id,
			      enc_ctx->enc_props.slice, isAV1, false,
			      &xrm_props);
	if (ret == XRM_ERROR) {
		return ret;
	}

	ret = xma_log_init(XMA_WARNING_LOG, XMA_LOG_TYPE_CONSOLE,
			   &enc_ctx->filter_log);
	if (ret != XMA_SUCCESS) {
		enc_ctx->filter_log = enc_ctx->log;
	}

	XmaInitParameter xma_init_param;
	memset(&xma_init_param, 0, sizeof(XmaInitParameter));
	strcpy(enc_ctx->app_name, "ma35_encoder_app");
	xma_init_param.app_name = enc_ctx->app_name;
	xma_init_param.device = enc_ctx->enc_props.device_id;

	XmaParameter params[1];
	uint32_t api_version = XMA_API_VERSION_1_1;

	params[0].name = (char *)XMA_API_VERSION;
	params[0].type = XMA_UINT32;
	params[0].length = sizeof(uint32_t);
	params[0].value = &api_version;

	xma_init_param.params = params;
	xma_init_param.param_cnt = 1;

	if ((ret = xma_initialize(enc_ctx->filter_log, &xma_init_param,
				  &enc_ctx->handle)) != XMA_SUCCESS) {
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "XMA Initialization failed\n");
		return ret;
	}

	enc_ctx->xma_upload_props.handle = enc_ctx->handle;
	enc_ctx->xma_enc_props.handle = enc_ctx->handle;

	enc_get_xma_props(enc_ctx->handle, &enc_ctx->enc_props,
			  &enc_ctx->xma_upload_props, &enc_ctx->xma_enc_props);

	enc_ctx->enc_frame_props.format = enc_ctx->xma_enc_props.format;
	enc_ctx->enc_frame_props.sw_format = enc_ctx->xma_enc_props.sw_format;
	enc_ctx->enc_frame_props.width = enc_ctx->enc_props.width;
	enc_ctx->enc_frame_props.height = enc_ctx->enc_props.height;
	int32_t planes = xma_frame_planes_get(enc_ctx->handle,
					      &enc_ctx->enc_frame_props);
	int32_t plane;
	for (plane = 0; plane < planes; plane++) {
		enc_ctx->enc_frame_props.linesize[plane] =
			xma_frame_get_plane_stride(enc_ctx->handle,
						   &enc_ctx->enc_frame_props,
						   plane);
	}

	enc_ctx->upload_session =
		xma_filter_session_create(&enc_ctx->xma_upload_props);
	enc_ctx->enc_session = xma_enc_session_create(&enc_ctx->xma_enc_props);
	if (!enc_ctx->enc_session) {
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "Failed to create encoder session\n");
		return XMA_ERROR;
	}

	return ret;
}

int32_t encoder_send_frame(XmaFrame *input_xframe, EncoderCtx *enc_ctx)
{
	int ret = INT32_MIN;
	if (input_xframe) {
		ret = xma_filter_session_send_frame(enc_ctx->upload_session,
						    input_xframe);
		if (ret != XMA_SUCCESS) {
			xma_logmsg(enc_ctx->log, XMA_ERROR_LOG,
				   enc_ctx->app_name,
				   "Upload filter failed to send frame %d\n",
				   enc_ctx->num_frames_sent);
			return ret;
		}
		input_xframe = xma_frame_alloc(enc_ctx->handle,
					       &enc_ctx->enc_frame_props, true);
		if (!input_xframe) {
			xma_logmsg(enc_ctx->log, XMA_ERROR_LOG,
				   enc_ctx->app_name,
				   "Failed to allocate frame\n");
			return ret;
		}
		do {
			ret = xma_filter_session_recv_frame(
				enc_ctx->upload_session, input_xframe);
			if (ret == XMA_TRY_AGAIN) {
				usleep(100);
			}
		} while (ret == XMA_TRY_AGAIN);
		if (ret != XMA_SUCCESS) {
			xma_logmsg(enc_ctx->log, XMA_ERROR_LOG,
				   enc_ctx->app_name,
				   "Upload filter failed to recieve frame %d\n",
				   enc_ctx->num_frames_sent);
			return ret;
		}

		if (enc_ctx->dyn_params_fp) {
			if (!enc_ctx->dyn_params) {
				enc_ctx->dyn_params = xma_side_data_alloc(
					enc_ctx->handle,
					XMA_FRAME_SIDE_DATA_DYN_ENC_PARAMS,
					XMA_HOST_BUFFER_TYPE,
					sizeof(XmaDynamicEncParams));
			}
		}

		ret = xma_enc_session_send_frame(enc_ctx->enc_session,
						 input_xframe);
	} else if (!enc_ctx->flush_sent) {
		ret = xma_enc_session_send_frame(enc_ctx->enc_session, NULL);
		if (ret != XMA_FLUSH_AGAIN) {
			enc_ctx->flush_sent = true;
		}
	}

	if (ret == XMA_ERROR) {
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "Encoder failed to send frame %d\n",
			   enc_ctx->num_frames_sent);
	} else if (ret == XMA_SUCCESS || ret == XMA_SEND_MORE_DATA) {
		if (!enc_ctx->flush_sent) {
			enc_ctx->num_frames_sent++;
		}
	} else if (ret == INT32_MIN) {
		ret = XMA_SUCCESS;
	}

	return ret;
}

int64_t get_dts(EncoderCtx *enc_ctx)
{
	int64_t dts = 0;
	int fps_denominator = 1;
	dts = enc_ctx->dts_array.array[0];
	da_erase(enc_ctx->dts_array, 0);
	dts = dts - (enc_ctx->enc_props.num_bframes * fps_denominator);
	return dts;
}

bool is_keyframe(EncoderCtx *enc_ctx, XmaDataBuffer *output_xma_buffer,
		 int32_t recv_size)
{
	switch (enc_ctx->codec) {
	case ENCODER_ID_H264:
		return obs_avc_keyframe(output_xma_buffer->data.buffer,
					recv_size);
	case ENCODER_ID_HEVC:
		return obs_hevc_keyframe(output_xma_buffer->data.buffer,
					 recv_size);
	case ENCODER_ID_AV1:
		return obs_av1_keyframe(output_xma_buffer->data.buffer,
					recv_size);
		break;
	default:
		return false;
	}
	return false;
}

void get_headers(EncoderCtx *enc_ctx, struct encoder_packet *packet)
{
	struct encoder_packet new_packet = {0};
	switch (enc_ctx->codec) {
	case ENCODER_ID_H264:
		obs_extract_avc_headers(packet->data, packet->size,
					&new_packet.data, &new_packet.size,
					&enc_ctx->header_data,
					&enc_ctx->header_size,
					&enc_ctx->sei_data, &enc_ctx->sei_size);
		break;
	case ENCODER_ID_HEVC:
		obs_extract_hevc_headers(packet->data, packet->size,
					 &new_packet.data, &new_packet.size,
					 &enc_ctx->header_data,
					 &enc_ctx->header_size,
					 &enc_ctx->sei_data,
					 &enc_ctx->sei_size);
		break;
	case ENCODER_ID_AV1:
		obs_extract_av1_headers(packet->data, packet->size,
					&new_packet.data, &new_packet.size,
					&enc_ctx->header_data,
					&enc_ctx->header_size);
		break;
	}
	bfree(new_packet.data);
}

int32_t encoder_process_frame(struct encoder_frame *frame,
			      struct encoder_packet *packet,
			      bool *received_packet, EncoderCtx *enc_ctx)
{
	*received_packet = false;
	enc_ctx->enc_frame_props.format = XMA_YUV420P_FMT_TYPE;
	enc_ctx->input_xframe = xma_frame_alloc(
		enc_ctx->handle, &enc_ctx->enc_frame_props, false);
	int32_t num_planes = xma_frame_planes_get(enc_ctx->handle,
						  &enc_ctx->enc_frame_props);
	for (int i = 0; i < num_planes; i++) {
		enc_ctx->input_xframe->data[i].buffer_type =
			XMA_HOST_BUFFER_TYPE;
		uint8_t *p = enc_ctx->input_xframe->data[i].host;
		int32_t linesize = get_valid_line_size(enc_ctx, i);
		if (linesize == XMA_ERROR) {
			return XMA_ERROR;
		}
		int32_t lines = get_valid_lines(enc_ctx, i);
		int total_line_size = enc_ctx->enc_frame_props.linesize[i];
		for (int h = 0; h < lines; h++) {
			memcpy(p, frame->data[i] + h * linesize, linesize);
			p += total_line_size;
		}
	}
	da_push_back(enc_ctx->dts_array, &frame->pts);

	enc_ctx->input_xframe->frame_rate.numerator =
		enc_ctx->xma_enc_props.framerate.numerator;
	enc_ctx->input_xframe->frame_rate.denominator =
		enc_ctx->xma_enc_props.framerate.denominator;
	enc_ctx->input_xframe->time_base.numerator =
		enc_ctx->xma_enc_props.framerate.numerator;
	enc_ctx->input_xframe->time_base.denominator =
		enc_ctx->xma_enc_props.framerate.denominator;
	enc_ctx->input_xframe->pts = frame->pts;
	int ret = encoder_send_frame(enc_ctx->input_xframe, enc_ctx);
	if (ret == XMA_ERROR || ret == XMA_SEND_MORE_DATA) {
		return ret;
	}
	XmaDataBuffer *output_xma_buffer =
		xma_data_buffer_alloc(enc_ctx->handle, 0, true);

	int32_t count = 0;
	int32_t recv_size = 0;
	do {
		ret = xma_enc_session_recv_data(enc_ctx->enc_session,
						output_xma_buffer, &recv_size);
		if (ret == XMA_RESEND_AND_RECV) {
			usleep(100);
		}
	} while ((ret == XMA_RESEND_AND_RECV) && (count++ < 1000));

	if (ret == XMA_SUCCESS) {
		*received_packet = true;
		da_resize(enc_ctx->packet_data, 0);
		da_push_back_array(enc_ctx->packet_data,
				   output_xma_buffer->data.buffer, recv_size);
		packet->data = enc_ctx->packet_data.array;
		packet->size = enc_ctx->packet_data.num;
		packet->pts = output_xma_buffer->pts;
		packet->dts = get_dts(enc_ctx);
		packet->keyframe =
			is_keyframe(enc_ctx, output_xma_buffer, recv_size);
		packet->type = OBS_ENCODER_VIDEO;
		if (enc_ctx->num_frames_received == 0) {
			get_headers(enc_ctx, packet);
		}
		enc_ctx->num_frames_received++;
	} else if (ret == XMA_ERROR) {
		xma_logmsg(enc_ctx->log, XMA_ERROR_LOG, enc_ctx->app_name,
			   "Failed to receive frame %d\n",
			   enc_ctx->num_frames_received);
	}
	xma_data_buffer_free(output_xma_buffer);
	return ret;
}

int32_t encoder_destroy(EncoderCtx *enc_ctx)
{
	if (enc_ctx->input_xframe) {
		xma_frame_free(enc_ctx->input_xframe);
		enc_ctx->input_xframe = NULL;
	}
	if (enc_ctx->upload_session != NULL) {
		xma_filter_session_destroy(enc_ctx->upload_session);
		enc_ctx->upload_session = NULL;
	}

	if (enc_ctx->enc_session != NULL) {
		xma_enc_session_destroy(enc_ctx->enc_session);
		enc_ctx->enc_session = NULL;
	}
	xrm_enc_release(&enc_ctx->xrm_enc_ctx);

	enc_free_xma_props(&enc_ctx->xma_enc_props);

	if (enc_ctx->handle) {
		xma_release(enc_ctx->handle);
		enc_ctx->handle = NULL;
	}

	if (enc_ctx->log != enc_ctx->filter_log) {
		xma_log_release(enc_ctx->filter_log);
		enc_ctx->filter_log = NULL;
	}
	xma_log_release(enc_ctx->log);
	enc_ctx->log = NULL;

	da_free(enc_ctx->dts_array);
	da_free(enc_ctx->packet_data);

	bfree(enc_ctx->header_data);
	if (enc_ctx->codec != ENCODER_ID_AV1) {
		bfree(enc_ctx->sei_data);
	}
	bfree(enc_ctx);

	return 0;
}
