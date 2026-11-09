# TerraVision 6.0 — threading originale (ispezione del sorgente)

Questa nota descrive il codice presente nello snapshot `Source/TerraVision-6.0-HEAD`; non attribuisce thread a funzioni senza una chiamata esplicita nel sorgente.

## Layer di astrazione

- API pubblica: `pthr/report.h`, funzione `pthr_create`.
- Implementazione: `pthr/report.c`, funzione `pthr_create` (HEAD, circa riga 549).
- La funzione delega la creazione a `PR_CreateThread(PR_USER_THREAD, ..., PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 1<<16)`.
- NSPR è il runtime portabile incluso in `nsprnew/`.
- `nsprnew/config/Linux.mk` seleziona per Linux `USE_PTHREADS=1`, `IMPL_STRATEGY=_PTH` e definisce `_REENTRANT`, salvo override `CLASSIC_NSPR=1`.

## Call site verificati

| File | Chiamata / nome passato | Funzione funzionale |
|---|---|---|
| `TerraVision/ThreeDCore.c` | `pthr_create(..., "Visibility 3D", ...)` | Visibilità 3D |
| `TerraVision/ThreeDCore.c` | `pthr_create(..., ".geo Discovery", ...)` | Discovery delle risorse geografiche |
| `TerraVision/TsCache.c` | `pthr_create(..., name, ...)` | Lettura/cache delle tile |
| `TerraVision/DotGeo.c` | `pthr_create(..., "Load Terrain", ...)` | Caricamento terreno |
| `TerraVision/DotGeo.c` | `pthr_create(..., "Load Model", ...)` | Caricamento modelli |
| `tsm/tsmConnection_local.c` | `pthr_create(..., buf, ...)` | Gestione connessione locale |
| `tsm/tsmHost_web.c` | `pthr_create(..., buf, ...)` | Worker per connessioni Web |
| `tsm/tsmTile_dpss.c` | `pthr_create(..., "DPSS Requester", ...)` | Richieste DPSS |
| `tsm/tsmTile_dpss.c` | `pthr_create(..., "DPSS Reconnect", ...)` | Riconnessione DPSS |
| `TerraVision/OAA.c` | `pthr_create(..., "OAA", ...)` | Ciclo OAA |

## Cosa significa per Monterey

La build raccomandata compila l'originale come ELF i386 dentro Debian x86_64, con NSPR Linux e pthread POSIX. Sono thread reali dell'applicazione ospite. Su un Mac Intel la VM può eseguire più vCPU; su Apple Silicon la VM x86_64 è emulata e si consiglia di cominciare con 1 vCPU: i thread restano concorrenti e schedulati dal guest, ma non possono essere eseguiti contemporaneamente su più core virtuali se la VM ne espone uno solo.

Questo non è un porting del motore ai thread nativi di macOS e non è un rendering WebGL: è il codice TerraVision storico eseguito nel proprio ambiente Linux compatibile.
