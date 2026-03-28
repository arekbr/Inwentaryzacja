#define MyAppName "Inwentaryzacja"
#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif
#ifndef MyAppSource
  #error MyAppSource must be defined
#endif
#ifndef MyAppOutput
  #error MyAppOutput must be defined
#endif

[Setup]
AppId={{E8E42F73-8ED7-490D-B1C1-620D244FAAB4}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher=SMOK
DefaultDirName={autopf}\Inwentaryzacja
DefaultGroupName=Inwentaryzacja
UninstallDisplayIcon={app}\Inwentaryzacja.exe
Compression=lzma
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\images\installericon.ico
OutputDir={#MyAppOutput}
OutputBaseFilename=Inwentaryzacja_{#MyAppVersion}_Windows_x64_Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#MyAppSource}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Inwentaryzacja"; Filename: "{app}\Inwentaryzacja.exe"
Name: "{group}\Odinstaluj Inwentaryzacja"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Inwentaryzacja"; Filename: "{app}\Inwentaryzacja.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Utwórz skrót na pulpicie"; Flags: unchecked

[Run]
Filename: "{app}\Inwentaryzacja.exe"; Description: "Uruchom Inwentaryzacja"; Flags: nowait postinstall skipifsilent
