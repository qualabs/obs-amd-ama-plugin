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
			  XmaEncoderProperties *xma_enc_props)
{
	xma_enc_props->handle = handle;
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

void enc_free_xma_props(XmaEncoderProperties *xma_enc_props)
{
	if (xma_enc_props->params) {
		free(xma_enc_props->params);
	}
}

int32_t encoder_reserve(AmaCtx *ctx)
{
	da_init(ctx->dts_array);
	XrmInterfaceProperties xrm_props;
	memset(&xrm_props, 0, sizeof(xrm_props));

	xrm_props.width = ctx->enc_props.width;
	xrm_props.height = ctx->enc_props.height;
	xrm_props.fps_num = ctx->enc_props.fps;
	xrm_props.fps_den = 1;
	xrm_props.enc_cores = ctx->enc_props.cores;
	switch (ctx->enc_props.preset) {
	case XMA_ENC_PRESET_SLOW:
		strcpy(xrm_props.preset, "slow");
	case XMA_ENC_PRESET_MEDIUM:
		strcpy(xrm_props.preset, "medium");
	case XMA_ENC_PRESET_FAST:
		strcpy(xrm_props.preset, "fast");
	default:
		strcpy(xrm_props.preset, "medium");
	}
	xrm_props.is_la_enabled = ctx->enc_props.lookahead_depth != 0;
	bool isAV1 = ctx->codec == ENCODER_ID_AV1;
	ctx->xrm_enc_ctx.slice_id = ctx->enc_props.slice;

	return xrm_enc_reserve(&ctx->xrm_enc_ctx, ctx->enc_props.device_id,
			       ctx->enc_props.slice, isAV1, false, &xrm_props);
}

int32_t encoder_create(AmaCtx *ctx)
{
	ctx->xma_enc_props.handle = ctx->handle;
	enc_get_xma_props(ctx->handle, &ctx->enc_props, &ctx->xma_enc_props);

	ctx->enc_frame_props.format = ctx->xma_enc_props.format;
	ctx->enc_frame_props.sw_format = ctx->xma_enc_props.sw_format;
	ctx->enc_frame_props.width = ctx->enc_props.width;
	ctx->enc_frame_props.height = ctx->enc_props.height;
	int32_t planes =
		xma_frame_planes_get(ctx->handle, &ctx->enc_frame_props);
	int32_t plane;
	for (plane = 0; plane < planes; plane++) {
		ctx->enc_frame_props.linesize[plane] =
			xma_frame_get_plane_stride(
				ctx->handle, &ctx->enc_frame_props, plane);
	}

	ctx->enc_session = xma_enc_session_create(&ctx->xma_enc_props);
	if (!ctx->enc_session) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Failed to create encoder session\n");
		return XMA_ERROR;
	}

	return XMA_SUCCESS;
}

int64_t get_dts(AmaCtx *ctx)
{
	int64_t dts = 0;
	int fps_denominator = 1;
	dts = ctx->dts_array.array[0];
	da_erase(ctx->dts_array, 0);
	dts = dts - (ctx->enc_props.num_bframes * fps_denominator);
	return dts;
}

bool is_keyframe(AmaCtx *ctx, XmaDataBuffer *output_xma_buffer,
		 int32_t recv_size)
{
	switch (ctx->codec) {
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

void get_headers(AmaCtx *ctx, struct encoder_packet *packet)
{
	struct encoder_packet new_packet = {0};
	switch (ctx->codec) {
	case ENCODER_ID_H264:
		obs_extract_avc_headers(packet->data, packet->size,
					&new_packet.data, &new_packet.size,
					&ctx->header_data, &ctx->header_size,
					&ctx->sei_data, &ctx->sei_size);
		break;
	case ENCODER_ID_HEVC:
		obs_extract_hevc_headers(packet->data, packet->size,
					 &new_packet.data, &new_packet.size,
					 &ctx->header_data, &ctx->header_size,
					 &ctx->sei_data, &ctx->sei_size);
		break;
	case ENCODER_ID_AV1:
		obs_extract_av1_headers(packet->data, packet->size,
					&new_packet.data, &new_packet.size,
					&ctx->header_data, &ctx->header_size);
		break;
	}
	bfree(new_packet.data);
}

int32_t encoder_send_frame(AmaCtx *ctx)
{
	int ret = INT32_MIN;
	if (ctx->encoder_input_xframe) {

		if (ctx->dyn_params_fp) {
			if (!ctx->dyn_params) {
				ctx->dyn_params = xma_side_data_alloc(
					ctx->handle,
					XMA_FRAME_SIDE_DATA_DYN_ENC_PARAMS,
					XMA_HOST_BUFFER_TYPE,
					sizeof(XmaDynamicEncParams));
			}
		}

		ret = xma_enc_session_send_frame(ctx->enc_session,
						 ctx->encoder_input_xframe);
	} else if (!ctx->flush_sent) {
		ret = xma_enc_session_send_frame(ctx->enc_session, NULL);
		if (ret != XMA_FLUSH_AGAIN) {
			ctx->flush_sent = true;
		}
	}

	if (ret == XMA_ERROR) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Encoder failed to send frame %d\n",
			   ctx->num_frames_sent);
	} else if (ret == XMA_SUCCESS || ret == XMA_SEND_MORE_DATA) {
		if (!ctx->flush_sent) {
			ctx->num_frames_sent++;
		}
	} else if (ret == INT32_MIN) {
		ret = XMA_SUCCESS;
	}

	return ret;
}

int32_t encoder_process_frame(struct encoder_packet *packet,
			      bool *received_packet, AmaCtx *ctx)
{
	*received_packet = false;

	ctx->encoder_input_xframe->frame_rate.numerator =
		ctx->xma_enc_props.framerate.numerator;
	ctx->encoder_input_xframe->frame_rate.denominator =
		ctx->xma_enc_props.framerate.denominator;
	int ret = encoder_send_frame(ctx);
	if (ret == XMA_ERROR || ret == XMA_SEND_MORE_DATA) {
		return ret;
	}
	XmaDataBuffer *output_xma_buffer =
		xma_data_buffer_alloc(ctx->handle, 0, true);

	int32_t count = 0;
	int32_t recv_size = 0;
	do {
		ret = xma_enc_session_recv_data(ctx->enc_session,
						output_xma_buffer, &recv_size);
		if (ret == XMA_RESEND_AND_RECV) {
			usleep(100);
		}
	} while ((ret == XMA_RESEND_AND_RECV) && (count++ < 1000));

	if (ret == XMA_SUCCESS) {
		*received_packet = true;
		da_resize(ctx->packet_data, 0);
		da_push_back_array(ctx->packet_data,
				   output_xma_buffer->data.buffer, recv_size);
		packet->data = ctx->packet_data.array;
		packet->size = ctx->packet_data.num;
		packet->pts = output_xma_buffer->pts;
		packet->dts = get_dts(ctx);
		packet->keyframe =
			is_keyframe(ctx, output_xma_buffer, recv_size);
		packet->type = OBS_ENCODER_VIDEO;
		if (ctx->num_frames_received == 0) {
			get_headers(ctx, packet);
		}
		ctx->num_frames_received++;
	} else if (ret == XMA_ERROR) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Failed to receive frame %d\n",
			   ctx->num_frames_received);
	}
	xma_data_buffer_free(output_xma_buffer);
	return ret;
}

int32_t encoder_destroy(AmaCtx *ctx)
{
	if (ctx->encoder_input_xframe) {
		xma_frame_free(ctx->encoder_input_xframe);
		ctx->encoder_input_xframe = NULL;
	}

	if (ctx->enc_session != NULL) {
		xma_enc_session_destroy(ctx->enc_session);
		ctx->enc_session = NULL;
	}
	xrm_enc_release(&ctx->xrm_enc_ctx);

	enc_free_xma_props(&ctx->xma_enc_props);

	da_free(ctx->dts_array);
	da_free(ctx->packet_data);

	bfree(ctx->header_data);
	if (ctx->codec != ENCODER_ID_AV1) {
		bfree(ctx->sei_data);
	}

	return 0;
}
