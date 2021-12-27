#ifndef USB_VIDEO_H
#define USB_VIDEO_H

#include "os.h"
#include "usb/usb.h"


// Video Control - interface descriptor subtypes
#define VC_HEADER          1
#define VC_INPUT_TERMINAL  2
#define VC_OUTPUT_TERMINAL 3
#define VC_SELECTOR_UNIT   4
#define VC_PROCESSING_UNIT 5
#define VC_EXTENSION_UNIT  6
#define VC_ENCODING_UNIT   7

// Video Streaming - interface descriptor subtypes
#define VS_INPUT_HEADER          1
#define VS_OUTPUT_HEADER         2
#define VS_STILL_IMAGE_FRAME     3
#define VS_FORMAT_UNCOMPRESSED   4
#define VS_FRAME_UNCOMPRESSED    5
#define VS_FORMAT_MJPEG          6
#define VS_FRAME_MJPEG           7
#define VS_FORMAT_MPEG2TS        0xa
#define VS_FORMAT_DV             0xc
#define VS_COLORFORMAT           0xd
#define VS_FORMAT_FRAME_BASED    0x10
#define VS_FRAME_FRAME_BASED     0x11
#define VS_FORMAT_STREAM_BASED   0x12

// Video - endpoint descriptors subtypes
#define VID_EP_GENERAL       1
#define VID_EP_ENDPOINT      2
#define VID_EP_INTERRUPT     3

// USB Video terminal types
#define UVUTT_VENDOR                0x100 // USB vendor specific
#define UVUTT_STREAMING             0x101 // USB streaming

// USB Video input terminal types
#define UVITT_VENDOR                0x200 // Input Vendor specific
#define UVITT_CAMERA                0x201 // Camera sensor
#define UVITT_MEDIA_TRANSPORT       0x202 // Sequential media

// USB Video output terminal types
#define UVOTT_VENDOR                0x300 // Output Vendor specific
#define UVOTT_DISPLAY               0x301 // Generic display
#define UVOTT_MEDIA_TRANSPORT       0x302 // Sequential media

// USB Video external terminal types
#define UVETT_VENDOR                0x400 // External Vendor specific
#define UVETT_COMPOSITE_CONNECTOR   0x401 // Composite video connector
#define UVETT_SVIDEO_CONNECTOR      0x402 // S-Video connector
#define UVETT_COMPONENT_CONNECTOR   0x403 // Component video connector

// Video Control - interface control selectors
#define VC_VIDEO_POWER_MODE_CONTROL     1
#define VC_REQUEST_ERROR_CODE_CONTROL   2

// Selector unit control selectors
#define VC_INPUT_SELECT_CONTROL         1

// Camera terminal control selectors
#define VC_CS_SCANNING_MODE_CONTROL        1
#define VC_CS_AE_MODE_CONTROL              2
#define VC_CS_AE_PRIORITY_CONTROL          3
#define VC_CS_EXPOSURE_TIME_ABS_CONTROL    4
#define VC_CS_EXPOSURE_TIME_REL_CONTROL    5
#define VC_CS_FOCUS_ABS_CONTROL            6
#define VC_CS_FOCUS_REL_CONTROL            7
#define VC_CS_FOCUS_AUTO_CONTROL           8
#define VC_CS_IRIS_ABS_CONTROL             9
#define VC_CS_IRIS_REL_CONTROL             0xa
#define VC_CS_ZOOM_ABS_CONTROL             0xb
#define VC_CS_ZOOM_REL_CONTROL             0xc
#define VC_CS_PANTILT_ABS_CONTROL          0xd
#define VC_CS_PANTILT_REL_CONTROL          0xe
#define VC_CS_ROLL_ABS_CONTROL             0xf
#define VC_CS_ROLL_REL_CONTROL             0x10
#define VC_CS_PRIVACY_CONTROL              0x11

// Processing unit control selectors
#define VC_PU_BACKLIGHT_COMP_CONTROL       1
#define VC_PU_BRIGHTNESS_CONTROL           2
#define VC_PU_CONTRAST_CONTROL             3
#define VC_PU_GAIN_CONTROL                 4
#define VC_PU_POWER_LINE_FREQ_CONTROL      5
#define VC_PU_HUE_CONTROL                  6
#define VC_PU_SATURATION_CONTROL           7
#define VC_PU_SHARPNESS_CONTROL            8
#define VC_PU_GAMMA_CONTROL                9
#define VC_PU_WB_TEMP_CONTROL              0xa
#define VC_PU_WB_TEMP_AUTO_CONTROL         0xb
#define VC_PU_WB_COMPONENT_CONTROL         0xc
#define VC_PU_WB_COMPONENT_AUTO_CONTROL    0xd
#define VC_PU_DIGITAL_MULT_CONTROL         0xe
#define VC_PU_DIGITAL_MULT_LIMIT_CONTROL   0xf
#define VC_PU_HUE_AUTO_CONTROL             0x10
#define VC_PU_ANALOG_VIDEO_STD_CONTROL     0x11
#define VC_PU_ANALOG_LOCK_STATUS_CONTROL   0x12

// Video Streaming - interface control selectors
#define VS_CS_PROBE_CONTROL                1
#define VS_CS_COMMIT_CONTROL               2
#define VS_CS_STILL_PROBE_CONTROL          3
#define VS_CS_STILL_COMMIT_CONTROL         4
#define VS_CS_STILL_IMAGE_TRIGGER_CONTROL  5
#define VS_CS_STREAM_ERROR_COE_CONTROL     6
#define VS_CS_GENERATE_KEY_FRAME_CONTROL   7
#define VS_CS_UPDATE_FRAME_SEG_CONTROL     8
#define VS_CS_SYNCH_DELAY_CONTROL          9

// Usb Video Requests
#define VID_SET_CUR   1
#define VID_SET_MIN   2
#define VID_SET_MAX   3
#define VID_SET_RES   4

#define VID_GET_CUR   0x81
#define VID_GET_MIN   0x82
#define VID_GET_MAX   0x83
#define VID_GET_RES   0x84

#define VID_GET_LEN   0x85
#define VID_GET_INFO  0x86
#define VID_GET_DEF   0x87

// MJPEG Video Format
#define VID_MJPEG_FRAME_ID       BIT(0) // alternating bit for frame
#define VID_MJPEG_END_OF_FRAME   BIT(1) // last fragment of frame
#define VID_MJPEG_HAS_PTS        BIT(2) // presence of the PTS field (presentation time stamp)
#define VID_MJPEG_HAS_SCR        BIT(3) // presence of the SCR field (source clock reference)
#define VID_MJPEG_STILL_IMAGE    BIT(5) // part of a still image
#define VID_MJPEG_ERROR          BIT(6) // error in the device streaming
#define VID_MJPEG_END_OF_HEADER  BIT(7) // end of header bytes


// cf. Table 4-75 Video Probe and Commit Controls
/*
During Probe and Commit, the following fields, if supported, shall be negotiated in order of decreasing priority:
- formatIndex
- frameIndex
- maxPayloadTransferSize
- usage
- layoutPerStream
Fields set to zero by the host with their associated hint bit set to 1, all the remaining fields set to zero by the host
*/
typedef struct // length: 48
{
    // The hint bitmap indicates to the video streaming interface which fields shall be kept constant during stream parameter negotiation (indicative only).
    // The hint bitmap is set by the host, and is read-only for the video streaming interface.
    uint16_t hint_frameInterval  :  1; // D0: frameInterval
    uint16_t hint_keyFrameRate   :  1; // D1: keyFrameRate
    uint16_t hint_PFrameRate     :  1; // D2: PFrameRate
    uint16_t hint_compQuality    :  1; // D3: compQuality
    uint16_t hint_compWindowSize :  1; // D4: compWindowSize
    uint16_t hint_reserved       : 11; // D15..5: reserved (0)

    uint8_t  formatIndex;
    uint8_t  frameIndex;
    uint32_t frameInterval;
    uint16_t keyFrameRate;
    uint16_t PFrameRate;
    uint16_t compQuality;
    uint16_t compWindowSize;
    uint16_t delay;
    uint32_t maxVideoFrameSize;
    uint32_t maxPayloadTransferSize;
    uint32_t clockFrequency;
    uint8_t  framingInfo;
    uint8_t  preferredVersion;
    uint8_t  minVersion;
    uint8_t  maxVersion;
    uint8_t  usage;
    uint8_t  bitDepthLuma;
    uint8_t  settings; // A bitmap of flags to discover and control specific features of a temporally encoded video stream
    uint8_t  maxNumberOfRefFramesPlus1;
    uint16_t rateControlModes;   // This field contains 4 subfields, each of which is a 4 bit number
    uint16_t layoutPerStream[4]; // This field contains 4 subfields, each of which is a 2 byte number
} __attribute__((packed)) usb_vidStreamingControlParameterBlock_t;


typedef struct // length: 11
{
    uint8_t  formatIndex;
    uint8_t  frameIndex;
    uint8_t  compressionIndex; // The index value ranges from 1 to the number of Still Image Compression Patterns of the selected Still Image Frame descriptor.
    uint32_t maxVideoFrameSize;
    uint32_t maxPayloadTransferSize;
} __attribute__((packed)) usb_vidStillParameterBlock_t; // for VS_CS_STILL_PROBE/COMMIT_CONTROL


/*
Video Control (VC) Interface Descriptors:
    Input Terminal
    Output Terminal
    Camera Terminal
    Selector Unit
    Processing Unit
    Extension Unit
*/

typedef struct
{
    uint8_t  length;             // size of this descriptor
    uint8_t  descType;           // descriptor type (0x24)
    uint8_t  descSubtype;        // subtype (1)
    uint8_t  cdUVCLo;            // lo byte of spec version
    uint8_t  cdUVCHi;            // hi byte of spec version
    uint16_t totalLength;        // total length of all descriptors
    uint32_t clockFreq;          // clock frequency (deprecated)
    uint8_t  inCollection;       // The number of Video/MidiStreaming
    uint8_t  interfaceNr;        // Interface number of the first VideoStreaming interface
} __attribute__((packed)) usb_vidHeaderDesc_t;

typedef struct
{
    uint8_t  length;            // size of this descriptor
    uint8_t  descType;          // descriptor type (0x24)
    uint8_t  descSubtype;       // Subtype (2)
    uint8_t  terminalID;        // unique ID
    uint16_t terminalType;      // terminal type
    uint8_t  associatedTerminal;// associated terminal ID
    uint8_t  terminal;          // string descriptor
    uint8_t  bytes[10];         // depending on the type, extra bytes may follow
} __attribute__((packed)) usb_vidInputTermDesc_t;

typedef struct
{
    uint8_t  length;            // size of this descriptor
    uint8_t  descType;          // descriptor type (0x24)
    uint8_t  descSubtype;       // subtype (3)
    uint8_t  terminalID;        // unique ID
    uint16_t terminalType;      // terminal type
    uint8_t  associatedTerminal;// associated terminal ID
    uint8_t  srcID;             // ID of the Unit or Terminal to which this Terminal is connected.
    uint8_t  terminal;          // string descriptor
    uint8_t  bytes[10];         // depending on the type, extra bytes may follow
} __attribute__((packed)) usb_vidOutputTermDesc_t;

typedef struct
{
    uint8_t  length;              // size of this descriptor
    uint8_t  descType;            // descriptor type (0x24)
    uint8_t  descSubtype;         // subtype (4)
    uint8_t  terminalID;          // unique ID
    uint16_t terminalType;        // terminal type
    uint8_t  associatedTerminal;  // associated terminal ID
    uint8_t  srcID;               // ID of the unit or terminal to which this terminal is connected.
    uint8_t  terminal;            // string descriptor
    uint16_t focalLengthMin;      // focal length minimum
    uint16_t focalLengthMax;      // focal length maximum
    uint16_t ocularFocalLength;   // ocular focal length
    uint8_t  controlSize;         // element size of control array
    uint32_t controls;            // controls bitmap
} __attribute__((packed)) usb_vidSelectorUnitDesc_t;

typedef struct
{
    uint8_t  length;              // size of this descriptor
    uint8_t  descType;            // descriptor type (0x24)
    uint8_t  descSubtype;         // dubtype (5)
    uint8_t  uintID;              // unique ID
    uint8_t  sourceID;            // connected input pin
    uint16_t maxMultiplier;       // unused
    uint8_t  controlSize;         // size of the controls field in byte
    uint32_t controls;            // controls bitmap, e.g. brightness
} __attribute__((packed)) usb_vidProcessingUnitDesc_t;

typedef struct
{
    uint8_t  length;              // size of this descriptor (24+p+n)
    uint8_t  descType;            // descriptor type (0x24)
    uint8_t  descSubtype;         // subtype (6)
    uint8_t  unitID;              // a non-zero constant that uniquely identifies the Unit within the video function,
                                  // used in all requests to address this Unit.
    uint32_t guidFormat_Data1;    // Globally Unique Identifier used to identify extension unit
    uint16_t guidFormat_Data2;
    uint16_t guidFormat_Data3;
    uint8_t  guidFormat_Data4[8];
    uint8_t  numControls;         // number of controls in this extension unit
    uint8_t  numInPins;           // number of input pins of this unit: p
    uint8_t  sourceID;            // ID of the Unit or Terminal to which the first Input Pin of this Extension Unit is connected
    uint8_t  bytes[10];           // different data, cf. spec
} __attribute__((packed)) usb_vidExtensionUnitDesc_t;

/////////////////////
// Video Streaming //
/////////////////////

/*
Video Streaming (VS) Interface Descriptors:
    Input Header
    Output Header
    Payload Format
    Video Frame
    Still Image Frame
    Color Matching
*/

typedef struct
{
    uint8_t  length;            // descriptor size
    uint8_t  descType;          // descriptor type (0x24)
    uint8_t  descSubtype;       // subtype (1)
    uint8_t  numFormats;        // number of video payload format descriptors following
    uint16_t totalLength;       // length of descriptor
    uint8_t  endpointAddress;   // endpoint for bulk or isochronous video data
    uint8_t  info;              // capabilities bitmap
    uint8_t  terminalLink;      // terminal linked to this interface
    uint8_t  stillCaptureMethod;// method of still image capture supported
    uint8_t  triggerSupport;    // triggering support
    uint8_t  triggerUsage;      // trigger behavior
    uint8_t  controlSize;       // element size in the control array
    uint8_t  controls;          // controls bitmap
} __attribute__((packed)) usb_vidInputHeaderIFDesc_t;

typedef struct
{
    uint8_t  length;            // descriptor size
    uint8_t  descType;          // descriptor type (0x24)
    uint8_t  descSubtype;       // subtype (2)
    uint8_t  numFormats;        // number of video payload format descriptors following
    uint16_t totalLength;       // length of descriptor
    uint8_t  endpointAddress;   // endpoint for bulk or isochronous video data
    uint8_t  terminalLink;      // terminal linked to this interface
    uint8_t  controlSize;       // size of an element in the control array
    uint8_t  controls;          // controls bitmap
} __attribute__((packed)) usb_vidOutputHeaderIFDesc_t;

// this data structure is of no use, as it's highly variable in size
typedef struct
{
    uint8_t  length;                 // descriptor size
    uint8_t  descType;               // descriptor type (0x24)
    uint8_t  descSubtype;            // subtype (3)
    uint8_t  endpointAddress;        // endpoint for bulk or isochronous video data
    uint8_t  numImageSizePatterns;   // number of image size patterns of this format
    uint8_t  imageSizePatterns;      // array of width/heights
    uint8_t  numCompressionPatterns; // number of compression patterns of this format
    uint8_t  compression[10];        // array of compressions
} __attribute__((packed)) usb_vidStillImageFrameIFDesc_t;

typedef struct
{
    uint8_t  length;                // descriptor size (27)
    uint8_t  descType;              // descriptor type (0x24)
    uint8_t  descSubtype;           // subtype (4)
    uint8_t  formatIndex;           // index of this Format descriptor
    uint8_t  numFrameDescriptors;   // number of frame descriptors following
    uint32_t guidFormat_Data1;      // Globally Unique Identifier used to identify stream-encoding format
    uint16_t guidFormat_Data2;          // YUY2: 32595559-0000-0010-8000-00AA00389B71
    uint16_t guidFormat_Data3;          // NV12: 3231564E-0000-0010-8000-00AA00389B71
    uint8_t  guidFormat_Data4[8];
    uint8_t  BitsPerPixel;          // number of bits per pixel used to specify color in the decoded video frame
    uint8_t  defaultFrameIndex;     // optimum frame index (used to select resolution)
    uint8_t  aspectRatioX;          // X dimension of the picture aspect ratio // e.g., aspectRatioX: 16, aspectRatioY: 9 for a 16:9 display
    uint8_t  aspectRatioY;          // Y dimension of the picture aspect ratio
    uint8_t  interlaceFlags;        // specifies interlace information
    bool     copyprotectInfo;       // copy protect information
} __attribute__((packed)) usb_vidUncompressedFormatDesc_t;

typedef struct
{
    uint8_t  length;               // descriptor size (38)
    uint8_t  descType;             // descriptor type (0x24)
    uint8_t  descSubtype;          // descriptor subtype (5)
    uint8_t  frameIndex;           // index of this Frame Descriptor
    uint8_t  capabilites;          // capabilities bitmap
    uint16_t width;                // width
    uint16_t height;               // height
    uint32_t minBitRate;           // min bitrate at default compression
    uint32_t maxBitRate;           // max bitrate at default compression
    uint32_t maxVideoFrameBufSize; // deprecated
    uint32_t defaultFrameInterval; // default frame interval
    uint8_t  frameIntervalType;    // number of supported frame intervals (0: continuous, 1...255: fixed, number of discrete frame intervals supported)
    uint32_t frameArray[6];        // frame interval array or min/max/step
                                   // frame interval is the average display time of a single frame in 100ns units
} __attribute__((packed)) usb_vidUncompressedFrameDesc_t;


/////////////////////
// MJPEG Streaming //
/////////////////////

typedef struct
{
    uint8_t  headerLength; // size of this header
    uint8_t  header;       // bitfield header field
    uint32_t PTS;          // presentation time stamp
    uint32_t SCR_31_0;     // source clock reference bit 31-0
    uint16_t SCR_47_32;    // source clock reference nit 47-32
} __attribute__((packed)) usb_vidMJPEGStreamHeader_t;

typedef struct
{
    uint8_t length;                // descriptor size (11)
    uint8_t descType;              // descriptor type (0x24)
    uint8_t descSubtype;           // subtype (6)
    uint8_t formatIndex;           // index of this Format descriptor
    uint8_t numFrameDescriptors;   // number of frame descriptors following
    uint8_t flags;                 // specifies characteristics of this format
    uint8_t defaultFrameIndex;     // optimum frame index (used to select resolution)
    uint8_t aspectRatioX;          // X dimension of the picture aspect ratio
    uint8_t aspectRatioY;          // Y dimension of the picture aspect ratio
    uint8_t interlaceFlags;        // specifies interlace information
    bool    copyprotectInfo;       // copy protect information
} __attribute__((packed)) usb_vidMJPEGFormatDesc_t;

typedef struct
{
    uint8_t  length;               // descriptor size
    uint8_t  descType;             // descriptor type (0x24)
    uint8_t  descSubtype;          // descriptor subtype (7)
    uint8_t  frameIndex;           // index of this Frame Descriptor
    uint8_t  capabilites;          // capabilities bitmap
    uint16_t width;                // width
    uint16_t height;               // height
    uint32_t minBitRate;           // min bitrate at default compression
    uint32_t maxBitRate;           // max bitrate at default compression
    uint32_t maxVideoFrameBufSize; // deprecated
    uint32_t defaultFrameInterval; // default frame interval
    uint8_t  frameIntervalType;    // number of supported frame intervals (continuous or fixed)
    uint32_t frameArray[6];        // frame interval array or min/max/step
} __attribute__((packed)) usb_vidMJPEGFrameDesc_t;

typedef struct
{
    uint8_t length;             // descriptor size (6)
    uint8_t descType;           // descriptor type (0x24)
    uint8_t descSubtype;        // subtype (13)
    uint8_t colorPrimaries;     // color primaries and reference white
    uint8_t transferChar;       // transfer characteristics (gamma)
    uint8_t matrixCoefficients; // matrix for computing luma and chroma
} __attribute__((packed)) usb_vidColorFormatIFDesc_t;


// Video Control (VC) Interrupt Endpoint Descriptor
typedef struct
{
    uint8_t  length;               // descriptor size
    uint8_t  descType;             // descriptor type (0x25)
    uint8_t  descSubtype;          // descriptor subtype (3)
    uint16_t maxTransferSize;      // 32-byte status packet
} __attribute__((packed)) usb_vidInterruptEndpoint_t;

// usb video frame
typedef struct
{
    uint8_t  frameIndex;           // index of this Frame Descriptor
    uint8_t  capabilites;          // capabilities bitmap
    uint16_t width;                // width
    uint16_t height;               // height
    uint32_t minBitRate;           // min bitrate at default compression
    uint32_t maxBitRate;           // max bitrate at default compression
    uint32_t maxVideoFrameBufSize; // deprecated
    uint32_t defaultFrameInterval; // default frame interval
    uint8_t  frameIntervalType;    // number of supported frame intervals (0: continuous, 1...255: fixed, number of discrete frame intervals supported)
    uint32_t frameArray[6];        // frame interval array or min/max/step
                                   // frame interval is the average display time of a single frame in 100ns units
} usb_vidFrame_t;

// usb video device interface
typedef struct
{
    usb_interface_t*        interface;
    usb_endpoint_t*         endpointIsochronous[16];
    usb_endpoint_t*         endpointInterrupt;
    usb_endpoint_t*         endpointBulk;
    uint8_t                 endpointNumberStill;
    uint8_t                 countIsoEP;
    uint8_t                 firstVSInterfaceNumber; // from VC Header
    uint8_t                 cameraTerminalID;
    uint8_t                 uncompressedFormatIndex; // from VS_FORMAT_UNCOMPRESSED
    uint8_t                 defaultFrameIndex; // from VS_FORMAT_UNCOMPRESSED
    uint8_t                 highestFrameIndex; // from all VS_FRAME
    uint8_t                 frameIndex;    // from VS_FRAME_UNCOMPRESSED
    uint32_t                frameInterval; // VS_FRAME_UNCOMPRESSED
    uint16_t                keyFrameRate;  // VS_FRAME_UNCOMPRESSED
    uint16_t                PFrameRate;
    uint16_t                compQuality;
    uint16_t                compWindowSize;
    uint32_t                maxVideoFrameSize;
    uint32_t                maxPayloadTransferSize;
    uint32_t                clockFrequency;
    usb_vidFrame_t          frame[20];
    uint16_t                width;
    uint16_t                height;
    uint8_t                 MJPEGFormatIndex;
    uint8_t                 MJPEGdefaultFrameIndex;
} usb_video_t;


void usb_setupVideo(usb_interface_t* interface);
void usb_destroyVideo(usb_interface_t* interface);


#endif
