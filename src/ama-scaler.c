#include "ama-scaler.h"

#define XLNX_SCALER_APP_MODULE "scaler_app"

static void scal_xma_params_update(ScalerProps *scal_props,
				   XmaParameter *custom_xma_params,
				   uint32_t *param_cnt)
{
	custom_xma_params[*param_cnt].name = XMA_SCALER_PARAM_LATENCY_LOGGING;
	custom_xma_params[*param_cnt].type = XMA_INT32;
	custom_xma_params[*param_cnt].length =
		sizeof(scal_props->latency_logging);
	custom_xma_params[*param_cnt].value = &(scal_props->latency_logging);
	(*param_cnt)++;

	return;
}

static int32_t scal_create_xma_props(ScalerProps *param_ctx, int session_id,
				     XmaScalerProperties *scaler_xma_props)
{
	// setup frame poperties
	scaler_xma_props->hwscaler_type = XMA_ABR_SCALER_TYPE;
	scaler_xma_props->param_cnt = 0;
	scaler_xma_props->params = (XmaParameter *)calloc(
		1, MAX_SCALER_PARAMS * sizeof(XmaParameter));
	scaler_xma_props->num_outputs = param_ctx->num_outputs[session_id];
	scaler_xma_props->input.format = XMA_VPE_FMT_TYPE;
	scaler_xma_props->input.sw_format = param_ctx->input_pix_fmt;
	scaler_xma_props->input.bits_per_pixel =
		param_ctx->input_bits_per_pixel;
	scaler_xma_props->input.width = param_ctx->input_width;
	scaler_xma_props->input.height = param_ctx->input_height;
	scaler_xma_props->input.stride = param_ctx->input_width;
	scaler_xma_props->input.framerate.numerator = param_ctx->input_fps_num;
	scaler_xma_props->input.framerate.denominator =
		param_ctx->input_fps_den;
	if (session_id > 0) {
		scaler_xma_props->input.framerate.numerator /= 2;
		scaler_xma_props->param_cnt = 1;
		scaler_xma_props->params[0].name =
			(char *)XMA_SCALER_PARAM_MIX_RATE;
		scaler_xma_props->params[0].length = sizeof(void *);
	}

	// assign output params
	int prop_index = 0;
	for (uint output_id = 0; output_id < param_ctx->num_outputs[0];
	     output_id++) {
		if (param_ctx->is_halfrate[output_id] && session_id > 0) {
			/* We only want to add full rate outputs to the second session */
			continue;
		}
		XmaScalerInOutProperties *out_props =
			&scaler_xma_props->output[prop_index];
		out_props->sw_format = param_ctx->output_pix_fmts[output_id];
		out_props->format = XMA_VPE_FMT_TYPE;
		out_props->bits_per_pixel =
			param_ctx->output_bits_per_pixels[output_id];
		out_props->width = param_ctx->output_widths[output_id];
		out_props->height = param_ctx->output_heights[output_id];
		out_props->stride = param_ctx->output_widths[output_id];
		out_props->framerate.numerator =
			scaler_xma_props->input.framerate.numerator;
		out_props->framerate.denominator = 1;
		prop_index++;
	}

	scal_xma_params_update(param_ctx, scaler_xma_props->params,
			       &scaler_xma_props->param_cnt);
	return XMA_SUCCESS;
}

static int scal_create_frame_props(ScalerCtx *ctx)
{
	XmaScalerProperties props = ctx->abr_xma_props[0];
	XmaFrameProperties *fprops = &ctx->input_fprops;
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
	for (uint output_id = 0; output_id < ctx->abr_params.num_outputs[0];
	     output_id++) {
		fprops = &ctx->output_fprops[output_id];
		fprops->width = props.output[output_id].width;
		fprops->height = props.output[output_id].height;
		fprops->format = props.output[output_id].format;
		fprops->sw_format = props.output[output_id].sw_format;
		fprops->bits_per_pixel = props.output[output_id].bits_per_pixel;
		for (int plane_id = 0;
		     plane_id < xma_frame_planes_get(ctx->handle, fprops);
		     plane_id++) {
			fprops->linesize[plane_id] = xma_frame_get_plane_stride(
				ctx->handle, fprops, plane_id);
		}
	}
	return XMA_SUCCESS;
}

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      ScalerCtx *scl_ctx)
{
	(void)settings;
	(void)encoder;
	int32_t ret = XMA_ERROR;
	// Reserve xrm resource
	XrmInterfaceProperties input_props;
	memset(&input_props, 0, sizeof(input_props));
	const ScalerProps *scale_props = &scl_ctx->abr_params;
	input_props.width = scale_props->input_width;
	input_props.height = scale_props->input_height;
	input_props.fps_num = scale_props->input_fps_num;
	input_props.fps_den = scale_props->input_fps_den;

	int num_outputs = scale_props->num_outputs[0];
	XrmInterfaceProperties *out_props =
		calloc(1, num_outputs * sizeof(XrmInterfaceProperties));
	for (int i = 0; i < num_outputs; i++) {
		out_props[i].width = scale_props->output_widths[i];
		out_props[i].height = scale_props->output_heights[i];
		out_props[i].fps_num = scale_props->output_fps_nums[i];
		out_props[i].fps_den = scale_props->output_fps_dens[i];
		if (scale_props->is_halfrate[i]) {
			out_props[i].fps_den *= 2;
		}
	}
	if (xrm_scale_reserve(&scl_ctx->scaler_xrm_ctx, scale_props->device_id,
			      &input_props, out_props,
			      num_outputs) == XRM_ERROR) {
		return XMA_ERROR;
	}
	free(out_props);
	XmaInitParameter xma_init_param;
	memset(&xma_init_param, 0, sizeof(XmaInitParameter));
	char m_app_name[32];
	strcpy(m_app_name, XLNX_SCALER_APP_MODULE);
	xma_init_param.app_name = m_app_name;
	xma_init_param.device = scl_ctx->abr_params.device_id;
	xma_init_param.params = NULL;
	xma_init_param.param_cnt = 0;

	if ((ret = xma_initialize(scl_ctx->log, &xma_init_param,
				  &scl_ctx->handle)) != XMA_SUCCESS) {
		xma_logmsg(scl_ctx->log, XMA_ERROR_LOG, XLNX_SCALER_APP_MODULE,
			   "XMA Initialization failed\n");
		return XMA_ERROR;
	}
	xma_logmsg(scl_ctx->log, XMA_INFO_LOG, XLNX_SCALER_APP_MODULE,
		   "XMA initialization success\n");

	scl_ctx->abr_xma_props[0].handle = scl_ctx->handle;
	if (scl_ctx->abr_params.is_halfrate_used) {
		scl_ctx->abr_xma_props[1].handle = scl_ctx->handle;
	}
	return ret;
}

int32_t scaler_process_frame(struct encoder_frame *frame, ScalerCtx *scl_ctx)
{
	(void)frame;
	(void)scl_ctx;
	return 0;
}

void scal_cleanup_props(XmaScalerProperties *scaler_xma_props)
{
	if (scaler_xma_props->params) {
		free(scaler_xma_props->params);
	}
}

int32_t scaler_destroy(ScalerCtx *scl_ctx)
{
	if (!scl_ctx) {
		return XMA_ERROR;
	}
	for (int session_id = 0; session_id < scl_ctx->num_sessions;
	     session_id++) {
		if (scl_ctx->session[session_id]) {
			xma_scaler_session_destroy(
				scl_ctx->session[session_id]);
		}
	}
	if (scl_ctx->handle) {
		xma_release(scl_ctx->handle);
	}
	if (scl_ctx->log) {
		xma_log_release(scl_ctx->log);
	}
	xrm_scale_release(&scl_ctx->scaler_xrm_ctx);
	if (scl_ctx->abr_params.is_halfrate_used) {
		scal_cleanup_props(&scl_ctx->abr_xma_props[1]);
	}
	scal_cleanup_props(&scl_ctx->abr_xma_props[0]);
	for (uint i = 0; i < scl_ctx->abr_params.num_outputs[0]; i++) {
		if (scl_ctx->output_xframe_list[i]) {
			xma_frame_free(scl_ctx->output_xframe_list[i]);
		}
	}
	if (scl_ctx->input_xframe) {
		xma_frame_free(scl_ctx->input_xframe);
	}
	return XMA_SUCCESS;
}
