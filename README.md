# Struktur des Shared Memory

### SharedMemory-Struktur

```c
typedef struct {
    int processAmount;
    int processesCreated;
    pthread_mutex_t mutex;
    pthread_cond_t cattr;
    Bcast broadcastMsg;
    int barrier_all;
    int barrier_all2;
    sem_t messages;
    pthread_cond_t allCreated;
    logger log;
    process p[];
} SharedMem;
```

Wir führen hier ein neues Struct Namens "Shared-Memory" ein, und es hat folgende Inhalte:

- *`int processAmmount;`*
Anzahl der Prozesse die bei Programmstart eingegeben wurden
- *`int processesCreated;`*
Die anzahl der aktuell erstellten Prozesse
- *`pthread_mutex_t mutex`*
Der Mutex, der den Shared-Memory blockiert
- *`pthread_cond_t cattr;`*
die Condition, die wartet, wenn die Barrier-Funktion aufgerufen wird
- *`int barrierall;`*
Die Abbruchbedingung, die die Barrier beendet
- *`pthread_cond_t allCreated;`*
Die Condition die wartet, wenn noch nicht alle Prozesse gestartet wurden.
- *`logger log`*
Das struct log
- *`process p[]`*
Ein Struct Array, welches alle Prozessdaten wie `pid` und `rank` des jeweiligen prozesses beinhaltet. Dieses Struct entspricht der Größe `sizeof(process * processAmmount)`

### Message-Struktur

```c
typedef struct {
    bool full;
    int srcRank;
    int destRank;
    char buffer[message_max_size];
    OSMP_Datatype datatype;
    int msgLen;
    int nextMsg;
} message;
```

Das struct Message liegt in dem Struct des Shared-Memory. Es hat folgende Inhalte:

- *`bool full;`*
Der Boolean, der agiebt, ob die Message voll ist oder nicht
- *`int srcRank;`*
Dieser Integer hat den Rank vom sendenden Prozess abgespeichert
- *`int destRank;`*
Dieser Integer hat den Rank vom empfangenden Prozess abgespeichert
- *`char buffer[message_max_size];`*
Dieses Array hat als Inhalt die eigendliche Nachticht und wird zu anfang auf die maximal zulässige Anzahl von Zeichen der Nachricht gesetzt
- *`OSMP_Datatype datatype;`*
Der Datentyp der Nachricht
- *`int msgLen;`*
Länge der Nachrich
- *`int nextMsg;`*
Das ist die Slot-ID der nächsten Nachricht.

## Process-Struktur

```c
typedef struct {
    message msg[OSMP_MAX_MESSAGES_PROC];
    pid_t pid;
    int rank;
    int firstEmptySlot;
    int numberOfMessages;
    int firstmsg;
    sem_t empty;
    sem_t full;
} process;
```

- *`message msg[OSMP_MAX_MESSAGE_PROC]`*
Das Message-Struct mit der maximalen Anzahl pro Prozess;
- *`pid_t pid;`*
Die Prozessnummer des Prozesses
- *`int rank;`*
der Rang des Prozesses
- `int firstEmptySlot`
Der Slot des Prozesses, der frei ist. Er ist gleich 16, wenn alle Plätze belegt sind
- *`int numberOfMessages;`*
Die Anzahl der Messages, die der Prozess noch nicht gelesen hat.
- *`int firstmsg`*
    
    Der Index der ersten Nachricht
    
- *`sem_t empty;`*
Die Semaphore, die wartet, wenn der Prozess keine Nachrichten mehr empfangen kann.
- *`sem_t full;`*
Die Semaphore, die wartet, wenn der Prozess keine Nachrichten hat.

### Broadcast-Struktur

```c
typedef struct {
    char buffer[message_max_size];
    int msgLen;
    OSMP_Datatype datatype;
    int srcRank;
} Bcast;
```

- *`char buffer[message_max_size];`*
Der Buffer für den B-Cast in der größe von `max_message_size`
- *`int msgLen;`*
Die Länge des Buffers
- *`OSMP_Datatype datatype;`*
Der Datentyp der Nachricht
- *`int srcRank;`*
Dieser Integer hat den Rank vom Sendenden Prozess abgespeichert

### Logger-Struktur

```c
typedef struct {
    int logIntensity;
    char logPath[256];
} logger;
```

- *`int logIntensity;`*
Der Log-Typ, der übergeben wird
- *`char logPath[256];`*
Der Logging-Path, der übergeben wird

## Semaphoren-Verwaltung

Hier sind die Mutex- und die Semaphorennutzung in unserem OSMP-Projekt erklärt:

Wir verwenden in unserem Programm insgesammt 3 Semaphoren. Ein Semaphore ist für die Messages, und ist dafür zuständig, die Prozesse zu blockieren, wenn mehr als 256 Nachrichten existieren. Die beiden anderen, sind jeweils dafür da, die schreibenden Prozesse warten zu lassen, wenn der Postkasten voll ist und die lesenden Prozesse warten zu lassen, wenn der Postkasten leer ist. 

### OSMP_Barrier

Der Aufruf pthread_mutex_lock blockiert den Zugriff auf den Mutex durch andere Prozesse und sperrt den Mutex. Es wird überprüft ob n-1 Prozesse bereits die Barriere erreicht haben Wenn die Bedingung falsch ist, wird der Prozess in eine Warteschleife gebracht, indem pthread_cond_wait aufgerufen wird. Dadurch wird der Thread in einen wartenden Zustand versetzt und der Mutex wird freigegeben, um anderen Prozessen den Zugriff auf den Mutex zu ermöglichen. Wenn die Bedingung wahr ist, werden die folgenden Schritte ausgeführt:

1. pthread_cond_broadcast(&shm->cattr) wird aufgerufen und befreit alle wartenden Prozesse.
2. Prozess n ruft pthread_mutex_unlock auf und ermöglicht einem Prozess aus der Warteschleife zu kommen.

Wenn der Prozess aus der Warteschleife herauskommt (Durch den pthread_cond_broadcast und dem danach folgendem unlock), versucht er erneut, den Mutex zu sperren und die Bedingung zu überprüfen. Wenn die Bedingung wahr ist, wird pthread_mutex_unlock aufgerufen, wenn jedoch die Bedingung weiterhin falsch ist, kehrt der Prozess in die Warteschleife zurück. Dies wird wiederholt, bis alle Prozesse die Barrier verlassen haben.

![OSMP_Barrier](./Images/OSMP_Barrier.png)

### OSMP_Bcast

Die OSMP_Bcast schaut als erstes, ob die Funktion vom senden oder den Empangenden Prozessen aufgerufen wurde. Die Mutexe sind bei den beiden Funktionen gleich. Es wird versucht die Mutex zu locken, damit die Prozesse nicht gegenseitig schreiben. Danach werden die Messages beschrieben, und dann der Mutex wieder freigegeben. 

![OSMP_Bcast](./Images/OSMP_Bcast.png)

### OSMP_Send

Die OSMP_Send prüft zuerst, ob die empty-Mutex frei ist, diese wird beim initialisieren auf 16 gesetzt. Wenn noch keine 16 Nachrichten angekommen sind kann der Prozess weitergehen, und zählt die Semaphore um eins nach unten. Danach wird der Mutex des shared-memorys angefordert, und sobald das funktioniert hat, der Shared-Memory des empfangenden Prozesses beschreiben. Danach wird die Mutex wieder freigegeben, und sem_post aufgerufen, sodass der (wartende) Empfängerprozess seine Nachricht lesen kann. 

![OSMP_Send](./Images/OSMP_Send.png)

### OSMP_Recv

Die OSMP_Send prüft zuerst, ob die full-Mutex frei ist, diese wird beim initialisieren auf 0 gesetzt. Wenn mindestens eine Nachricht angekommen sind kann der Prozess weitergehen, und zählt die Semaphore um eins nach oben. Danach wird der Mutex des shared-memorys angefordert, und sobald das funktioniert hat, der Shared-Memory des eigenen Prozesses gelesen. Danach wird die Mutex wieder freigegeben, und sem_post aufgerufen, sodass der (wartende) Sende-Prozess seine Nachricht schreiben kann. 

![OSMP_Recv](./Images/OSMP_Recv.png)

### OSMP_Size

Im der Funktion wird der mutex des SharedMems verwendet, um die Anzahl der Prozesse zurückzugeben. Der Zugriff auf den SharedMemory ist blockiert, solange der Mutex gesperrt ist. Sobald die Variable ausgelesen wurde, wird der Mutex freigegeben.

![OSMP_Size](./Images/OSMP_Size.png)

### *isend / *ircv

Die Mutexe dienen hier dazu, die Requests vor gleichzeitigen Zugriffen zu schützen. Zunächst wird der Mutex des Requests gelockt, bevor die Parameter in das IRequest-Struct kopiert werden. Danach wird der Thread erstellt und die Mutex wieder freigegeben. Dadurch wird sichergestellt, dass kein anderer Thread auf den Request zugreift, bevor dieser vollständig initialisiert wurde.

![*isend und *ircv](./Images/isend_ircv.png)

### OSMP_ISend

Um gleichzeitige Zugriffe auf den Request zu vermeiden, wird zunächst der Mutex der Request-Struktur gesperrt. Anschließend werden die Parameter des Requests auf die Funktion übertragen und ein neuer Thread wird erstellt, der die Funktion isend ausführt. Sobald der Thread erstellt wurde, wird der Mutex der Request-Struktur wieder freigegeben, damit andere Prozesse auf den Request zugreifen können.

![OSMP_ISend](./Images/OSMP_ISend.png)

### OSMP_Irecv

Zunächst wird der Mutex der IRequest-Struktur, auf die der Request zeigt, gesperrt, um gleichzeitige Zugriffe auf den Request zu vermeiden. Danach werden die Parameter des Requests auf die Parameter der Funktion kopiert. Anschließend wird ein neuer Thread erstellt, der mit der Funktion ircv aufgerufen wird. Sobald der Thread erstellt wurde, wird der Mutex der Request-Struktur wieder freigegeben, damit andere Prozesse auf den Request zugreifen können.

![OSMP_IRecv](./Images/OSMP_IRecv.png)
#
#
#
#
#
#
#
#
#
#
#
#

# Danke für's lesen!
#
![](./Images/sl.gif)