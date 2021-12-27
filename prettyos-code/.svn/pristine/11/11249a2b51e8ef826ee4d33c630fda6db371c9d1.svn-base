#ifndef CDI_MISC_H
#define CDI_MISC_H

#include "util/types.h"
#include <cdi.h>

#define CDI_GLUE(x, y) x ## y
#define CDI_BUILD_ASSERT(cnt) CDI_GLUE(__cdi_build_assert, cnt)
#define CDI_BUILD_BUG_ON(x) \
        struct CDI_BUILD_ASSERT(__COUNTER__) { int assertion[(x) ? -1 : 1]; };


/* Registiert einen neuen IRQ-Handler.
   irq:    Nummer des zu reservierenden IRQ
   device: Geraet, das dem Handler als Parameter uebergeben werden soll */
void cdi_register_irq(uint8_t irq, void (*handler)(struct cdi_device*), struct cdi_device* device);

/* Setzt den IRQ-Zaehler fuer cdi_wait_irq zurueck.
   irq:    Nummer des IRQ
   return: 0 bei Erfolg, -1 im Fehlerfall */
int cdi_reset_wait_irq(uint8_t irq);

/* Wartet bis der IRQ aufgerufen wurde. Der interne Zähler muss zuerst mit cdi_reset_wait_irq zurückgesetzt werden. Damit auch die IRQs abgefangen
   werden können, die kurz vor dem Aufruf von dieser Funktion aufgerufen werden, sieht der korrekte Ablauf wie folgt aus:
   - cdi_reset_wait_irq
   - Hardware ansprechen und Aktionen ausführen, die schließlich den IRQ auslösen
   - cdi_wait_irq
   Der entsprechende IRQ muss zuvor mit cdi_register_irq registriert worden sein. Der registrierte Handler wird ausgeführt, bevor diese Funktion
   erfolgreich zurückkehrt.
   irq:     Nummer des IRQ auf den gewartet werden soll
   timeout: Anzahl der Millisekunden, die maximal gewartet werden sollen
   return:  0 wenn der irq aufgerufen wurde, -1 sonst. */
int cdi_wait_irq(uint8_t irq, uint32_t timeout);

/* Reserviert IO-Ports
   return: 0 wenn die Ports erfolgreich reserviert wurden, -1 sonst. */
int cdi_ioports_alloc(uint16_t start, uint16_t count);

/* Gibt reservierte IO-Ports frei
   return: 0 wenn die Ports erfolgreich freigegeben wurden, -1 sonst. */
int cdi_ioports_free(uint16_t start, uint16_t count);

// Unterbricht die Ausfuehrung fuer mehrere Millisekunden
void cdi_sleep_ms(uint32_t ms);

/* Returns the number of milliseconds elapsed since an implementation specific
   point in time, not later than the first call of this function. The resolution
   too is implementation specific. The return value must grow monotonically
   (until it overflows due to the return type's limit). */
uint64_t cdi_elapsed_ms(void);

// Waits until the condition cond yields true or more than timeout milliseconds have passed.
#define CDI_CONDITION_WAIT(cond, timeout) \
    do { \
        uint64_t start = cdi_elapsed_ms(); \
        while (!(cond) && cdi_elapsed_ms() - start < timeout); \
    } while (0)

// CDI_CONDITION_WAIT() with cdi_sleep_ms(sleep) between each evaluation.
#define CDI_CONDITION_WAIT_SLEEP(cond, timeout, sleep) \
    do { \
        uint64_t start = cdi_elapsed_ms(); \
        while (!(cond) && cdi_elapsed_ms() - start < timeout) { \
             cdi_sleep_ms(sleep); \
        } \
    } while (0)

/* Casts object (must be a pointer) to a pointer to an object of type to which
   must be a subclass of object's type. field is the entry in to which contains
   object.

   Example:
   struct cdi_driver* driver;
   struct cdi_storage_driver* storage_driver =
       CDI_UPCAST(driver, struct cdi_storage_driver, drv); */
#define CDI_UPCAST(object, to, field) \
    /* Type compatibility check */ \
    ((void)((object)-&((to*)0)->field), \
    /* The cast itself */ \
    (to*)((char*)(object)-(char*)&((to*)0)->field))

/* Functions for endianness conversion. Be aware that most if not all CDI
   drivers only support little endian machines as a host. */
static inline uint16_t cdi_be16_to_cpu(uint16_t x)
{
    return (x >> 8) | (x << 8);
}
static inline uint32_t cdi_be32_to_cpu(uint32_t x)
{
    return ((uint32_t)cdi_be16_to_cpu(x) << 16) | (uint32_t)cdi_be16_to_cpu(x >> 16);
}
static inline uint64_t cdi_be64_to_cpu(uint64_t x)
{
    return ((uint64_t)cdi_be32_to_cpu(x) << 32) | (uint64_t)cdi_be32_to_cpu(x >> 32);
}
static inline uint16_t cdi_cpu_to_be16(uint16_t x)
{
    return cdi_be16_to_cpu(x);
}
static inline uint32_t cdi_cpu_to_be32(uint32_t x)
{
    return cdi_be32_to_cpu(x);
}
static inline uint64_t cdi_cpu_to_be64(uint64_t x)
{
    return cdi_be64_to_cpu(x);
}
static inline uint16_t cdi_le16_to_cpu(uint16_t x)
{
    return x;
}
static inline uint32_t cdi_le32_to_cpu(uint32_t x)
{
    return x;
}
static inline uint64_t cdi_le64_to_cpu(uint64_t x)
{
    return x;
}
static inline uint16_t cdi_cpu_to_le16(uint16_t x)
{
    return cdi_le16_to_cpu(x);
}
static inline uint32_t cdi_cpu_to_le32(uint32_t x)
{
    return cdi_le32_to_cpu(x);
}
static inline uint64_t cdi_cpu_to_le64(uint64_t x)
{
    return cdi_le64_to_cpu(x);
}

#endif
