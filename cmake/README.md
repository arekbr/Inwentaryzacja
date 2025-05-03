# Inwentaryzacja

Aplikacja do inwentaryzacji retro komputerów, zbudowana w Qt 6. Ten projekt jest w pełni zautomatyzowany – po sklonowaniu repozytorium wystarczy uruchomić jeden skrypt, aby pobrać wszystkie zależności, skompilować projekt i uzyskać gotową aplikację.

## Wymagania
- Dostęp do internetu (do pobierania Qt, vcpkg i zależności).
- System operacyjny: Windows, Linux, macOS (Intel lub ARM).
- Nic więcej! Wszystkie narzędzia (CMake, Qt, MinGW, vcpkg) zostaną pobrane automatycznie.

## Instrukcja budowania

1. **Sklonuj repozytorium**:
   ```bash
   git clone https://github.com/twoje_repo/projektinwentaryzacja.git
   cd projektinwentaryzacja
   ```

2. **Uruchom skrypt bootstrap**:
   - **Linux/macOS**:
     ```bash
     chmod +x bootstrap.sh
     ./bootstrap.sh
     ```
   - **Windows** (w Command Prompt lub PowerShell):
     ```cmd
     bootstrap.bat
     ```

3. **Gotowe!**
   - Skrypt pobierze i skonfiguruje wszystkie narzędzia (Qt, CMake, vcpkg, MinGW na Windows).
   - Zbuduje projekt i zależności (MySQL Connector/C, OpenSSL, pluginy Qt).
   - Gotowa aplikacja będzie w folderze `build/install`:
     - **Linux**: Uruchom `build/install/bin/Inwentaryzacja`.
     - **macOS**: Uruchom `open build/install/Inwentaryzacja.app`.
     - **Windows**: Uruchom `build\install\bin\Inwentaryzacja.exe`.

## Użycie w Qt Creatorze
1. Otwórz Qt Creator.
2. Wybierz `File > Open File or Project` i wskaż `CMakeLists.txt`.
3. Skonfiguruj kit (użyj Qt z folderu `qt` w repozytorium).
4. Kliknij `Build > Run CMake`, potem `Build > Build Project`.

## Uwagi
- Pierwsza kompilacja może potrwać dłużej (pobieranie Qt i zależności).
- Jeśli masz już Qt lub MinGW, skrypt je wykryje i użyje.
- Projekt działa na Windows (MinGW), Linux, macOS (Intel/ARM).

## Licencja
[Tu wstaw licencję, np. MIT]

## Kontakt
[Tu wstaw dane kontaktowe]