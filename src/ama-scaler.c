#include "ama-scaler.h"

#define XLNX_SCALER_APP_MODULE "scaler_app"

static int32_t scal_create_xma_props(AmaCtx *ctx, int session_id,
				     XmaScalerProperties *scaler_xma_props)
{
	// setup frame poperties
	scaler_xma_props->hwscaler_type = XMA_ABR_SCALER_TYPE;
	scaler_xma_props->param_cnt = 0;
	scaler_xma_props->params = (XmaParameter *)calloc(
		1, MAX_SCALER_PARAMS * sizeof(XmaParameter));
	scaler_xma_props->num_outputs = 1;
	scaler_xma_props->input.format = XMA_VPE_FMT_TYPE;
	scaler_xma_props->input.sw_format = ctx->abr_params.pix_fmt;
	scaler_xma_props->input.bits_per_pixel = ctx->abr_params.bits_per_pixel;
	scaler_xma_props->input.width = ctx->abr_params.width;
	scaler_xma_props->input.height = ctx->abr_params.height;
	scaler_xma_props->input.stride = ctx->abr_params.width;
	scaler_xma_props->input.framerate.numerator = ctx->enc_props.fps;
	scaler_xma_props->input.framerate.denominator = 1;
	if (session_id > 0) {
		scaler_xma_props->input.framerate.numerator /= 2;
		scaler_xma_props->param_cnt = 1;
		scaler_xma_props->params[0].name =
			(char *)XMA_SCALER_PARAM_MIX_RATE;
		scaler_xma_props->params[0].length = sizeof(void *);
	}

	// assign output params
	XmaScalerInOutProperties *out_props = &scaler_xma_props->output[0];
	out_props->sw_format = ctx->enc_frame_props.sw_format;
	out_props->format = XMA_VPE_FMT_TYPE;
	out_props->bits_per_pixel = ctx->enc_frame_props.bits_per_pixel;
	out_props->width = ctx->enc_frame_props.width;
	out_props->height = ctx->enc_frame_props.height;
	out_props->stride = ctx->enc_frame_props.width;
	out_props->framerate.numerator = ctx->enc_props.fps;
	out_props->framerate.denominator = 1;
	return XMA_SUCCESS;
}

static int scal_create_frame_props(AmaCtx *ctx)
{
	XmaScalerProperties props = ctx->abr_xma_props;
	XmaFrameProperties *fprops = &ctx->scaler_input_fprops;
	// Create an input frame for abr scaler
	fprops->width = props.input.width;
	fprops->height = props.input.height;
	fprops->format = props.input.format;
	fprops->sw_format = props.input.sw_format;
	fprops->bits_per_pixel = props.input.bits_per_pixel;
	for (int plane_id = 0;
	     plane_id < xma_frame_planes_get(ctx->handle, fprops); plane_id++) {
		fprops->linesize[plane_id] = xma_frame_get_plane_stride(
			ctx->handle, fprops, plane_id);
	}

	// Create an array of output frames for abr scaler
	fprops = &ctx->enc_frame_props;
	fprops->width = ctx->enc_frame_props.width;
	fprops->height = ctx->enc_frame_props.height;
	fprops->format = ctx->enc_frame_props.format;
	fprops->sw_format = ctx->enc_frame_props.sw_format;
	fprops->bits_per_pixel = ctx->enc_frame_props.bits_per_pixel;
	for (int plane_id = 0;
	     plane_id < xma_frame_planes_get(ctx->handle, fprops); plane_id++) {
		fprops->linesize[plane_id] = xma_frame_get_plane_stride(
			ctx->handle, fprops, plane_id);
	}
	return XMA_SUCCESS;
}

int32_t scaler_reserve(AmaCtx *ctx)
{
	// Reserve xrm resource
	int32_t ret = XRM_SUCCESS;
	XrmInterfaceProperties input_props;
	memset(&input_props, 0, sizeof(input_props));
	const ScalerProps *scale_props = &ctx->abr_params;
	input_props.width = scale_props->width;
	input_props.height = scale_props->height;
	input_props.fps_num = ctx->enc_props.fps;
	input_props.fps_den = 1;

	XrmInterfaceProperties *out_props =
		calloc(1, sizeof(XrmInterfaceProperties));
	out_props->width = ctx->enc_props.width;
	out_props->height = ctx->enc_props.height;
	out_props->fps_num = ctx->enc_props.fps;
	out_props->fps_den = 1;
	if (xrm_scale_reserve(&ctx->scaler_xrm_ctx, ctx->enc_props.device_id,
			      &input_props, out_props, 1) == XRM_ERROR) {
		return XRM_ERROR;
	}
	free(out_props);
	return ret;
}

int32_t scaler_create(AmaCtx *ctx)
{
	int32_t ret = XMA_SUCCESS;
	ret = scal_create_xma_props(ctx, 0, &ctx->abr_xma_props);
	ctx->abr_xma_props.handle = ctx->handle;
	ctx->scl_session = xma_scaler_session_create(&ctx->abr_xma_props);
	if (!ctx->scl_session) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, XLNX_SCALER_APP_MODULE,
			   "Failed to create scaler session\n");
		return XMA_ERROR;
	}
	return ret;
}

int32_t scaler_process_frame(struct encoder_frame *frame, AmaCtx *ctx)
{
	(void)frame;
	(void)ctx;
	return 0;
}

void scal_cleanup_props(XmaScalerProperties *scaler_xma_props)
{
	if (scaler_xma_props->params) {
		free(scaler_xma_props->params);
	}
}

int32_t scaler_destroy(AmaCtx *ctx)
{
	if (!ctx) {
		return XMA_ERROR;
	}
	if (ctx->scl_session) {
		xma_scaler_session_destroy(ctx->scl_session);
	}
	if (ctx->handle) {
		xma_release(ctx->handle);
	}
	if (ctx->log) {
		xma_log_release(ctx->log);
	}
	xrm_scale_release(&ctx->scaler_xrm_ctx);
	scal_cleanup_props(&ctx->abr_xma_props);
	if (ctx->scaler_input_xframe) {
		xma_frame_free(ctx->scaler_input_xframe);
	}
	return XMA_SUCCESS;
}
