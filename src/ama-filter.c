#include <ama-filter.h>

int get_valid_line_size(AmaCtx *ctx, int plane)
{
	XmaFrameProperties *fprops = &ctx->enc_frame_props;
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

int get_valid_lines(AmaCtx *ctx, int plane)
{
	XmaFrameProperties *fprops = &ctx->enc_frame_props;
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
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Pixel format not supported!\n");
		return XMA_ERROR;
	}
}

int32_t filter_get_xma_props(EncoderProperties *enc_props,
			     XmaFilterProperties *xma_upload_props)
{
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
	return XMA_SUCCESS;
}

int32_t filter_create(AmaCtx *ctx)
{
    ama_initialize_sdk(ctx);
	ctx->xma_upload_props.handle = ctx->handle;
	filter_get_xma_props(&ctx->enc_props, &ctx->xma_upload_props);
	ctx->upload_session = xma_filter_session_create(&ctx->xma_upload_props);
    return XMA_SUCCESS;
}

int32_t filter_upload_frame(struct encoder_frame *frame, AmaCtx *ctx)
{
	ctx->enc_frame_props.format = XMA_YUV420P_FMT_TYPE;
	ctx->encoder_input_xframe =
		xma_frame_alloc(ctx->handle, &ctx->enc_frame_props, false);
	int32_t num_planes =
		xma_frame_planes_get(ctx->handle, &ctx->enc_frame_props);
	for (int i = 0; i < num_planes; i++) {
		ctx->encoder_input_xframe->data[i].buffer_type =
			XMA_HOST_BUFFER_TYPE;
		uint8_t *p = ctx->encoder_input_xframe->data[i].host;
		int32_t linesize = get_valid_line_size(ctx, i);
		if (linesize == XMA_ERROR) {
			return XMA_ERROR;
		}
		int32_t lines = get_valid_lines(ctx, i);
		int total_line_size = ctx->enc_frame_props.linesize[i];
		for (int h = 0; h < lines; h++) {
			memcpy(p, frame->data[i] + h * linesize, linesize);
			p += total_line_size;
		}
	}
	int ret = INT32_MIN;
	da_push_back(ctx->dts_array, &frame->pts);
	ctx->encoder_input_xframe->pts = frame->pts;
	ret = xma_filter_session_send_frame(ctx->upload_session,
					    ctx->encoder_input_xframe);
	if (ret != XMA_SUCCESS) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Upload filter failed to send frame %d\n",
			   ctx->num_frames_sent);
		return ret;
	}
	ctx->encoder_input_xframe =
		xma_frame_alloc(ctx->handle, &ctx->enc_frame_props, true);
	if (!ctx->encoder_input_xframe) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Failed to allocate frame\n");
		return ret;
	}
	do {
		ret = xma_filter_session_recv_frame(ctx->upload_session,
						    ctx->encoder_input_xframe);
		if (ret == XMA_TRY_AGAIN) {
			usleep(100);
		}
	} while (ret == XMA_TRY_AGAIN);
	if (ret != XMA_SUCCESS) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Upload filter failed to recieve frame %d\n",
			   ctx->num_frames_sent);
		return ret;
	}
	return XMA_SUCCESS;
}

int32_t filter_destroy(AmaCtx *ctx)
{
	if (ctx->upload_session != NULL) {
		xma_filter_session_destroy(ctx->upload_session);
		ctx->upload_session = NULL;
	}
    return 0;
}
