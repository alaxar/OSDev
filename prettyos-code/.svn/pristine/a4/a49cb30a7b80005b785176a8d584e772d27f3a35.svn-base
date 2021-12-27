#ifndef CDI_AUDIO_H
#define CDI_AUDIO_H

#include "util/types.h"

#include "cdi.h"
#include "cdi/lists.h"
#include "cdi/mem.h"


/* Possible sample formats (i.e., signed/unsigned integer, floating point numbers, one sample's size, ...).
   Little endian is used, unless marked otherwise. */
typedef enum {
    // 16 bit signed integer
    CDI_AUDIO_16SI = 0,
    // 8 bit signed integer
    CDI_AUDIO_8SI,
    // 32 bit signed integer
    CDI_AUDIO_32SI
} cdi_audio_sample_format_t;

// A stream's playback position. Contains the index of both the current buffer and the frame being played.
typedef struct cdi_audio_position {
    size_t buffer;
    size_t frame;
} cdi_audio_position_t;

// A stream's status.
typedef enum {
    // Stopped
    CDI_AUDIO_STOP = 0,
    // According to the device type either playing or recording.
    CDI_AUDIO_PLAY = 1
} cdi_audio_status_t;

/* Describes a CDI.audio device, i.e. a playback or recording device. If a sound card provides both functions,
   the driver must register one CDI.audio device per function. */
struct cdi_audio_device {
    struct cdi_device dev;

    // If this value is not 0, this device is a recording device, thus one can only read data. Otherwise, only play back is possible.
    int record;

    /* Play back devices may provide several logical streams which are mixed by the device. If this is either not true or it is a
       recording device, the list's size is exactly 1. It contains elements of type struct cdi_audio_stream. */
    cdi_list_t streams;
};

/* Describes a CDI.audio device's stream.
   Each stream consists of a set of buffers containing samples being played consecutively and as a loop. */
struct cdi_audio_stream {
    // Device to which this stream belongs.
    struct cdi_audio_device* device;

    // Contains the number of available data buffers.
    size_t num_buffers;

    // Contains the number of samples per buffer.
    size_t buffer_size;

    // If 0, the sample rate is adjustable. Otherwise, it isn't and is set to a fixed value in Hz (which is given here).
    int fixed_sample_rate;

    // Format of the samples contained in the buffers.
    cdi_audio_sample_format_t sample_format;
};

struct cdi_audio_driver {
    struct cdi_driver drv;

    /* Sends or receives data to/from a device's buffer (receives from recording devices, sends otherwise).
       The amount of data transmitted equals the buffer's size (depends on the sample format and the number of
       samples per buffer) reduced by the offset.
       stream: Stream to be used.
       buffer: Buffer to be used.
       memory: Memory buffer being either source or destination.
       offset: The content from memory is copied to the buffer respecting this offset (in samples) in the buffer.
               Thus, the data is copied to this offset until the end of the buffer has been reached (recording
               devices ignore this).
       return: 0 on success, -1 on error. */
    int (*transfer_data)(struct cdi_audio_stream* stream, size_t buffer,
        struct cdi_mem_area* memory, size_t offset);

    /* Stops a CDI.audio device or makes it record or play, respectively. If it is stopped, its position is
       automatically set to the beginning of the first buffer.
       device: The device
       status: Status to be set.
       return: Actual new status. */
    cdi_audio_status_t (*change_device_status)(struct cdi_audio_device* device,
        cdi_audio_status_t status);

    /* Sets a stream's volume.
       volume: 0x00 is mute, 0xFF is full volume. */
    void (*set_volume)(struct cdi_audio_stream* stream, uint8_t volume);

    /* Changes a stream's sample rate.
       stream:      Stream to be adjusted.
       sample_rate: New sample rate, if nonpositive, the current sample rate won't be changed.
       return:      Returns the actual new sample rate (may differ from the parameter
                    and will, if the latter was nonpositive). */
    int (*set_sample_rate)(struct cdi_audio_stream* stream, int sample_rate);

    /* Returns the stream's current playback/recording position.
       stream:   Stream to be queried.
       position: Pointer to a structure receiving the index of both the
                 current buffer and the frame played right now. */
    void (*get_position)(struct cdi_audio_stream* stream,
        cdi_audio_position_t* position);

    /* Changes the number of channels used by a CDI.audio device (this changes the frames' size, hence the
       number of actually used samples per buffer may be changed, too: If this size equals e.g. 0xFFFF, only
       0xFFFE samples are used when working with two channels (because there is no half of a frame)).
       dev:      CDI.audio device
       channels: Number of channels to be used (if invalid, nothing will be changed).
       return:   Actually used number of channels now. */
    int (*set_number_of_channels)(struct cdi_audio_device* dev, int channels);
};


/* Is called by a driver when having completed a buffer (i.e., completely played or recorded). May be called by an ISR.
   stream: Stream being concerned.
   buffer: Index of the completed buffer. */
void cdi_audio_buffer_completed(struct cdi_audio_stream* stream, size_t buffer);


#endif
