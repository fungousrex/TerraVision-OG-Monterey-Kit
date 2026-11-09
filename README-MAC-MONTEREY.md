# TerraVision 6.0 originale — guida Monterey / build Linux i386

Questa cartella NON contiene il viewer HTML ricostruito. Contiene lo snapshot del sorgente storico SRI TerraVision 6.0 (revisioni HEAD ricostruite dal repository CVS/RCS originale) e una procedura di build per ottenere l'applicazione originale `TclTV` dentro una VM Linux su macOS Monterey.

## Stato reale del materiale

- Archivio CVS dell'utente: `terravision.zip`, 19 MB, ZIP verificato con `unzip -t`; SHA-256 osservato durante l'analisi: `a890f3c461b3745fe47a5e7f068c98a6e4c3b357267af7e24f5d60ef68e2e810`.
- Snapshot sorgente ricostruito: 3.705 file dalle revisioni HEAD RCS, 56.845.622 byte, 30 directory principali; nessun errore nel parsing delle 3.705 revisioni.
- `RELEASE` identifica `TERRAVISION_RELEASE "6.0"`, data RCS 31 ottobre 2002.
- L'archivio `terravision.zip` dell'utente conserva il repository RCS/CVS storico e le revisioni. Il file ZIP originale è incluso senza modifiche in `Source/original-archives/terravision-cvs-original.zip`. La cartella `Source/TerraVision-6.0-HEAD` è la working tree snapshot leggibile: non contiene i metadati `,v` né tutta la cronologia.
- Il precedente `TclTV` sciolto è un ELF Linux i386, non un eseguibile macOS. Il suo runtime storico (Tcl/Tk 8.4, GLX/X11 e librerie Linux) non può essere lanciato direttamente in Monterey.

## Perché VM Linux, non app macOS nativa

Il front-end originale `TclTV` usa Tk/Togl e OpenGL GLX/X11 (`TclTV/main.c`, `libTogl`, `glt/glmX.c`, `TerraVision/TVGLX.c`). Il codice NSPR incluso è del periodo 1998–2002; il suo supporto Linux presuppone ABI x86 a 32 bit. La strategia meno invasiva è quindi una VM Debian x86_64 e una build i386 con `gcc -m32`. Non si converte il renderer in Metal/WebGL e non si sostituisce il motore.

## 1. UTM e Debian

1. Su macOS Monterey (macOS 12) installa UTM dalla [pagina delle release ufficiali](https://github.com/utmapp/UTM/releases). La release stabile corrente elencata è v5.0.8; la linea 5.0 richiede macOS 12 o superiore, quindi Monterey è compatibile.
2. Scarica Debian 12.15 **amd64 netinst** da <https://www.debian.org/releases/bookworm/debian-installer/>.
3. Crea la VM con architettura **x86_64** e installa Debian con ambiente desktop **Xfce**. Assegna 6–8 GB RAM e almeno 40 GB di disco.
   - Mac Intel: scegli **Virtualize x86_64** e assegna 4 vCPU come punto di partenza.
   - Mac Apple Silicon: scegli **Emulate x86_64** (non aarch64). Parti da **1 vCPU**: l'emulazione CPU x86 su ARM è più delicata e UTM avverte che forzare molti core può causare problemi di correttezza. Il motore mantiene comunque i suoi thread POSIX; su 1 vCPU verranno schedulati dal kernel, ma non eseguiti in parallelo su più core.
4. Durante l'installazione Debian scegli Xfce e i normali strumenti di sistema. La rete della VM deve funzionare per scaricare pacchetti e Tcl/Tk.

Debian 12.15 è in LTS fino al 30 giugno 2028 e il supporto LTS comprende i386 e amd64. Debian segnala tuttavia che Bookworm è stata superata da Debian 13: qui la scegliamo intenzionalmente per la compatibilità più conservativa con codice del 2002.

## 2. Porta la cartella del kit nella VM

Scarica `TerraVision-OG-Monterey-Kit.zip` sul Mac e crea una cartella condivisa UTM che punti alla cartella in cui si trova lo ZIP. Nel guest Debian, monta la condivisione secondo il backend UTM: per QEMU/VirtFS usa `sudo mkdir -p /mnt/utm && sudo mount -t 9p -o trans=virtio,version=9p2000.L share /mnt/utm`; per il backend Apple/VirtioFS usa `sudo mkdir -p /mnt/utm && sudo mount -t virtiofs share /mnt/utm`. I due metodi sono alternativi, non vanno eseguiti entrambi.

Copia il kit dalla condivisione nella home della VM e lavora su una copia locale (non compilare direttamente dal mount):

```bash
cp /mnt/utm/TerraVision-OG-Monterey-Kit.zip "$HOME/"
cd "$HOME"
unzip -q TerraVision-OG-Monterey-Kit.zip
cd TerraVision-OG-Monterey-Kit
```

Se il mount appare con un nome diverso, usa il percorso effettivo visibile in `/mnt/utm`. In alternativa, con la condivisione SPICE WebDAV attiva, UTM la espone al guest su `http://127.0.0.1:9843`; `spice-webdavd` e `davfs2` sono compresi nello script dipendenze.

Per eseguire tutto il percorso di build con un solo comando (dipendenze, test i386, Tcl/Tk 8.4.20 e TerraVision), usa:

```bash
bash scripts/99-build-original.sh
```

In alternativa, per eseguire manualmente i passaggi e fermarti dopo ogni fase:

```bash
bash scripts/00-debian-dependencies.sh
bash scripts/01-check-32bit.sh
```

Il secondo comando deve stampare `ELF 32-bit` e `TerraVision i386 toolchain: OK`. Se fallisce, non procedere: manca il supporto multilib/i386. Lo script installa anche Motif e GLw perché il progetto originale configura automaticamente il frontend XmTV quando trova X11, oltre a TclTV.

## 3. Costruisci Tcl/Tk 8.4.20 a 32 bit

```bash
bash scripts/02-build-tcltk84.sh
```
Lo script usa un solo job come valore predefinito, in modo conservativo anche con l'emulazione x86_64 su Apple Silicon. Su un Mac Intel puoi aumentare la velocità, ad esempio con `TV_JOBS=2 bash scripts/02-build-tcltk84.sh`; applica lo stesso prefisso al comando di build TerraVision se vuoi più job.

Lo script scarica gli archivi ufficiali Tcl 8.4.20 e Tk 8.4.20 dal sito Tcl, controlla gzip, compila in ABI i386 e li installa in `~/opt/terravision-tcl84`. Non abilita i thread interni di Tcl: il parallelismo del motore TerraVision è implementato dal layer NSPR/pthr, non dal mainloop Tk.

## 4. Compila il TerraVision originale

```bash
bash scripts/03-build-original-terravision.sh
```
Anche qui il default è un job. Su Mac Intel puoi eseguire, ad esempio, `TV_JOBS=4 bash scripts/03-build-original-terravision.sh`; su Apple Silicon emulato consiglio di restare a un job.

Lo script:

- copia lo snapshot sorgente in `~/TerraVision-OG/source` senza modificare quello incluso nel kit, quindi rende scrivibile la sola working copy perché le revisioni RCS conservano permessi storici di sola lettura;
- applica SOLO alla working copy la correzione necessaria affinché il vecchio makefile NSPR `config/Linux.mk`, che forza `CC=gcc` e `CCC=g++`, usi `gcc -m32` e `g++ -m32`;
- configura il progetto con `--build=i686-pc-linux-gnu --host=i686-pc-linux-gnu`, quindi la vecchia NSPR seleziona il backend Linux x86;
- passa `--with-pthreads`, selezionando il backend POSIX pthreads;
- punta TclTV alla distribuzione Tcl/Tk 8.4.20 e compila il progetto in una directory separata.

Il log viene salvato in `~/TerraVision-OG/build-terravision.log`. La compilazione completa non è stata eseguita in questo ambiente di preparazione: codice del 2002 e compilatori moderni possono richiedere ulteriori correzioni. Se lo script si ferma, conserva il log completo; è il dato necessario per correggere il primo errore reale senza patch casuali.

## 5. Avvia l'app originale

Dalla cartella del kit:

```bash
bash scripts/04-run-original-terravision.sh
```

Il launcher prepara `~/.tv/` solo per i file mancanti, conserva quelli già presenti, controlla che `TclTV` sia ELF i386 e che `ldd` non segnali librerie mancanti, poi avvia il vero eseguibile compilato `TclTV` passando `-tcldir` alla directory Tcl originale.

Il controllo può essere richiamato anche separatamente con `bash scripts/05-check-original-build.sh`.

Per avvio manuale:

```bash
export TVHOME="$HOME/TerraVision-OG/source"
export TCL_LIBRARY="$HOME/opt/terravision-tcl84/lib/tcl8.4"
export TK_LIBRARY="$HOME/opt/terravision-tcl84/lib/tk8.4"
export LD_LIBRARY_PATH="$HOME/opt/terravision-tcl84/lib:$HOME/TerraVision-OG/build/nsprnew/dist/lib"
cd "$HOME/TerraVision-OG/source/TclTV"
"$HOME/TerraVision-OG/build/TclTV/TclTV" -tcldir "$HOME/TerraVision-OG/source/TclTV"
```

## 6. Threading originale: cosa viene mantenuto

Il sorgente avvia thread separati per attività di visibilità e discovery (`TerraVision/ThreeDCore.c`, `pthr_create`), lettura/cache (`TerraVision/TsCache.c`), caricamento terreno e modelli (`TerraVision/DotGeo.c`), richieste Web e connessioni (`tsm/tsmHost_web.c`, `tsm/tsmConnection_local.c`). L'API di astrazione è nel modulo `pthr/`; NSPR fornisce il runtime portabile. L'opzione `--with-pthreads` seleziona pthread POSIX su Linux. Non confondere `pthr/` (wrapper/sincronizzazione) con il sottoprogetto obsoleto `thread/`.

## 7. Dati geografici: limite del repository

Il codice contiene gli strumenti di produzione `tsmUtils/make_oi.c` e `tsmUtils/make_dem.c` e la documentazione `docs/guide/create/index.html`. Il formato di lavoro originale è una coppia di piramidi multi-risoluzione: OI (immagini) e DEM (elevazione), co-registrate geograficamente. Gli esempi storici usano PPM/PGM/RGB per immagini e raw 32-bit per elevazione.

Il sorgente non contiene i dataset geografici completi `.oi`/`.dem`: i 38 JPG e 3 PNG rilevati sono immagini di manuale/test. `installer/win/tcltk/config/database.xml` punta a `http://terravision.dsi.internet2.edu/tvdata`, endpoint del 2001; il DNS non si è risolto nell'ambiente di analisi, quindi la sua disponibilità oggi non è confermata. Il launcher copia il database storico così com'è, senza inventare URL sostitutivi.

Quando il motore è compilato, per aprire un terreno locale il percorso storico documentato è: menu **File → Open Dataset → Open Terrain**, pulsante **Servers**, **Add Server**, selezionare la directory che contiene il `PyramidUrls`/piramidi e poi scegliere il geoset. Per generare una coppia locale si potranno usare `make_oi` e `make_dem` inclusi nel progetto; prima occorre verificare i loro help e i vincoli del dataset di partenza.

## 8. Inventario sorgente

Directory principali (30). L'elenco dei 3.705 file e delle dimensioni in byte è in `docs/SOURCE-FILELIST.tsv`; i thread sono mappati in `docs/ARCHITETTURA-THREADING.md`.

`DIS`, `PluginTV`, `TclTV`, `TerraVision`, `XmTV`, `conversions`, `docs`, `etc`, `ghttp`, `glt`, `installer`, `jpeg`, `libTogl`, `libgeotiff`, `libpng`, `libungif`, `libxml`, `nsprnew`, `nsprwin`, `plugin`, `pthr`, `thread`, `tiff`, `tsm`, `tsmUtils`, `tvutils`, `vrml2`, `winbuild`, `xpm`, `zlib`.

File importanti:
- `RELEASE`, `README`, `INSTALL`, `LICENSE`, `ChangeLog` — release, architettura, istruzioni, licenza e storia.
- `TerraVision/ThreeDCore.c`, `ThreeDDraw.c`, `ThreeDNav.c`, `Visible.c`, `TsCache.c`, `DotGeo.c`, `TVGLX.c` — motore 3D, visibilità, cache, caricamento e GLX.
- `TclTV/main.c`, `TclTV/TerraVision.tcl`, `TclTV/*options.tcl`, `TclTV/paths.tcl` — frontend originale.
- `pthr/report.c`, `pthr/report.h`, `pthr/timestamp.c` — astrazione thread e primitive di sincronizzazione.
- `nsprnew/` — Netscape Portable Runtime incorporato.
- `libTogl/` — integrazione Tk/OpenGL.
- `tsm/` — tsmApi, database, connessioni locali/Web/WMS e tile.
- `tsmUtils/make_oi.c`, `tsmUtils/make_dem.c`, `tsmUtils/database.c`, `tsmUtils/untile.c` — strumenti sui dataset.
- `installer/win/tcltk/config/` — config/database/viewpoints e quattro path originali.
- `docs/user_guide/`, `docs/guide/create/`, `docs/tvbase/` — manuale e API.

## Verifica e limiti

Completato nell'analisi: integrità ZIP, parsing di tutti i 3.705 archivi RCS, ricostruzione delle revisioni HEAD, inventario dei moduli, verifica del codice di startup/config e analisi statica della pipeline pthread/GLX.

Non ancora completato: build completa del programma, avvio di TclTV in una VM, rendering GLX nel Mac dell'utente, caricamento di un dataset reale. Il kit prepara una procedura riproducibile per provarli; non dichiara questi test superati.
