#ifndef _AMA_ENCODER_H_
#define _AMA_ENCODER_H_

#include <obs-module.h>
#include <plugin-support.h>
#include <xma.h>
#include <xrm.h>
#include <xrm_enc_interface.h>

#define ENC_MAX_PARAMS 13

#define DEFAULT_DEVICE_ID 0
#define DEFAULT_SLICE_ID 0

#define ENC_DEFAULT_DEVICE_TYPE XMA_ENC_DEVICE_TYPE_1
#define ENC_DEFAULT_WIDTH 1920
#define ENC_DEFAULT_HEIGHT 1080
#define ENC_SUPPORTED_MIN_BITRATE -1
#define ENC_SUPPORTED_MAX_BITRATE 35000000
#define ENC_DEFAULT_BITRATE 200
#define ENC_DEFAULT_MAX_BITRATE ENC_SUPPORTED_MIN_BITRATE
#define ENC_CRF_DISABLE 0
#define ENC_IDR_ENABLE 1
#define ENC_DEFAULT_FRAMERATE 30
#define ENC_DEFAULT_GOP_SIZE 30
#define ENC_DEFAULT_MIN_QP -1
#define ENC_DEFAULT_MAX_QP -1
#define ENC_DEFAULT_NUM_B_FRAMES -1
#define ENC_AQ_GAIN_NOT_USED 255
#define ENC_DEFAULT_SPATIAL_AQ 1
#define ENC_DEFAULT_TEMPORAL_AQ 1
#define ENC_DEFAULT_QP -1
#define ENC_DEFAULT_QP_MODE 0
#define ENC_RC_MODE_DEFAULT -1
#define ENC_DEFAULT_LEVEL 0
#define ENC_MIN_LOOKAHEAD_DEPTH 0
#define ENC_MAX_LOOKAHEAD_DEPTH 53

/* H264 Encoder supported profiles */
typedef enum h264Profiles {
	ENC_H264_BASELINE,
	ENC_H264_MAIN,
	ENC_H264_HIGH,
	ENC_H264_HIGH_10,
	ENC_H264_HIGH_10_INTRA
} h264Profiles;

/* HEVC Encoder supported profiles */
typedef enum HevcProfiles {
	ENC_HEVC_MAIN = 100,
	ENC_HEVC_MAIN_INTRA,
	ENC_HEVC_MAIN_10,
	ENC_HEVC_MAIN10_INTRA
} HevcProfiles;

typedef struct {
	int64_t bitrate;
	int64_t max_bitrate;
	uint32_t device_id;
	int32_t width;
	int32_t height;
	XmaFormatType pix_fmt;
	int32_t fps;
	int32_t gop_size;
	int32_t qp;
	int32_t min_qp;
	int32_t max_qp;
	int32_t temp_aq_gain;
	int32_t spat_aq_gain;
	int32_t spatial_aq;
	int32_t temporal_aq;
	int32_t qp_mode;
	int32_t rc_mode;
	int32_t crf;
	int32_t force_idr;
	int32_t slice;
	int32_t codec_id;
	int32_t device_type;
	int32_t num_bframes;
	int32_t preset;
	int32_t profile;
	int32_t level;
	int32_t tier;
	int32_t lookahead_depth;
	int32_t tune_metrics;
	int32_t dynamic_gop;
	char expert_options[2048];
	int32_t latency_logging;
} EncoderProperties;

typedef struct DynIdrFrames {
	uint32_t *idr_arr;
	uint32_t idr_arr_idx;
	size_t len_idr_arr;
} DynIdrFrames;

/* Encoder Context */
typedef struct {
	char app_name[32];
	obs_encoder_t *enc_handle;
	obs_data_t *settings;
	XmaLogHandle log;
	XmaLogHandle filter_log;
	XmaHandle handle;
	size_t num_frames_sent;
	size_t num_frames_received;
	XmaFilterSession *upload_session;
	XmaEncoderSession *enc_session;
	XrmEncodeContext xrm_enc_ctx;
	EncoderProperties enc_props;
	DynIdrFrames *dynamic_idr;
	uint8_t *header_data;
	size_t header_size;
	uint8_t *sei_data;
	size_t sei_size;
	XmaFilterProperties xma_upload_props;
	XmaFrameProperties enc_frame_props;
	XmaFrame *input_xframe;
	XmaEncoderProperties xma_enc_props;
	bool flush_sent;
	int32_t pts;
	FILE *stat_data;
	FILE *dyn_params_fp;
	XmaFrameSideData *dyn_params;
} EncoderCtx;

int32_t encoder_create(obs_data_t *settings, obs_encoder_t *encoder,
		       EncoderCtx *enc_ctx);

int32_t encoder_process_frame(struct encoder_frame *frame,
			      struct encoder_packet *packet,
			      bool *received_packet, EncoderCtx *enc_ctx);

int32_t encoder_destroy(EncoderCtx *enc_ctx);

#endif // _AMA_ENCODER_H_
