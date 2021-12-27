#ifndef OS_H
#define OS_H

#include "util/types.h"


// Additional features (should usually be enabled per default)
/// #define _BOOTSCREEN_         // Enables the bootscreen displayed at startup
#define _EHCI_ENABLE_        // EHCI driver will be installed and used in favor of UHCI/OHCI if supported by the attached device
#define _OHCI_ENABLE_        // OHCI driver will be installed
#define _UHCI_ENABLE_        // UHCI driver will be installed
#define _XHCI_ENABLE_        // xHCI driver will be installed
/// #define _ENABLE_HDD_         // HDD driver will be enabled | !!!DRIVER IN DEVELOPMENT STAGE, NEVER USE IT ON REAL HW!!!
#define _AUDIO_ENABLE_       // Audio drivers will be enabled | Does not work well so far, so not enabled
#define _IOAPIC_ENABLE_      // Enables I/O-APIC and disables PIC | Does only work well on Intel chipsets so far, so not enabled
#define _NETWORK_ENABLE_     // Network drivers will be enabled

// Additional debug output (should be disabled per default)
#define _SERIAL_LOG_           // Enables log information over the COM-Ports
/// #define _DIAGNOSIS_            // General diagnosis output - activates prints to the screen about some details and memory use
/// #define _TASKING_DIAGNOSIS_    // Diagnosis output about tasking and scheduler
/// #define _PAGING_DIAGNOSIS_     // Diagnosis output about PD, PT etc.
/// #define _MALLOC_FREE_LOG_      // Shows information about malloc/free and heap expansion
/// #define _MEMLEAK_FIND_         // Counter of all (successful) malloc and free calls showing memory leaks in info bar
/// #define _VM_DIAGNOSIS_         // Information about the vm86 task, but critical
/// #define _DEVMGR_DIAGNOSIS_     // E.g. sectorRead, sectorWrite
/// #define _CACHE_DIAGNOSIS_      // Read cache logger
/// #define _IRQ_DIAGNOSIS_        // Diagnosis about interrupts, PIC and APIC

/// #define _OHCI_DIAGNOSIS_       // Debug OHCI
/// #define _UHCI_DIAGNOSIS_       // Debug UHCI
/// #define _EHCI_DIAGNOSIS_       // Debug EHCI
/// #define _XHCI_DIAGNOSIS_       // Debug xHCI

/// #define _USB_FUNCTION_DIAGNOSIS_ // Show USB Functions
/// #define _USB_DIAGNOSIS_          // Debug USB transfers
/// #define _USB_HUB_DIAGNOSIS_      // Debug USB Hub devices
/// #define _USB_HID_DIAGNOSIS_      // Debug USB Human Interface Devices
/// #define _USB_TRANSFER_DIAGNOSIS_ // Debug USB transfers and transactions

/// #define _VBE_DEBUG_            // Debug output of the VBE driver
/// #define _FLOPPY_DIAGNOSIS_     // Information about the floppy(-motor)
/// #define _RAMDISK_DIAGNOSIS_    // Information about the ramdisk
/// #define _HDD_DIAGNOSIS_        // Information about HDDs
/// #define _FAT_DIAGNOSIS_        // Only as transition state during implementation of FAT 12/16/32
/// #define _NETWORK_DATA_         // Information about networking packets
/// #define _NETWORK_DIAGNOSIS_    // Information about the network adapters
/// #define _ARP_DEBUG_            // Information about ARP
/// #define _DHCP_DEBUG_           // Information about DHCP
/// #define _ICMP_DEBUG_           // Analysis of ICMP information besides echo request/response
/// #define _UDP_DEBUG_            // Information about UDP
/// #define _TCP_DEBUG_            // Information about TCP
/// #define _NETBIOS_DEBUG_        // NetBIOS packet analysis
/// #define _USERHEAP_DIAGNOSIS_   // User-heap diagnostics
/// #define _ACPI_DIAGNOSIS_       // Diagnostics information about ACPI tables and parsers

// output of the serial log to COMx:
#define SER_LOG_TCP      1
#define SER_LOG_HEAP     1
#define SER_LOG_PAGE     1
#define SER_LOG_VM86     2
#define SER_LOG_FAT      1
#define SER_LOG_HRDDSK   1
#define SER_LOG_USERHEAP 1
#define SER_LOG_EHCI_ITD 1
#define SER_LOG          1

extern const char* const version; // PrettyOS version string
extern struct todoList kernel_idleTasks;


#endif
