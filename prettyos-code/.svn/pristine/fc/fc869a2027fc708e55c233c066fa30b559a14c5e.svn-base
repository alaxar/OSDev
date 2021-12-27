/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "usb_video.h"
#include "video/videomanager.h"
#include "video/videoutils.h"
#include "usb/usb.h"
#include "usb/usb_hc.h"
#include "kheap.h"
#include "video/console.h"
#include "timer.h"
#include "storage/devicemanager.h"
#include "serial.h"

#define STILLPROBE
#define STILL
#define POWER
#define SCANMODE
#define ERRORREQ
//#define VS_ERRORREQ

#define PREFERRED_WIDTH     160
#define DELAY_AFTER_START  2000

static void parseTotalConfigDescriptor(usb_interface_t* interface);
static void usb_vidshowStreamingControlParameterBlock(usb_vidStreamingControlParameterBlock_t* pb);

#ifdef STILLPROBE
static void usb_vidshowStillParameterBlock(usb_vidStillParameterBlock_t* pb);
static void usb_vidStillProbeControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStillParameterBlock_t* parameterBlock);
static void usb_vidStillCommitControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStillParameterBlock_t* parameterBlock);
#endif

static void usb_vidRequest(usb_device_t* device, uint8_t videoRequest, uint8_t vidControlSelector, uint8_t vidTerminalOrUnitID, uint8_t vidInterfaceID, uint16_t parameterBlockLength, void* parameterBlock);

static void usb_vidProbeControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStreamingControlParameterBlock_t* parameterBlock);
static void usb_vidCommitControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStreamingControlParameterBlock_t* parameterBlock);

#ifdef SCANMODE
static void usb_vidScanningMode(usb_video_t* video, usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte);
#endif

#ifdef STILL
static void usb_vidStillImage(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte);
#endif
static void usb_vidshowVideoHeaderDescriptor(usb_vidHeaderDesc_t* descriptor);
static void usb_vidshowInputTermDescriptor(usb_vidInputTermDesc_t* descriptor);
static void usb_vidshowSelectorUnitDescriptor(usb_vidSelectorUnitDesc_t* descriptor);
static void usb_vidshowProcessingUnitDescriptor(usb_vidProcessingUnitDesc_t* descriptor);
static void usb_vidshowUncompressedFormatDescriptor(usb_vidUncompressedFormatDesc_t* descriptor);
static void usb_vidshowUncompressedFrameDescriptor(usb_vidUncompressedFrameDesc_t* descriptor);
static void usb_vidshowStillImageFrameDescriptor(usb_vidStillImageFrameIFDesc_t* descriptor);
#ifdef POWER
static void usb_vidPowerMode(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte);
#endif

#ifdef ERRORREQ
static void usb_vidVideoControlError(usb_device_t* device, uint8_t vidInterfaceID, void* parameterByte);
#endif

#ifdef VS_ERRORREQ
static void usb_vidVideoStreamError(usb_device_t* device, uint8_t vidInterfaceID, void* parameterByte);
#endif

static void saveYUYV(const diskType_t* YUVYDest, uint8_t* YUYVbuffer, uint32_t size, uint8_t type);


void usb_setupVideo(usb_interface_t* interface)
{
    usb_video_t* video = malloc(sizeof(usb_video_t), 0, "usb_video_t");
    interface->data = video;
    video->interface = interface;

    serial_log(SER_LOG_EHCI_ITD, "\n\nvideo interface number: %u", video->interface->descriptor.interfaceNumber);

    textColor(HEADLINE);
    serial_log(SER_LOG_EHCI_ITD, "\n\nSetup Video...");
    textColor(TEXT);

    parseTotalConfigDescriptor(interface);

    // isochronous endpoints
    video->countIsoEP = 0;

    serial_log(SER_LOG_EHCI_ITD, "\n\nEndpoints of video device:");
    serial_log(SER_LOG_EHCI_ITD, "--------------------------");

    for (dlelement_t* el = (&interface->device->endpoints)->head; el; el = el->next)
    {
        usb_endpoint_t* ep = el->data;
        serial_log(SER_LOG_EHCI_ITD, "\naddr: %u type: %u dir: %u mps: %u interval: %u", ep->address, ep->type, ep->dir, ep->mps, ep->interval);

        switch (ep->type)
        {
            case EP_CONTROL:
                serial_log(SER_LOG_EHCI_ITD, "  - ep Control");
            case EP_ISOCHRONOUS:
                video->endpointIsochronous[video->countIsoEP] = ep;
                serial_log(SER_LOG_EHCI_ITD, "  - ep Iso: %u  i: \t%u mps: %u", video->endpointIsochronous[video->countIsoEP]->address, video->countIsoEP, video->endpointIsochronous[video->countIsoEP]->mps);
                video->countIsoEP++; // number of isochronous endpoints
                break;
            case EP_INTERRUPT:
                video->endpointInterrupt = ep;
                serial_log(SER_LOG_EHCI_ITD, "  - ep Interrupt: %u", video->endpointInterrupt->address);
                break;
            case EP_BULK:
                video->endpointBulk = ep;
                serial_log(SER_LOG_EHCI_ITD, "  - ep Bulk: %u", video->endpointBulk->address);
                break;
        }
    }

    serial_log(SER_LOG_EHCI_ITD, "\n\nClock frequency of the webcam: %u", video->clockFrequency);

    /////////////////
    // Preparation //
    /////////////////

    serial_log(SER_LOG_EHCI_ITD, "\n\nVideo Streaming, Probe & Commit Control:");

  #ifdef ERRORREQ
    uint8_t parameterByteError = 42;
  #endif

  #ifdef VS_ERRORREQ
    uint8_t parameterByteVSError = 42;
  #endif

    usb_vidStreamingControlParameterBlock_t vs_pb = {0};

    ///////////////////////////////////////////////
    // Retrieve range information and set values //
    ///////////////////////////////////////////////

    // Probe DEFAULT
    usb_vidProbeControl(interface->device, VID_GET_DEF, video->firstVSInterfaceNumber, &vs_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidshowStreamingControlParameterBlock(&vs_pb);


    // Select values
    vs_pb.formatIndex = video->uncompressedFormatIndex;

    uint16_t PreferredWidth = PREFERRED_WIDTH; // <=============== preferred Width

    for (uint8_t i=0; i<video->highestFrameIndex; i++)
    {
        if (video->frame[i].width == PreferredWidth)
        {
            video->width  = video->frame[i].width;
            video->height = video->frame[i].height;
            vs_pb.frameIndex = video->frameIndex = video->frame[i].frameIndex;

            if (video->frame[i].frameIntervalType == 0)
            {
                vs_pb.frameInterval = video->frameInterval = video->frame[i].defaultFrameInterval;
            }
            else
            {
                //vs_pb.frameInterval = video->frameInterval = video->frame[i].defaultFrameInterval;
                vs_pb.frameInterval = video->frameInterval = video->frame[i].frameArray[(video->frame[i].frameIntervalType)-1];
            }

            vs_pb.maxVideoFrameSize = video->maxVideoFrameSize = video->frame[i].maxVideoFrameBufSize;
            break;
        }
    }

    vs_pb.keyFrameRate  = 0;
    vs_pb.maxPayloadTransferSize = 0;
    // vs_pb.delay = ...; // cf. USB_Video_FAQ_1.5: always set by device
    // vs_pb.usage = ...;
    // vs_pb.settings = ...;
    // vs_pb.rateControlModes = ...;
    // vs_pb.preferredVersion = ...;
    // vs_pb.minVersion = ...;
    // vs_pb.maxVersion = ...;
    // vs_pb.maxNumberOfRefFramesPlus1 = ...;
    // vs_pb.layoutPerStream = ...;
    // vs_pb.framingInfo = ...;
    // vs_pb.clockFrequency = ...;
    vs_pb.PFrameRate   = 0;
    vs_pb.compQuality = 0;
    vs_pb.compWindowSize = 0;

    vs_pb.hint_frameInterval    = 1;
    vs_pb.hint_keyFrameRate     = 0;
    vs_pb.hint_PFrameRate       = 0;
    vs_pb.hint_compQuality      = 0;
    vs_pb.hint_compWindowSize   = 0;

    // Probe SET
    usb_vidProbeControl(interface->device, VID_SET_CUR, video->firstVSInterfaceNumber, &vs_pb);
    //sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif


    // Probe GET
    usb_vidProbeControl(interface->device, VID_GET_CUR, video->firstVSInterfaceNumber, &vs_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidshowStreamingControlParameterBlock(&vs_pb);

    // Save KeyFrameRate, PFrameRate, CompQuality, CompWindowSize, MaxVideoFrameSize,
    // Delay and MaxPayloadTransferSize fields returned from device.
    video->keyFrameRate             = vs_pb.keyFrameRate;
    video->PFrameRate               = vs_pb.PFrameRate;
    video->compQuality              = vs_pb.compQuality;
    video->compWindowSize           = vs_pb.compWindowSize;
    video->maxVideoFrameSize        = vs_pb.maxVideoFrameSize;
    video->maxPayloadTransferSize   = vs_pb.maxPayloadTransferSize; // maximum number of bytes that the device can transmit or receive in a single payload transfer.

    // Commit SET
    usb_vidCommitControl(interface->device, VID_SET_CUR, video->firstVSInterfaceNumber, &vs_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    // Commit GET /// TEST
    usb_vidCommitControl(interface->device, VID_GET_CUR, video->firstVSInterfaceNumber, &vs_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidshowStreamingControlParameterBlock(&vs_pb);

  #ifdef STILLPROBE
    // Negotiate StillParameterblock
    usb_vidStillParameterBlock_t still_pb = {0};
    still_pb.formatIndex = video->uncompressedFormatIndex;
    still_pb.frameIndex  = video->frameIndex;
    // still_pb->... = ...;

    // StillProbe SET
    usb_vidStillProbeControl(interface->device, VID_SET_CUR, video->firstVSInterfaceNumber, &still_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    // StillProbe GET
    usb_vidStillProbeControl(interface->device, VID_GET_CUR, video->firstVSInterfaceNumber, &still_pb);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif
    usb_vidshowStillParameterBlock(&still_pb);

    usb_vidStillCommitControl(interface->device, VID_SET_CUR, video->firstVSInterfaceNumber, &still_pb);
    sleepMilliSeconds(10);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif
  #endif

    /////////////////////////////////////
    // select endpoint with enough mps //
    /////////////////////////////////////

    uint8_t numIdealEP = 0;
    //numIdealEP = video->countIsoEP - 1; //<== just the highest one to handle burst?

    for (; numIdealEP < video->countIsoEP; numIdealEP++)
    {
        if (video->endpointIsochronous[numIdealEP]->mps >= 3072/*video->maxPayloadTransferSize*/)
        {
            break;
        }
    }

    serial_log(SER_LOG_EHCI_ITD, "\n\nSelected: Alternate Interface: %u EP mps: %u", numIdealEP + 1, video->endpointIsochronous[numIdealEP]->mps);

  #ifdef SCANMODE
    uint8_t parameterByteScanGet = 0; // Progressive = 1, Interlaced = 0
    usb_vidScanningMode(video, interface->device, VID_GET_CUR, 0, &parameterByteScanGet);
    serial_log(SER_LOG_EHCI_ITD, "\nOriginal scanning mode from camera: %u", parameterByteScanGet);
  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidScanningMode(video, interface->device, VID_GET_MIN, 0, &parameterByteScanGet);
    serial_log(SER_LOG_EHCI_ITD, "\nMIN scanning mode from camera: %u", parameterByteScanGet);
  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidScanningMode(video, interface->device, VID_GET_MAX, 0, &parameterByteScanGet);
    serial_log(SER_LOG_EHCI_ITD, "\nMAX scanning mode from camera: %u", parameterByteScanGet);
  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    uint8_t parameterByteScanSet = 1; ///TEST/// // Progressive = 1, Interlaced = 0
    usb_vidScanningMode(video, interface->device, VID_SET_CUR, 0, &parameterByteScanSet);
    sleepMilliSeconds(10);
  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif

    usb_vidScanningMode(video, interface->device, VID_GET_CUR, 0, &parameterByteScanGet);
    serial_log(SER_LOG_EHCI_ITD, "\nWe set scanning mode to %u and get: %u", parameterByteScanSet, parameterByteScanGet);
  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif
  #endif

  #ifdef POWER
    uint8_t parameterBytePower = 0;
    usb_vidPowerMode(interface->device, VID_GET_CUR, 0, &parameterBytePower);
    serial_log(SER_LOG_EHCI_ITD, "\nOriginal Power mode: %u", parameterBytePower);
    parameterBytePower = 0;
    usb_vidPowerMode(interface->device, VID_SET_CUR, 0, &parameterBytePower);
    sleepMilliSeconds(50);
    usb_vidPowerMode(interface->device, VID_GET_CUR, 0, &parameterBytePower);
    serial_log(SER_LOG_EHCI_ITD, "\nPower mode: %u", parameterBytePower);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif
  #endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Start Videostreaming!
    usb_setInterface(interface->device, numIdealEP + 1, video->firstVSInterfaceNumber); // alternate interface defines ... byte per transaction
    sleepMilliSeconds(DELAY_AFTER_START);

  #ifdef VS_ERRORREQ
    usb_vidVideoStreamError(interface->device, video->firstVSInterfaceNumber, &parameterByteVSError); ///TEST
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteVSError);
  #endif

    //////////////////////////////////////
    /*
    Still Image
    This control shall only be set while streaming is occurring,
    and the hardware shall reset it to the “Normal Operation” mode after the still image has been sent.
    */
    //////////////////////////////////////

  #ifdef STILL
    // parameterByte: 0: Normal operation, 1: Transmit still image, 2: Transmit still image via dedicated bulk pipe, 3: Abort still image transmission
    uint8_t parameterByteStill = 1;
    usb_vidStillImage(interface->device, VID_SET_CUR, video->firstVSInterfaceNumber, &parameterByteStill);

  #ifdef ERRORREQ
    usb_vidVideoControlError(interface->device, 0, &parameterByteError);
    serial_log(SER_LOG_EHCI_ITD, "  Error code: %u", parameterByteError);
  #endif
  #endif


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////  I S O C H R O N O U S   T R A N S F E R  ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint16_t times     =   12; // number of iTD in chain
    uint16_t numChains =   32; // number of iTD chains

    bool rawData        = false; // dataSize
    bool withoutZero    = false; // fileSize
    bool withoutHeader  = true; // videoSize
#ifdef STILL
    bool stillImage     = true; // stillSize
#endif

    size_t dataSize = times*numChains*8*3072; // max. payload of all iTD chains
    void* YUYVrawBuffer = malloc(dataSize, 0, "YUYV RawData");
    //serial_log(SER_LOG_EHCI_ITD, "\n\nYUYVrawBuffer: %X", YUYVrawBuffer);
    memset(YUYVrawBuffer, 0, dataSize);

    usb_IsochronousInTransfer(interface->device,
        video->endpointIsochronous[numIdealEP], // ideal EP
        times*numChains*8*3072,                 // length currently not used at isochronous transfer/transaction system
        YUYVrawBuffer, times, numChains);       // buffer for each transfer

    if (rawData)
    {
        printf("\n\nNow saving and analysing raw YUYV video data %u byte - ", dataSize);
        serial_log(SER_LOG_EHCI_ITD, "\n\nSave now raw YUYV video data %u byte - ", dataSize);
        saveYUYV(&USB_MSD, (uint8_t*)YUYVrawBuffer, dataSize, 0);
        serial_log(SER_LOG_EHCI_ITD, "DONE.");
        printf("DONE");
    }

    //Shrink video data - take 0x00000000 away (only four-byte-wise!)
    size_t fileSize = dataSize;
    void* YUYVrawWithoutZeroBuffer = malloc(dataSize, 0, "YUYV RawDataWithoutZero");
    memset(YUYVrawWithoutZeroBuffer, 0, dataSize);

    size_t pos=0;
    for (size_t i=0; i<dataSize; i+=4)
    {
        if ( (*(uint32_t*)(YUYVrawBuffer+i)) == 0x00000000) // no content
        {
            fileSize -= 4; // do not copy these four bytes
        }
        else
        {
            *(uint32_t*)(YUYVrawWithoutZeroBuffer + pos) = *(uint32_t*)(YUYVrawBuffer + i); // copy these four data bytes
            pos += 4;
        }
    }

    free(YUYVrawBuffer); // Not needed anymore

    if (withoutZero)
    {
        printf("\n\nNow saving raw YUYV video data (w/o zero) %u byte - ", fileSize);
        serial_log(SER_LOG_EHCI_ITD, "\n\nSave now raw YUYV video data (w/o zero) %u byte - ", fileSize);
        saveYUYV(&USB_MSD, (uint8_t*)YUYVrawWithoutZeroBuffer, fileSize, 1);
        serial_log(SER_LOG_EHCI_ITD, "DONE.");
        printf("DONE");
    }

  #ifdef STILL
    //Shrink video data to STILL Frame <== does not work perfect
    size_t stillSize = fileSize;
    void* YUYVstillBuffer = malloc(fileSize, 0, "YUYV stillData");
    memset(YUYVstillBuffer, 0, fileSize);

    pos=0;
    bool STIflag  = false;

    for (uint32_t i=0; i<fileSize; i+=4)
    {
        uint8_t  firstByte  = *(uint8_t*) (YUYVrawWithoutZeroBuffer+i+0); // headerLength
        uint8_t  secondByte = *(uint8_t*) (YUYVrawWithoutZeroBuffer+i+1); // BFH: bit field header

        if ((firstByte == 0X0C) && (IS_BIT_SET(secondByte, 7)) && (IS_BIT_SET(secondByte, 2))) // identifies a valid header
        {
            STIflag = IS_BIT_SET(secondByte, 5);  // identifies a valid STI header
            i+=12; // jump behind header
            stillSize -= 12;
        }

        if (STIflag)
        {
            *(uint32_t*)(YUYVstillBuffer + pos) = *(uint32_t*)(YUYVrawWithoutZeroBuffer + i); // copy these four data bytes
            pos += 4;
        }
        else
        {
            stillSize -= 4;
        }
    }

    if (stillImage)
    {
        printf("\n\nNow saving still image YUYV video data %u byte - ", stillSize);
        serial_log(SER_LOG_EHCI_ITD, "\nSave now still image YUYV video data %u byte - ", stillSize);
        saveYUYV(&USB_MSD, (uint8_t*)YUYVstillBuffer, stillSize, 2);
        serial_log(SER_LOG_EHCI_ITD, "DONE.");
        printf("DONE");
    }
  #endif

    // Analyze shrinked video data // cf. USB Video Class 1.5, chapter 2.4.3.3 "Video and Still Image Payload Headers"
    serial_log(SER_LOG_EHCI_ITD, "\nHeader Analysis of YUYVrawWithoutZeroBuffer:\n");

    //Strip the headers
    size_t videoSize = fileSize;
    void* YUYVvideoBuffer = malloc(videoSize, 0, "YUYV videoData");
    memset(YUYVvideoBuffer, 0, videoSize);

    uint8_t* old_i = YUYVrawWithoutZeroBuffer;
    pos=0;

    for (uint8_t* i=YUYVrawWithoutZeroBuffer; (uintptr_t)i < ((uintptr_t)YUYVrawWithoutZeroBuffer+fileSize); i+=4)
    {
        uint8_t  firstByte      = *(uint8_t*) (i+0); // headerLength
        uint8_t  secondByte     = *(uint8_t*) (i+1); // BFH: bit field header
        uint32_t PTS            = *(uint32_t*)(i+2); // PTS: Presentation Time Stamp: start of raw frame capture (native device clock units)
        uint32_t SCRticks       = *(uint32_t*)(i+6); // SCR: bits 31-0  (native device clock units)
        uint16_t SCRframeNum    = *(uint16_t*)(i+10);// SCR: bits 42-32 (native device clock units)


        // Bit field header field BFH[0]: EOH ERR STI RES SCR PTS EOF FID
        if ((firstByte == 12)                // HLE: Header Length (12)
            && (IS_BIT_SET(secondByte,  2))  // PTS: Presentation Time Stamp // TODO: not every cam must have this feature ??
            && (IS_BIT_SET(secondByte,  3))  // SCR: Source Clock Reference  // TODO: not every cam must have this feature ??
            && (!IS_BIT_SET(secondByte, 4))  // RES: Reserved // Payload specific bit. See individual payload specifications for use.
            //&& (!IS_BIT_SET(secondByte, 6))  // ERR: Error Bit               // TODO: what if error happens?
            //&& (IS_BIT_SET(secondByte,  7)) // EOH: End of Header
            )
        {   // identifies a header with EOH bit set, incl. PTS and SCR, w/o reserved bit and w/o error bit
            size_t delta_i = i - old_i;
            old_i = i;
            static uint32_t n=1;
            serial_log(SER_LOG_EHCI_ITD, "\nnr: %u i:%u \tFID: %u ", n, delta_i, secondByte & BIT(0));

            if (IS_BIT_SET(secondByte, 2)) // PTS: Presentation Time Stamp // native device clock units when the raw frame capture begins.
            {
                // FID: Frame Identifier toggles at each frame start boundary and stays constant for the rest of the frame
                serial_log(SER_LOG_EHCI_ITD, "PTS: %u ", PTS);
            }
            if (IS_BIT_SET(secondByte, 3)) // SCR: Source Clock Reference
            {
                static uint32_t oldSCR = 0;
                uint32_t newSCR = SCRticks;
                uint32_t cF = video->clockFrequency / 1000000; // MHz
                serial_log(SER_LOG_EHCI_ITD, "SCR: %u delta: %u µs \tnum: %u \t", newSCR/cF, ((int32_t)(newSCR-oldSCR))/cF, SCRframeNum);
                oldSCR = newSCR;
            }
            if (IS_BIT_SET(secondByte, 1)) // EOF: End of Frame // last header group in the packet
            {
                serial_log(SER_LOG_EHCI_ITD, "EOF ");
            }
            if (IS_BIT_SET(secondByte, 5)) // STI: Still Image // only used for methods 2 and 3 of still image capture
            {
                serial_log(SER_LOG_EHCI_ITD, "STI ");
            }
            if (IS_BIT_SET(secondByte, 6)) // ERR: Error Bit // The Stream Error Code control would reflect the cause of the error
            {
                serial_log(SER_LOG_EHCI_ITD, "ERR ");
            }
            if (IS_BIT_SET(secondByte, 4)) // Reserved Bit // Payload specific bit. See individual payload specifications for use.
            {
                serial_log(SER_LOG_EHCI_ITD, "RES");
            }

            i+=8; // correct i+=4 for header length (12)
            n++;
            videoSize -= 12;
        }
        else
        {
            // YUYV video data
            *(uint32_t*)(YUYVvideoBuffer + pos) = *(uint32_t*)i; // copy these four data bytes (2 macropixels with YUV2 packed format)
            pos += 4;
        }
    }

    if (withoutHeader)
    {
        printf("\n\nNow saving YUYV video data (w/o zero and w/o header) %u byte - ", videoSize);
        serial_log(SER_LOG_EHCI_ITD, "\n\nSave now YUYV video data  (w/o zero and w/o header) %u byte - ", videoSize);
        saveYUYV(&USB_MSD, (uint8_t*)YUYVvideoBuffer, videoSize, 3); // for the viewer
        serial_log(SER_LOG_EHCI_ITD, "DONE.");
        printf("DONE");
    }

    // show video data with vbe/vm86
    video_install();
    list_t modelist = list_init();
    video_createModeList(&modelist);
    videoMode_t* mode = 0;
    uint16_t modenumber = 22; // 640 * 480, 32 bit
    dlelement_t* e = list_getElement(&modelist, modenumber);
    if (e != 0)
    {
        mode = e->data;
    }
    video_setMode(mode);

    serial_log(SER_LOG_EHCI_ITD, "\n\nvideo->width: %u video->height: %u", video->width, video->height);
    serial_log(SER_LOG_EHCI_ITD, "\n\nfile sizes: with header: %u  w/o header %u  ", fileSize, videoSize);
  #ifdef STILL
    serial_log(SER_LOG_EHCI_ITD, "still image: %u", stillSize);
  #endif

    video_clearScreen(video_currentMode->device, black);
    showYUYV(video_currentMode->device, video->height, video->width, YUYVrawWithoutZeroBuffer, fileSize);
    waitForKeyStroke();

    video_clearScreen(video_currentMode->device, black);
    showYUYV(video_currentMode->device, video->height, video->width, YUYVvideoBuffer, videoSize);
    waitForKeyStroke();
  #ifdef STILL
    video_clearScreen(video_currentMode->device, black);
    showYUYV(video_currentMode->device, video->height, video->width, YUYVstillBuffer, stillSize);
    waitForKeyStroke();
  #endif

    video_setMode(0); // Return to 80x50 text mode

    free(YUYVrawWithoutZeroBuffer);
    free(YUYVvideoBuffer);
  #ifdef STILL
    free(YUYVstillBuffer);
  #endif
}

void usb_destroyVideo(usb_interface_t* interface)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nDestroy Video Device Interface: %X", interface->data);
    free(interface->data);
}

// Video class-specific requests
static void usb_vidRequest(usb_device_t* device,
                           uint8_t videoRequest,
                           uint8_t vidControlSelector,    // CS
                           uint8_t vidTerminalOrUnitID ,  // virtual entity "interface" = 0
                           uint8_t vidInterfaceID,        // VC = 0, VS = 1, ..., n
                           uint16_t parameterBlockLength,
                           void* parameterBlock)
{
    // requestType GET: 0xA1, SET: 0x21
    switch(videoRequest)
    {
        case VID_SET_CUR:                                           //hiVal           loVal  hiIndex                    loIndex
            usb_controlOut(device, parameterBlock, 0x21, VID_SET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        case VID_GET_CUR:
            usb_controlIn(device, parameterBlock, 0xA1, VID_GET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        case VID_GET_MIN:
            usb_controlIn(device, parameterBlock, 0xA1, VID_GET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        case VID_GET_MAX:
            usb_controlIn(device, parameterBlock, 0xA1, VID_GET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        case VID_GET_RES:
            usb_controlIn(device, parameterBlock, 0xA1, VID_GET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        case VID_GET_INFO:
            usb_controlIn(device, parameterBlock, 0xA1, VID_GET_CUR, vidControlSelector, 0, vidTerminalOrUnitID << 8 | vidInterfaceID, parameterBlockLength);
            break;
        }
}

static void usb_vidProbeControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStreamingControlParameterBlock_t* parameterBlock)
{
    printf("\n\nProbeControl videoRequest: ");
    serial_log(SER_LOG_EHCI_ITD, "\n\nProbeControl videoRequest: ");

    switch(videoRequest)
    {
    case VID_GET_CUR:
        printf("GET_CUR");
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
        break;
    case VID_SET_CUR:
        printf("SET_CUR");
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
        break;
    case VID_GET_MIN:
        printf("GET_MIN");
        serial_log(SER_LOG_EHCI_ITD, "GET_MIN");
        break;
    case VID_GET_MAX:
        printf("GET_MAX");
        serial_log(SER_LOG_EHCI_ITD, "GET_MAX");
        break;
    case VID_GET_DEF:
        printf("GET_DEF");
        serial_log(SER_LOG_EHCI_ITD, "GET_DEF");
        break;
    }

    usb_vidRequest(device, videoRequest, VS_CS_PROBE_CONTROL, 0, vidInterfaceID, 48, parameterBlock);
}

static void usb_vidCommitControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID,  usb_vidStreamingControlParameterBlock_t* parameterBlock)
{
    printf("\n\nCommitControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nCommitControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
    usb_vidRequest(device, videoRequest, VS_CS_COMMIT_CONTROL, 0, vidInterfaceID, 48, parameterBlock);
}

#ifdef STILLPROBE
static void usb_vidStillProbeControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStillParameterBlock_t* parameterBlock)
{
    printf("\n\nStillProbeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillProbeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
    usb_vidRequest(device, videoRequest, VS_CS_STILL_PROBE_CONTROL, 0, vidInterfaceID, 11, parameterBlock);
}

static void usb_vidStillCommitControl(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, usb_vidStillParameterBlock_t* parameterBlock)
{
    printf("\n\nStillCommitControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillCommitControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
    usb_vidRequest(device, videoRequest, VS_CS_STILL_COMMIT_CONTROL, 0, vidInterfaceID, 11, parameterBlock);
}
#endif

#ifdef STILL
static void usb_vidStillImage(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte)
{
    printf("\n\nStillImage videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillImage videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillImage parameter byte: %u", *(uint8_t*)parameterByte);
    usb_vidRequest(device, videoRequest, VS_CS_STILL_IMAGE_TRIGGER_CONTROL, 0, vidInterfaceID, 1, parameterByte);
}
#endif

#ifdef POWER
static void usb_vidPowerMode(usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte)
{
    printf("\n\nPowerModeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nPowerModeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");
    usb_vidRequest(device, videoRequest, VC_VIDEO_POWER_MODE_CONTROL, 0, vidInterfaceID, 1, parameterByte);
}
#endif

#ifdef ERRORREQ
static void usb_vidVideoControlError(usb_device_t* device, uint8_t vidInterfaceID, void* parameterByte) //read-only
{
    printf("\n\nErrorVideoControl: ");
    serial_log(SER_LOG_EHCI_ITD, "\n\nErrorVideoControl: ");

    //usb_vidRequest(device, videoRequest, VC_REQUEST_ERROR_CODE_CONTROL, VC_SELECTOR_UNIT, vidInterfaceID, 1, parameterByte);
    usb_vidRequest(device, VID_GET_CUR, VC_REQUEST_ERROR_CODE_CONTROL, 0, vidInterfaceID, 1, parameterByte);
    uint8_t err = *(uint8_t*)parameterByte;

    // UVC spec 1.5:  4.2.1.2 Request Error Code Control

    // Not ready: The device has not completed a previous operation.
    //            The device will recover from this state as soon as the previous operation has completed.

    // Wrong State: The device is in a state that disallows the specific request.
    //              The device will remain in this state until a specific action from the host or the user is completed.

    // Power: The actual Power Mode of the device is not sufficient to complete the Request.

    // Out of Range: Result of a SET_CUR Request when attempting to set a value outside of the MIN and MAX range,
    //               or a value that does not satisfy the constraint on resolution (see section 4.2.2, “Unit and Terminal Control Requests”).

    // Invalid Unit: The Unit ID addressed in this Request is not assigned.

    // Invalid Control: The Control addressed by this Request is not supported.

    // Invalid Request: This Request is not supported by the Control.

    // Invalid value with range: Results of a SET_CUR Request when attempting to set a value that is inside the MIN and MAX range
    //                           but is not supported.

    if (err == 0)
        textColor(SUCCESS);
    else
        textColor(ERROR);
    static const char* errors[] =
    {
        "\nNot ready",
        "\nWrong state",
        "\nPower mode not sufficient",
        "\nOut of MIN and MAX range",
        "\nInvalid unit",
        "\nInvalid control",
        "\nInvalid request",
        "\nInvalid value within range",
    };
    if (err < 8)
    {
        puts(errors[err]);
        serial_log(SER_LOG_EHCI_ITD, errors[err]);
    }
    else
    {
        puts("\nReserved or unknown");
        serial_log(SER_LOG_EHCI_ITD, "\nReserved or unknown");
    }
    textColor(TEXT);

    serial_log(SER_LOG_EHCI_ITD, " <<<<< <<<<< <<<<< <<<<< <<<<< <<<<< <<<<< <<<<<");
}

#endif

#ifdef VS_ERRORREQ
static void usb_vidVideoStreamError(usb_device_t* device, uint8_t vidInterfaceID, void* parameterByte) // read-only
{
    printf("\n\nErrorVideoStream: ");
    serial_log(SER_LOG_EHCI_ITD, "\n\nErrorVideoStream: ");

    //usb_vidRequest(device, videoRequest, VC_REQUEST_ERROR_CODE_CONTROL, VC_SELECTOR_UNIT, vidInterfaceID, 1, parameterByte);
    usb_vidRequest(device, VID_GET_CUR, VS_CS_STREAM_ERROR_COE_CONTROL, 0, vidInterfaceID, 1, parameterByte);
    uint8_t err = *(uint8_t*) parameterByte;


    // UVC spec 1.5:  4.3.1.7 Stream Error Code Control

    switch (err)
    {
        case 0:  serial_log(SER_LOG_EHCI_ITD, "\nNo error");                    break;
        case 1:  serial_log(SER_LOG_EHCI_ITD, "\nProtected content");           break;
        case 2:  serial_log(SER_LOG_EHCI_ITD, "\nInput buffer underrun");       break;
        case 3:  serial_log(SER_LOG_EHCI_ITD, "\nData discontinuity");          break;
        case 4:  serial_log(SER_LOG_EHCI_ITD, "\nOutput buffer underrun");      break;
        case 5:  serial_log(SER_LOG_EHCI_ITD, "\nOutput buffer overrun");       break;
        case 6:  serial_log(SER_LOG_EHCI_ITD, "\nFormat change");               break;
        case 7:  serial_log(SER_LOG_EHCI_ITD, "\nStill image capture error");   break;
        default: serial_log(SER_LOG_EHCI_ITD, "\nReserved or unknown");         break;
    }
    serial_log(SER_LOG_EHCI_ITD, " <<<<< <<<<< <<<<< <<<<< <<<<< <<<<< <<<<< <<<<<");

    switch (err)
    {
        case 0:  textColor(SUCCESS); printf("\nNo error"); textColor(TEXT);     break;
        case 1:  printfe("\nProtected content");                                break;
        case 2:  printfe("\nInput buffer underrun");                            break;
        case 3:  printfe("\nData discontinuity");                               break;
        case 4:  printfe("\nOutput buffer underrun");                           break;
        case 5:  printfe("\nOutput buffer overrun");                            break;
        case 6:  printfe("\nFormat change");                                    break;
        case 7:  printfe("\nStill image capture error");                        break;
        default: printfe("\nReserved or unknown");                              break;
    }
}
#endif

#ifdef SCANMODE
static void usb_vidScanningMode(usb_video_t* video, usb_device_t* device, uint8_t videoRequest, uint8_t vidInterfaceID, void* parameterByte) //TODO: variable Terminal ID
{
    printf("\n\nScanningModeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        printf("GET_CUR");
    if (videoRequest == VID_SET_CUR)
        printf("SET_CUR");
    serial_log(SER_LOG_EHCI_ITD, "\n\nScanningModeControl videoRequest: ");
    if (videoRequest == VID_GET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "GET_CUR");
    if (videoRequest == VID_SET_CUR)
        serial_log(SER_LOG_EHCI_ITD, "SET_CUR");

    usb_vidRequest(device, videoRequest,
                   VC_CS_SCANNING_MODE_CONTROL, 0, // wValue
                   video->cameraTerminalID << 8 /*cam terminal ID from Input Terminal Desc.*/ | vidInterfaceID, // wIndex
                   1, parameterByte);
}
#endif


// Analysis functions

static void usb_vidshowStreamingControlParameterBlock(usb_vidStreamingControlParameterBlock_t* pb)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nStreamingControlParameterBlock");
    serial_log(SER_LOG_EHCI_ITD, "\n------------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nformatIndex:              %u", pb->formatIndex);
    serial_log(SER_LOG_EHCI_ITD, "\nframeIndex:               %u", pb->frameIndex);
    serial_log(SER_LOG_EHCI_ITD, "\nframeInterval:            %u", pb->frameInterval);
    serial_log(SER_LOG_EHCI_ITD, "\nmaxVideoFrameSize:        %u", pb->maxVideoFrameSize);
    serial_log(SER_LOG_EHCI_ITD, "\nmaxPayloadTransferSize:   %u", pb->maxPayloadTransferSize);
    serial_log(SER_LOG_EHCI_ITD, "\ndelay:                    %u", pb->delay);
    serial_log(SER_LOG_EHCI_ITD, "\nkeyFrameRate:             %u", pb->keyFrameRate);
    serial_log(SER_LOG_EHCI_ITD, "\nPFrameRate:               %u", pb->PFrameRate);
    serial_log(SER_LOG_EHCI_ITD, "\nCompQuality:              %u", pb->compQuality);
    serial_log(SER_LOG_EHCI_ITD, "\nCompWindowSize:           %u", pb->compWindowSize);
}

#ifdef STILLPROBE
static void usb_vidshowStillParameterBlock(usb_vidStillParameterBlock_t* pb)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillParameterBlock");
    serial_log(SER_LOG_EHCI_ITD, "\n------------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nformatIndex:              %u", pb->formatIndex);
    serial_log(SER_LOG_EHCI_ITD, "\nframeIndex:               %u", pb->frameIndex);
    serial_log(SER_LOG_EHCI_ITD, "\ncompressionIndex:         %u", pb->compressionIndex);
    serial_log(SER_LOG_EHCI_ITD, "\nmaxVideoFrameSize:        %u", pb->maxVideoFrameSize);
    serial_log(SER_LOG_EHCI_ITD, "\nmaxPayloadTransferSize:   %u", pb->maxPayloadTransferSize);
}
#endif

//////////////////////////////
// Video Control

static void usb_vidshowVideoHeaderDescriptor(usb_vidHeaderDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nVideoControlHeaderDescriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n----------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nclass specification: %u.%u", descriptor->cdUVCHi, descriptor->cdUVCLo);
    //serial_log(SER_LOG_EHCI_ITD, "\nTotal size of class-specific descriptors: %u", descriptor->totalLength);
    serial_log(SER_LOG_EHCI_ITD, "\nClock Frequency: %u Hz", descriptor->clockFreq);
    serial_log(SER_LOG_EHCI_ITD, "\nnr. of streaming interfaces: %u", descriptor->inCollection);
    serial_log(SER_LOG_EHCI_ITD, "\tVideoStreaming interface: %u", descriptor->interfaceNr);
}

static void usb_vidshowInputTermDescriptor(usb_vidInputTermDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nInput Terminal Descriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n--------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nInput Terminal ID: %xh", descriptor->terminalID);
    serial_log(SER_LOG_EHCI_ITD, "\nInput Terminal Type: %xh (camera sensor: 201h)", descriptor->terminalType);
    serial_log(SER_LOG_EHCI_ITD, "\nassociated terminal ID: %xh", descriptor->associatedTerminal);
    serial_log(SER_LOG_EHCI_ITD, "\nterminal: %xh (string desc.)", descriptor->terminal);

    for (uint8_t i=0; i<10; i+=2)
    {
        serial_log(SER_LOG_EHCI_ITD, "\nextra byte %u and %u: %xh %xh", i , i+1, descriptor->bytes[i], descriptor->bytes[i+1]);
    }
}

static void usb_vidshowSelectorUnitDescriptor(usb_vidSelectorUnitDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nSelector Unit Descriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n-------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\n...");
}

static void usb_vidshowProcessingUnitDescriptor(usb_vidProcessingUnitDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nProcessing Unit Descriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n---------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\ncontrol size: %u", descriptor->controlSize);

    // Control is supported for the video stream
    if (descriptor->controls & BIT(0))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Brightness");
    if (descriptor->controls & BIT(1))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Contrast");
    if (descriptor->controls & BIT(2))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Hue");
    if (descriptor->controls & BIT(3))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Saturation");
    if (descriptor->controls & BIT(4))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Sharpness");
    if (descriptor->controls & BIT(5))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Gamma");
    if (descriptor->controls & BIT(6))  serial_log(SER_LOG_EHCI_ITD, "\nPU - White Balance Temperature");
    if (descriptor->controls & BIT(7))  serial_log(SER_LOG_EHCI_ITD, "\nPU - White Balance Component");
    if (descriptor->controls & BIT(8))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Backlight Compensation");
    if (descriptor->controls & BIT(9))  serial_log(SER_LOG_EHCI_ITD, "\nPU - Gain");
    if (descriptor->controls & BIT(10)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Power Line Frequency");
    if (descriptor->controls & BIT(11)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Hue, Auto");
    if (descriptor->controls & BIT(12)) serial_log(SER_LOG_EHCI_ITD, "\nPU - White Balance Temperature, Auto");
    if (descriptor->controls & BIT(13)) serial_log(SER_LOG_EHCI_ITD, "\nPU - White Balance Component, Auto");
    if (descriptor->controls & BIT(14)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Digital Multiplier");
    if (descriptor->controls & BIT(15)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Digital Multiplier Limit");
    if (descriptor->controls & BIT(16)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Analog Video Standard");
    if (descriptor->controls & BIT(17)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Analog Video Lock Status");
    if (descriptor->controls & BIT(18)) serial_log(SER_LOG_EHCI_ITD, "\nPU - Contrast, Auto");
}

//////////////////////////////
// Video Streaming

static void usb_vidshowUncompressedFormatDescriptor(usb_vidUncompressedFormatDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nUncompressed Format Descriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n---------------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nFormat Index: %u", descriptor->formatIndex);
    serial_log(SER_LOG_EHCI_ITD, "\tNumber of frame descriptors: %u", descriptor->numFrameDescriptors);

    serial_log(SER_LOG_EHCI_ITD, "\nGUID: ");
    serial_log(SER_LOG_EHCI_ITD, "%X", descriptor->guidFormat_Data1);
    serial_log(SER_LOG_EHCI_ITD, "-");
    serial_log(SER_LOG_EHCI_ITD, "%x", descriptor->guidFormat_Data2);
    serial_log(SER_LOG_EHCI_ITD, "-");
    serial_log(SER_LOG_EHCI_ITD, "%x", descriptor->guidFormat_Data3);
    serial_log(SER_LOG_EHCI_ITD, "-");
    for (uint8_t i=0; i<8; i++)
    {
        if (i==2)
            serial_log(SER_LOG_EHCI_ITD, "-");
        serial_log(SER_LOG_EHCI_ITD, "%y", descriptor->guidFormat_Data4[i]);
    }

    serial_log(SER_LOG_EHCI_ITD, "\nBits Per Pixel: %u", descriptor->BitsPerPixel);
    serial_log(SER_LOG_EHCI_ITD, "\tDefault Frame Index: %u", descriptor->defaultFrameIndex);
    serial_log(SER_LOG_EHCI_ITD, "\nPicture aspect ratio X: %u", descriptor->aspectRatioX);
    serial_log(SER_LOG_EHCI_ITD, "\tPicture aspect ratio Y: %u", descriptor->aspectRatioY);
    serial_log(SER_LOG_EHCI_ITD, "\nInterlace Info: %u", descriptor->interlaceFlags);
}

static void usb_vidshowUncompressedFrameDescriptor(usb_vidUncompressedFrameDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nUncompressed Frame Descriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n--------------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nframeIndex: %u",              descriptor->frameIndex);
    serial_log(SER_LOG_EHCI_ITD, "\tcapabilites: %yh",            descriptor->capabilites);
    serial_log(SER_LOG_EHCI_ITD, "\nwidth: %u",                   descriptor->width);
    serial_log(SER_LOG_EHCI_ITD, "\theight: %u  ",                descriptor->height);
    serial_log(SER_LOG_EHCI_ITD, "\nminBitRate: %u  ",            descriptor->minBitRate);
    serial_log(SER_LOG_EHCI_ITD, "\tmaxBitRate: %u  ",            descriptor->maxBitRate);
    serial_log(SER_LOG_EHCI_ITD, "\ndefaultFrameInterval: %u  ",  descriptor->defaultFrameInterval);
    serial_log(SER_LOG_EHCI_ITD, "\tframeIntervalType: %u  ",     descriptor->frameIntervalType);
    serial_log(SER_LOG_EHCI_ITD, "\nmaxVideoFrameBufSize: %u",    descriptor->maxVideoFrameBufSize);

    serial_log(SER_LOG_EHCI_ITD, "\nframeInterval: (in 100 ns) ");
    for (uint8_t i=0; i<descriptor->frameIntervalType; i++)
    {
        serial_log(SER_LOG_EHCI_ITD, "\t[%u]: %u", i, descriptor->frameArray[i]);
    }
}

static void usb_vidshowStillImageFrameDescriptor(usb_vidStillImageFrameIFDesc_t* descriptor)
{
    serial_log(SER_LOG_EHCI_ITD, "\n\nStillImageFrameDescriptor:");
    serial_log(SER_LOG_EHCI_ITD, "\n--------------------------");
    serial_log(SER_LOG_EHCI_ITD, "\nendpoint addr: %u",  descriptor->endpointAddress);
    serial_log(SER_LOG_EHCI_ITD, "\nnumber of image size patterns of this format:  %u", descriptor->numImageSizePatterns);
    serial_log(SER_LOG_EHCI_ITD, "\nnumber of compression patterns of this format: %u", descriptor->numCompressionPatterns);
}



static void parseTotalConfigDescriptor(usb_interface_t* interface)
{
    usb_video_t* video = (usb_video_t*)(interface->data);
    void* addr  = interface->device->totalConfigDescriptor;
    void* lastByte = addr + (*(uint16_t*)(addr+2)); // totalLength (WORD)
    serial_log(SER_LOG_EHCI_ITD, "\nconfig descriptor - total length: %u", lastByte - addr);

    // Analyze descriptors
    while ((uintptr_t)addr < (uintptr_t)lastByte)
    {
        uint8_t length  = *(uint8_t*)(addr+0);
        uint8_t type    = *(uint8_t*)(addr+1);
        uint8_t subtype = *(uint8_t*)(addr+2); // video class descriptors own subtypes

        ////////////////////////////////
        // video control              //
        ////////////////////////////////

        if (length == 13 && type == 0x24 && subtype == VC_HEADER)
        {
            usb_vidHeaderDesc_t* descriptor = addr;
            usb_vidshowVideoHeaderDescriptor(descriptor);

            video->firstVSInterfaceNumber = descriptor->interfaceNr;
            serial_log(SER_LOG_EHCI_ITD, "\n\nvideo->firstVSInterfaceNumber: %u", video->firstVSInterfaceNumber);

            video->clockFrequency = descriptor->clockFreq;
        }
        else if (length == 18 && type == 0x24 && subtype == VC_INPUT_TERMINAL)
        {
            usb_vidInputTermDesc_t* descriptor = addr;
            usb_vidshowInputTermDescriptor(descriptor);
            video->cameraTerminalID = descriptor->terminalID;
        }
        else if (length == 9 && type == 0x24 && subtype == VC_OUTPUT_TERMINAL)
        {
            usb_vidOutputTermDesc_t* descriptor = addr;
            serial_log(SER_LOG_EHCI_ITD, "\n\nOutput Terminal Type: %xh (USB streaming: 101h), Terminal ID: %u ", descriptor->terminalType, descriptor->terminalID);
            serial_log(SER_LOG_EHCI_ITD, "\nAssociated Terminal ID: %u, SourceTerminal ID: %u, ", descriptor->associatedTerminal, descriptor->srcID);
        }
        else if (length == 17 && type == 0x24 && subtype == VC_SELECTOR_UNIT)
        {
            usb_vidSelectorUnitDesc_t* descriptor = addr;
            usb_vidshowSelectorUnitDescriptor(descriptor);
        }
        else if ((length == 11 || length == 12) && type == 0x24 && subtype == VC_PROCESSING_UNIT)
        {
            usb_vidProcessingUnitDesc_t* descriptor = addr;
            usb_vidshowProcessingUnitDescriptor(descriptor);
        }
        else if (length >= 24 && type == 0x24 && subtype == VC_EXTENSION_UNIT)
        {
            serial_log(SER_LOG_EHCI_ITD, "\n\nVideo Control: EXTENSION_UNIT");
        }

        // video endpoint
        else if (length == 5 && type == 0x25 && subtype == VID_EP_INTERRUPT)
        {
            serial_log(SER_LOG_EHCI_ITD, "\n\nVideo Class specific Interrupt EP desc");
            usb_vidInterruptEndpoint_t* descriptor = addr;
            serial_log(SER_LOG_EHCI_ITD, "\nmax. Transfersize Interrupt Endpoint: %u", descriptor->maxTransferSize);
        }

        ////////////////////////////////
        // video streaming            //
        ////////////////////////////////

        else if (length == 14 && type == 0x24 && subtype == VS_INPUT_HEADER)
        {
            serial_log(SER_LOG_EHCI_ITD, "\n\n\nVIDEO_STREAM: HEADER");
            serial_log(SER_LOG_EHCI_ITD, "\n--------------------");
            usb_vidInputHeaderIFDesc_t* descriptor = addr;
            serial_log(SER_LOG_EHCI_ITD, "\nstillCaptureMethod: %u", descriptor->stillCaptureMethod);
        }
        else if (length == 10 && type == 0x24 && subtype == VS_OUTPUT_HEADER)
        {
        }
        else if (length != 9 && type == 0x24 && subtype == VS_STILL_IMAGE_FRAME)
        {
            usb_vidStillImageFrameIFDesc_t* descriptor = addr;
            usb_vidshowStillImageFrameDescriptor(descriptor);
            video->endpointNumberStill = descriptor->endpointAddress;
        }
        else if (length == 27 && type == 0x24 && subtype == VS_FORMAT_UNCOMPRESSED)
        {
            usb_vidUncompressedFormatDesc_t* descriptor = addr;
            usb_vidshowUncompressedFormatDescriptor(descriptor);
            video->uncompressedFormatIndex = descriptor->formatIndex;
            video->defaultFrameIndex = descriptor->defaultFrameIndex;
        }
        else if (length > 12 && type == 0x24 && subtype == VS_FRAME_UNCOMPRESSED)
        {
            usb_vidUncompressedFrameDesc_t* descriptor = addr;
            usb_vidshowUncompressedFrameDescriptor(descriptor);

            video->frame[descriptor->frameIndex - 1].capabilites          = descriptor->capabilites;
            video->frame[descriptor->frameIndex - 1].defaultFrameInterval = descriptor->defaultFrameInterval;
            video->frame[descriptor->frameIndex - 1].frameIndex           = descriptor->frameIndex;
            video->frame[descriptor->frameIndex - 1].frameIntervalType    = descriptor->frameIntervalType;
            video->frame[descriptor->frameIndex - 1].height               = descriptor->height;
            video->frame[descriptor->frameIndex - 1].width                = descriptor->width;
            video->frame[descriptor->frameIndex - 1].maxBitRate           = descriptor->maxBitRate;
            video->frame[descriptor->frameIndex - 1].maxVideoFrameBufSize = descriptor->maxVideoFrameBufSize;
            video->frame[descriptor->frameIndex - 1].minBitRate           = descriptor->minBitRate;
            video->frame[descriptor->frameIndex - 1].frameArray[0]        = descriptor->frameArray[0];
            video->frame[descriptor->frameIndex - 1].frameArray[1]        = descriptor->frameArray[1];
            video->frame[descriptor->frameIndex - 1].frameArray[2]        = descriptor->frameArray[2];
            video->frame[descriptor->frameIndex - 1].frameArray[3]        = descriptor->frameArray[3];
            video->frame[descriptor->frameIndex - 1].frameArray[4]        = descriptor->frameArray[4];
            video->frame[descriptor->frameIndex - 1].frameArray[5]        = descriptor->frameArray[5];

            if (descriptor->frameIndex == video->defaultFrameIndex)
            {
                video->frameInterval = descriptor->defaultFrameInterval;
                video->maxVideoFrameSize = descriptor->maxVideoFrameBufSize;
            }
            if (descriptor->frameIndex > video->highestFrameIndex)
            {
                video->highestFrameIndex = descriptor->frameIndex;
            }
        }
        else if (length == 11 && type == 0x24 && subtype == VS_FORMAT_MJPEG)
        {
            usb_vidMJPEGFormatDesc_t* descriptor = addr;
            //usb_vidshowMJPEGFormatDescriptor(descriptor);
            video->MJPEGFormatIndex = descriptor->formatIndex;
            video->MJPEGdefaultFrameIndex = descriptor->defaultFrameIndex;
            serial_log(SER_LOG_EHCI_ITD, "\n\nVS_FORMAT_MJPEG");
            serial_log(SER_LOG_EHCI_ITD, "\n------------------------");
            serial_log(SER_LOG_EHCI_ITD, "\nMJPEG Format Index: %u, default frame index: %u", descriptor->formatIndex, descriptor->defaultFrameIndex);
        }
        else if (length > 26 && type == 0x24 && subtype == VS_FRAME_MJPEG)
        {
            serial_log(SER_LOG_EHCI_ITD, "\n\nVS_FRAME_MJPEG");
            serial_log(SER_LOG_EHCI_ITD, "\n------------------------");
            usb_vidMJPEGFrameDesc_t* descriptor = addr;
            serial_log(SER_LOG_EHCI_ITD, "\nMJPEG Frame Index: %u \twidth: %u height: %u", descriptor->frameIndex, descriptor->width, descriptor->height);
        }
        else if (length == 6 && type == 0x24 && subtype == VS_COLORFORMAT)
        {
            serial_log(SER_LOG_EHCI_ITD, "\n\nVIDEO_STREAM COLORFORMAT");
            serial_log(SER_LOG_EHCI_ITD, "\n------------------------");
            usb_vidColorFormatIFDesc_t* descriptor = addr;
            serial_log(SER_LOG_EHCI_ITD, "\ncolorPrimaries:     %yh 1: BT.709, sRGB (default)", descriptor->colorPrimaries);
            serial_log(SER_LOG_EHCI_ITD, "\nmatrixCoefficients: %yh 1: BT.709 (default)", descriptor->matrixCoefficients);
            serial_log(SER_LOG_EHCI_ITD, "\ntransferChar:       %yh 4: SMPTE 170M (BT.601, default)", descriptor->transferChar);
        }
        addr += length;
    }//while
}

extern array(disk_t*) disks; // HACK
static void saveYUYV(const diskType_t* YUVYDest, uint8_t* YUYVbuffer, uint32_t size, uint8_t type)
{
    if (YUYVbuffer == 0)
    {
        printfe("No valid YUYV buffer pointer!");
        return;
    }

    char Pfad[20] = {0};

    for (int i = 0; i < disks.size; i++) // HACK
    {
        if (disks.data[i] && disks.data[i]->type == YUVYDest && (disks.data[i]->partition[0]->subtype == FS_FAT12 || disks.data[i]->partition[0]->subtype == FS_FAT16 || disks.data[i]->partition[0]->subtype == FS_FAT32))
        {
            switch (type)
            {
            case 0:
                snprintf(Pfad, 20, "%u:/videoraw.yuv", i+1);
                break;
            case 1:
                snprintf(Pfad, 20, "%u:/videohdr.yuv", i+1);
                break;
            case 2:
                snprintf(Pfad, 20, "%u:/videosti.yuv", i+1);
                break;
            case 3:
                snprintf(Pfad, 20, "%u:/videodat.yuv", i+1);
                break;
            }
        }
    }

    if (Pfad[0] == 0)
        return; // No destination

    file_t* file = fopen(Pfad, "a+");

    if (file) // check for NULL pointer, otherwise #PF
    {
        // printf("\nfile for storage opened");
        fwrite(YUYVbuffer, 1, size, file);
        fclose(file);
    }
    else
    {
        puts("\nError: file could not be opened!");
    }
}


/*
* Copyright (c) 2015-2017 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
