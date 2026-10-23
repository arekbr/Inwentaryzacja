# ZADANIA — wydanie 1.7.0: hasło MySQL w sejfie, stały podpis macOS, MySQL na Windows, modele AI 5.5

Zlecenie usera 23.10.2026 (odpowiedź na listę propozycji po 1.6.1):
„1. CI - ok. trzeba zrobic. 2. na razie nie. 3. hasło … poprawic 4. windows bez mysql? poprawic
5. poprawić. 5. poprawic jesli to zle. 6. poprawic”.

## A. Hasło MySQL w systemowym sejfie

- **CO:** hasło bazy MySQL przestaje leżeć jawnym tekstem w `inwentaryzacja.ini` (`Database/MySQL/Password`)
  — trzyma je sejf systemowy (Pęk kluczy / Menedżer poświadczeń / Secret Service), jak klucz API od 1.6.0.
- **GOTOWE GDY:** po pierwszym starcie 1.7.0 ini nie ma klucza `MySQL\Password`, okno konfiguracji pokazuje
  hasło z sejfu, połączenie z bazą działa; zmiana hasła w oknie trafia do sejfu, nie do ini.
- **SPRAWDZĘ TAK:** testy w `tests/repository_tests.cpp` (cocoa, usługa `Inwentaryzacja-test`): migracja
  hasła z ini, zapis/odczyt/usunięcie, okno konfiguracji czyta i zapisuje hasło przez sejf — CZERWONE na 1.6.1.
  Na maku: ini po starcie bez hasła, lista 1871 eksponatów przez WG.
- **BRZEGI:** sejf odmówił przy migracji → hasło zostaje w ini i działa, okno pokazuje ostrzeżenie;
  sejf niedostępny przy odczycie → puste pole + ostrzeżenie, OK bez zmiany pola NIE kasuje hasła w sejfie;
  zapis nieudany → ostrzeżenie, bez jawnego zapisu do ini (bez „insecure fallback”).
- **WARSTWA:** `SecretStore` (odczyt/zapis sekretu z migracją z ini) | dwóch odbiorców: klucz API i hasło |
  prostsze odrzucone: hasło w `AiEnrichmentService` miesza odpowiedzialności, kopia ~60 linii migracji =
  dwa miejsca do poprawiania tego samego błędu.

## B. Stały podpis aplikacji macOS w CI

- **CO:** CI podpisuje `.app` własnym certyfikatem (samopodpisanym, 10 lat), a nie ad hoc — wymaganie
  podpisu (`designated`) przestaje zależeć od skrótu binarki, więc pęk kluczy nie pyta po każdej aktualizacji.
- **POMIAR PRZED (23.10, mak, próbny pęk kluczy, `SecKeychainSetUserInteractionAllowed(false)`):**
  ad hoc — druga kompilacja dostaje odmowę (-25293); certyfikat samopodpisany — druga kompilacja czyta
  wpis bez pytania (0), także bez pola OU (`TeamIdentifier=not set`). Mechanizm działa bez Apple Developer ID.
- **GOTOWE GDY:** `codesign -d -r-` paczek z CI pokazuje `certificate leaf = H"…"`, nie `cdhash`;
  tag `v*` bez sekretu certyfikatu = błąd joba (nie cichy powrót do ad hoc).
- **SPRAWDZĘ TAK:** wymaganie podpisu w obu dmg z CI; po instalacji 1.7.0 na maku jedno „Zawsze pozwalaj”
  (wpis klucza API zakładało ad hoc 1.6.1), potem lokalna kompilacja podpisana tym samym certyfikatem
  startuje BEZ pytania — powtórzenie oryginalnego objawu.
- **BRZEGI:** brak sekretu w PR z forka / Dependabocie → ad hoc z ostrzeżeniem; codesign nie znajduje
  tożsamości spoza listy wyszukiwania pęków (zmierzone) → pęk podpisu dopisany do listy w jobie.
- **SEKRET:** p12 + hasło w Memory Alpha (`inwentaryzacja-codesign-macos`), w GitHubie sekrety
  `MACOS_CODESIGN_P12_BASE64` / `MACOS_CODESIGN_P12_PASSWORD`. Zmiana certyfikatu = jedno pytanie po aktualizacji.
- **POZA:** Gatekeeper/kwarantanna (wymaga Developer ID + notaryzacji, płatne) — `xattr` u Łukasza zostaje.

## C. Sterownik MySQL w paczce Windows

- **CO:** paczka Windows ma `sqldrivers/qsqlmysql.dll` + `libmariadb.dll` (od 1.0 nie miała — Qt z aqt
  nie zawiera pluginu, a `package-windows.ps1` kopiował go tylko „jeśli jest”).
- **GOTOWE GDY:** test `sqlDriver_mysqlPluginLoads` uruchomiony w CI z KATALOGU PACZKI (kopia stage)
  przechodzi; ten sam test na paczce 1.6.1 jest czerwony.
- **SPRAWDZĘ TAK:** krok CI po pakowaniu; ZIP z CI rozpakowany i sprawdzony od środka; jeśli Parallels
  z Windows jest dostępny — połączenie z bazą przez WG z maszyny wirtualnej.
- **BRZEGI:** plugin musi być zbudowany z tych samych źródeł Qt (6.8.2) co reszta; zależności DLL
  libmariadb muszą leżeć obok exe; brak pluginu = błąd pakowania, nie cichy brak.

## D. Modele AI 5.5

- **CO:** lista modeli: Claude Opus 5.5 (domyślny), Sonnet 5.5, Haiku 5.5; ceny z jednej tabeli
  (okno konfiguracji, potwierdzenie kosztu, okno porównania). Odpowiedź czytana po typie bloku
  (Opus 5.5 zawsze myśli — pierwszy blok to `thinking`, a 1.6.1 brał `content[0]` i zgłaszałby błąd),
  `stop_reason: "refusal"` obsłużony, `max_tokens` z miejscem na myślenie, `effort` jawnie,
  awaryjny model po odmowie (`fallbacks: "default"`) dla Opus/Sonnet 5.5.
- **GOTOWE GDY:** `GET /v1/models` z kluczem usera zawiera wszystkie trzy ID; jedno prawdziwe wywołanie
  z prawdziwym zdjęciem zwraca opis; koszt w oknie zgadza się z `usage` × cennik.
- **SPRAWDZĘ TAK:** testy budowy zapytania i parsowania odpowiedzi (bloki thinking + text, odmowa);
  jedno płatne wywołanie (~$0,05) Opus 5.5 na zdjęciu z bazy.
- **BRZEGI:** zapisany w ini stary model (`claude-opus-4-7`) — okno wybiera domyślny i zapisuje przy OK;
  Haiku 5.5 bez serwerowego `fallbacks` (nie obsługuje).

## E. AI bierze zdjęcia z podglądu

- **CO:** „Wzbogać opis AI” używa bajtów zdjęć już wczytanych do miniatur zamiast drugiego zapytania SQL.
- **POMIAR (23.10, baza muzeum):** 2720 zdjęć, średnio 359 KB, max 1015 KB; do 11 na eksponat ⇒ drugie
  zapytanie ściągało ponownie do ~5 MB przez WG, blokując okno. Nie błąd poprawności — zbędny transfer.
- **GOTOWE GDY:** AI dostaje bajty wczytane przy otwarciu podglądu (`m_photoBlobs`, także zdjęcia bez
  miniatury). **Osobnego testu NIE MA** — ścieżka kliknięcia kończy się zapytaniem sieciowym i oknami
  modalnymi; sprawdzone przeglądem kodu i prawdziwym wywołaniem w T4. Formaty w bazie 23.10:
  2718 JPEG + 2 PNG.

## F. Karta 1.6.1 — odhaczyć T5–T6 (wykonane 23.10).

## POZA (świadomie)
- macOS 12+ dla Intela — decyzja usera „na razie nie” (23.10). Obraz Intel ma `minos 15.0`.
- Linki w `descriptionView` podglądu bez `openLinks=false` (stary kod, osobne zlecenie).
- Windows: `Inwentaryzacja.exe` to program KONSOLOWY (`qt6_add_executable` bez `WIN32`) — obok okna
  aplikacji otwiera się czarne okno konsoli (zrzut z VM 23.10). Stan sprzed 1.7.0, propozycja.

## Przyrosty
- [x] T1 karta + gałąź `feature/sejf-hasla-podpis-1.7.0`
- [x] T2 A: `SecretStore` + klucz API na nim (3 testy klucza z 1.6.0 PASS) + hasło MySQL — 2 nowe testy
  CZERWONE na 1.6.1 (hasło zostaje w ini), zielone na 1.7.0; offscreen 37 PASS / 7 SKIP, cocoa 7/7
- [x] T3 E: zdjęcia z podglądu (bez osobnego testu — patrz E)
- [x] T4 D: `/v1/models` z kluczem usera: opus/sonnet/haiku-5-5 dostępne, obraz obsługiwany, Haiku bez
  modeli zastępczych. Prawdziwe wywołanie na zdjęciu C64 z bazy (1800×1355): Opus 25 s 3594/2067 tok.
  $0,056 · Sonnet 13 s 3594/1321 $0,020 · Haiku 10 s 3594/1692 $0,001 — estymata skalibrowana (zdjęcie
  ~3000 tok.). Test `aiEnrichment_realCallReturnsDescription` tylko z `INW_PRAWDZIWE_AI=1`.
- [x] T5 B: certyfikat RSA 3072 do 2036-10-20 (sha1 640b38df…), `bw` `inwentaryzacja-codesign-macos`
  (zwrot z sejfu = plik), sekrety GitHub, `ci/import-macos-codesign.sh`; próba na sucho na maku:
  `designated => … certificate root = H"640b…"`, `codesign --verify --deep --strict` OK
- [x] T6 C: budowa QMYSQL na Windows (vcpkg `libmariadb`, `qt-cmake` wg dokumentacji Qt 6.8) + test
  z kopii katalogu paczki w CI; konta bazy: 12× `mysql_native_password` (wbudowane w libmariadb)
- [x] T6a Windows w CI i w VM (23.10):
  - 🔴 pierwsza sonda w CI (`& exe`) wróciła z kodem 0 BEZ wyjścia — fałszywa zieleń; teraz `-o plik`,
    wymagana linia PASS, kontrola negatywna bez `libmariadb.dll` (dała `FAIL!`, kod 1);
  - połączenie z MariaDB 11.8.6 w CI: `TLSv1.3` (libmariadb na Schannel: WS2_32/CRYPT32/bcrypt/Secur32);
  - 🔴 **znalezisko poza listą: paczki Windows 1.6.0 i 1.6.1 nie startowały** — exe importuje `z.dll`
    (vcpkg przemianował zlib z `zlib1.dll`), a skrypt kopiował tylko `zlib1.dll` „jeśli jest”
    (1.5.0: import `zlib1.dll`, w zipie jest; 1.6.0/1.6.1: import `z.dll`, w zipie brak). 1.7.0 kopiuje
    cały `bin` vcpkg + statyczna kontrola importów exe i pluginu (kontrola negatywna bez `z.dll`);
  - VM „Windows 11” (ARM64, sieć tymczasowo „shared” przez WG maka): paczka z CI startuje, łączy się
    z bazą muzeum — „Lista przedmiotów (1871 / 1871)”; hasło z ini przeniesione do Menedżera poświadczeń
    (`mysql_password@Inwentaryzacja`), ini bez hasła. Posprzątane (poświadczenie, ini, pliki), sieć
    przywrócona na „bridged” (ten sam MAC i adres), VM uśpiona jak zastana;
  - GitGuardian zgłosił hasła serwera testowego CI (`e593c91`) — skan gałęzi wobec sekretów z `bw`:
    0 trafień; od `ecc8568` hasła losowane w każdym przebiegu i maskowane.
- [x] T6b przegląd kodu (/code-review medium, 10 pozycji): przyjęte 1 (nieudany zapis nie zostawia
  starej wartości w ini), 2 (koszt wg modelu, który odpowiedział), 3 (AI dostaje też zdjęcia bez
  miniatury), 4 („Anuluj” w oknie oczekiwania — limit 180 s), 5 (`VCPKG_ROOT`), 7 (`package-macos.sh`
  sprawdza plugin i libmariadb W PACZCE — zielone na 1.6.1, czerwone bez pluginu); 6 (kopiowanie
  wszystkich DLL z vcpkg) ODRZUCONE po pomiarze: `bin` ma tylko `z.dll` + `libmariadb.dll`, a lista
  nazw to dokładnie błąd, przez który 1.6.0–1.6.1 nie miały `z.dll`; odrzucone: 8 (hasło czytane także przy SQLite — ostrzeżenie
  w oknie wystarcza), 9 (zdublowana ścieżka ini — stan sprzed zmiany, osobny refaktor), 10 (estymata
  2000 tok. wyjścia — zmierzone 1321–2067)
- [ ] T7 F + wersja 1.7.0, przegląd kodu, PR → dev → main, tag, release, paczki sprawdzone od środka
- [ ] T8 instalacja na maku (jedno pytanie) + test bez pytania; paczka dla Łukasza 1.7.0
