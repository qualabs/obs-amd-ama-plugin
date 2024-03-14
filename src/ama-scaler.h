#ifndef _AMA_SCALER_H_
#define _AMA_SCALER_H_

#include <obs-module.h>

#include <xma.h>
#include <xrm.h>
#include <xrm_scale_interface.h>

typedef enum { PLANE_Y = 0, PLANE_U, PLANE_V } PlaneId;

typedef struct ScalerCtx {
	// ScalerProps     abr_params;
	XmaScalerProperties abr_xma_props[2]; // All rate and full rate sessions
	XmaFilterProperties xma_upload_props;
	XmaFrameProperties upload_fprops;
	XmaFilterProperties xma_download_props[MAX_SCALER_OUTPUTS];
	XmaFrameProperties download_fprops[MAX_SCALER_OUTPUTS];
	XrmScaleContext scaler_xrm_ctx;
	XmaLogHandle log;
	XmaHandle handle;
	int num_sessions;
	XmaScalerSession *session[2];
	XmaFilterSession *upload_session;
	XmaFilterSession *download_sessions[MAX_SCALER_OUTPUTS];
	XmaFrame *input_xframe;
	XmaFrameProperties input_fprops;
	XmaFrame *output_xframe_list[MAX_SCALER_OUTPUTS];
	XmaFrameProperties output_fprops[MAX_SCALER_OUTPUTS];
	size_t num_frames_scaled;
	int pts;
} ScalerCtx;

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      ScalerCtx scl_ctx);

int32_t scaler_process_frame();

int32_t scaler_destroy();

#endif // _AMA_scaler_H_
