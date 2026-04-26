# Phase 2 deep audit — ItemRepository + DatabaseBackupService — 2026-04-26

**Tool:** qt-cpp-review skill, Phase 2 Agent 2 (Ownership & Lifecycle) + Agent 5 (Error Handling & Validation), parallel.
**Branch:** dev (post v1.3.1 release).
**Trigger:** Plan C z audytu 2026-04-26 — selektywny deep dive na 2 najbardziej skomplikowane klasy (CRUD core + NEW v1.3.0 backup service).

## TL;DR

5 high + 5 investigate per agent = **10 high findings + 10 investigation targets**. ItemRepository = **mature**, jedyna realna luka funkcjonalna (I-1: photos w editMode). DatabaseBackupService = **świeży kod**, wymaga rundy hardening (timeout, password leak, error reporting).

**Plan v1.3.2 patch (4 fixy, ~2h):** E-1 + E-2 + E-3 + O-4. **v1.4 RFC:** E-4, E-5, O-5, O-6, I-1.

## High findings (>80 confidence)

### Ownership & Lifecycle (Agent 2)

| ID | File:Line | Conf | Co | Fix kierunek |
|---|---|---|---|---|
| O-2 | `DatabaseBackupService.cpp:142-171` | 82 | Pętla `while (state != NotRunning)` bez deadline'u — mysqldump hang = wątek wisi w nieskończoność | `QDeadlineTimer` 30 min default + cancellation predicate |
| O-3 | `DatabaseBackupService.cpp:119-121` | 85 | Hasło MySQL w `QString` lokalnym + `MYSQL_PWD` env — memory dump exposes credential | Po `process.start()` overwrite + clear; long-term `--defaults-extra-file` |
| O-4 | `ItemRepository.cpp:151, 162` | 80 | Same `QSqlQuery` re-prepared 2× w `deleteItem` — MySQL driver cachuje stmt handles, hit `max_prepared_stmt_count` w long-running | 2 osobne `QSqlQuery` lub `query.clear()` między prepares |
| O-5 | `ItemRepository.h:32, 54` + `DatabaseBackupService.h:28, 51` | 88 | `m_db` lifetime contract nieudokumentowany — `removeDatabase()` z innego miejsca = dangling | Dokumentacja w nagłówku + `m_db.isValid()` precondition; lub przyjmować connection name (QString) |
| O-6 | `ItemRepository.cpp:11`, `DatabaseBackupService.cpp:14` | 80 | `QObject::tr()` z free functions = wszystkie stringi pod context "QObject" — kolizje przy translacji | `Q_DECLARE_TR_FUNCTIONS(ItemRepository)` w nagłówku, switch na `ItemRepository::tr(...)` |

### Error Handling & Validation (Agent 5)

| ID | File:Line | Conf | Co | Fix kierunek |
|---|---|---|---|---|
| E-1 | `DatabaseBackupService.cpp:149-165` | 85 | `gzwrite` sprawdza tylko `written == 0` (nie `written != size`); error message porzuca `stderrBuffer` z mysqldump | `written != size` check + dołączyć `stderrBuffer` (operator widzi "access denied for user X") |
| E-2 | `DatabaseBackupService.cpp:142-171` | 88 | Brak twardego timeoutu / idle-timeout — patrz O-2 (overlap) | dwa progi: globalny deadline + idle-timeout (30s bez bytes → kill) |
| E-3 | `DatabaseBackupService.cpp:119-121` | 90 | `MYSQL_PWD` przez env — leak `/proc/<pid>/environ` + memory dump | `--defaults-extra-file=/tmp/X.cnf` z `[client]\nuser=...\npassword=...`, chmod 600 (`QTemporaryFile` setAutoRemove) |
| E-4 | `DatabaseBackupService.cpp:99-108, 216-223` | 82 | `outputPath` bez walidacji + `QFile::remove(outputPath)` przed renamem (nie atomic cross-FS) | opcjonalny `allowedDirectory` parametr; rename target → `.old` przed rename tmp → target |
| E-5 | `DatabaseBackupService.h:18-46` | 81 | Brak ścieżki backupu dla SQLite (default backend!) — twardy reject | (a) native SQLite backup przez `sqlite3_backup_*`, lub (b) miękki komunikat z hint do menu Plik |

## Investigation targets (60-79)

### Ownership

| ID | File:Line | Conf | Co |
|---|---|---|---|
| I-O-1 | `DatabaseBackupService.cpp:142-171` | 70 | Polling 100ms vs `readyReadStandardOutput` signal — verify pod prawdziwym MySQL load |
| I-O-2 | `DatabaseBackupService.cpp:118` | 65 | `QProcess` w const member function + worker thread bez event loop — semantyka `waitForFinished` różna |
| I-O-3 | `ItemRepository.cpp:43, 102` | 65 | Photos + item w jednej transakcji — 200 zdjęć × 50MB może hit `max_allowed_packet` |
| I-O-4 | `DatabaseBackupService.cpp:107` | 70 | `tempOutputPath = outputPath + ".tmp"` — concurrent backups do tego samego dir = collision (brak `mkstemp`) |
| I-O-5 | `ItemRepository.cpp:101` | 62 | Photo binding implicit copy — 50MB × N peak RSS doubles podczas commit |

### Error Handling

| ID | File:Line | Conf | Co |
|---|---|---|---|
| I-E-1 | `ItemRepository.cpp:100-119` | 75 | **Photos pomijane przy editMode** — silent data loss czy by design? Wymaga decyzji produktowej + komentarz lub fix |
| I-E-2 | `ItemRepository.cpp:36-41, 121-127` | 65 | `rollback()` po failed `commit()` w MySQL InnoDB → warning (transakcja już zakończona) |
| I-E-3 | `DatabaseBackupService.cpp:206-214` | 70 | Race między `QFile::remove(outputPath)` a `rename` — okno gdzie outputPath nie istnieje |
| I-E-4 | `DatabaseBackupService.cpp:140-145` | 68 | `stderrBuffer` rośnie bez limitu — tysiące warningów = puchnie do GB |
| I-E-5 | `ItemRepository.cpp:219-229` | 72 | `updateItemsColumn` `arg(columnName)` bez whitelist — dzisiaj safe (2 hardcoded callsite), defense-in-depth dla przyszłych |

## Plan v1.3.2 patch (4 fixy)

Priorytet: security (E-3) + reliability (E-2, E-1) + perf debt (O-4).

1. **E-3 + O-3 (overlap):** `--defaults-extra-file` zamiast `MYSQL_PWD`. `QTemporaryFile` chmod 600, sekcja `[client]\nuser=...\npassword=...`. `setAutoRemove(true)`.
2. **E-2 + O-2 (overlap):** `QDeadlineTimer` (30 min default, konfigurowalny w konstruktorze) + idle-timeout 30s bez nowych bytes na stdout → `process.kill(); waitForFinished(5000)` + komunikat "Backup przerwany — przekroczono czas oczekiwania".
3. **E-1:** `gzwrite` sprawdzaj `written != static_cast<int>(stdoutData.size())` jako error; każdy komunikat błędu zapisu dołącza `stderrBuffer.left(2000)`.
4. **O-4:** w `deleteItem` użyj 2 osobnych `QSqlQuery` (każdy własny scope) lub `query.clear()` między prepares.

**Test plan:**
- Unit test `databaseBackupService_timesOut_whenMysqldumpHangs` (mock long-running QProcess)
- Unit test `databaseBackupService_doesNotLeakPasswordInProcessEnv` (sprawdzić że env nie ma `MYSQL_PWD`)
- Unit test `databaseBackupService_includesStderrInError` (sprawdzić że error message zawiera mysqldump stderr)
- Existing `repository_tests` muszą nadal przejść (regression)

**Estimate:** ~2h pracy + 2× CI cycle = release v1.3.2 w jednej sesji.

## Plan v1.4 RFC (większe zmiany)

- **E-4:** `allowedDirectory` parameter + `.old` rotation pattern (zachowaj stary backup gdy nowy padnie)
- **E-5:** SQLite native backup (`sqlite3_backup_init/_step/_finish`) lub explicit komunikat "Plik → Backup"
- **O-5:** `m_db` contract — przyjmuj connection name (`QString`) zamiast handle, re-resolve per call
- **O-6:** `Q_DECLARE_TR_FUNCTIONS` + przeniesienie translation calls
- **I-E-1:** decyzja produktowa o photos w editMode (silent data loss vs by design)
- **I-E-5:** column whitelist guard w `updateItemsColumn` (defense-in-depth)

## Werdykt

ItemRepository nie wymaga większych zmian — mature CRUD, transakcje OK, prepared statements wszędzie. Jedna luka: editMode photos.

DatabaseBackupService (NEW v1.3.0) wymaga rundy hardening — typowe grabie nowego procesowego I/O w Qt (timeout, password leak, error reporting). Te fixy to v1.3.2 priorytet.

**Phase 2 deep audit miał wartość** — Phase 1 lint dał 0 hits dla tych plików, ale agenty znalazły 10 high-confidence findings w realnych obszarach security + reliability. Wzorzec na przyszłość: dla nowych klas z procesowym I/O lub credential handling — uruchamiać Phase 2 selektywnie (Agent 2 + Agent 5).
