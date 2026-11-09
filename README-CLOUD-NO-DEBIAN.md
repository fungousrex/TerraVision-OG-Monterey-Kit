# TerraVision originale su Monterey senza installare Debian sul Mac

## Che cosa fa questa modalità

Questa variante usa GitHub Codespaces come macchina Linux remota. Il Mac esegue soltanto Safari e il client web; il programma vero `TclTV` viene compilato ed eseguito nel Codespace, con X11/OpenGL Mesa, e la sua finestra viene trasmessa tramite noVNC. Non è il viewer HTML ricostruito: l'eseguibile resta il TclTV del sorgente TerraVision 6.0.

**Non è un'app macOS nativa e richiede un account GitHub/Codespaces, connessione Internet e le eventuali quote o tariffe del servizio.** Il container remoto è basato su Debian Bookworm ma non devi scaricare ISO, creare una VM, installare Debian o configurare UTM sul Mac.

## Preparazione una tantum

1. Scarica ed estrai `TerraVision-OG-Codespaces.zip` sul Mac.
2. Installa GitHub Desktop: https://desktop.github.com/ e accedi al tuo account GitHub.
3. Apri Terminale, entra nella cartella estratta e inizializza il repository locale con `git init -b main`, poi esegui `git add .` e `git commit -m "Import TerraVision 6.0 source"`. In GitHub Desktop scegli **File → Add Local Repository** e seleziona quella cartella.
4. In GitHub Desktop premi **Publish repository**, mantieni selezionato **Keep this code private** e pubblica. Non selezionare Public.
5. Apri il repository su github.com → **Code → Codespaces → Create codespace on main**. Scegli, se disponibile, una macchina da 4 core e almeno 8 GB RAM. Il devcontainer incluso costruisce automaticamente l'ambiente Linux x86_64 e abilita le librerie i386. Il desktop remoto viene avviato automaticamente; se non compare la porta 6080, nel terminale esegui `bash scripts/98-cloud-desktop.sh`.

## Compilazione

Nel terminale del Codespace:

```bash
bash scripts/99-build-original.sh
```

Questo compila Tcl/Tk 8.4.20 e il sorgente originale TerraVision. Il primo build richiede rete e può impiegare parecchio tempo. Se termina con errore, salva il log `~/TerraVision-OG/build-terravision.log`: non procedere con patch casuali.

## Avviare la finestra originale nel browser

Nel terminale del Codespace:

```bash
bash scripts/98-cloud-desktop.sh
```

Apri la scheda **PORTS**, individua la porta **6080** e seleziona **Open in Browser**. La pagina noVNC mostra il desktop Linux remoto.

Torna al terminale del Codespace e lancia:

```bash
bash scripts/97-cloud-launch-terravision.sh
```

La finestra originale TclTV apparirà nel desktop remoto. Se il renderer OpenGL non inizializza nel display virtuale, controlla `~/.tv-cloud/xvfb.log`, `~/.tv-cloud/websockify.log` e il log di build. Il rendering remoto usa Mesa software: il programma è quello originale ma le prestazioni grafiche non equivalgono alla GPU Intel del Mac.

## Dati geografici

La sorgente non include le piramidi originali `.oi` e `.dem`. Il catalogo incluso si riferisce a un server storico e la disponibilità attuale non è garantita. Per una scena reale bisogna importare/creare dataset TerraVision con `make_oi` e `make_dem` o recuperare dataset compatibili.

## Sicurezza e costi

- Repository GitHub **Private**.
- Non rendere pubblica la porta 6080; lascia la visibilità **Private**.
- Chiudi/elimina il Codespace quando hai finito: può consumare quote di calcolo/storage.
- Il server VNC ascolta su localhost nel container; noVNC usa il port-forward autenticato di Codespaces.

## Limiti dichiarati

Questa procedura non è stata eseguita end-to-end su un Codespace reale in preparazione del kit. I file e la configurazione sono predisposti, ma la compilazione può evidenziare incompatibilità del codice del 2002 con toolchain e pacchetti attuali. Non viene dichiarato un build riuscito finché il log non mostra la produzione dell'eseguibile ELF i386 e il controllo `05-check-original-build.sh` passa.
