# Universal macOS Build Scripts dla Inwentaryzacja

## 🎯 Cel
Te skrypty umożliwiają budowanie aplikacji Inwentaryzacja jako **Universal Binary** dla macOS, wspierającego zarówno architektury **Apple Silicon (ARM64)** jak i **Intel (x86_64)**.

## 📋 Wymagania

### System
- **macOS Ventura (13.0) lub nowszy**
- **Xcode 15.2+ z SDK 14.0+** (zalecane dla universal builds)
- **Xcode Command Line Tools**

### Narzędzia
- **Homebrew** (`/opt/homebrew` dla ARM, opcjonalnie `/usr/local` dla Intel)
- **CMake 3.20+**
- **Ninja** (generator build)
- **Qt 6.9.0** (jedna z opcji):
  - Universal binary Qt (zalecane)
  - Osobne instalacje dla ARM64 i x86_64
  - Qt z MaintenanceTool

### Biblioteki
- **MariaDB** (przez Homebrew)
- **OpenSSL 3+**
- **ICU4C**

## 🚀 Sposób użycia

### Krok 1: Bootstrap środowiska
```bash
chmod +x scripts_build_macos_universal/*.sh
cd /ścieżka/do/projektu/Inwentaryzacja
cp scripts_build_macos_universal/*.sh .
./1.bootstrap_macos_universal.sh
```

Bootstrap zapyta o:
- **Tryb budowania**: Universal / Native / Intel only / ARM64 only
- **Konfigurację Qt**: Universal Qt / Dual-arch Qt / Single Qt
- **Ścieżki** do instalacji Qt

### Krok 2: Budowa pluginu MySQL (opcjonalnie)
```bash
./2.build_qt_mysql_plugin_macos_universal.sh
```
- Buduje plugin `libqsqlmysql.dylib` dla wybranych architektur
- Łączy w universal binary (jeśli dual-arch)
- Wymaga źródeł Qt

### Krok 3: Sanityzacja pluginów SQL (opcjonalnie) 
```bash
./3.sanitize_qt_sql_plugins_macos_universal.sh
```
- Usuwa problematyczne pluginy SQL
- Sprawdza missing dependencies
- Tworzy backup

### Krok 4: Budowa aplikacji
```bash
./4.build_inwentaryzacja_macos_universal.sh
```
- Buduje główną aplikację
- Tworzy universal binary (jeśli wybrany)
- Weryfikuje architektury

### Krok 5: Deployment
```bash
./5.deploy_macos_universal.sh
```
- Kopiuje wszystkie zależności
- Naprawia ścieżki bibliotek (`install_name_tool`)
- Tworzy relocatable bundle
- Generuje skrypt uruchamiający

### Krok 6: Uruchomienie i test
```bash
./6.run_inwentaryzacja_macos_universal.sh
```
- Weryfikuje aplikację
- Oferuje opcje debugowania
- Test kompatybilności architektur

### Krok 7: Pakowanie DMG
```bash
./7.make_dmg_macos_universal.sh
```
- Tworzy bundle `.app`
- Generuje `Info.plist`
- Pakuje do DMG z installer UI
- Wersjonowanie automatyczne

## 🏗️ Obsługiwane tryby budowania

### 1. Universal Binary (zalecane)
```bash
# Wymaga:
# - Universal Qt LUB
# - Osobne Qt dla ARM64 + x86_64
# - Odpowiednie MariaDB dla każdej arch

Wynik: Jedna aplikacja działająca na obu architekturach
```

### 2. Native Build
```bash
# Buduje dla aktualnej architektury systemu
# Prostsze, szybsze
Wynik: arm64 na Apple Silicon, x86_64 na Intel Mac
```

### 3. Intel Only
```bash  
# Wymusza x86_64 (działa przez Rosetta na ARM)
Wynik: Aplikacja x86_64
```

### 4. Apple Silicon Only
```bash
# Wymusza arm64 (nie działa na Intel Mac)
Wynik: Aplikacja arm64  
```

## 📁 Struktura po budowie

```
projekt/
├── qt_env_universal.sh              # Konfiguracja środowiska
├── build_inwentaryzacja_universal/  # Build universal (lub _intel/_arm)
├── build_qt_sql_drivers_universal/  # MySQL plugin universal
├── deploy/                          # Gotowa aplikacja + deps
│   ├── Inwentaryzacja              # Główna aplikacja  
│   ├── run_Inwentaryzacja.sh       # Skrypt uruchamiający
│   ├── *.dylib                     # Biblioteki
│   ├── platforms/                  # Qt platform plugins
│   ├── sqldrivers/                 # SQL drivers (+ MySQL)
│   └── imageformats/               # Image format plugins
└── Inwentaryzacja_X.Y.Z_Universal.dmg  # Finalny DMG
```

## ⚙️ Konfiguracja Qt

### Opcja A: Universal Qt (najprościej)
```bash
# Qt zbudowane jako universal binary
# Jedna instalacja obsługuje obie architektury
~/Qt/6.9.0/macos/  # Universal installation
```

### Opcja B: Dual-arch Qt  
```bash
# Osobne instalacje dla każdej architektury
~/Qt/6.9.0/macos_intel/     # x86_64
~/Qt/6.9.0/macos_arm/       # arm64
```

### Opcja C: Single-arch Qt
```bash
# Tylko dla native/single builds
~/Qt/6.9.0/macos/           # Jedna architektura
```

## 🔧 Rozwiązywanie problemów

### Plugin MySQL nie buduje się
```bash
# Sprawdź źródła Qt:
ls ~/Qt/6.9.0/Src/qtbase/src/plugins/sqldrivers/mysql/

# Zainstaluj MariaDB:
brew install mariadb

# Sprawdź ścieżki w bootstrap
```

### Universal binary nie powstaje
```bash
# Sprawdź czy Qt jest universal:
file ~/Qt/6.9.0/macos/bin/qmake

# Sprawdź czy Homebrew ma obie architektury:
lipo -info /opt/homebrew/lib/libmariadb.dylib
```

### Aplikacja nie uruchamia się
```bash
# Sprawdź kod signing:
codesign -dv deploy/Inwentaryzacja

# Usuń quarantine:
xattr -cr deploy/

# Debug dependencies:
export QT_DEBUG_PLUGINS=1
./deploy/run_Inwentaryzacja.sh
```

### Gatekeeper blokuje aplikację
```bash
# System Preferences → Security & Privacy → Allow
# Lub podpisz kod:
codesign -s "Developer ID" deploy/Inwentaryzacja
```

## 📊 Weryfikacja architektury

```bash
# Sprawdź architekturę aplikacji:
lipo -info deploy/Inwentaryzacja

# Sprawdź wszystkie komponenty:
find deploy -name "*.dylib" -exec lipo -info {} \;

# Sprawdź aplikację w DMG:
hdiutil attach Inwentaryzacja_*.dmg
lipo -info "/Volumes/Inwentaryzacja */Inwentaryzacja.app/Contents/MacOS/Inwentaryzacja"
```

## 🔄 Workflow CI/CD

```bash
# Dla automatizacji:
export CI_BUILD=1  # Wyłącza interaktywne prompty
./1.bootstrap_macos_universal.sh
./2.build_qt_mysql_plugin_macos_universal.sh  
./4.build_inwentaryzacja_macos_universal.sh
./5.deploy_macos_universal.sh
./7.make_dmg_macos_universal.sh

# Wynik: Inwentaryzacja_VERSION_Universal.dmg
```

## 🆚 Różnice vs skrypty Ventura

| Aspekt | Ventura Scripts | Universal Scripts |
|--------|----------------|-------------------|
| **Architektury** | Single (host) | Universal / Multi-arch |
| **Qt Support** | Single install | Universal + Dual-arch |
| **MariaDB** | Single Homebrew | Multi-arch aware |
| **DMG** | Basic | Bundle + installer UI |
| **Deployment** | Simple copy | Relocatable bundle |
| **Compatibility** | Ventura+ | Ventura+ Universal |

## 💡 Wskazówki

- **Universal builds** zajmują ~2x więcej miejsca ale działają na wszystkich Mac
- **Dual-arch setup** wymaga ~2x więcej czasu budowania  
- **Plugin MySQL** jest opcjonalny - SQLite zawsze działa
- **DMG packaging** automatycznie wykrywa architekturę i odpowiednio nazywa pliki
- **Deploy directory** jest relocatable - można kopiować między maszynami

## 🔗 Powiązane pliki

- `CMakeLists.txt` - Konfiguracja CMake z obsługą macOS
- `version.txt` - Wersjonowanie automatyczne
- `.vscode/launch.json` - Debug config (Linux)
- `scripts_build_macos_ventura/` - Oryginalne skrypty single-arch
