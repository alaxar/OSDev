#ifndef CDI_FS
#define CDI_FS

#include "util/types.h"
#include <cdi.h>
#include <cdi/lists.h>
#include <cdi/cache.h>


struct cdi_fs_filesystem;
// Diese Struktur wird fuer jeden Dateisystemtreiber einmal erstellt
struct cdi_fs_driver {
    struct cdi_driver drv;

    /* Check whether there is a valid file system on the device. If so, and the
       file system has a name, the name is returned in a string newly allocated
       with malloc(). This function is optional for file system drivers.
       fs:     Isn't a fully initialised file system; it can only be used to access
               the storage device.
       return: If the file system is valid 1, otherwise 0. */
    int (*fs_probe)(struct cdi_fs_filesystem* fs, char** volname);

    /* Neues Dateisystem initialisieren; Diese Funktion muss das root_object in
       der Dateisystemstruktur eintragen.
       return: Wenn das Dateisystem erfolgreich initialisiert wurde 1, sonst
               0. Falls ein Fehler auftritt, muss das error-Feld in der
               Dateisystemstruktur gesetzt werden. */
    int (*fs_init)(struct cdi_fs_filesystem* fs);

    /* Dateisystem deinitialisieren
       return: Wenn das Dateisystem erfolgreich deinitialisiert wurde 1, 0
               sonst. Falls ein Fehler auftritt, muss das error-Feld in der
               Dateisystemstruktur gesetzt werden. */
    int (*fs_destroy)(struct cdi_fs_filesystem* fs);
};

struct cdi_fs_res;
// Diese Struktur wird fuer jedes eingebundene Dateisystem einmal erstellt.
struct cdi_fs_filesystem {
    struct cdi_fs_driver* driver; // Treiber, dem das Dateisystem gehoert
    struct cdi_fs_res*    root_res; // Wurzelverzeichnis des Dateisystems

    /* Falls ein gravierender Fehler auftritt, wird diese Fehlernummer gesetzt.
       Wenn sie != 0 ist wird das Dateisystem fuer Schreibzugriffe gesperrt. */
    int                   error;
    int                   read_only; // Das Dateisystem darf nicht geschrieben werden. Damit schlaegt unter anderem cdi_fs_write_data fehl.

    /* Hier sollte man wohl noch ein paar allgemeine Mount-Optionen oder
       sonstige Flags die das ganze Dateisystem betreffen... */

    void*                 opaque; // Zeiger den der Treiber fuer eigene Daten zum Dateisystem benutzen kann
    cdi_fs_osdep          osdep;  // OS-spezifische Daten
};


// XXX Bei den Fehlernummern weiss ich noch nicht wirklich, was da notwendig ist, deshalb lasse ich das mal so stehen.
typedef enum {
    // Kein Fehler aufgetreten
    CDI_FS_ERROR_NONE = 0,
    // Fehler bei Eingabe/Ausgabeoperationen
    CDI_FS_ERROR_IO,
    // Operation nicht unterstuetzt
    CDI_FS_ERROR_ONS,
    // Ressource nicht gefunden
    CDI_FS_ERROR_RNF,
    // Beim lesen einer Datei wurde das Ende erreicht
    CDI_FS_ERROR_EOF,
    // Dateisystem ist nur lesbar und es wurde versucht zu schreiben
    CDI_FS_ERROR_RO,
    // Interner Fehler
    CDI_FS_ERROR_INTERNAL,
    // Funktion noch nicht implementiert
    CDI_FS_ERROR_NOT_IMPLEMENTED,
    // Unbekannter Fehler
    CDI_FS_ERROR_UNKNOWN
} cdi_fs_error_t;

// Der Stream stellt die Verbindung zwischen Aufrufer und Ressource dar.
struct cdi_fs_stream {
    struct cdi_fs_filesystem* fs;    // Dateisystem
    struct cdi_fs_res*        res;   // Betroffene Ressource
    cdi_fs_error_t            error; // Fehlernummer
};


// Metaeigenschaften, die Ressourcen haben koennen
typedef enum {
    // R  Groesse der Datei auslesen
    CDI_FS_META_SIZE,
    // R  Anzahl der Benutzten Dateisystemblocks (Irgendwo muesste man dann auch auf diese Blockgroesse zurgreiffen koennen)
    CDI_FS_META_USEDBLOCKS,
    // R  Optimale Blockgroesse mit der man auf die Datei zugreiffen sollte
    CDI_FS_META_BESTBLOCKSZ,
    // R  Interne Blockgroesse fuer USEDBLOCKS
    CDI_FS_META_BLOCKSZ,
    // R  Zeitpunkt an dem die Ressource erstellt wurde
    CDI_FS_META_CREATETIME,
    // RW Letzter Zugriff auf die Ressource, auch lesend
    CDI_FS_META_ACCESSTIME,
    // RW Letzte Veraenderung der Ressource
    CDI_FS_META_CHANGETIME
} cdi_fs_meta_t;


// Siese Struktur stellt die Moeglichkeiten, die an einer Ressource zur Verfuegung stehen, dar.
struct cdi_fs_res_flags {
    int  remove;       // Ressource loeschen
    int  rename;       // Ressource umbenennen
    int  move;         // Ressource verschieben
    int  read;         // Lesender Zugriff gestattet
    int  write;        // Schreibender Zugriff gestattet
    int  execute;      // Ausfuehren gestattet
    int  browse;       // Auflisten der Verzeichniseintraege gestattet
    int  read_link;    // Aufloesen des Links
    int  write_link;   // Aendern des Links
    int  create_child; // Anlegen eines Untereintrags
};


struct cdi_fs_res_res;
struct cdi_fs_res_file;
struct cdi_fs_res_dir;
struct cdi_fs_res_link;
struct cdi_fs_res_special;

// Typ der eine Ressource, die zu der Klasse der Spezialdateien gehoert noch genauer beschreibt
typedef enum {
    CDI_FS_BLOCK,
    CDI_FS_CHAR,
    CDI_FS_FIFO,
    CDI_FS_SOCKET
} cdi_fs_res_type_t;

// Konstanten fuer die einzelnen Klassen, um sie beim Funktionsaufruf zum zuweisen einer Klasse, identifizieren zu koennen.
typedef enum {
    CDI_FS_CLASS_FILE,
    CDI_FS_CLASS_DIR,
    CDI_FS_CLASS_LINK,
    CDI_FS_CLASS_SPECIAL
} cdi_fs_res_class_t;

// Dieser Typ dient dazu Ressourcen ganz oder teilweise zu sperren
typedef enum {
    CDI_FS_LOCK_NONE,
    CDI_FS_LOCK_WRITE,
    CDI_FS_LOCK_ALL
} cdi_fs_lock_t;

/* Das Dateisystem wird hier nur mit abstrakten Strukturen vom Typ
   cdi_fs_res dargestellt. Diese können beispielsweise sowohl regulaere Datei als auch Verzeichnis gleichzeitig darstellen.
   Weiter gilt, dass Ressourcen, die zu keiner Klasse gehoeren, nicht persistent sind. */
struct cdi_fs_res {
    char*                   name; // Name of the resource
    cdi_fs_lock_t           lock; // Lock of the resource

    /* Flag ob die Ressource geladen ist(1) oder nicht(0). Ist sie danicht,
       muss nur name und res definiert sein. In res darf nur load aufgerufen
       werden. */
    int                     loaded;

    int                     stream_cnt; // Referenzzaehler fuer Implementation. Muss beim erstellen der Ressource mit 0 initialisiert werden
    struct cdi_fs_res*      parent;     // Verweis auf das Elternobjekt
    cdi_list_t              children;   // Liste mit allfaelligen Kindobjekten
    char*                   link_path;  // Link-Pfad
    cdi_list_t              acl;        // ACL: see struct cdi_fs_acl_entry
    struct cdi_fs_res_flags flags;      // Flags

    // Einzelne Klassen, zu denen die Ressourcen gehoeren kann, oder Null falls es zu einer Bestimmten Klasse nicht gehoert.
    struct cdi_fs_res_res*  res;
    struct cdi_fs_res_file* file;
    struct cdi_fs_res_dir*  dir;
    struct cdi_fs_res_link* link;
    struct cdi_fs_res_special* special;

    // Falls die Ressource zu einer Spezialklasse gehoert, wird hier angegeben, um welchen Typ von Spezialressource sie gehoert.
    cdi_fs_res_type_t       type;
};



/* Diese Dateisystemobjekte werden in Klassen eingeteilt, die das eigentliche "Verhalten" der Ressourcen steuern.
   Diese Klassen beinhalten die moeglichen Operationen und auch die Eigenschaften, die fuer die Ressourcen gelten,
   denen diese Klassen zugeordnet sind.
   Das Definieren der einzelnen Klassen uebernehmen dann die einzelnen Treiber.
   Die Flags koennen von der Ressource ueberschrieben werden. Es koennen allerdings nur Flags deaktiviert werden,
   die in der Klasse gesetzt sind und nicht umgekehrt.
   Das Selbe gilt auch fuer Klassen bei denen NULL-Pointer fuer Operationen eingetragen sind. Wenn zum Beispiel
   fuer write NULL eingetragen wird, dann bringt ein gesetztes write-Flag nichts. */

// Diese Klasse gilt unabhaengig von den andern, also egal welche anderen Klassen angegeben sind, diese muss angegeben werden.
struct cdi_fs_res_res {
    /* Ressource laden
       stream: Stream
       return: Falls die Ressource erfolgreich geladen wurde 1, sonst 0 */
    int (*load)(struct cdi_fs_stream* stream);

    /* Ressource entladen; Darf von der Implementation nur aufgerufen werden, wenn keine geladenen Kind-Ressourcen existieren.
       Das gilt aber nur fuer Verzeichnisse. Wenn andere Kind-Eintraege existieren, werden die nicht beruecksichtigt.
       stream: Stream
       return: Falls die Ressource erfolgreich entladen wurde 1, sonst 0 */
    int (*unload)(struct cdi_fs_stream* stream);

    /* Ressource entfernen. Diese Funktion wird nur aufgerufen, wenn die Ressource keiner Klasse mehr zugewiesen ist.
       stream: Stream
       return: Falls die Ressource erfolgreich geloescht wurde 1, sonst 0 */
    int (*remove)(struct cdi_fs_stream* stream);

    /* Namen einer Ressource aendern. Der Parameter name ist nur der Resourcennamen ohne Pfad. Zum verschieben wird move() benutzt.
       stream: Stream
       name:   Neuer Name
       return: Falls die Ressource erfolgreich umbenennt wurde 1, sonst 0 */
    int (*rename)(struct cdi_fs_stream* stream, const char* name);

    /* Ressource innerhalb des Dateisystems verschieben. Das Verschieben ueber Dateisystemgrenzen hinweg wird per kopieren und loeschen durchgefuehrt.
       stream: Stream
       dest:   Pointer auf die Ressource, in die die Ressource verschoben werden soll
       return: Falls die Ressource erfolgreich verschoben wurde 1, sonst 0 */
    int (*move)(struct cdi_fs_stream* stream, struct cdi_fs_res* dest);

    /* Diese Ressource einer neuen Klasse zuweisen. Diese Funktion wird nur aufgerufen, wenn die Ressource nicht bereits
       dieser Klasse zugewiesen ist.
       stream: Stream
       class:  Konstante fuer den Typ der klasse, der die Ressource zugewiesen werden soll.
       return: 1 falls die Ressource erfolgreich der Klasse zugewiesen wurde, 0 sonst */
     int (*assign_class)(struct cdi_fs_stream* stream,
        cdi_fs_res_class_t class);

    /* Diese Ressource aus einer Klasse entfernen. Diese Funktion wird nur aufgerufen, wenn die Ressource zu dieser Klasse gehoert.
       Bei Verzeichnissen muss von der Implementierung garantiert werden, dass diese Funktion nicht aufgerufen wird,
       solange das Verzeichnis noch Kindressourcen hat.
       stream: Stream
       class:  Konstante fuer den Typ der klasse, aus der die Ressource entfernt werden soll.
       return: 1 falls die Ressource erfolgreich aus der Klasse entfernt wurde, 0 sonst */
     int (*remove_class)(struct cdi_fs_stream* stream,
        cdi_fs_res_class_t class);

    /* Metaeigenschaft lesen
       stream: Stream
       meta:   Konstante fuer die gewuenschte Metaeigenschaft
       return: Wert der Metaeigenschaft */
    int64_t (*meta_read)(struct cdi_fs_stream* stream, cdi_fs_meta_t meta);

    /* Metaeigenschaft schreiben
       stream: Stream
       meta:   Konstante fuer die gewuenschte Metaeigenschaft
       value:  Neuen Wert fuer die Metaeigenschaft
       return: Falls die Metaeigenschaft erfolgreich geaendert wurde 1, sonst 0 */
    int (*meta_write)(struct cdi_fs_stream* stream, cdi_fs_meta_t meta,
        int64_t value);
};

struct cdi_fs_res_file {
    // Dateien in dieser Klasse sind grunsaetzlich ausfuehrbar, wenn in der Flag-Struktur in der Ressource nicht anders angegeben
    int                     executable;

    /* Daten aus dieser Datei lesen. Wird nur aufgerufen, wenn es durch die Flags oder Berechtigungen nicht verhindert wird.
       Im Fehlerfall wird je nach Fehler die Fehlernummer im Handle und die im Device gesetzt.
       stream: Stream
       start:  Position von der an gelesen werden soll
       size:   Groesse der zu lesenden Daten
       buffer: Puffer in den die Daten gelsen werden sollen
       return: Gelesene Bytes, oder 0 im Fehlerfall */
    size_t (*read)(struct cdi_fs_stream* stream, uint64_t start, size_t size,
        void* buffer);

    /* Daten in diese Datei schreiben. Wird nur aufgerufen, wenn es durch die Flags oder Berechtigungen nicht verhindert wird.
       Im Fehlerfall wird je nach Fehler die Fehlernummer im Handle und die im Device gesetzt.
       stream: Stream
       start:  Position an die geschrieben werden soll
       size:   Groesse der zu schreibenden Daten
       buffer: Puffer aus dem die Daten gelesen werden sollen
       return: Geschriebene Bytes oder 0 im Fehlerfall */
    size_t (*write)(struct cdi_fs_stream* stream, uint64_t start, size_t size,
        const void* buffer);


    /* Groesse der Datei anpassen. Diese Funktion kann sowohl fuers Vergroessern, als auch fuers Verkleinern benutzt werden.
       Im Fehlerfall wird je nach Fehler die Fehlernummer im Handle und die im Device gesetzt.
       stream: Stream
       size:   Neue Groesse der Datei
       return: 1 bei Erfolg, im Fehlerfall 0 */
    int (*truncate)(struct cdi_fs_stream* stream, uint64_t size);
};

struct cdi_fs_res_dir {
    /* Diese Funktion gibt einen Pointer auf die Liste mit den Eintraegen zurueck. Hier wird nicht einfach fix der Pointer in fs_res genommen,
       damit dort auch "versteckte" Eintraege vorhanden sein koennten. (Ich meine hier nicht irgend ein versteckt-Flag dass die Dateien vor dem
       Benutzer Verstecken soll, sondern eher von fuer den internen Gebrauch angelegten Eintraegen.
       Diese Liste muss nicht vom Aufrufer freigegeben werden, da einige Treiber hier direkt children aus fs_res benutzen, und andere dafuer eine
       eigene Liste erstellen, die sie intern abspeichern.
       stream: Stream
       return: Pointer auf eine Liste mit den Untereintraegen vom Typ cdi_fs_res. */
    cdi_list_t (*list)(struct cdi_fs_stream* stream);

    /* Neue Ressource in der Aktuellen erstellen. Diese wird erstmal noch keiner Klasse zugewiesen. Diese Funktion wird mit einem NULL-Pointer als
       Ressource im Stream aufgerufen. Dieser NULL-Pointer muss bei Erfolgreichem Beenden durch einen Pointer auf die neue Ressource ersetzt
       worden sein.
       stream: Mit NULL-Pointer als Ressource
       name:   Name der neuen Ressource
       parent: Ressource, der die neue Ressource als Kindressource zugeordnet werden soll.
       return: Falls die Ressource erfolgreich erstellt wurde 1, sonst 0 */
    int (*create_child)(struct cdi_fs_stream* stream,
        const char* name, struct cdi_fs_res* parent);
};

struct cdi_fs_res_link {
    /* Diese Funktion liest den Pfad aus, auf den der Link zeigt
       stream: Stream
       return: Pointer auf einen Buffer der den Pfad beinhaltet. Dieses Puffer darf vom Aufrufer nicht veraendert werden. */
    const char* (*read_link)(struct cdi_fs_stream* stream);

    /* Aendert den Pfad auf den der Link zeigt
       stream: Stream
       path:   Neuer Pfad
       return: Wenn der Link erfolgreich geschrieben wurde 1, 0 sonst */
    int (*write_link)(struct cdi_fs_stream* stream, const char* path);
};

struct cdi_fs_res_special {
    /* Geraeteadresse der Spezialdatei Lesen
       stream: Stream
       dev:    Pointer auf die Variable in der die Geraeteadresse gespeichert werden soll.
       return: Falls die Geraeteadresse erfolgreich gelesen wurde 1, sonst 0 */
    int (*dev_read)(struct cdi_fs_stream* stream, uint64_t* dev);

    /* Geraeteadresse der Spezialdatei Aendern
       stream: Stream
       dev:    Die neue Geraeteadresse
       return: Falls die Geraeteadresse erfolgreich geaendert wurde 1, sonst 0 */
    int (*dev_write)(struct cdi_fs_stream* stream, uint64_t dev);
};


/* Die Berechtigunen werden mit Access controll lists, kurz ACLs verwaltet. Diese werden in Form von Listen gespeichert. Diese Listen enthalten
   eintraege von verschiedenen Typen. */
typedef enum {
    CDI_FS_ACL_USER_NUMERIC,  // UID
    CDI_FS_ACL_USER_STRING,   // username as a string
    CDI_FS_ACL_GROUP_NUMERIC, // GID
    CDI_FS_ACL_GROUP_STRING   // Groupname as a string
} cdi_fs_acl_entry_type_t;


// Der Basiseintrag in einer ACL, von dem die anderen Typen der Eintraege abgeleitet sind.
struct cdi_fs_acl_entry {
    cdi_fs_acl_entry_type_t type;  // Type of the entry, one of the constants above
    struct cdi_fs_res_flags flags; // Flags
};


// Eintraege fuer die einzelnen ACL-Eintragstypen; see: struct cdi_fs_acl_entry
struct cdi_fs_acl_entry_usr_num {
    struct cdi_fs_acl_entry entry;
    int                     user_id; // UserID
};

struct cdi_fs_acl_entry_usr_str {
    struct cdi_fs_acl_entry entry;
    char*                   user_name; // Username
};

struct cdi_fs_acl_entry_grp_num {
    struct cdi_fs_acl_entry entry;
    int                     group_id; // GroupID
};

struct cdi_fs_acl_entry_grp_str {
    struct cdi_fs_acl_entry entry;
    char*                   group_name; //Groupname
};

/* Dateisystemtreiber-Struktur initialisieren
   driver: Zeiger auf den Treiber */
void cdi_fs_driver_init(struct cdi_fs_driver* driver);

/* Dateisystemtreiber-Struktur zerstoeren
   driver: Zeiger auf den Treiber */
void cdi_fs_driver_destroy(struct cdi_fs_driver* driver);

/* Dateisystemtreiber registrieren
   driver: Zeiger auf den Treiber */
void cdi_fs_driver_register(struct cdi_fs_driver* driver);


/* Quelldateien fuer ein Dateisystem lesen
   XXX Brauchen wir hier auch noch irgendwas errno-Maessiges?
   fs:     Pointer auf die FS-Struktur des Dateisystems
   start:  Position von der an gelesen werden soll
   size:   Groesse des zu lesenden Datenblocks
   buffer: Puffer in dem die Daten abgelegt werden sollen
   return: die Anzahl der gelesenen Bytes */
size_t cdi_fs_data_read(struct cdi_fs_filesystem* fs, uint64_t start, size_t size, void* buffer);

/* Quellmedium eines Dateisystems beschreiben
   XXX Brauchen wir hier auch noch irgendwas errno-Maessiges?
   fs:     Pointer auf die FS-Struktur des Dateisystems
   start:  Position an die geschrieben werden soll
   size:   Groesse des zu schreibenden Datenblocks
   buffer: Puffer aus dem die Daten gelesen werden sollen
   return: die Anzahl der geschriebenen Bytes */
size_t cdi_fs_data_write(struct cdi_fs_filesystem* fs, uint64_t start, size_t size, const void* buffer);

#endif
