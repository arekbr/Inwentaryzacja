# Inwentaryzacja v1.3.0-beta.1 (pre-release)

Cel: wersja beta do testów stabilności przed pełnym wydaniem.

Najważniejsze zmiany:
- Usprawnione filtry i autouzupełnianie (mniej zbędnych połączeń, cache nazw).
- Reuse `QGraphicsScene` dla miniaturek zdjęć (mniej alokacji, stabilniejsze UI).
- Transakcje przy zapisie rekordów i zdjęć; poprawiona obsługa błędów.
- Migracja UUID uruchamiana bezpiecznie (pomijana na pustej bazie, po schemacie SQLite).
- Skrypty Windows: stabilniejsze ścieżki, build Release, ikona EXE, upload instalatora do GitHub Release.
- Skrypty dla Debiana działają bez twardych ścieżek; wykrywanie systemowego `libqsqlmysql.so`.

Naprawiono:
- Zapis nowych rekordów dla MySQL (UUID bez klamer, zgodne z VARCHAR(36)).
- Okno listy uruchamia się w trybie zmaksymalizowanym po wyborze bazy.
- Styl STANDARD: przywrócono natywną strzałkę list rozwijanych.
- CMake akceptuje wersje z sufiksem beta w `version.txt`.

Instalacja/uruchomienie (Debian):
1. `./scripts_build_debian/1.bootstrap_linux.sh`
2. `./scripts_build_debian/3.build_inwentaryzacja_linux.sh`
3. `./scripts_build_debian/4.deploy_linux.sh`
4. `./scripts_build_debian/6.make_deb_linux.sh`

Instalacja/uruchomienie (Windows):
1. `.\scripts_build_windows\1.bootstrap_windows.ps1`
2. `.\scripts_build_windows\2.build_qt_mysql_plugin_windows.ps1`
3. `.\scripts_build_windows\3.build_project_windows.ps1`
4. `.\scripts_build_windows\4.deploy_windows.ps1`
5. `.\scripts_build_windows\6.make_installer_windows.ps1`

Uwagi:
- Jeśli brakuje `libqsqlmysql.so`, doinstaluj `libqt6sql6-mysql` albo zbuduj plugin ze źródeł Qt.
- To pre-release: zbieramy feedback przed finalnym wydaniem.
