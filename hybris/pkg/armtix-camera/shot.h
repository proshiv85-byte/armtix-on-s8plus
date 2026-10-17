
struct camera2_node_input {
	uint32_t                   cropRegion[4];        /*     0    16 */

	/* size: 16, cachelines: 1, members: 1 */
	/* last cacheline: 16 bytes */
};

struct camera2_node_output {
	uint32_t                   cropRegion[4];        /*     0    16 */

	/* size: 16, cachelines: 1, members: 1 */
	/* last cacheline: 16 bytes */
};

struct camera2_node {
	uint32_t                   vid;                  /*     0     4 */
	uint32_t                   request;              /*     4     4 */
	struct camera2_node_input  input;                /*     8    16 */
	struct camera2_node_output output;               /*    24    16 */
	uint32_t                   pixelformat;          /*    40     4 */

	/* size: 44, cachelines: 1, members: 5 */
	/* last cacheline: 44 bytes */
};


struct camera2_node_group {
	struct camera2_node        leader;               /*     0    44 */
	struct camera2_node        capture[6];           /*    44   264 */

	/* size: 308, cachelines: 5, members: 2 */
	/* last cacheline: 52 bytes */
};

enum colorcorrection_mode {
	COLORCORRECTION_MODE_TRANSFORM_MATRIX = 1,
	COLORCORRECTION_MODE_FAST             = 2,
	COLORCORRECTION_MODE_HIGH_QUALITY     = 3,
};


struct rational {
	int32_t                    num;                  /*     0     4 */
	int32_t                    den;                  /*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

enum processing_mode {
	PROCESSING_MODE_OFF          = 1,
	PROCESSING_MODE_FAST         = 2,
	PROCESSING_MODE_HIGH_QUALITY = 3,
	PROCESSING_MODE_MINIMAL      = 4,
	PROCESSING_MODE_MANUAL       = 100,
};

struct camera2_colorcorrection_ctl {
	enum colorcorrection_mode  mode;                 /*     0     4 */
	struct rational            transform[9];         /*     4    72 */
	/* --- cacheline 1 boundary (64 bytes) was 12 bytes ago --- */
	float                      gains[4];             /*    76    16 */
	enum processing_mode       aberrationCorrectionMode; /*    92     4 */
	uint32_t                   vendor_hue;           /*    96     4 */
	uint32_t                   vendor_saturation;    /*   100     4 */
	uint32_t                   vendor_brightness;    /*   104     4 */
	uint32_t                   vendor_contrast;      /*   108     4 */

	/* size: 112, cachelines: 2, members: 8 */
	/* last cacheline: 48 bytes */
};

enum aa_ae_antibanding_mode {
	AA_AE_ANTIBANDING_OFF       = 1,
	AA_AE_ANTIBANDING_50HZ      = 2,
	AA_AE_ANTIBANDING_60HZ      = 3,
	AA_AE_ANTIBANDING_AUTO      = 4,
	AA_AE_ANTIBANDING_AUTO_50HZ = 100,
	AA_AE_ANTIBANDING_AUTO_60HZ = 101,
};

enum aa_ae_lock {
	AA_AE_LOCK_OFF = 1,
	AA_AE_LOCK_ON  = 2,
};

enum aa_aemode {
	AA_AEMODE_OFF                  = 1,
	AA_AEMODE_ON                   = 2,
	AA_AEMODE_ON_AUTO_FLASH        = 3,
	AA_AEMODE_ON_ALWAYS_FLASH      = 4,
	AA_AEMODE_ON_AUTO_FLASH_REDEYE = 5,
	AA_AEMODE_CENTER               = 100,
	AA_AEMODE_AVERAGE              = 101,
	AA_AEMODE_MATRIX               = 102,
	AA_AEMODE_SPOT                 = 103,
	AA_AEMODE_CENTER_TOUCH         = 104,
	AA_AEMODE_AVERAGE_TOUCH        = 105,
	AA_AEMODE_MATRIX_TOUCH         = 106,
	AA_AEMODE_SPOT_TOUCH           = 107,
	UNKNOWN_AA_AE_MODE             = 108,
};

enum aa_ae_precapture_trigger {
	AA_AE_PRECAPTURE_TRIGGER_IDLE   = 0,
	AA_AE_PRECAPTURE_TRIGGER_START  = 1,
	AA_AE_PRECAPTURE_TRIGGER_CANCEL = 2,
};

enum aa_afmode {
	AA_AFMODE_OFF                = 1,
	AA_AFMODE_AUTO               = 2,
	AA_AFMODE_MACRO              = 3,
	AA_AFMODE_CONTINUOUS_VIDEO   = 4,
	AA_AFMODE_CONTINUOUS_PICTURE = 5,
	AA_AFMODE_EDOF               = 6,
};

enum aa_af_trigger {
	AA_AF_TRIGGER_IDLE   = 0,
	AA_AF_TRIGGER_START  = 1,
	AA_AF_TRIGGER_CANCEL = 2,
};

enum aa_awb_lock {
	AA_AWB_LOCK_OFF = 1,
	AA_AWB_LOCK_ON  = 2,
};

enum aa_awbmode {
	AA_AWBMODE_OFF                 = 1,
	AA_AWBMODE_WB_AUTO             = 2,
	AA_AWBMODE_WB_INCANDESCENT     = 3,
	AA_AWBMODE_WB_FLUORESCENT      = 4,
	AA_AWBMODE_WB_WARM_FLUORESCENT = 5,
	AA_AWBMODE_WB_DAYLIGHT         = 6,
	AA_AWBMODE_WB_CLOUDY_DAYLIGHT  = 7,
	AA_AWBMODE_WB_TWILIGHT         = 8,
	AA_AWBMODE_WB_SHADE            = 9,
	AA_AWBMODE_WB_CUSTOM_K         = 10,
};

enum aa_capture_intent {
	AA_CAPTURE_INTENT_CUSTOM                              = 0,
	AA_CAPTURE_INTENT_PREVIEW                             = 1,
	AA_CAPTURE_INTENT_STILL_CAPTURE                       = 2,
	AA_CAPTURE_INTENT_VIDEO_RECORD                        = 3,
	AA_CAPTURE_INTENT_VIDEO_SNAPSHOT                      = 4,
	AA_CAPTURE_INTENT_ZERO_SHUTTER_LAG                    = 5,
	AA_CAPTURE_INTENT_MANUAL                              = 6,
	AA_CAPTURE_INTENT_STILL_CAPTURE_OIS_SINGLE            = 100,
	AA_CAPTURE_INTENT_STILL_CAPTURE_OIS_MULTI             = 101,
	AA_CAPTURE_INTENT_STILL_CAPTURE_OIS_BEST              = 102,
	AA_CAPTURE_INTENT_STILL_CAPTURE_COMP_BYPASS           = 103,
	AA_CAPTURE_INTENT_STILL_CAPTURE_OIS_DEBLUR            = 104,
	AA_CAPTURE_INTENT_STILL_CAPTURE_DEBLUR_DYNAMIC_SHOT   = 105,
	AA_CAPTURE_INTENT_STILL_CAPTURE_OIS_DYNAMIC_SHOT      = 106,
	AA_CAPTURE_INTENT_STILL_CAPTURE_EXPOSURE_DYNAMIC_SHOT = 107,
	AA_CAPTURE_INTENT_STILL_CAPTURE_CANCEL                = 108,
};

enum aa_effect_mode {
	AA_EFFECT_OFF              = 1,
	AA_EFFECT_MONO             = 2,
	AA_EFFECT_NEGATIVE         = 3,
	AA_EFFECT_SOLARIZE         = 4,
	AA_EFFECT_SEPIA            = 5,
	AA_EFFECT_POSTERIZE        = 6,
	AA_EFFECT_WHITEBOARD       = 7,
	AA_EFFECT_BLACKBOARD       = 8,
	AA_EFFECT_AQUA             = 9,
	AA_EFFECT_EMBOSS           = 100,
	AA_EFFECT_EMBOSS_MONO      = 101,
	AA_EFFECT_SKETCH           = 102,
	AA_EFFECT_RED_YELLOW_POINT = 103,
	AA_EFFECT_GREEN_POINT      = 104,
	AA_EFFECT_BLUE_POINT       = 105,
	AA_EFFECT_MAGENTA_POINT    = 106,
	AA_EFFECT_WARM_VINTAGE     = 107,
	AA_EFFECT_COLD_VINTAGE     = 108,
	AA_EFFECT_WASHED           = 109,
	AA_EFFECT_BEAUTY_FACE      = 110,
};

enum aa_mode {
	AA_CONTROL_OFF            = 1,
	AA_CONTROL_AUTO           = 2,
	AA_CONTROL_USE_SCENE_MODE = 3,
	AA_CONTROL_OFF_KEEP_STATE = 4,
};

enum aa_scene_mode {
	AA_SCENE_MODE_DISABLED                = 1,
	AA_SCENE_MODE_FACE_PRIORITY           = 2,
	AA_SCENE_MODE_ACTION                  = 3,
	AA_SCENE_MODE_PORTRAIT                = 4,
	AA_SCENE_MODE_LANDSCAPE               = 5,
	AA_SCENE_MODE_NIGHT                   = 6,
	AA_SCENE_MODE_NIGHT_PORTRAIT          = 7,
	AA_SCENE_MODE_THEATRE                 = 8,
	AA_SCENE_MODE_BEACH                   = 9,
	AA_SCENE_MODE_SNOW                    = 10,
	AA_SCENE_MODE_SUNSET                  = 11,
	AA_SCENE_MODE_STEADYPHOTO             = 12,
	AA_SCENE_MODE_FIREWORKS               = 13,
	AA_SCENE_MODE_SPORTS                  = 14,
	AA_SCENE_MODE_PARTY                   = 15,
	AA_SCENE_MODE_CANDLELIGHT             = 16,
	AA_SCENE_MODE_BARCODE                 = 17,
	AA_SCENE_MODE_HIGH_SPEED_VIDEO        = 18,
	AA_SCENE_MODE_HDR                     = 19,
	AA_SCENE_MODE_FACE_PRIORITY_LOW_LIGHT = 20,
	AA_SCENE_MODE_NIGHT_CAPTURE           = 100,
	AA_SCENE_MODE_ANTISHAKE               = 101,
	AA_SCENE_MODE_LLS                     = 102,
	AA_SCENE_MODE_FDAE                    = 103,
	AA_SCENE_MODE_DUAL                    = 104,
	AA_SCENE_MODE_DRAMA                   = 105,
	AA_SCENE_MODE_ANIMATED                = 106,
	AA_SCENE_MODE_PANORAMA                = 107,
	AA_SCENE_MODE_GOLF                    = 108,
	AA_SCENE_MODE_PREVIEW                 = 109,
	AA_SCENE_MODE_VIDEO                   = 110,
	AA_SCENE_MODE_SLOWMOTION_2            = 111,
	AA_SCENE_MODE_SLOWMOTION_4_8          = 112,
	AA_SCENE_MODE_DUAL_PREVIEW            = 113,
	AA_SCENE_MODE_DUAL_VIDEO              = 114,
	AA_SCENE_MODE_120_PREVIEW             = 115,
	AA_SCENE_MODE_LIGHT_TRACE             = 116,
	AA_SCENE_MODE_FOOD                    = 117,
	AA_SCENE_MODE_AQUA                    = 118,
	AA_SCENE_MODE_THERMAL                 = 119,
	AA_SCENE_MODE_VIDEO_COLLAGE           = 120,
	AA_SCENE_MODE_PRO_MODE                = 121,
	AA_SCENE_MODE_COLOR_IRIS              = 122,
	AA_SCENE_MODE_FACE_LOCK               = 123,
	AA_SCENE_MODE_LIVE_OUTFOCUS           = 124,
};

enum aa_videostabilization_mode {
	VIDEO_STABILIZATION_MODE_OFF = 0,
	VIDEO_STABILIZATION_MODE_ON  = 1,
};

enum aa_afmode_ext {
	AA_AFMODE_EXT_OFF                  = 1,
	AA_AFMODE_EXT_ADVANCED_MACRO_FOCUS = 2,
	AA_AFMODE_EXT_FOCUS_LOCATION       = 3,
	AA_AFMODE_EXT_FIXED_FACE           = 4,
};

enum aa_ae_flashmode {
	AA_FLASHMODE_OFF       = 1,
	AA_FLASHMODE_START     = 2,
	AA_FLASHMODE_CANCEL    = 3,
	AA_FLASHMODE_ON        = 4,
	AA_FLASHMODE_AUTO      = 5,
	AA_FLASHMODE_CAPTURE   = 6,
	AA_FLASHMODE_ON_ALWAYS = 7,
};

enum aa_isomode {
	AA_ISOMODE_AUTO   = 1,
	AA_ISOMODE_MANUAL = 2,
};

enum aa_cameraid {
	AA_CAMERAID_FRONT = 1,
	AA_CAMERAID_REAR  = 2,
};

enum aa_videomode {
	AA_VIDEOMODE_OFF = 0,
	AA_VIDEOMODE_ON  = 1,
};

enum aa_ae_facemode {
	AA_AE_FACEMODE_OFF = 0,
	AA_AE_FACEMODE_ON  = 1,
};

enum aa_afstate {
	AA_AFSTATE_INACTIVE           = 1,
	AA_AFSTATE_PASSIVE_SCAN       = 2,
	AA_AFSTATE_PASSIVE_FOCUSED    = 3,
	AA_AFSTATE_ACTIVE_SCAN        = 4,
	AA_AFSTATE_FOCUSED_LOCKED     = 5,
	AA_AFSTATE_NOT_FOCUSED_LOCKED = 6,
	AA_AFSTATE_PASSIVE_UNFOCUSED  = 7,
};

struct camera2_aa_ctl {
	enum aa_ae_antibanding_mode aeAntibandingMode;   /*     0     4 */
	int32_t                    aeExpCompensation;    /*     4     4 */
	enum aa_ae_lock            aeLock;               /*     8     4 */
	enum aa_aemode             aeMode;               /*    12     4 */
	uint32_t                   aeRegions[5];         /*    16    20 */
	uint32_t                   aeTargetFpsRange[2];  /*    36     8 */
	enum aa_ae_precapture_trigger aePrecaptureTrigger; /*    44     4 */
	enum aa_afmode             afMode;               /*    48     4 */
	uint32_t                   afRegions[5];         /*    52    20 */
	/* --- cacheline 1 boundary (64 bytes) was 8 bytes ago --- */
	enum aa_af_trigger         afTrigger;            /*    72     4 */
	enum aa_awb_lock           awbLock;              /*    76     4 */
	enum aa_awbmode            awbMode;              /*    80     4 */
	uint32_t                   awbRegions[5];        /*    84    20 */
	enum aa_capture_intent     captureIntent;        /*   104     4 */
	enum aa_effect_mode        effectMode;           /*   108     4 */
	enum aa_mode               mode;                 /*   112     4 */
	enum aa_scene_mode         sceneMode;            /*   116     4 */
	enum aa_videostabilization_mode videoStabilizationMode; /*   120     4 */
	float                      vendor_aeExpCompensationStep; /*   124     4 */
	/* --- cacheline 2 boundary (128 bytes) --- */
	uint32_t                   vendor_afmode_option; /*   128     4 */
	enum aa_afmode_ext         vendor_afmode_ext;    /*   132     4 */
	enum aa_ae_flashmode       vendor_aeflashMode;   /*   136     4 */
	enum aa_isomode            vendor_isoMode;       /*   140     4 */
	uint32_t                   vendor_isoValue;      /*   144     4 */
	int32_t                    vendor_awbValue;      /*   148     4 */
	enum aa_cameraid           vendor_cameraId;      /*   152     4 */
	enum aa_videomode          vendor_videoMode;     /*   156     4 */
	enum aa_ae_facemode        vendor_aeFaceMode;    /*   160     4 */
	enum aa_afstate            vendor_afState;       /*   164     4 */
	int32_t                    vendor_exposureValue; /*   168     4 */
	uint32_t                   vendor_touchAeDone;   /*   172     4 */
	uint32_t                   vendor_touchBvChange; /*   176     4 */
	uint32_t                   vendor_captureCount;  /*   180     4 */
	uint32_t                   vendor_captureExposureTime; /*   184     4 */
	float                      vendor_objectDistanceCm; /*   188     4 */
	/* --- cacheline 3 boundary (192 bytes) --- */
	uint32_t                   vendor_reserved[9];   /*   192    36 */

	/* size: 228, cachelines: 4, members: 36 */
	/* last cacheline: 36 bytes */
};

enum demosaic_processing_mode {
	DEMOSAIC_PROCESSING_MODE_FAST         = 1,
	DEMOSAIC_PROCESSING_MODE_HIGH_QUALITY = 2,
};

struct camera2_demosaic_ctl {
	enum demosaic_processing_mode mode;              /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};


struct camera2_edge_ctl {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

typedef long long unsigned int __u64;

enum flash_mode {
	CAM2_FLASH_MODE_NONE   = 0,
	CAM2_FLASH_MODE_OFF    = 1,
	CAM2_FLASH_MODE_SINGLE = 2,
	CAM2_FLASH_MODE_TORCH  = 3,
	CAM2_FLASH_MODE_BEST   = 100,
	CAM2_FLASH_MODE_LCD    = 101,
};

struct camera2_flash_ctl {
	uint32_t                   firingPower;          /*     0     4 */

	/* XXX 4 bytes hole, try to pack */

	uint64_t                   firingTime;           /*     8     8 */
	enum flash_mode            flashMode;            /*    16     4 */

	/* size: 24, cachelines: 1, members: 3 */
	/* sum members: 16, holes: 1, sum holes: 4 */
	/* padding: 4 */
	/* last cacheline: 24 bytes */
};

struct camera2_hotpixel_ctl {
	enum processing_mode       mode;                 /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_jpeg_ctl {
	uint8_t                    gpsLocation;          /*     0     1 */

	/* XXX 7 bytes hole, try to pack */

	double                     gpsCoordinates[3];    /*     8    24 */
	uint8_t                    gpsProcessingMethod[32]; /*    32    32 */
	/* --- cacheline 1 boundary (64 bytes) --- */
	uint64_t                   gpsTimestamp;         /*    64     8 */
	uint32_t                   orientation;          /*    72     4 */
	uint8_t                    quality;              /*    76     1 */
	uint8_t                    thumbnailQuality;     /*    77     1 */

	/* XXX 2 bytes hole, try to pack */

	uint32_t                   thumbnailSize[2];     /*    80     8 */

	/* size: 88, cachelines: 2, members: 8 */
	/* sum members: 79, holes: 2, sum holes: 9 */
	/* last cacheline: 24 bytes */
};

enum optical_stabilization_mode {
	OPTICAL_STABILIZATION_MODE_OFF             = 0,
	OPTICAL_STABILIZATION_MODE_ON              = 1,
	OPTICAL_STABILIZATION_MODE_STILL           = 100,
	OPTICAL_STABILIZATION_MODE_STILL_ZOOM      = 101,
	OPTICAL_STABILIZATION_MODE_VIDEO           = 102,
	OPTICAL_STABILIZATION_MODE_SINE_X          = 103,
	OPTICAL_STABILIZATION_MODE_SINE_Y          = 104,
	OPTICAL_STABILIZATION_MODE_CENTERING       = 105,
	OPTICAL_STABILIZATION_MODE_VDIS            = 106,
	OPTICAL_STABILIZATION_MODE_VIDEO_RATIO_4_3 = 107,
};

struct camera2_lens_ctl {
	float                      aperture;             /*     0     4 */
	float                      filterDensity;        /*     4     4 */
	float                      focalLength;          /*     8     4 */
	float                      focusDistance;        /*    12     4 */
	enum optical_stabilization_mode opticalStabilizationMode; /*    16     4 */

	/* size: 20, cachelines: 1, members: 5 */
	/* last cacheline: 20 bytes */
};

struct camera2_noisereduction_ctl {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

enum metadata_mode {
	METADATA_MODE_NONE = 0,
	METADATA_MODE_FULL = 1,
};

struct camera2_request_ctl {
	uint32_t                   frameCount;           /*     0     4 */
	uint32_t                   id;                   /*     4     4 */
	enum metadata_mode         metadataMode;         /*     8     4 */
	uint32_t                   vendor_frameCount;    /*    12     4 */

	/* size: 16, cachelines: 1, members: 4 */
	/* last cacheline: 16 bytes */
};

struct camera2_scaler_ctl {
	uint32_t                   cropRegion[4];        /*     0    16 */

	/* size: 16, cachelines: 1, members: 1 */
	/* last cacheline: 16 bytes */
};

enum sensor_test_pattern_mode {
	SENSOR_TEST_PATTERN_MODE_OFF                     = 1,
	SENSOR_TEST_PATTERN_MODE_SOLID_COLOR             = 2,
	SENSOR_TEST_PATTERN_MODE_COLOR_BARS              = 3,
	SENSOR_TEST_PATTERN_MODE_COLOR_BARS_FADE_TO_GRAY = 4,
	SENSOR_TEST_PATTERN_MODE_PN9                     = 5,
	SENSOR_TEST_PATTERN_MODE_CUSTOM1                 = 257,
};

struct camera2_sensor_ctl {
	uint64_t                   exposureTime;         /*     0     8 */
	uint64_t                   frameDuration;        /*     8     8 */
	uint32_t                   sensitivity;          /*    16     4 */
	int32_t                    testPatternData[4];   /*    20    16 */
	enum sensor_test_pattern_mode testPatternMode;   /*    36     4 */

	/* size: 40, cachelines: 1, members: 5 */
	/* last cacheline: 40 bytes */
};

struct camera2_shading_ctl {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

enum facedetect_mode {
	FACEDETECT_MODE_OFF    = 1,
	FACEDETECT_MODE_SIMPLE = 2,
	FACEDETECT_MODE_FULL   = 3,
};

enum stats_mode {
	STATS_MODE_OFF = 1,
	STATS_MODE_ON  = 2,
};

struct camera2_stats_ctl {
	enum facedetect_mode       faceDetectMode;       /*     0     4 */
	enum stats_mode            histogramMode;        /*     4     4 */
	enum stats_mode            sharpnessMapMode;     /*     8     4 */
	enum stats_mode            hotPixelMapMode;      /*    12     4 */
	enum stats_mode            lensShadingMapMode;   /*    16     4 */

	/* size: 20, cachelines: 1, members: 5 */
	/* last cacheline: 20 bytes */
};

enum tonemap_mode {
	TONEMAP_MODE_CONTRAST_CURVE = 1,
	TONEMAP_MODE_FAST           = 2,
	TONEMAP_MODE_HIGH_QUALITY   = 3,
	TONEMAP_MODE_GAMMA_VALUE    = 4,
	TONEMAP_MODE_PRESET_CURVE   = 5,
};

enum tonemap_presetCurve {
	TONEMAP_PRESET_CURVE_SRGB   = 0,
	TONEMAP_PRESET_CURVE_REC709 = 1,
};

struct camera2_tonemap_ctl {
	float                      curveBlue[64];        /*     0   256 */
	/* --- cacheline 4 boundary (256 bytes) --- */
	float                      curveGreen[64];       /*   256   256 */
	/* --- cacheline 8 boundary (512 bytes) --- */
	float                      curveRed[64];         /*   512   256 */
	/* --- cacheline 12 boundary (768 bytes) --- */
	float                      curve;                /*   768     4 */
	enum tonemap_mode          mode;                 /*   772     4 */
	float                      gamma;                /*   776     4 */
	enum tonemap_presetCurve   presetCurve;          /*   780     4 */

	/* size: 784, cachelines: 13, members: 7 */
	/* last cacheline: 16 bytes */
};

enum led_transmit {
	TRANSMIT_OFF = 0,
	TRANSMIT_ON  = 1,
};

struct camera2_led_ctl {
	enum led_transmit          transmit;             /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

enum blacklevel_lock {
	BLACK_LEVEL_LOCK_OFF = 0,
	BLACK_LEVEL_LOCK_ON  = 1,
};

struct camera2_blacklevel_ctl {
	enum blacklevel_lock       lock;                 /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

typedef long long int __s64;

struct camera2_sync_ctl {
	int64_t                    frameNumber;          /*     0     8 */

	/* size: 8, cachelines: 1, members: 1 */
	/* last cacheline: 8 bytes */
};

struct camera2_reprocess_ctl {
	float                      effectiveExposureFactor; /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_entry_ctl {
	uint32_t                   lowIndexParam;        /*     0     4 */
	uint32_t                   highIndexParam;       /*     4     4 */
	uint32_t                   parameter[2048];      /*     8  8192 */

	/* size: 8200, cachelines: 129, members: 3 */
	/* last cacheline: 8 bytes */
};

struct camera2_ctl {
	struct camera2_colorcorrection_ctl color;        /*     0   112 */
	/* --- cacheline 1 boundary (64 bytes) was 48 bytes ago --- */
	struct camera2_aa_ctl      aa;                   /*   112   228 */
	/* --- cacheline 5 boundary (320 bytes) was 20 bytes ago --- */
	struct camera2_demosaic_ctl demosaic;            /*   340     4 */
	struct camera2_edge_ctl    edge;                 /*   344     8 */

	/* XXX last struct has 3 bytes of padding */

	struct camera2_flash_ctl   flash;                /*   352    24 */

	/* XXX last struct has 4 bytes of padding, 1 hole */

	struct camera2_hotpixel_ctl hotpixel;            /*   376     4 */

	/* XXX 4 bytes hole, try to pack */

	/* --- cacheline 6 boundary (384 bytes) --- */
	struct camera2_jpeg_ctl    jpeg;                 /*   384    88 */

	/* XXX last struct has 2 holes */

	/* --- cacheline 7 boundary (448 bytes) was 24 bytes ago --- */
	struct camera2_lens_ctl    lens;                 /*   472    20 */
	struct camera2_noisereduction_ctl noise;         /*   492     8 */

	/* XXX last struct has 3 bytes of padding */

	struct camera2_request_ctl request;              /*   500    16 */
	/* --- cacheline 8 boundary (512 bytes) was 4 bytes ago --- */
	struct camera2_scaler_ctl  scaler;               /*   516    16 */

	/* XXX 4 bytes hole, try to pack */

	struct camera2_sensor_ctl  sensor;               /*   536    40 */
	/* --- cacheline 9 boundary (576 bytes) --- */
	struct camera2_shading_ctl shading;              /*   576     8 */

	/* XXX last struct has 3 bytes of padding */

	struct camera2_stats_ctl   stats;                /*   584    20 */
	struct camera2_tonemap_ctl tonemap;              /*   604   784 */
	/* --- cacheline 21 boundary (1344 bytes) was 44 bytes ago --- */
	struct camera2_led_ctl     led;                  /*  1388     4 */
	struct camera2_blacklevel_ctl blacklevel;        /*  1392     4 */

	/* XXX 4 bytes hole, try to pack */

	struct camera2_sync_ctl    sync;                 /*  1400     8 */
	/* --- cacheline 22 boundary (1408 bytes) --- */
	struct camera2_reprocess_ctl reprocess;          /*  1408     4 */
	struct camera2_entry_ctl   vendor_entry;         /*  1412  8200 */

	/* size: 9616, cachelines: 151, members: 20 */
	/* sum members: 9600, holes: 3, sum holes: 12 */
	/* padding: 4 */
	/* member types with holes: 2, total: 3 */
	/* paddings: 4, sum paddings: 13 */
	/* last cacheline: 16 bytes */
};


struct camera2_colorcorrection_dm {
	enum colorcorrection_mode  mode;                 /*     0     4 */
	struct rational            transform[9];         /*     4    72 */
	/* --- cacheline 1 boundary (64 bytes) was 12 bytes ago --- */
	float                      gains[4];             /*    76    16 */
	enum processing_mode       aberrationCorrectionMode; /*    92     4 */
	uint32_t                   vendor_hue;           /*    96     4 */
	uint32_t                   vendor_saturation;    /*   100     4 */
	uint32_t                   vendor_brightness;    /*   104     4 */
	uint32_t                   vendor_contrast;      /*   108     4 */

	/* size: 112, cachelines: 2, members: 8 */
	/* last cacheline: 48 bytes */
};

enum ae_state {
	AE_STATE_INACTIVE                 = 1,
	AE_STATE_SEARCHING                = 2,
	AE_STATE_CONVERGED                = 3,
	AE_STATE_LOCKED                   = 4,
	AE_STATE_FLASH_REQUIRED           = 5,
	AE_STATE_PRECAPTURE               = 6,
	AE_STATE_LOCKED_CONVERGED         = 10,
	AE_STATE_LOCKED_FLASH_REQUIRED    = 11,
	AE_STATE_SEARCHING_FLASH_REQUIRED = 12,
};

enum awb_state {
	AWB_STATE_INACTIVE  = 1,
	AWB_STATE_SEARCHING = 2,
	AWB_STATE_CONVERGED = 3,
	AWB_STATE_LOCKED    = 4,
};

struct camera2_aa_dm {
	enum aa_ae_antibanding_mode aeAntibandingMode;   /*     0     4 */
	int32_t                    aeExpCompensation;    /*     4     4 */
	enum aa_ae_lock            aeLock;               /*     8     4 */
	enum aa_aemode             aeMode;               /*    12     4 */
	uint32_t                   aeRegions[5];         /*    16    20 */
	uint32_t                   aeTargetFpsRange[2];  /*    36     8 */
	enum aa_ae_precapture_trigger aePrecaptureTrigger; /*    44     4 */
	enum ae_state              aeState;              /*    48     4 */
	enum aa_afmode             afMode;               /*    52     4 */
	uint32_t                   afRegions[5];         /*    56    20 */
	/* --- cacheline 1 boundary (64 bytes) was 12 bytes ago --- */
	enum aa_af_trigger         afTrigger;            /*    76     4 */
	enum aa_afstate            afState;              /*    80     4 */
	enum aa_awb_lock           awbLock;              /*    84     4 */
	enum aa_awbmode            awbMode;              /*    88     4 */
	uint32_t                   awbRegions[5];        /*    92    20 */
	enum aa_capture_intent     captureIntent;        /*   112     4 */
	enum awb_state             awbState;             /*   116     4 */
	enum aa_effect_mode        effectMode;           /*   120     4 */
	enum aa_mode               mode;                 /*   124     4 */
	/* --- cacheline 2 boundary (128 bytes) --- */
	enum aa_scene_mode         sceneMode;            /*   128     4 */
	enum aa_videostabilization_mode videoStabilizationMode; /*   132     4 */
	float                      vendor_aeExpCompensationStep; /*   136     4 */
	uint32_t                   vendor_afmode_option; /*   140     4 */
	enum aa_afmode_ext         vendor_afmode_ext;    /*   144     4 */
	enum aa_ae_flashmode       vendor_aeflashMode;   /*   148     4 */
	enum aa_isomode            vendor_isoMode;       /*   152     4 */
	uint32_t                   vendor_isoValue;      /*   156     4 */
	int32_t                    vendor_awbValue;      /*   160     4 */
	enum aa_cameraid           vendor_cameraId;      /*   164     4 */
	enum aa_videomode          vendor_videoMode;     /*   168     4 */
	enum aa_ae_facemode        vendor_aeFaceMode;    /*   172     4 */
	enum aa_afstate            vendor_afState;       /*   176     4 */
	int32_t                    vendor_exposureValue; /*   180     4 */
	uint32_t                   vendor_touchAeDone;   /*   184     4 */
	uint32_t                   vendor_touchBvChange; /*   188     4 */
	/* --- cacheline 3 boundary (192 bytes) --- */
	uint32_t                   vendor_captureCount;  /*   192     4 */
	uint32_t                   vendor_captureExposureTime; /*   196     4 */
	float                      vendor_objectDistanceCm; /*   200     4 */
	uint32_t                   vendor_reserved[9];   /*   204    36 */

	/* size: 240, cachelines: 4, members: 39 */
	/* last cacheline: 48 bytes */
};

struct camera2_demosaic_dm {
	enum demosaic_processing_mode mode;              /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_edge_dm {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

enum flash_state {
	FLASH_STATE_UNAVAILABLE = 0,
	FLASH_STATE_CHARGING    = 1,
	FLASH_STATE_READY       = 2,
	FLASH_STATE_FIRED       = 3,
	FLASH_STATE_PARTIAL     = 4,
};

struct camera2_flash_dm {
	uint32_t                   firingPower;          /*     0     4 */

	/* XXX 4 bytes hole, try to pack */

	uint64_t                   firingTime;           /*     8     8 */
	enum flash_mode            flashMode;            /*    16     4 */
	enum flash_state           flashState;           /*    20     4 */
	uint32_t                   vendor_firingStable;  /*    24     4 */
	uint32_t                   vendor_decision;      /*    28     4 */
	uint32_t                   vendor_flashReady;    /*    32     4 */
	uint32_t                   vendor_flashOffReady; /*    36     4 */

	/* size: 40, cachelines: 1, members: 8 */
	/* sum members: 36, holes: 1, sum holes: 4 */
	/* last cacheline: 40 bytes */
};

struct camera2_hotpixel_dm {
	enum processing_mode       mode;                 /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_jpeg_dm {
	uint8_t                    gpsLocation;          /*     0     1 */

	/* XXX 7 bytes hole, try to pack */

	double                     gpsCoordinates[3];    /*     8    24 */
	uint8_t                    gpsProcessingMethod[32]; /*    32    32 */
	/* --- cacheline 1 boundary (64 bytes) --- */
	uint64_t                   gpsTimestamp;         /*    64     8 */
	uint32_t                   orientation;          /*    72     4 */
	uint8_t                    quality;              /*    76     1 */

	/* XXX 3 bytes hole, try to pack */

	uint32_t                   size;                 /*    80     4 */
	uint8_t                    thumbnailQuality;     /*    84     1 */

	/* XXX 3 bytes hole, try to pack */

	uint32_t                   thumbnailSize[2];     /*    88     8 */

	/* size: 96, cachelines: 2, members: 9 */
	/* sum members: 83, holes: 3, sum holes: 13 */
	/* last cacheline: 32 bytes */
};

enum lens_state {
	LENS_STATE_STATIONARY = 0,
	LENS_STATE_MOVING     = 1,
};

struct camera2_lens_dm {
	float                      aperture;             /*     0     4 */
	float                      filterDensity;        /*     4     4 */
	float                      focalLength;          /*     8     4 */
	float                      focusDistance;        /*    12     4 */
	float                      focusRange[2];        /*    16     8 */
	enum optical_stabilization_mode opticalStabilizationMode; /*    24     4 */
	enum lens_state            state;                /*    28     4 */
	float                      poseRotation[4];      /*    32    16 */
	float                      poseTranslation[3];   /*    48    12 */
	float                      intrinsicCalibration[5]; /*    60    20 */
	/* --- cacheline 1 boundary (64 bytes) was 16 bytes ago --- */
	float                      radialDistortion[6];  /*    80    24 */

	/* size: 104, cachelines: 2, members: 11 */
	/* last cacheline: 40 bytes */
};

struct camera2_noisereduction_dm {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

struct camera2_request_dm {
	uint32_t                   frameCount;           /*     0     4 */
	uint32_t                   id;                   /*     4     4 */
	enum metadata_mode         metadataMode;         /*     8     4 */
	uint8_t                    pipelineDepth;        /*    12     1 */

	/* XXX 3 bytes hole, try to pack */

	uint32_t                   vendor_frameCount;    /*    16     4 */

	/* size: 20, cachelines: 1, members: 5 */
	/* sum members: 17, holes: 1, sum holes: 3 */
	/* last cacheline: 20 bytes */
};

struct camera2_scaler_dm {
	uint32_t                   cropRegion[4];        /*     0    16 */

	/* size: 16, cachelines: 1, members: 1 */
	/* last cacheline: 16 bytes */
};


struct camera2_sensor_dm {
	uint64_t                   exposureTime;         /*     0     8 */
	uint64_t                   frameDuration;        /*     8     8 */
	uint32_t                   sensitivity;          /*    16     4 */

	/* XXX 4 bytes hole, try to pack */

	uint64_t                   timeStamp;            /*    24     8 */
	float                      temperature;          /*    32     4 */
	struct rational            neutralColorPoint[3]; /*    36    24 */

	/* XXX 4 bytes hole, try to pack */

	/* --- cacheline 1 boundary (64 bytes) --- */
	double                     noiseProfile[4][2];   /*    64    64 */
	/* --- cacheline 2 boundary (128 bytes) --- */
	float                      profileHueSatMap[2][2][2][3]; /*   128    96 */
	/* --- cacheline 3 boundary (192 bytes) was 32 bytes ago --- */
	float                      profileToneCurve[32][2]; /*   224   256 */
	/* --- cacheline 7 boundary (448 bytes) was 32 bytes ago --- */
	float                      greenSplit;           /*   480     4 */
	int32_t                    testPatternData[4];   /*   484    16 */
	enum sensor_test_pattern_mode testPatternMode;   /*   500     4 */
	uint64_t                   rollingShutterSkew;   /*   504     8 */

	/* size: 512, cachelines: 8, members: 13 */
	/* sum members: 504, holes: 2, sum holes: 8 */
};

struct camera2_shading_dm {
	enum processing_mode       mode;                 /*     0     4 */
	uint8_t                    strength;             /*     4     1 */

	/* size: 8, cachelines: 1, members: 2 */
	/* padding: 3 */
	/* last cacheline: 8 bytes */
};

enum stats_scene_flicker {
	STATISTICS_SCENE_FLICKER_NONE = 1,
	STATISTICS_SCENE_FLICKER_50HZ = 2,
	STATISTICS_SCENE_FLICKER_60HZ = 3,
};

enum stats_lowlightmode {
	STATE_LLS_LEVEL_ZSL                         = 0,
	STATE_LLS_LEVEL_LOW                         = 1,
	STATE_LLS_LEVEL_HIGH                        = 2,
	STATE_LLS_LEVEL_SIS                         = 3,
	STATE_LLS_LEVEL_ZSL_LIKE                    = 4,
	STATE_LLS_LEVEL_ZSL_LIKE1                   = 7,
	STATE_LLS_LEVEL_SHARPEN_SINGLE              = 8,
	STATE_LLS_MANUAL_ISO                        = 9,
	STATE_LLS_LEVEL_FLASH                       = 16,
	STATE_LLS_LEVEL_MULTI_MERGE_2               = 18,
	STATE_LLS_LEVEL_MULTI_MERGE_3               = 19,
	STATE_LLS_LEVEL_MULTI_MERGE_4               = 20,
	STATE_LLS_LEVEL_MULTI_PICK_2                = 34,
	STATE_LLS_LEVEL_MULTI_PICK_3                = 35,
	STATE_LLS_LEVEL_MULTI_PICK_4                = 36,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_2     = 50,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_3     = 51,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_4     = 52,
	STATE_LLS_LEVEL_FLASH_2                     = 66,
	STATE_LLS_LEVEL_FLASH_3                     = 67,
	STATE_LLS_LEVEL_FLASH_4                     = 68,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_LOW_2 = 82,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_LOW_3 = 83,
	STATE_LLS_LEVEL_MULTI_MERGE_INDICATOR_LOW_4 = 84,
	STATE_LLS_LEVEL_FLASH_LOW_2                 = 98,
	STATE_LLS_LEVEL_FLASH_LOW_3                 = 99,
	STATE_LLS_LEVEL_FLASH_LOW_4                 = 100,
	STATE_LLS_LEVEL_DUMMY                       = 150,
};

enum stats_wdrAutoState {
	STATE_WDR_AUTO_OFF      = 1,
	STATE_WDR_AUTO_REQUIRED = 2,
};

struct camera2_stats_dm {
	enum facedetect_mode       faceDetectMode;       /*     0     4 */
	uint32_t                   faceIds[16];          /*     4    64 */
	/* --- cacheline 1 boundary (64 bytes) was 4 bytes ago --- */
	uint32_t                   faceLandmarks[16][6]; /*    68   384 */
	/* --- cacheline 7 boundary (448 bytes) was 4 bytes ago --- */
	uint32_t                   faceRectangles[16][4]; /*   452   256 */
	/* --- cacheline 11 boundary (704 bytes) was 4 bytes ago --- */
	uint8_t                    faceScores[16];       /*   708    16 */
	uint32_t                   faceSrcImageSize[2];  /*   724     8 */
	uint32_t                   faces[14];            /*   732    56 */
	/* --- cacheline 12 boundary (768 bytes) was 20 bytes ago --- */
	uint32_t                   histogram[768];       /*   788  3072 */
	/* --- cacheline 60 boundary (3840 bytes) was 20 bytes ago --- */
	enum stats_mode            histogramMode;        /*  3860     4 */
	int32_t                    sharpnessMap[2][2][3]; /*  3864    48 */
	/* --- cacheline 61 boundary (3904 bytes) was 8 bytes ago --- */
	enum stats_mode            sharpnessMapMode;     /*  3912     4 */
	uint8_t                    lensShadingCorrectionMap; /*  3916     1 */

	/* XXX 3 bytes hole, try to pack */

	float                      lensShadingMap[2][2][4]; /*  3920    64 */
	/* --- cacheline 62 boundary (3968 bytes) was 16 bytes ago --- */
	enum stats_scene_flicker   sceneFlicker;         /*  3984     4 */
	enum stats_mode            hotPixelMapMode;      /*  3988     4 */
	int32_t                    hotPixelMap[21][2];   /*  3992   168 */
	/* --- cacheline 65 boundary (4160 bytes) --- */
	enum stats_mode            lensShadingMapMode;   /*  4160     4 */
	enum stats_lowlightmode    vendor_LowLightMode;  /*  4164     4 */
	uint32_t                   vendor_lls_tuning_set_index; /*  4168     4 */
	uint32_t                   vendor_lls_brightness_index; /*  4172     4 */
	enum stats_wdrAutoState    vendor_wdrAutoState;  /*  4176     4 */

	/* size: 4180, cachelines: 66, members: 21 */
	/* sum members: 4177, holes: 1, sum holes: 3 */
	/* last cacheline: 20 bytes */
};

struct camera2_tonemap_dm {
	float                      curveBlue[64];        /*     0   256 */
	/* --- cacheline 4 boundary (256 bytes) --- */
	float                      curveGreen[64];       /*   256   256 */
	/* --- cacheline 8 boundary (512 bytes) --- */
	float                      curveRed[64];         /*   512   256 */
	/* --- cacheline 12 boundary (768 bytes) --- */
	float                      curve;                /*   768     4 */
	enum tonemap_mode          mode;                 /*   772     4 */
	float                      gamma;                /*   776     4 */
	enum tonemap_presetCurve   presetCurve;          /*   780     4 */

	/* size: 784, cachelines: 13, members: 7 */
	/* last cacheline: 16 bytes */
};

struct camera2_led_dm {
	enum led_transmit          transmit;             /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_blacklevel_dm {
	enum blacklevel_lock       lock;                 /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_sync_dm {
	int32_t                    maxLatency;           /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_reprocess_dm {
	float                      effectiveExposureFactor; /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_entry_dm {
	uint32_t                   lowIndexParam;        /*     0     4 */
	uint32_t                   highIndexParam;       /*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

struct camera2_dm {
	struct camera2_colorcorrection_dm color;         /*     0   112 */
	/* --- cacheline 1 boundary (64 bytes) was 48 bytes ago --- */
	struct camera2_aa_dm       aa;                   /*   112   240 */
	/* --- cacheline 5 boundary (320 bytes) was 32 bytes ago --- */
	struct camera2_demosaic_dm demosaic;             /*   352     4 */
	struct camera2_edge_dm     edge;                 /*   356     8 */

	/* XXX last struct has 3 bytes of padding */
	/* XXX 4 bytes hole, try to pack */

	struct camera2_flash_dm    flash;                /*   368    40 */

	/* XXX last struct has 1 hole */

	/* --- cacheline 6 boundary (384 bytes) was 24 bytes ago --- */
	struct camera2_hotpixel_dm hotpixel;             /*   408     4 */

	/* XXX 4 bytes hole, try to pack */

	struct camera2_jpeg_dm     jpeg;                 /*   416    96 */

	/* XXX last struct has 3 holes */

	/* --- cacheline 8 boundary (512 bytes) --- */
	struct camera2_lens_dm     lens;                 /*   512   104 */
	/* --- cacheline 9 boundary (576 bytes) was 40 bytes ago --- */
	struct camera2_noisereduction_dm noise;          /*   616     8 */

	/* XXX last struct has 3 bytes of padding */

	struct camera2_request_dm  request;              /*   624    20 */

	/* XXX last struct has 1 hole */

	/* --- cacheline 10 boundary (640 bytes) was 4 bytes ago --- */
	struct camera2_scaler_dm   scaler;               /*   644    16 */

	/* XXX 4 bytes hole, try to pack */

	struct camera2_sensor_dm   sensor;               /*   664   512 */

	/* XXX last struct has 2 holes */

	/* --- cacheline 18 boundary (1152 bytes) was 24 bytes ago --- */
	struct camera2_shading_dm  shading;              /*  1176     8 */

	/* XXX last struct has 3 bytes of padding */

	struct camera2_stats_dm    stats;                /*  1184  4180 */

	/* XXX last struct has 1 hole */

	/* --- cacheline 83 boundary (5312 bytes) was 52 bytes ago --- */
	struct camera2_tonemap_dm  tonemap;              /*  5364   784 */
	/* --- cacheline 96 boundary (6144 bytes) was 4 bytes ago --- */
	struct camera2_led_dm      led;                  /*  6148     4 */
	struct camera2_blacklevel_dm blacklevel;         /*  6152     4 */
	struct camera2_sync_dm     sync;                 /*  6156     4 */
	struct camera2_reprocess_dm reprocess;           /*  6160     4 */
	struct camera2_entry_dm    vendor_entry;         /*  6164     8 */

	/* size: 6176, cachelines: 97, members: 20 */
	/* sum members: 6160, holes: 3, sum holes: 12 */
	/* padding: 4 */
	/* member types with holes: 5, total: 8 */
	/* paddings: 3, sum paddings: 9 */
	/* last cacheline: 32 bytes */
};

struct camera2_obj_af_info {
	int32_t                    focusState;           /*     0     4 */
	int32_t                    focusROILeft;         /*     4     4 */
	int32_t                    focusROIRight;        /*     8     4 */
	int32_t                    focusROITop;          /*    12     4 */
	int32_t                    focusROIBottom;       /*    16     4 */
	int32_t                    focusWeight;          /*    20     4 */
	int32_t                    w_movement;           /*    24     4 */
	int32_t                    h_movement;           /*    28     4 */
	int32_t                    w_velocity;           /*    32     4 */
	int32_t                    h_velocity;           /*    36     4 */

	/* size: 40, cachelines: 1, members: 10 */
	/* last cacheline: 40 bytes */
};

struct camera2_hrm_sensor_info {
	uint32_t                   visible_data;         /*     0     4 */
	uint32_t                   ir_data;              /*     4     4 */
	int32_t                    flicker_data;         /*     8     4 */
	int32_t                    status;               /*    12     4 */

	/* size: 16, cachelines: 1, members: 4 */
	/* last cacheline: 16 bytes */
};

typedef short unsigned int __u16;

struct camera2_illuminaion_sensor_info {
	uint16_t                   visible_cdata;        /*     0     2 */
	uint16_t                   visible_rdata;        /*     2     2 */
	uint16_t                   visible_gdata;        /*     4     2 */
	uint16_t                   visible_bdata;        /*     6     2 */
	uint16_t                   visible_gain;         /*     8     2 */
	uint16_t                   visible_exptime;      /*    10     2 */
	uint16_t                   ir_north;             /*    12     2 */
	uint16_t                   ir_south;             /*    14     2 */
	uint16_t                   ir_east;              /*    16     2 */
	uint16_t                   ir_west;              /*    18     2 */
	uint16_t                   ir_gain;              /*    20     2 */
	uint16_t                   ir_exptime;           /*    22     2 */

	/* size: 24, cachelines: 1, members: 12 */
	/* last cacheline: 24 bytes */
};

struct camera2_gyro_sensor_info {
	float                      x;                    /*     0     4 */
	float                      y;                    /*     4     4 */
	float                      z;                    /*     8     4 */

	/* size: 12, cachelines: 1, members: 3 */
	/* last cacheline: 12 bytes */
};

struct camera2_accelerometer_sensor_info {
	float                      x;                    /*     0     4 */
	float                      y;                    /*     4     4 */
	float                      z;                    /*     8     4 */

	/* size: 12, cachelines: 1, members: 3 */
	/* last cacheline: 12 bytes */
};

struct camera2_proximity_sensor_info {
	int32_t                    flicker_data;         /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

struct camera2_aa_uctl {
	struct camera2_obj_af_info af_data;              /*     0    40 */
	struct camera2_hrm_sensor_info hrmInfo;          /*    40    16 */
	struct camera2_illuminaion_sensor_info illuminationInfo; /*    56    24 */
	/* --- cacheline 1 boundary (64 bytes) was 16 bytes ago --- */
	struct camera2_gyro_sensor_info gyroInfo;        /*    80    12 */
	struct camera2_accelerometer_sensor_info accInfo; /*    92    12 */
	struct camera2_proximity_sensor_info proximityInfo; /*   104     4 */

	/* size: 108, cachelines: 2, members: 6 */
	/* last cacheline: 44 bytes */
};

struct camera2_af_uctl {
	uint32_t                   vs2Length;            /*     0     4 */
	uint32_t                   vendorSpecific2[400]; /*     4  1600 */

	/* size: 1604, cachelines: 26, members: 2 */
	/* last cacheline: 4 bytes */
};

struct camera2_lens_uctl {
	uint32_t                   pos;                  /*     0     4 */
	uint32_t                   posSize;              /*     4     4 */
	uint32_t                   direction;            /*     8     4 */
	uint32_t                   slewRate;             /*    12     4 */
	uint32_t                   oisCoefVal;           /*    16     4 */

	/* size: 20, cachelines: 1, members: 5 */
	/* last cacheline: 20 bytes */
};

struct camera2_sensor_uctl {
	uint64_t                   dynamicFrameDuration; /*     0     8 */
	uint32_t                   analogGain;           /*     8     4 */
	uint32_t                   digitalGain;          /*    12     4 */
	uint64_t                   longExposureTime;     /*    16     8 */
	uint64_t                   shortExposureTime;    /*    24     8 */
	uint32_t                   longAnalogGain;       /*    32     4 */
	uint32_t                   shortAnalogGain;      /*    36     4 */
	uint32_t                   longDigitalGain;      /*    40     4 */
	uint32_t                   shortDigitalGain;     /*    44     4 */
	uint64_t                   exposureTime;         /*    48     8 */
	uint32_t                   frameDuration;        /*    56     4 */
	uint32_t                   sensitivity;          /*    60     4 */
	/* --- cacheline 1 boundary (64 bytes) --- */
	uint32_t                   CtrlFrameDuration;    /*    64     4 */

	/* size: 72, cachelines: 2, members: 13 */
	/* padding: 4 */
	/* last cacheline: 8 bytes */
};

struct camera2_flash_uctl {
	uint32_t                   firingPower;          /*     0     4 */

	/* XXX 4 bytes hole, try to pack */

	uint64_t                   firingTime;           /*     8     8 */
	enum flash_mode            flashMode;            /*    16     4 */

	/* size: 24, cachelines: 1, members: 3 */
	/* sum members: 16, holes: 1, sum holes: 4 */
	/* padding: 4 */
	/* last cacheline: 24 bytes */
};

enum mcsc_port {
	MCSC_PORT_NONE = -1,
	MCSC_PORT_0    = 0,
	MCSC_PORT_1    = 1,
	MCSC_PORT_2    = 2,
	MCSC_PORT_3    = 3,
	MCSC_PORT_4    = 4,
	MCSC_PORT_MAX  = 5,
};

struct camera2_scaler_uctl {
	uint32_t                   sourceAddress[17];    /*     0    68 */
	/* --- cacheline 1 boundary (64 bytes) was 4 bytes ago --- */
	uint32_t                   txcTargetAddress[17]; /*    68    68 */
	/* --- cacheline 2 boundary (128 bytes) was 8 bytes ago --- */
	uint32_t                   txpTargetAddress[17]; /*   136    68 */
	/* --- cacheline 3 boundary (192 bytes) was 12 bytes ago --- */
	uint32_t                   ixcTargetAddress[17]; /*   204    68 */
	/* --- cacheline 4 boundary (256 bytes) was 16 bytes ago --- */
	uint32_t                   ixpTargetAddress[17]; /*   272    68 */
	/* --- cacheline 5 boundary (320 bytes) was 20 bytes ago --- */
	uint32_t                   sccTargetAddress[17]; /*   340    68 */
	/* --- cacheline 6 boundary (384 bytes) was 24 bytes ago --- */
	uint32_t                   scpTargetAddress[17]; /*   408    68 */
	/* --- cacheline 7 boundary (448 bytes) was 28 bytes ago --- */
	uint32_t                   sc0TargetAddress[17]; /*   476    68 */
	/* --- cacheline 8 boundary (512 bytes) was 32 bytes ago --- */
	uint32_t                   sc1TargetAddress[17]; /*   544    68 */
	/* --- cacheline 9 boundary (576 bytes) was 36 bytes ago --- */
	uint32_t                   sc2TargetAddress[17]; /*   612    68 */
	/* --- cacheline 10 boundary (640 bytes) was 40 bytes ago --- */
	uint32_t                   sc3TargetAddress[17]; /*   680    68 */
	/* --- cacheline 11 boundary (704 bytes) was 44 bytes ago --- */
	uint32_t                   sc4TargetAddress[17]; /*   748    68 */
	/* --- cacheline 12 boundary (768 bytes) was 48 bytes ago --- */
	uint32_t                   sc5TargetAddress[17]; /*   816    68 */
	/* --- cacheline 13 boundary (832 bytes) was 52 bytes ago --- */
	uint32_t                   dxcTargetAddress[17]; /*   884    68 */
	/* --- cacheline 14 boundary (896 bytes) was 56 bytes ago --- */
	uint32_t                   orientation;          /*   952     4 */
	enum mcsc_port             mcsc_sub_blk_port[2]; /*   956     8 */

	/* size: 964, cachelines: 16, members: 16 */
	/* last cacheline: 4 bytes */
};

enum companion_drc_mode {
	COMPANION_DRC_OFF = 1,
	COMPANION_DRC_ON  = 2,
};

enum companion_wdr_mode {
	COMPANION_WDR_OFF        = 1,
	COMPANION_WDR_ON         = 2,
	COMPANION_WDR_AUTO       = 3,
	COMPANION_WDR_AUTO_LIKE  = 4,
	TOTALCOUNT_COMPANION_WDR = 5,
	COMPANION_WDR_UNKNOWN    = 6,
};

enum companion_paf_mode {
	COMPANION_PAF_OFF = 1,
	COMPANION_PAF_ON  = 2,
};

enum companion_caf_mode {
	COMPANION_CAF_OFF = 1,
	COMPANION_CAF_ON  = 2,
};

enum companion_lsc_mode {
	COMPANION_LSC_OFF = 1,
	COMPANION_LSC_ON  = 2,
};

enum companion_bpc_mode {
	COMPANION_BPC_OFF = 1,
	COMPANION_BPC_ON  = 2,
};

enum companion_bypass_mode {
	COMPANION_FULL_BYPASS_OFF = 1,
	COMPANION_FULL_BYPASS_ON  = 2,
};

enum companion_disparity_mode {
	COMPANION_DISPARITY_OFF           = 1,
	COMPANION_DISPARITY_SAD           = 2,
	COMPANION_DISPARITY_CENSUS_MEAN   = 3,
	COMPANION_DISPARITY_CENSUS_CENTER = 4,
};

struct camera2_companion_uctl {
	enum companion_drc_mode    drc_mode;             /*     0     4 */
	enum companion_wdr_mode    wdr_mode;             /*     4     4 */
	enum companion_paf_mode    paf_mode;             /*     8     4 */
	enum companion_caf_mode    caf_mode;             /*    12     4 */
	enum companion_lsc_mode    lsc_mode;             /*    16     4 */
	enum companion_bpc_mode    bpc_mode;             /*    20     4 */
	enum companion_bypass_mode bypass_mode;          /*    24     4 */
	enum companion_disparity_mode disparity_mode;    /*    28     4 */

	/* size: 32, cachelines: 1, members: 8 */
	/* last cacheline: 32 bytes */
};

struct camera2_fd_uctl {
	enum facedetect_mode       faceDetectMode;       /*     0     4 */
	uint32_t                   faceIds[16];          /*     4    64 */
	/* --- cacheline 1 boundary (64 bytes) was 4 bytes ago --- */
	uint32_t                   faceLandmarks[16][6]; /*    68   384 */
	/* --- cacheline 7 boundary (448 bytes) was 4 bytes ago --- */
	uint32_t                   faceRectangles[16][4]; /*   452   256 */
	/* --- cacheline 11 boundary (704 bytes) was 4 bytes ago --- */
	uint8_t                    faceScores[16];       /*   708    16 */
	uint32_t                   faces[16];            /*   724    64 */
	/* --- cacheline 12 boundary (768 bytes) was 20 bytes ago --- */
	uint32_t                   vendorSpecific[32];   /*   788   128 */

	/* size: 916, cachelines: 15, members: 7 */
	/* last cacheline: 20 bytes */
};

enum camera2_drc_mode {
	DRC_OFF = 1,
	DRC_ON  = 2,
};

struct camera2_drc_uctl {
	enum camera2_drc_mode      uDrcEn;               /*     0     4 */

	/* size: 4, cachelines: 1, members: 1 */
	/* last cacheline: 4 bytes */
};

enum camera_vt_mode {
	VT_MODE_OFF = 0,
	VT_MODE_1   = 1,
	VT_MODE_2   = 2,
	VT_MODE_3   = 3,
	VT_MODE_4   = 4,
};

enum camera_flash_mode {
	CAMERA_FLASH_MODE_OFF     = 0,
	CAMERA_FLASH_MODE_AUTO    = 1,
	CAMERA_FLASH_MODE_ON      = 2,
	CAMERA_FLASH_MODE_RED_EYE = 3,
	CAMERA_FLASH_MODE_TORCH   = 4,
};

enum camera_op_mode {
	CAMERA_OP_MODE_GED      = 0,
	CAMERA_OP_MODE_TW       = 1,
	CAMERA_OP_MODE_HAL3_GED = 2,
	CAMERA_OP_MODE_HAL3_TW  = 3,
	CAMERA_OP_MODE_FAC      = 4,
	CAMERA_OP_MODE_HAL3_FAC = 5,
};

enum camera2_is_hw_lls_mode {
	CAMERA_HW_LLS_OFF = 0,
	CAMERA_HW_LLS_ON  = 1,
};

enum camera2_is_hw_lls_progress {
	CAMERA_HW_LLS_FIRST_FRAME        = 0,
	CAMERA_HW_LLS_SECOND_FRAME       = 1,
	CAMERA_HW_LLS_INTERMEDIATE_FRAME = 2,
	CAMERA_HW_LLS_LAST_FRAME         = 3,
};

struct camera2_is_hw_lls_uctl {
	enum camera2_is_hw_lls_mode hwLlsMode;           /*     0     4 */
	enum camera2_is_hw_lls_progress hwLlsProgress;   /*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

enum aa_cameratype {
	AA_CAMERATYPE_SINGLE = 1,
	AA_CAMERATYPE_WIDE   = 2,
	AA_CAMERATYPE_TELE   = 3,
};

struct camera2_uctl {
	uint32_t                   uUpdateBitMap;        /*     0     4 */
	uint32_t                   uFrameNumber;         /*     4     4 */
	struct camera2_aa_uctl     aaUd;                 /*     8   108 */
	/* --- cacheline 1 boundary (64 bytes) was 52 bytes ago --- */
	struct camera2_af_uctl     af;                   /*   116  1604 */
	/* --- cacheline 26 boundary (1664 bytes) was 56 bytes ago --- */
	struct camera2_lens_uctl   lensUd;               /*  1720    20 */

	/* XXX 4 bytes hole, try to pack */

	/* --- cacheline 27 boundary (1728 bytes) was 16 bytes ago --- */
	struct camera2_sensor_uctl sensorUd;             /*  1744    72 */

	/* XXX last struct has 4 bytes of padding */

	/* --- cacheline 28 boundary (1792 bytes) was 24 bytes ago --- */
	struct camera2_flash_uctl  flashUd;              /*  1816    24 */

	/* XXX last struct has 4 bytes of padding, 1 hole */

	struct camera2_scaler_uctl scalerUd;             /*  1840   964 */
	/* --- cacheline 43 boundary (2752 bytes) was 52 bytes ago --- */
	struct camera2_companion_uctl companionUd;       /*  2804    32 */
	/* --- cacheline 44 boundary (2816 bytes) was 20 bytes ago --- */
	struct camera2_fd_uctl     fdUd;                 /*  2836   916 */
	/* --- cacheline 58 boundary (3712 bytes) was 40 bytes ago --- */
	struct camera2_drc_uctl    drcUd;                /*  3752     4 */
	enum camera_vt_mode        vtMode;               /*  3756     4 */
	float                      zoomRatio;            /*  3760     4 */
	enum camera_flash_mode     flashMode;            /*  3764     4 */
	enum camera_op_mode        opMode;               /*  3768     4 */
	struct camera2_is_hw_lls_uctl hwlls_mode;        /*  3772     8 */
	/* --- cacheline 59 boundary (3776 bytes) was 4 bytes ago --- */
	uint32_t                   statsRoi[4];          /*  3780    16 */
	enum aa_cameratype         masterCam;            /*  3796     4 */
	uint32_t                   reserved[1];          /*  3800     4 */

	/* size: 3808, cachelines: 60, members: 19 */
	/* sum members: 3800, holes: 1, sum holes: 4 */
	/* padding: 4 */
	/* member types with holes: 1, total: 1 */
	/* paddings: 2, sum paddings: 8 */
	/* last cacheline: 32 bytes */
};







struct camera2_aa_udm {
	struct camera2_obj_af_info af_data;              /*     0    40 */
	struct camera2_hrm_sensor_info hrmInfo;          /*    40    16 */
	struct camera2_illuminaion_sensor_info illuminationInfo; /*    56    24 */
	/* --- cacheline 1 boundary (64 bytes) was 16 bytes ago --- */
	struct camera2_gyro_sensor_info gyroInfo;        /*    80    12 */
	struct camera2_accelerometer_sensor_info accInfo; /*    92    12 */
	struct camera2_proximity_sensor_info proximityInfo; /*   104     4 */

	/* size: 108, cachelines: 2, members: 6 */
	/* last cacheline: 44 bytes */
};

struct camera2_lens_udm {
	uint32_t                   pos;                  /*     0     4 */
	uint32_t                   posSize;              /*     4     4 */
	uint32_t                   direction;            /*     8     4 */
	uint32_t                   slewRate;             /*    12     4 */
	uint32_t                   oisCoefVal;           /*    16     4 */

	/* size: 20, cachelines: 1, members: 5 */
	/* last cacheline: 20 bytes */
};

struct camera2_sensor_udm {
	uint64_t                   dynamicFrameDuration; /*     0     8 */
	uint32_t                   analogGain;           /*     8     4 */
	uint32_t                   digitalGain;          /*    12     4 */
	uint64_t                   longExposureTime;     /*    16     8 */
	uint64_t                   shortExposureTime;    /*    24     8 */
	uint32_t                   longAnalogGain;       /*    32     4 */
	uint32_t                   shortAnalogGain;      /*    36     4 */
	uint32_t                   longDigitalGain;      /*    40     4 */
	uint32_t                   shortDigitalGain;     /*    44     4 */
	uint64_t                   timeStampBoot;        /*    48     8 */

	/* size: 56, cachelines: 1, members: 10 */
	/* last cacheline: 56 bytes */
};

struct camera2_flash_udm {
	uint32_t                   firingPower;          /*     0     4 */

	/* XXX 4 bytes hole, try to pack */

	uint64_t                   firingTime;           /*     8     8 */
	enum flash_mode            flashMode;            /*    16     4 */

	/* size: 24, cachelines: 1, members: 3 */
	/* sum members: 16, holes: 1, sum holes: 4 */
	/* padding: 4 */
	/* last cacheline: 24 bytes */
};

struct camera2_ae_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[400];  /*     4  1600 */
	/* --- cacheline 25 boundary (1600 bytes) was 4 bytes ago --- */
	uint32_t                   vs2Length;            /*  1604     4 */
	uint32_t                   vendorSpecific2[32];  /*  1608   128 */

	/* size: 1736, cachelines: 28, members: 4 */
	/* last cacheline: 8 bytes */
};

struct camera2_awb_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[400];  /*     4  1600 */
	/* --- cacheline 25 boundary (1600 bytes) was 4 bytes ago --- */
	uint32_t                   vs2Length;            /*  1604     4 */
	uint32_t                   vendorSpecific2[32];  /*  1608   128 */

	/* size: 1736, cachelines: 28, members: 4 */
	/* last cacheline: 8 bytes */
};

struct camera2_af_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[400];  /*     4  1600 */
	/* --- cacheline 25 boundary (1600 bytes) was 4 bytes ago --- */
	int32_t                    lensPositionInfinity; /*  1604     4 */
	int32_t                    lensPositionMacro;    /*  1608     4 */
	int32_t                    lensPositionCurrent;  /*  1612     4 */
	uint32_t                   vs2Length;            /*  1616     4 */
	uint32_t                   vendorSpecific2[32];  /*  1620   128 */

	/* size: 1748, cachelines: 28, members: 7 */
	/* last cacheline: 20 bytes */
};

struct camera2_as_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[400];  /*     4  1600 */

	/* size: 1604, cachelines: 26, members: 2 */
	/* last cacheline: 4 bytes */
};

struct camera2_ipc_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[400];  /*     4  1600 */

	/* size: 1604, cachelines: 26, members: 2 */
	/* last cacheline: 4 bytes */
};

struct camera2_rta_udm {
	uint32_t                   vsLength;             /*     0     4 */
	uint32_t                   vendorSpecific[100];  /*     4   400 */

	/* size: 404, cachelines: 7, members: 2 */
	/* last cacheline: 20 bytes */
};

struct camera2_internal_udm {
	uint32_t                   ProcessedFrameInfo;   /*     0     4 */
	uint32_t                   vendorSpecific1[400]; /*     4  1600 */
	/* --- cacheline 25 boundary (1600 bytes) was 4 bytes ago --- */
	uint32_t                   vendorSpecific2[400]; /*  1604  1600 */

	/* size: 3204, cachelines: 51, members: 3 */
	/* last cacheline: 4 bytes */
};

struct ysum_data {
	uint32_t                   lower_ysum_value;     /*     0     4 */
	uint32_t                   higher_ysum_value;    /*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

struct camera2_scaler_udm {
	struct ysum_data           ysumdata;             /*     0     8 */

	/* size: 8, cachelines: 1, members: 1 */
	/* last cacheline: 8 bytes */
};

struct camera2_pdaf_multi_result {
	uint16_t                   mode;                 /*     0     2 */
	uint16_t                   goalPos;              /*     2     2 */
	uint16_t                   reliability;          /*     4     2 */
	uint16_t                   focused;              /*     6     2 */

	/* size: 8, cachelines: 1, members: 4 */
	/* last cacheline: 8 bytes */
};

struct camera2_pdaf_single_result {
	uint16_t                   mode;                 /*     0     2 */
	uint16_t                   goalPos;              /*     2     2 */
	uint16_t                   reliability;          /*     4     2 */
	uint16_t                   currentPos;           /*     6     2 */

	/* size: 8, cachelines: 1, members: 4 */
	/* last cacheline: 8 bytes */
};

struct camera2_pdaf_udm {
	uint16_t                   numCol;               /*     0     2 */
	uint16_t                   numRow;               /*     2     2 */
	struct camera2_pdaf_multi_result multiResult[9][13]; /*     4   936 */
	/* --- cacheline 14 boundary (896 bytes) was 44 bytes ago --- */
	struct camera2_pdaf_single_result singleResult;  /*   940     8 */
	uint16_t                   lensPosResolution;    /*   948     2 */

	/* size: 950, cachelines: 15, members: 5 */
	/* last cacheline: 54 bytes */
};

struct camera2_companion_udm {
	enum companion_drc_mode    drc_mode;             /*     0     4 */
	enum companion_wdr_mode    wdr_mode;             /*     4     4 */
	enum companion_paf_mode    paf_mode;             /*     8     4 */
	enum companion_caf_mode    caf_mode;             /*    12     4 */
	enum companion_lsc_mode    lsc_mode;             /*    16     4 */
	enum companion_bpc_mode    bpc_mode;             /*    20     4 */
	enum companion_bypass_mode bypass_mode;          /*    24     4 */
	struct camera2_pdaf_udm    pdaf;                 /*    28   950 */

	/* XXX 2 bytes hole, try to pack */

	/* --- cacheline 15 boundary (960 bytes) was 20 bytes ago --- */
	enum companion_disparity_mode disparity_mode;    /*   980     4 */

	/* size: 984, cachelines: 16, members: 9 */
	/* sum members: 982, holes: 1, sum holes: 2 */
	/* last cacheline: 24 bytes */
};

struct camera2_fd_udm {
	uint32_t                   vendorSpecific[32];   /*     0   128 */

	/* size: 128, cachelines: 2, members: 1 */
};

struct camera2_ni_udm {
	uint32_t                   currentFrameNoiseIndex; /*     0     4 */
	uint32_t                   nextFrameNoiseIndex;  /*     4     4 */
	uint32_t                   nextNextFrameNoiseIndex; /*     8     4 */

	/* size: 12, cachelines: 1, members: 3 */
	/* last cacheline: 12 bytes */
};

struct camera2_udm {
	struct camera2_aa_udm      aa;                   /*     0   108 */
	/* --- cacheline 1 boundary (64 bytes) was 44 bytes ago --- */
	struct camera2_lens_udm    lens;                 /*   108    20 */
	/* --- cacheline 2 boundary (128 bytes) --- */
	struct camera2_sensor_udm  sensor;               /*   128    56 */
	struct camera2_flash_udm   flash;                /*   184    24 */

	/* XXX last struct has 4 bytes of padding, 1 hole */

	/* --- cacheline 3 boundary (192 bytes) was 16 bytes ago --- */
	struct camera2_ae_udm      ae;                   /*   208  1736 */
	/* --- cacheline 30 boundary (1920 bytes) was 24 bytes ago --- */
	struct camera2_awb_udm     awb;                  /*  1944  1736 */
	/* --- cacheline 57 boundary (3648 bytes) was 32 bytes ago --- */
	struct camera2_af_udm      af;                   /*  3680  1748 */
	/* --- cacheline 84 boundary (5376 bytes) was 52 bytes ago --- */
	struct camera2_as_udm      as;                   /*  5428  1604 */
	/* --- cacheline 109 boundary (6976 bytes) was 56 bytes ago --- */
	struct camera2_ipc_udm     ipc;                  /*  7032  1604 */
	/* --- cacheline 134 boundary (8576 bytes) was 60 bytes ago --- */
	struct camera2_rta_udm     rta;                  /*  8636   404 */
	/* --- cacheline 141 boundary (9024 bytes) was 16 bytes ago --- */
	struct camera2_internal_udm internal;            /*  9040  3204 */
	/* --- cacheline 191 boundary (12224 bytes) was 20 bytes ago --- */
	struct camera2_scaler_udm  scaler;               /* 12244     8 */
	struct camera2_companion_udm companion;          /* 12252   984 */

	/* XXX last struct has 1 hole */

	/* --- cacheline 206 boundary (13184 bytes) was 52 bytes ago --- */
	struct camera2_fd_udm      fd;                   /* 13236   128 */
	/* --- cacheline 208 boundary (13312 bytes) was 52 bytes ago --- */
	enum camera_vt_mode        vtMode;               /* 13364     4 */
	float                      zoomRatio;            /* 13368     4 */
	enum camera_flash_mode     flashMode;            /* 13372     4 */
	/* --- cacheline 209 boundary (13376 bytes) --- */
	enum camera_op_mode        opMode;               /* 13376     4 */
	struct camera2_ni_udm      ni;                   /* 13380    12 */
	uint32_t                   reserved[3];          /* 13392    12 */

	/* size: 13408, cachelines: 210, members: 20 */
	/* padding: 4 */
	/* member types with holes: 2, total: 2 */
	/* paddings: 1, sum paddings: 4 */
	/* last cacheline: 32 bytes */
};

struct camera2_shot {
	struct camera2_ctl         ctl;                  /*     0  9616 */

	/* XXX last struct has 4 bytes of padding, 3 holes */

	/* --- cacheline 150 boundary (9600 bytes) was 16 bytes ago --- */
	struct camera2_dm          dm;                   /*  9616  6176 */

	/* XXX last struct has 4 bytes of padding, 3 holes */

	/* --- cacheline 246 boundary (15744 bytes) was 48 bytes ago --- */
	struct camera2_uctl        uctl;                 /* 15792  3808 */

	/* XXX last struct has 4 bytes of padding, 1 hole */

	/* --- cacheline 306 boundary (19584 bytes) was 16 bytes ago --- */
	struct camera2_udm         udm;                  /* 19600 13408 */

	/* XXX last struct has 4 bytes of padding */

	/* --- cacheline 515 boundary (32960 bytes) was 48 bytes ago --- */
	uint32_t                   magicNumber;          /* 33008     4 */

	/* size: 33016, cachelines: 516, members: 5 */
	/* padding: 4 */
	/* member types with holes: 3, total: 7 */
	/* paddings: 4, sum paddings: 16 */
	/* last cacheline: 56 bytes */
};

struct camera2_shot_ext {
	uint32_t                   setfile;              /*     0     4 */
	struct camera2_node_group  node_group;           /*     4   308 */
	/* --- cacheline 4 boundary (256 bytes) was 56 bytes ago --- */
	uint32_t                   drc_bypass;           /*   312     4 */
	uint32_t                   dis_bypass;           /*   316     4 */
	/* --- cacheline 5 boundary (320 bytes) --- */
	uint32_t                   dnr_bypass;           /*   320     4 */
	uint32_t                   fd_bypass;            /*   324     4 */
	uint32_t                   free_cnt;             /*   328     4 */
	uint32_t                   request_cnt;          /*   332     4 */
	uint32_t                   process_cnt;          /*   336     4 */
	uint32_t                   complete_cnt;         /*   340     4 */
	uint32_t                   invalid;              /*   344     4 */
	uint32_t                   reserved[14];         /*   348    56 */
	/* --- cacheline 6 boundary (384 bytes) was 20 bytes ago --- */
	uint32_t                   timeZone[10][2];      /*   404    80 */

	/* XXX 4 bytes hole, try to pack */

	/* --- cacheline 7 boundary (448 bytes) was 40 bytes ago --- */
	struct camera2_shot        shot;                 /*   488 33016 */

	/* XXX last struct has 4 bytes of padding */

	/* size: 33504, cachelines: 524, members: 14 */
	/* sum members: 33500, holes: 1, sum holes: 4 */
	/* paddings: 1, sum paddings: 4 */
	/* last cacheline: 32 bytes */
};

