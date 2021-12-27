#ifndef AC97_VIA_H
#define AC97_VIA_H

#include "os.h"
#include "pci.h"

// Register offsets
#define AC97_VIA_R_SGD_CONTROL    0x01
#define AC97_VIA_R_SGD_TYPE       0x02
#define AC97_VIA_R_SGD_TABLE_BASE 0x04
#define AC97_VIA_W_SGD_CONTROL    0x11
#define AC97_VIA_W_SGD_TYPE       0x12
#define AC97_VIA_W_SGD_TABLE_BASE 0x14
#define AC97_VIA_ACCESS_CODEC     0x80

// Bits of the AC97_VIA_ACCESS_CODEC register
#define AC97_VIA_CODEC_SEL_PRIM   0
#define AC97_VIA_CODEC_PRIM_VALID BIT(25)
#define AC97_VIA_CODEC_SEL_SEC    BIT(30)
#define AC97_VIA_CODEC_SEC_VALID  BIT(27)
#define AC97_VIA_CODEC_BUSY       BIT(24)
#define AC97_VIA_CODEC_WRITE      0
#define AC97_VIA_CODEC_READ       BIT(23)

#define AC97_RESET                0x0000
#define AC97_MASTER_VOLUME        0x0002
#define AC97_AUX_OUT_VOLUME       0x0004
#define AC97_MONO_VOLUME          0x0006
#define AC97_MASTER_TONE          0x0008
#define AC97_PC_BEEP_VOLUME       0x000A
#define AC97_PHONE_VOLUME         0x000C
#define AC97_MIC_VOLUME           0x000E
#define AC97_LINE_IN_VOLUME       0x0010
#define AC97_CD_VOLUME            0x0012
#define AC97_VIDEO_VOLUME         0x0014
#define AC97_AUX_IN_VOLUME        0x0016
#define AC97_PCM_OUT_VOLUME       0x0018
#define AC97_RECORD_SELECT        0x001A
#define AC97_RECORD_GAIN          0x001C
#define AC97_RECORD_GAIN_MIC      0x001E
#define AC97_GENERAL_PURPOSE      0x0020
#define AC97_3D_CONTROL           0x0022
#define AC97_AUDIO_INT_PAGING     0x0024
#define AC97_POWERDOWN_CTRL_STS   0x0026
#define AC97_EXT_AUDIO_ID         0x0028
#define AC97_EXT_AUDIO_STS_CTRL   0x002A
#define AC97_FRONT_DAC_RATE       0x002C
#define AC97_SURR_DAC_RATE        0x002E
#define AC97_LFE_DAC_RATE         0x0030
#define AC97_LR_ADC_RATE          0x0032
#define AC97_MIC_ADC_RATE         0x0034
#define AC97_CENTER_LFE_VOLUME    0x0036
#define AC97_SURR_VOLUME          0x0038
#define AC97_S_PDIF_CTRL          0x003A
#define AC97_EXTENDED_MODEM_ID    0x003C // for details of the following modem regs cf. spec table 68
#define AC97_VENDOR_RESERVED_1    0x005A
#define AC97_PAGE_REGISTERS       0x0060
#define AC97_VENDOR_RESERVED_2    0x0070
#define AC97_VENDOR_ID_1          0x007C
#define AC97_VENDOR_ID_2          0x007E

#define AC97_VIA_SAMPLES_PER_BUF 65536


typedef struct
{
    uint32_t buf;
    uint32_t len      : 24;
    uint32_t reserved :  5;
    uint32_t stop     :  1; // Pause transfer at end of block. Set bit 2 of register 0 to resume
    uint32_t flag     :  1; // Pause transfer at end of block. Triggers FLAG interrupt
    uint32_t eol      :  1; // Last entry. Triggers EOL interrupt
} __attribute__((packed)) ac97Via_SGDEntry_t;

typedef struct
{
    pciDev_t*           device;
    ac97Via_SGDEntry_t* SGDtable;
    void*               buffers;
    uint16_t            numDesc;
    uint16_t            iobase;
} ac97Via_t;


void ac97Via_install(pciDev_t* device);
void ac97Via_start(ac97Via_t* ac97);
void ac97Via_stop(ac97Via_t* ac97);


#endif
