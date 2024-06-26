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

#include <unistd.h>
#include "ama-scaler.h"
#include "ama-context.h"

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
	ctx->abr_xma_props.num_outputs = 1;
	ctx->abr_xma_props.hwscaler_type = XMA_ABR_SCALER_TYPE;
	ctx->scl_session = xma_scaler_session_create(&ctx->abr_xma_props);
	if (!ctx->scl_session) {
		xma_logmsg(ctx->log, XMA_ERROR_LOG, XLNX_SCALER_APP_MODULE,
			   "Failed to create scaler session\n");
		return XMA_ERROR;
	}
	return ret;
}

int32_t scaler_process_frame(AmaCtx *ctx)
{
	ctx->scaler_input_fprops.format = XMA_VPE_FMT_TYPE;
	int send_rc, recv_rc;
	XmaFrame **encoder_xframe = &ctx->encoder_input_xframe;
	XmaFrame *scaler_xframe = ctx->scaler_input_xframe;
	//Send scaler xframe
	send_rc =
		xma_scaler_session_send_frame(ctx->scl_session, scaler_xframe);
	if (send_rc == XMA_SUCCESS) {
		*encoder_xframe = xma_frame_alloc(ctx->handle,
						  &ctx->enc_frame_props, true);
		if (!encoder_xframe) {
			return XMA_ERROR;
		}

		//Receive with the encoder xframe
		uint32_t counter = 1000000;
		do {
			recv_rc = xma_scaler_session_recv_frame_list(
				ctx->scl_session, encoder_xframe);
			if (recv_rc == XMA_TRY_AGAIN) {
				usleep(100);
			}
		} while ((recv_rc == XMA_TRY_AGAIN) && (--counter > 0));

		if (recv_rc == XMA_EOS) {
			xma_frame_free(*encoder_xframe);
			*encoder_xframe = NULL;

			send_rc = XMA_EOS;
			ctx->scl_session = 0;
		} else if (recv_rc != XMA_SUCCESS) {
			return XMA_ERROR;
		}
	}
	return XMA_SUCCESS;
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
