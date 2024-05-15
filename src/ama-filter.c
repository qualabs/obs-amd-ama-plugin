#include <ama-filter.h>
#include <ama-context.h>

int get_valid_lines(AmaCtx *ctx, int plane)
{
	XmaFrameProperties *fprops;
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	if (is_scaling) {
		fprops = &ctx->scaler_input_fprops;
	} else {
		fprops = &ctx->enc_frame_props;
	}
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

int32_t filter_get_xma_props(AmaCtx *ctx)
{
	EncoderProperties *enc_props = &ctx->enc_props;
	XmaFilterProperties *xma_upload_props = &ctx->xma_upload_props;
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	xma_upload_props->hwfilter_type = XMA_UPLOAD_FILTER_TYPE;
	xma_upload_props->param_cnt = 0;
	xma_upload_props->params = NULL;
	xma_upload_props->input.format = enc_props->pix_fmt;
	xma_upload_props->input.sw_format = enc_props->pix_fmt;
	if (is_scaling) {
		ScalerProps *scaler_props = &ctx->abr_params;
		xma_upload_props->input.width = scaler_props->width;
		xma_upload_props->input.height = scaler_props->height;
	} else {
		xma_upload_props->input.width = enc_props->width;
		xma_upload_props->input.height = enc_props->height;
	}
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
	filter_get_xma_props(ctx);
	ctx->upload_session = xma_filter_session_create(&ctx->xma_upload_props);
	return XMA_SUCCESS;
}

int32_t filter_upload_frame(struct encoder_frame *frame, AmaCtx *ctx)
{
	XmaFrame **input_x_frame;
	XmaFrameProperties *frame_properties;
	bool is_scaling = obs_data_get_bool(ctx->settings, "enable_scaling");
	// If scaling enabled use the scaler_input_xframe otherwise use
	// encoder_input_xframe with the respective props
	if (is_scaling) {
		input_x_frame = &ctx->scaler_input_xframe;
		frame_properties = &ctx->scaler_input_fprops;
	} else {
		input_x_frame = &ctx->encoder_input_xframe;
		frame_properties = &ctx->enc_frame_props;
	}
	frame_properties->format = XMA_YUV420P_FMT_TYPE;
	*input_x_frame = xma_frame_alloc(ctx->handle, frame_properties, false);
	int32_t num_planes =
		xma_frame_planes_get(ctx->handle, frame_properties);
	for (int i = 0; i < num_planes; i++) {
		(*input_x_frame)->data[i].buffer_type = XMA_HOST_BUFFER_TYPE;
		uint8_t *p = (*input_x_frame)->data[i].host;
		int32_t linesize = get_valid_line_size(ctx, i);
		if (linesize == XMA_ERROR) {
			return XMA_ERROR;
		}
		int32_t lines = get_valid_lines(ctx, i);
		int total_line_size = frame_properties->linesize[i];
		for (int h = 0; h < lines; h++) {
			memcpy(p, frame->data[i] + h * linesize, linesize);
			p += total_line_size;
		}
	}
	int ret = INT32_MIN;
	da_push_back(ctx->dts_array, &frame->pts);
	(*input_x_frame)->pts = frame->pts;
	ret = xma_filter_session_send_frame(ctx->upload_session,
					    *input_x_frame);
	if (ret != XMA_SUCCESS) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Upload filter failed to send frame %d\n",
			   ctx->num_frames_sent);
		return ret;
	}
	*input_x_frame = xma_frame_alloc(ctx->handle, frame_properties, true);
	if (!*input_x_frame) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, ctx->app_name,
			   "Failed to allocate frame\n");
		return ret;
	}
	do {
		ret = xma_filter_session_recv_frame(ctx->upload_session,
						    *input_x_frame);
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
