#ifndef USB_HID_H
#define USB_HID_H

#include "os.h"
#include "usb/usb.h"


enum usb_hidDescType    { DESC_HID=0x21,    DESC_REPORT=0x22, DESC_PHYSICAL=0x23, };
enum usb_hidRequest     { HID_GET_REPORT=1, HID_GET_IDLE=2,   HID_GET_PROTOCOL=3, HID_SET_REPORT=9, HID_SET_IDLE=10, HID_SET_PROTOCOL=11, };
enum usb_hidReportType  { HID_INPUT=1,      HID_OUTPUT=2,     HID_FEATURE=3, };
enum usb_hidProtocol    { HID_PROTO_BOOT=0, HID_PROTO_REPORT=1, };

// HID Usages Page ID
#define HUP_UNDEFINED            0x00
#define HUP_GENERIC_DESKTOP      0x01
#define HUP_SIMULATION           0x02
#define HUP_VR_CONTROLS          0x03
#define HUP_SPORTS_CONTROLS      0x04
#define HUP_GAMING_CONTROLS      0x05
#define HUP_KEYBOARD             0x07
#define HUP_LEDS                 0x08
#define HUP_BUTTON               0x09
#define HUP_ORDINALS             0x0A
#define HUP_TELEPHONY            0x0B
#define HUP_CONSUMER             0x0C
#define HUP_DIGITIZERS           0x0D
#define HUP_PHYSICAL_IFACE       0x0E
#define HUP_UNICODE              0x10
#define HUP_ALPHANUM_DISPLAY     0x14

#define HUP_MEDICAL              0x40

#define HUP_MONITOR              0x80
#define HUP_MONITOR_ENUM_VAL     0x81
#define HUP_VESA_VC              0x82
#define HUP_VESA_CMD             0x83
#define HUP_POWER                0x84
#define HUP_BATTERY_SYSTEM       0x85

#define HUP_BARCODE_SCANNER      0x8B
#define HUP_SCALE                0x8C

#define HUP_CAMERA_CONTROL       0x90
#define HUP_ARCADE               0x91

#define HUP_MICROSOFT            0xFF00

// HID Usages, generic desktop
#define HUG_POINTER              0x01
#define HUG_MOUSE                0x02
#define HUG_JOYSTICK             0x04
#define HUG_GAME_PAD             0x05
#define HUG_KEYBOARD             0x06
#define HUG_KEYPAD               0x07

#define HUG_X                    0x30
#define HUG_Y                    0x31
#define HUG_Z                    0x32
#define HUG_RX                   0x33
#define HUG_RY                   0x34
#define HUG_RZ                   0x35
#define HUG_SLIDER               0x36
#define HUG_DIAL                 0x37
#define HUG_WHEEL                0x38
#define HUG_HAT_SWITCH           0x39
#define HUG_COUNTED_BUFFER       0x3A
#define HUG_BYTE_COUNT           0x3B
#define HUG_MOTION_WAKEUP        0x3C

#define HUG_VX                   0x40
#define HUG_VY                   0x41
#define HUG_VZ                   0x42
#define HUG_VBRX                 0x43
#define HUG_VBRY                 0x44
#define HUG_VBRZ                 0x45
#define HUG_VNO                  0x46

#define HUG_SYSTEM_CONTROL       0x80
#define HUG_SYSTEM_POWER_DOWN    0x81
#define HUG_SYSTEM_SLEEP         0x82
#define HUG_SYSTEM_WAKEUP        0x83
#define HUG_SYSTEM_CONTEXT_MENU  0x84
#define HUG_SYSTEM_MAIN_MENU     0x85
#define HUG_SYSTEM_APP_MENU      0x86
#define HUG_SYSTEM_MENU_HELP     0x87
#define HUG_SYSTEM_MENU_EXIT     0x88
#define HUG_SYSTEM_MENU_SELECT   0x89
#define HUG_SYSTEM_MENU_RIGHT    0x8A
#define HUG_SYSTEM_MENU_LEFT     0x8B
#define HUG_SYSTEM_MENU_UP       0x8C
#define HUG_SYSTEM_MENU_DOWN     0x8D

// HID Usages Digitizers
#define HUD_UNDEFINED            0x00

#define HUD_TIP_PRESSURE         0x30
#define HUD_BARREL_PRESSURE      0x31
#define HUD_IN_RANGE             0x32
#define HUD_TOUCH                0x33
#define HUD_UNTOUCH              0x34
#define HUD_TAP                  0x35
#define HUD_QUALITY              0x36
#define HUD_DATA_VALID           0x37
#define HUD_TRANSDUCER_INDEX     0x38
#define HUD_TABLET_FKEYS         0x39
#define HUD_PROGRAM_CHANGE_KEYS  0x3A
#define HUD_BATTERY_STRENGTH     0x3B
#define HUD_INVERT               0x3C
#define HUD_X_TILT               0x3D
#define HUD_Y_TILT               0x3E
#define HUD_AZIMUTH              0x3F
#define HUD_ALTITUDE             0x40
#define HUD_TWIST                0x41
#define HUD_TIP_SWITCH           0x42
#define HUD_SEC_TIP_SWITCH       0x43
#define HUD_BARREL_SWITCH        0x44
#define HUD_ERASER               0x45
#define HUD_TABLET_PICK          0x46

// HID Usages LEDs
#define HUD_LED_NUM_LOCK         1
#define HUD_LED_CAPS_LOCK        2
#define HUD_LED_SCROLL_LOCK      3
#define HUD_LED_COMPOSE          4
#define HUD_LED_KANA             5

// Bits in the input/output/feature items
#define HIO_CONST                BIT(0)
#define HIO_VARIABLE             BIT(1)
#define HIO_RELATIVE             BIT(2)
#define HIO_WRAP                 BIT(3)
#define HIO_NONLINEAR            BIT(4)
#define HIO_NOPREF               BIT(5)
#define HIO_NULLSTATE            BIT(6)
#define HIO_VOLATILE             BIT(7)
#define HIO_BUFBYTES             BIT(8)


typedef struct
{
    uint8_t  descType;       // type of optional descriptor
    uint16_t descLength;     // total size of the optional descriptor
} __attribute__((packed)) usb_hidOptionalDescriptor_t ;


// HID Descriptor specifies the number, type, and size of HID Report Descriptors and Physical Descriptors
// that are associated with a HID class device
typedef struct
{
    uint8_t  length;         // total size of the HID descriptor
    uint8_t  descType;       // HID, report, physical
    uint16_t cdHID;          // HID class specification release
    uint8_t  countryCode;    // country code of the localized hardware, 0 = not localized, 9 = German, keyboards may indicate the language of the key caps
    uint8_t  numDescriptors; // number of class descriptors (at least one report descriptor)
    uint8_t  descType2;      // type of class descriptor
    uint16_t descLength;     // total size of the report descriptor
} __attribute__((packed)) usb_hidDescriptor_t;

typedef struct
{
    usb_interface_t* interface;
    usb_transfer_t   interruptTransfer;
    uint8_t*         buffer;
    uint8_t          LEDs;
} usb_hid_t;


void usb_setupHID(usb_interface_t* interface);
void usb_destroyHID(usb_interface_t* interface);


#endif
