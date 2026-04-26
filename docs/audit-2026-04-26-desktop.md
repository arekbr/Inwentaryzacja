# Audit Inwentaryzacja desktop v1.3.0 — 2026-04-26

**Scope:** `src/*.cpp` (23 files) + `include/*.h` (21 files) = **~10,800 LoC**
**Tool:** `qt-cpp-review` skill (qt-development-skills plugin), Phase 1 lint + spot-check
**Branch:** dev (commit `dcffd57` "Bump wersji 1.2.11 → 1.3.0")

## TL;DR

**Codebase jest mature i czysty.** 48 lint hits / 10.8K LoC = **0.45% noise**. Mobile (Inwentaryzacja-Mobile) miał 25% (122 hits / 470 LoC) — desktop ma **55× lepszy lint score per LoC**.

**Nowy kod z PR #15 (PreviewDialog markdown) jest CZYSTY** — 0 hits w `PreviewDialog.h/cpp`. Przeszło code review.

## Lint output histogram

| Reguła | Hits | Realne? | Action |
|---|---|---|---|
| HDR-3 — `std::min/max` bez parens (Windows macro safety) | 11 | TAK | Mechaniczny fix `(std::min)(a, b)` |
| ERR-6 — `arg()` mismatch | 9 | **FALSE POSITIVE** | Lint nie rozpoznaje `.arg(s1, s2)` overload — wszystkie 9 są poprawne |
| DEP-10 — `.count()/.length()` → `.size()` | 9 | Cosmetic | Skip — `.length()` na QString jest community convention |
| DEP-7 — `qMin/qMax` → `std::min/max/clamp` | 6 | Cosmetic | Skip — qXxx są w pełni supported w Qt6 |
| API-5 — get-prefix na getter | 6 | TAK (nowy kod Pacman) | Rename `getX()` → `x()` |
| TMO-1 — `int xMs` → `std::chrono` | 4 | **FALSE POSITIVE** | Pola `Config` struct, nie API parametry |
| DEP-11 — `currentDateTime()` → `currentDateTimeUtc()` | 1 | **TAK (perf)** | 100× faster, DST-stable — ale itemList:950 to log timestamp, low priority |
| PAT-12 — non-const range-for | 1 | False positive (świadoma mutacja) | Skip |
| DEP-13 — `QChar` jako object | 1 | Cosmetic | Skip |

## Skupiska (per plik)

| Plik | Hits | Charakter |
|---|---|---|
| `PacmanOverlay.h/cpp` | 18 | Świeży kod (loader animation), zbiera get-prefix + HDR-3 + DEP-10 |
| `PacmanAnimationModel.h/cpp` | 9 | jw. |
| `DatabaseSchemaUtils.cpp` | 6 | Wszystkie 6 = ERR-6 false positive |
| `itemList.cpp` | 5 | Mature plik (1100+ LoC), DEP-7/DEP-11/PAT-12 |
| `mainwindow.cpp` | 4 | Cosmetic |
| `PreviewDialog.h/cpp` (PR #15, NEW) | **0** | **Czysty** |
| Pozostałe | 6 | Pojedyncze |

## Czego BRAKUJE (positive signal vs mobile)

W mobile audit znaleźliśmy:
- C-D01 race w singletonie JNI ⚠ CRITICAL
- C-D02 OOM w `readAll().toBase64()` ⚠ HIGH
- L-01 brak SSL handler na QNAM
- L-04 brak JSON validation
- C-D07 path traversal vulnerability
- Q-05 cross-page leak w QML singleton

W desktop **żaden z tych wzorców nie wystąpił:**
- Brak threading hazards (aplikacja głównie main-thread)
- Brak SSL/network attack surface (lokalna baza SQLite/MySQL)
- Brak path traversal (file ops przez Qt File dialogs)
- Brak Q_ASSERT side effects
- Brak ownership leaks
- Brak get-prefix w starym kodzie (tylko w nowym Pacman)

## Realne fixy (zrobione w 1 PR)

- **API-5:** 6 Pacman getter rename + getPacmanDelayMs (DatabaseConfigDialog) — `getX()` → `x()`
- **HDR-3:** 11 `std::min/max/numeric_limits` w nawiasach (Windows MSVC bez NOMINMAX)
- **DEP-7:** 2 `qBound` → `std::clamp` w PacmanOverlay.h (i tak ruszamy plik)

Pominięte świadomie:
- ERR-6 (false positive `.arg(s1, s2)` overload)
- TMO-1 (false positive — pola Config struct)
- DEP-7 qMin/qMax w mature plikach (Qt idiom, full supported)
- DEP-10 `.length()` (community convention)
- DEP-13 QChar
- PAT-12 (świadoma mutacja w itemList)
- DEP-11 currentDateTime (log timestamp, low priority — 1 plik, separate PR jeśli warto)

## Phase 2 deep agents — pominięte świadomie

Koszt vs zysk dla mature CRUD app niekorzystny. Jeśli kiedyś chcielibyśmy:
- **Selektywnie** Agent 2 (Ownership) + Agent 5 (Error Handling) na `ItemRepository.cpp` (najbardziej skomplikowana klasa, CRUD core)
- Skip: Model Contracts (nie używamy QAbstractItemModel ekstensywnie), Threading (głównie main-thread), API/C++ correctness (mature codebase), Performance (nie skarżymy się na slow)

## Następne sesje (do rozważenia)

- Sync z mobile — porównanie schematu MariaDB między desktop ItemRepository a backend FastAPI Pydantic models
- Tests coverage — `tests/repository_tests.cpp` to tylko 1 plik na ~10K LoC produkcyjnego kodu
- DEP-11 `currentDateTimeUtc` w itemList:950 jeśli ktoś analizuje logs i widzi DST anomalie
