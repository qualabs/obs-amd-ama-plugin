#ifndef _AMA_ENCODER_H_
#define _AMA_ENCODER_H_

#include <obs-module.h>
#include <plugin-support.h>
#include <xma.h>
#include <xrm.h>
#include <xrm_enc_interface.h>
#include <util/darray.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define XLNX_ENC_APP_MODULE "ma35_encoder_app"

#define ENC_DEFAULT_LOG_LEVEL XMA_ERROR_LOG
#define ENC_DEFAULT_LOG_LOCATION XMA_LOG_TYPE_CONSOLE
#define ENC_DEFAULT_LOG_FILE (XLNX_ENC_APP_MODULE ".log")
#define ENC_MIN_NUM_B_FRAMES 0
#define ENC_MAX_NUM_B_FRAMES 3
#define ENC_MAX_NUM_B_FRAMES_AV1 7
#define ENC_DEFAULT_TIER -1
#define ENC_DYNAMIC_GOP_AUTO -1
#define ENC_DYNAMIC_GOP_DISABLE 0
#define ENC_DYNAMIC_GOP_ENABLE 1
#define ENC_DYNAMIC_GOP_DEFAULT ENC_DYNAMIC_GOP_AUTO
#define ENC_MIN_GOP_SIZE -1
#define ENC_MAX_GOP_SIZE 1000
#define ENC_SUPPORTED_MIN_QP -1
#define ENC_SUPPORTED_MAX_QP 51
#define ENC_SUPPORTED_MAX_AV1_QP 255
#define ENC_SPATIAL_AQ_DISABLE 0
#define ENC_SPATIAL_AQ_ENABLE 1
#define ENC_TEMPORAL_AQ_DISABLE 0
#define ENC_TEMPORAL_AQ_ENABLE 1
#define ENC_DEFAULT_TUNE_METRICS 1
#define ENC_MAX_TUNE_METRICS 4
#define ENC_CRF_ENABLE 1
#define ENC_CRF_ENABLE_ALIAS 255
#define ENC_CRF_DEFAULT ENC_CRF_DISABLE
#define ENC_IDR_DISABLE 0

#define ENC_MIN_LOOKAHEAD_DEPTH 0
#define ENC_MAX_LOOKAHEAD_DEPTH 53
#define ENC_DEFAULT_LOOKAHEAD_DEPTH ENC_MIN_LOOKAHEAD_DEPTH

#define ENC_SUPPORTED_MIN_WIDTH 64
#define ENC_DEFAULT_WIDTH 1920
#define ENC_SUPPORTED_MAX_WIDTH 3840

#define ENC_SUPPORTED_MIN_HEIGHT 64
#define ENC_DEFAULT_HEIGHT 1080
#define ENC_SUPPORTED_MAX_HEIGHT 2160

#define ENC_SUPPORTED_MAX_PIXELS \
	((ENC_SUPPORTED_MAX_WIDTH) * (ENC_SUPPORTED_MAX_HEIGHT))

#define ENC_MIN_MIN_QP ENC_SUPPORTED_MIN_QP
#define ENC_MAX_MIN_QP ENC_SUPPORTED_MAX_QP
#define ENC_DEFAULT_MIN_QP ENC_SUPPORTED_MIN_QP
#define ENC_MIN_MAX_QP ENC_SUPPORTED_MIN_QP
#define ENC_MAX_MAX_QP ENC_SUPPORTED_MAX_QP
#define ENC_DEFAULT_MAX_QP ENC_SUPPORTED_MIN_QP

#define ENC_SUPPORTED_MIN_AQ_GAIN 0
#define ENC_SUPPORTED_MAX_AQ_GAIN 255
#define ENC_AQ_GAIN_NOT_USED 255

#define ENC_OPTION_DISABLE 0
#define ENC_OPTION_ENABLE 1

#define ENC_LEVEL_10 10
#define ENC_LEVEL_11 11
#define ENC_LEVEL_12 12
#define ENC_LEVEL_13 13
#define ENC_LEVEL_20 20
#define ENC_LEVEL_21 21
#define ENC_LEVEL_22 22
#define ENC_LEVEL_30 30
#define ENC_LEVEL_31 31
#define ENC_LEVEL_32 32
#define ENC_LEVEL_40 40
#define ENC_LEVEL_41 41
#define ENC_LEVEL_42 42
#define ENC_LEVEL_50 50
#define ENC_LEVEL_51 51
#define ENC_LEVEL_52 52
#define ENC_LEVEL_53 53

#define ENC_PROFILE_AUTO -1
#define ENC_PROFILE_DEFAULT ENC_PROFILE_AUTO
#define ENC_AV1_MAIN 200

/* QP Mode supported values */
typedef enum QpModes {
	ENC_DEFAULT_QP_MODE = 0,
	ENC_QP_MODE_AUTO,
	ENC_QP_MODE_UNIFORM
} QpModes;

/*  RC Mode supported values */
typedef enum RcMode {
	ENC_RC_MODE_AUTO = -1,
	ENC_RC_MODE_CONSTANT_QP,
	ENC_RC_MODE_CBR,
	ENC_RC_MODE_VBR,
	ENC_RC_MODE_CVBR,
	ENC_RC_MODE_CABR,
	ENC_RC_MODE_DEFAULT = ENC_RC_MODE_AUTO
} RcMode;

typedef enum TuneMetrics {
	ENC_TUNE_METRICS_VQ = 1,
	ENC_TUNE_METRICS_PSNR,
	ENC_TUNE_METRICS_SSIM,
	ENC_TUNE_METRICS_VMAF,
	ENC_TUNE_METRICS_DEFAULT = ENC_TUNE_METRICS_VQ
} TuneMetrics;

typedef enum EncTiers {
	ENC_TIER_AUTO = -1,
	ENC_TIER_MAIN,
	ENC_TIER_HIGH,
	ENC_TIER_DEFAULT = ENC_TIER_AUTO
} EncTiers;

typedef enum EncoderCodecID {
	ENCODER_ID_H264 = XMA_H264_ENCODER_TYPE,
	ENCODER_ID_HEVC = XMA_HEVC_ENCODER_TYPE,
	ENCODER_ID_AV1 = XMA_AV1_ENCODER_TYPE,
} EncoderCodecID;

typedef enum EncoderDeviceType {
	ENCODER_DEVICE_TYPE_ANY = XMA_ENC_DEVICE_TYPE_ANY,
	ENCODER_DEVICE_TYPE_1 = XMA_ENC_DEVICE_TYPE_1,
	ENCODER_DEVICE_TYPE_2 = XMA_ENC_DEVICE_TYPE_2,
} EncoderDeviceType;

#define ENC_MAX_PARAMS 13

#define DEFAULT_DEVICE_ID 0
#define DEFAULT_SLICE_ID 0

#define ENC_DEFAULT_DEVICE_TYPE XMA_ENC_DEVICE_TYPE_1
#define ENC_SUPPORTED_MIN_BITRATE -1
#define ENC_SUPPORTED_MAX_BITRATE 3500000
#define ENC_DEFAULT_BITRATE 2500
#define ENC_DEFAULT_MAX_BITRATE ENC_SUPPORTED_MIN_BITRATE
#define ENC_CRF_DISABLE 0
#define ENC_IDR_ENABLE 1
#define ENC_DEFAULT_FRAMERATE 30
#define ENC_DEFAULT_GOP_SIZE 30
#define ENC_DEFAULT_NUM_B_FRAMES ENC_MIN_NUM_B_FRAMES
#define ENC_AQ_GAIN_NOT_USED 255
#define ENC_DEFAULT_SPATIAL_AQ 1
#define ENC_DEFAULT_TEMPORAL_AQ 1
#define ENC_DEFAULT_QP 30
#define ENC_DEFAULT_QP_MODE 0
#define ENC_RC_MODE_DEFAULT -1
#define ENC_DEFAULT_LEVEL 0

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
	int32_t cores;
	char expert_options[2048];
	int32_t latency_logging;
} EncoderProperties;

typedef struct DynIdrFrames {
	uint32_t *idr_arr;
	uint32_t idr_arr_idx;
	size_t len_idr_arr;
} DynIdrFrames;

typedef DARRAY(int64_t) int64_array_t;

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
	FILE *stat_data;
	FILE *dyn_params_fp;
	XmaFrameSideData *dyn_params;
	int64_array_t dts_array;
	int32_t codec;
	DARRAY(uint8_t) packet_data;
} EncoderCtx;

int32_t encoder_create(obs_data_t *settings, obs_encoder_t *encoder,
		       EncoderCtx *enc_ctx);

int32_t encoder_process_frame(struct encoder_frame *frame,
			      struct encoder_packet *packet,
			      bool *received_packet, EncoderCtx *enc_ctx);

int32_t encoder_destroy(EncoderCtx *enc_ctx);

#endif // _AMA_ENCODER_H_
