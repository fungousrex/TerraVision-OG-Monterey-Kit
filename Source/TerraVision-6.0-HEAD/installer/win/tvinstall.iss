;
; TerraVision installer script for the Windows platform.
;
; This is used by Inno Setup: http://www.jrsoftware.org/
;

[Setup]
AppName=TerraVision
AppVersion=6.0
AppVerName=TerraVision 6.0
AppPublisher=SRI International
AppPublisherURL=http://www.tvgeo.com
AppSupportURL=http://www.tvgeo.com
AppUpdatesURL=http://www.tvgeo.com
DefaultDirName={pf}\TerraVision 6.0
; show our custom uninstall icon for Win2k Add/Remove Programs
UninstallDisplayIcon={app}\icons\tv32.ico
; The name to create in the Start Menu - don't let user change it
DefaultGroupName=TerraVision 6.0
DisableProgramGroupPage=yes
; don't show component lists (we don't use any anyway)
AlwaysShowComponentsList=no
; highest ZIP compression possible
Compression=zip/9
; always provide an uninstall icons
AlwaysCreateUninstallIcon=yes
; require admin privileges for the install of the msvcrt system DLLs
AdminPrivilegesRequired=1
; specify the output installer .exe filename
OutputBaseFilename=tvinstall
OutputDir=.
; uncomment the following line if you want your installation to run on NT 3.51 too.
MinVersion=4,3.51

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
; install the TerraVision files binaries and DLLs
Source: "tcltk\bin\*.exe"; DestDir: "{app}\bin"; CopyMode: alwaysoverwrite
Source: "tcltk\bin\nspr4.dll"; DestDir: "{app}\bin"; CopyMode: alwaysoverwrite
Source: "tcltk\bin\tcl*.dll"; DestDir: "{app}\bin"; CopyMode: alwaysoverwrite
Source: "tcltk\bin\tk*.dll"; DestDir: "{app}\bin"; CopyMode: alwaysoverwrite
; install the release notes for this version
Source: "tcltk\README.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite;
; install the Tcl/Tk scripts for TerraVision's GUI
Source: "tcltk\tcl\*.*"; DestDir: "{app}\tcl"; CopyMode: alwaysoverwrite
; install the configuration files, e.g. the initial dataset database
Source: "tcltk\config\*.*"; DestDir: "{app}\config"; CopyMode: alwaysoverwrite
; install the Tcl/Tk library files
Source: "tcltk\lib\*.*"; DestDir: "{app}\lib"; CopyMode: alwaysoverwrite; Flags: recursesubdirs
; install the default icons
Source: "tcltk\icons\*.*"; DestDir: "{app}\icons"; CopyMode: alwaysoverwrite
; install the msvcrt DLL, if not present (just in case)
Source: "tcltk\bin\msvcrt.dll"; DestDir: "{sys}"; CopyMode: alwaysskipifsameorolder; Flags: restartreplace uninsneveruninstall

[Icons]
Name: "{group}\TerraVision"; Filename: "{app}\bin\TclTV.exe"; HotKey: "ctrl+alt+t"
Name: "{group}\Release Notes"; Filename: "{app}\README.txt"
Name: "{group}\Documentation\TerraVision Web Site"; Filename: "http://www.tvgeo.com/"; MinVersion: 4,4
Name: "{group}\Documentation\TerraVision FAQ"; Filename: "http://www.tvgeo.com/faq.shtml"; MinVersion: 4,4
Name: "{group}\Documentation\TerraVision User Guide"; Filename: "http://www.tvgeo.com/user_guide/"; MinVersion: 4,4
Name: "{userdesktop}\TerraVision"; Filename: "{app}\bin\TclTV.exe"; MinVersion: 4,4; Tasks: desktopicon

[Registry]
Root: HKLM; Subkey: "Software\SRI International\TerraVision"; ValueType: string; ValueName: "InstallDirectory"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\SRI International\TerraVision"; ValueType: string; ValueName: "CurrentVersion"; ValueData: "6.0"; Flags: uninsdeletekey

[Run]
Filename: "notepad.exe"; Parameters: "{app}\README.txt"; Description: "Read the Release Notes"; Flags: nowait postinstall skipifsilent
Filename: "{app}\bin\TclTV.exe"; Description: "Launch TerraVision"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: dirifempty; Name: "{app}"

