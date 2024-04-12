#ifndef _AMA_SCALER_H_
#define _AMA_SCALER_H_

#include <obs-module.h>

#include <xma.h>
#include <xrm.h>
#include <xrm_scale_interface.h>

#define MAX_SCALER_WIDTH 3840
#define MAX_SCALER_HEIGHT 2160
#define MIN_SCALER_WIDTH 284
#define MIN_SCALER_HEIGHT 160
#define MAX_SCALER_PARAMS 2

typedef enum { PLANE_Y = 0, PLANE_U, PLANE_V } PlaneId;

typedef struct ScalerProps {
    uint32_t device_id;
    int32_t  log_level;
    int32_t  latency_logging;
    bool     is_halfrate_used;
    uint32_t num_outputs[2];

    XmaFormatType input_pix_fmt;
    int32_t       input_bits_per_pixel;
    int32_t       input_width;
    int32_t       input_height;
    int32_t       input_stride;
    int32_t       input_fps_num;
    int32_t       input_fps_den;

    bool          is_halfrate[MAX_SCALER_OUTPUTS];
    XmaFormatType output_pix_fmts[MAX_SCALER_OUTPUTS];
    int32_t       output_bits_per_pixels[MAX_SCALER_OUTPUTS];
    int32_t       output_widths[MAX_SCALER_OUTPUTS];
    int32_t       output_heights[MAX_SCALER_OUTPUTS];
    int32_t       output_strides[MAX_SCALER_OUTPUTS];
    int32_t       output_fps_nums[MAX_SCALER_OUTPUTS];
    int32_t       output_fps_dens[MAX_SCALER_OUTPUTS];
    int32_t       coeff_loads[MAX_SCALER_OUTPUTS];

} ScalerProps;

typedef struct ScalerCtx {
	ScalerProps abr_params;
	XmaScalerProperties abr_xma_props[2]; // All rate and full rate sessions
	XrmScaleContext scaler_xrm_ctx;
	XmaLogHandle log;
	XmaHandle handle;
	int num_sessions;
	XmaScalerSession *session[2];
	XmaFrame *input_xframe;
	XmaFrameProperties input_fprops;
	XmaFrame *output_xframe_list[MAX_SCALER_OUTPUTS];
	XmaFrameProperties output_fprops[MAX_SCALER_OUTPUTS];
	size_t num_frames_scaled;
	int pts;
} ScalerCtx;

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      ScalerCtx *scl_ctx);

int32_t scaler_process_frame(struct encoder_frame *frame, ScalerCtx *scl_ctx);

int32_t scaler_destroy(ScalerCtx *scl_ctx);

#endif // _AMA_scaler_H_
