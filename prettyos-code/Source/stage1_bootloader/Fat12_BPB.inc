%ifndef FAT12_BPB_INC
%define FAT12_BPB_INC

[BITS 16]

; "FAT 12" file system is build up by four areas at Floppy Disk:
; - Boot Sector (also called BIOS Parameter Block, BPB)
; - File Allocation Table, FAT
; - Root Directory
; - Subdirectories and Files

; boot sector info block

OperatingSystemName db "PrettyOS"      ;  8 byte
BytesPerSec         dw 512
SecPerClus          db 1
ReservedSec         dw 1
NumFATs             db 2
RootEntries         dw 224
TotSec              dw 2880
MediaType           db 0xF0
FATSize             dw 9
SecPerTrack         dw 18
NumHeads            dw 2
HiddenSec           dd 0
TotSec32            dd 0
DriveNum            db 0
Reserved            db 0
BootSig             db 0x29
VolumeSerialNum     dd 0xD00FC0DE
VolumeLabel         db "PRETTY OS  "   ; 11 byte
FileSys             db "FAT12   "      ;  8 byte

%endif
