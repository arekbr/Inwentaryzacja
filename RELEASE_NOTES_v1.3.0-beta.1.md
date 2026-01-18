# Inwentaryzacja v1.3.0-beta.1 (pre-release)

Cel: wersja beta do testów stabilności przed pełnym wydaniem.

Najważniejsze zmiany:
- Usprawnione filtry i autouzupełnianie (mniej zbędnych połączeń, cache nazw).
- Reuse `QGraphicsScene` dla miniaturek zdjęć (mniej alokacji, stabilniejsze UI).
- Transakcje przy zapisie rekordów i zdjęć; poprawiona obsługa błędów.
- Migracja UUID uruchamiana bezpiecznie (pomijana na pustej bazie, po schemacie SQLite).
- Skrypty dla Debiana działają bez twardych ścieżek; wykrywanie systemowego `libqsqlmysql.so`.

Instalacja/uruchomienie (Debian):
1. `./scripts_build_debian/1.bootstrap_linux.sh`
2. `./scripts_build_debian/3.build_inwentaryzacja_linux.sh`
3. `./scripts_build_debian/4.deploy_linux.sh`
4. `./scripts_build_debian/6.make_deb_linux.sh`

Uwagi:
- Jeśli brakuje `libqsqlmysql.so`, doinstaluj `libqt6sql6-mysql` albo zbuduj plugin ze źródeł Qt.
- To pre-release: zbieramy feedback przed finalnym wydaniem.
