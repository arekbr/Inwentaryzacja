# ZADANIA — klucz API w systemowym sejfie + czytelne okno w trybie ciemnym

MASZYNA: mak `Mac-Studio-Arek`, plik `~/projektyQT/Inwentaryzacja-2026/ZADANIA-klucz-api-sejf.md`,
gałąź `feature/klucz-api-sejf` (od `dev` 9ea6d7e). Zlecenie 23.10.2026.

**CO:** klucz API Anthropic przestaje leżeć jawnym tekstem w `inwentaryzacja.ini` — trzyma go
systemowy sejf (macOS: Pęk kluczy, Windows: Menedżer poświadczeń, Linux: Secret Service).
Klucz zapisany przez starą wersję przenosi się sam przy pierwszym użyciu i znika z pliku.
Okno „Konfiguracja bazy danych” jest czytelne przy macOS w trybie ciemnym.

**GOTOWE GDY:**
1. Zapis klucza z okna → `grep -c '^anthropic_api_key=' inwentaryzacja.ini` = 0 **i**
   `security find-generic-password -s Inwentaryzacja -a anthropic_api_key` = exit 0.
2. Ini z kluczem ze starej wersji → po pierwszym odczycie klucz w sejfie (odczyt zwrotny = stary
   klucz), w ini 0 linii z kluczem.
3. Sejf niedostępny / zapis odrzucony → przy migracji klucz **zostaje** w ini (nie ginie);
   przy zapisie z okna komunikat błędu, do ini nic nie trafia.
4. Tryb Dark: tytuł sekcji AI, tekst checkboxa i wartość pola Port ciemne na jasnym tle —
   zrzut PRZED i PO.
5. CI 4 platformy (Debian, Windows, macOS Intel, macOS ARM) zielone na PR do `dev`.

**SPRAWDZĘ TAK:** `InwentaryzacjaTests` (nowe testy na testowej nazwie usługi
`Inwentaryzacja-test`, nie ruszają prawdziwego wpisu) + zrzut okna z izolowanym katalogiem
domowym (`CFFIXED_USER_HOME`), prawdziwe ini nietknięte (hash przed = po).
Kontrola negatywna (zmiana 23.10 po krytyce — „pusta usługa” nie psuje sejfu, odmowa jest
deterministyczna tylko przy pustej usłudze I pustym kluczu): test mutacyjny — usunięte kasowanie
z ini w migracji i w zapisie ⇒ testy migracji i zapisu CZERWONE.

**BRZEGI:**
- Brak sejfu (Linux bez Secret Service, sesja SSH) → zapis z okna = komunikat, bez jawnego
  zapisu awaryjnego; migracja zostawia ini bez zmian, klucz z ini dalej działa (jak w 1.5.0),
  okno pokazuje czerwone ostrzeżenie.
- Pusty klucz w oknie = usunięcie wpisu z sejfu (zachowanie jak dotąd: „puste = wyłączone”),
  ALE tylko gdy user zmienił pole (`isModified`) — puste pole po nieudanym odczycie sejfu
  i zwykłe OK na starcie NICZEGO nie kasują.
- Zapis nowego klucza z okna usuwa stary klucz z ini — inaczej migracja przy następnym starcie
  nadpisałaby nowy klucz starym.
- `ANTHROPIC_API_KEY` w środowisku → ma priorytet jak dotąd, sejf nieczytany.
- Użytkownik kliknie „Odmów” w pytaniu Pęku kluczy → błąd z sejfu, apka działa bez AI.
- Ini z kluczem **i** sejf z innym kluczem → wygrywa ini (to nowszy zapis, np. po powrocie
  do 1.5.0 i ponownym wpisaniu).
- Powrót do 1.5.0 → stara wersja nie widzi klucza w sejfie (trzeba wpisać ponownie).

**POZA ZAKRESEM:** hasło MySQL w tym samym ini (ten sam problem — osobne zlecenie), nieaktualna
lista modeli (Opus 4.7 / Sonnet 4.6), pełny tryb ciemny skórek. (Do 23.10 ~10:45 stało tu też
„pusta przestrzeń w oknie”; zgłoszenie usera „za małe pola w stosunku do tekstu” wciągnęło
układ okna do zakresu jako T01b — osobny commit, żeby dało się go cofnąć niezależnie.)

**ZAŁOŻENIA:**
- QtKeychain 0.17.0 (BSD-3, wydanie 2026-07-06) przez FetchContent, budowany statycznie —
  zero zmian w pakowaniu macOS/Windows; Debian dostaje `libsecret-1-dev` w CI i `libsecret-1-0`
  w Depends.
- Bez awaryjnego zapisu jawnym tekstem (`setInsecureFallback(false)`) — „plik txt to zły pomysł”.
- Wpis w sejfie: usługa `Inwentaryzacja`, klucz `anthropic_api_key`; w trybie testów
  (`QStandardPaths::isTestModeEnabled`) usługa `Inwentaryzacja-test`.
- Tryb ciemny: aplikacja wymusza jasny schemat kolorów (`QStyleHints::setColorScheme`, Qt ≥ 6.8) —
  wszystkie skórki mają stałe kolory projektowane pod jasny schemat.

**WARSTWY (CO | PO CO dziś | PROSTSZE ODRZUCONE, BO):**
- QtKeychain | jeden interfejs do trzech sejfów systemowych | własne Security.framework +
  CredWrite + libsecret = trzy implementacje do utrzymania; szyfrowanie w ini = klucz
  szyfrujący leży obok, pozorne bezpieczeństwo.
- Synchroniczne opakowanie zadań QtKeychain (lokalna pętla zdarzeń) w `AiEnrichmentService` |
  `hasApiKey()`, `readApiKey()` i okno oczekują wartości od razu | przepisanie wywołujących na
  asynchroniczne = większy diff w PreviewDialog i oknie konfiguracji. BEZ limitu czasu: porzucone
  zadanie blokowałoby kolejkę QtKeychain (jedno zadanie naraz, `keychain.cpp` JobExecutor).
- Pamięć podręczna klucza na proces dla `hasApiKey()`/`readApiKey()` | każdy PreviewDialog
  pytałby sejf, a przy podpisie ad hoc macOS może przy tym pytać usera | odczyt przy każdym
  podglądzie = pytania Pęku kluczy i pętla zdarzeń w konstruktorze za każdym razem.

**KRYTYKA (adwokat diabła, 23.10) — co przyjęte:** isModified (pkt 1), bez limitu czasu (2),
testy sejfu na macOS osobnym wpisem CTest na cocoa, bo offscreen nie opróżnia głównej kolejki
GCD (3, pomiar adwokata), mutacje zamiast fałszywej kontroli (4), zapis z okna kasuje ini (5),
próba GUI bez atrapy klucza w ini — prawdziwy wpis `Inwentaryzacja` nietknięty, exit 44 przed
i po (6), cache na proces (7), zrzuty 6 skórek stara/nowa — bez regresji (8).
**Odrzucone:** `color:` w default.qss zamiast schematu (leczy tylko tekst, nie elementy rysowane
przez Fusion ani pasek tytułu); migracja dwufazowa na Linuksie (libsecret pisze do kolekcji
domyślnej = trwałej).
**Po release (user):** kopia `inwentaryzacja.ini.bak-20261023-przed-sejfem` zawiera klucz
jawnym tekstem — usunąć po T06; klucz leżał jawnie → rotacja w konsoli Anthropic (9).

## Przyrosty

- [x] T01 Jasny schemat kolorów w src/main.cpp — zrzut PO: tytuł AI, checkbox, „3306” ciemne,
      pasek tytułu jasny (Qt 6.11.2, mak, Dark)
      SPRAWDZENIE: zrzut okna konfiguracji w trybie Dark PRZED (stara binarka) i PO — tytuł
      „AI — wzbogacanie opisów” i „3306” ciemne na jasnym.
- [x] T01b Pola MySQL nie ścinają tekstu — POMIAR: przed host 22 px / hint 29, stacked 150
      (sztywne min) przy potrzebie 178; po usunięciu min: 27/29, stacked 169 (resztę zjadała
      zawijana etykieta — heightForWidth poza minimum okna); po `adjustSize()`: 29/29, 178/178.
      Zrzut: „testowa_baza”, „uzytkownik_testowy” pełne, ostrzeżenie nieobcięte. (dopisane 23.10 na zgłoszenie „za małe pola w stosunku
      do tekstu”) w forms/DatabaseConfigDialog.ui (+ ewentualnie rozmiar startowy okna
      w src/DatabaseConfigDialog.cpp)
      HIPOTEZA: `stackedWidget` ma sztywne minimumSize 150 px < potrzeb strony MySQL; od v1.5
      (sekcja AI) okno startuje w rozmiarze minimalnym i układ ściska wiersze do 150 px.
      SPRAWDZENIE: pomiar wysokości pola vs sizeHint PRZED/PO + zrzut: „testowa_baza” z widocznym
      „_”, „uzytkownik_testowy” z pełnym „y”; ostrzeżenie pod kluczem nieobcięte.
- [x] T02 QtKeychain w CMakeLists.txt — `libqt6keychain.a`, `otool -L` 0 trafień keychain,
      linkowane Security+Foundation, `nm` 397 symboli QKeychain (URL + SHA256 archiwum 0.17.0).
- [x] T03 Sejf + migracja z ini w src/AiEnrichmentService.cpp i include/AiEnrichmentService.h —
      keychain_tests (cocoa) 3/3 PASS, repository_tests (offscreen) 32 PASS + 3 SKIP; mutacje
      (bez kasowania z ini) ⇒ 2 FAIL dokładnie w migracji i zapisie; po przywróceniu zielone;
      wpis `Inwentaryzacja-test` po testach nie istnieje (exit 44).
- [x] T04 Okno konfiguracji czyta i zapisuje sejf w src/DatabaseConfigDialog.cpp — zrzut:
      „🔒 Klucz trzymany w systemowym sejfie…”; prawdziwe ini hash 76af68e9… przed = po;
      prawdziwy wpis `Inwentaryzacja` exit 44 przed = po.
- [ ] T05 CI: libsecret w .github/workflows/build-installers-v2.yml i Depends w ci/package-debian.sh
      SPRAWDZENIE: PR do `dev`, cztery joby zielone.
- [ ] T05b Release v1.6.0 (zlecenie usera 23.10: „wypchnij, sprawdź paczki, zrób release
      oficjalny”) — version.txt 1.6.0, PR → dev, PR dev → main, tag v1.6.0
      SPRAWDZENIE: `gh release view v1.6.0` — 4 platformy w assetach (deb, exe/zip, dmg Intel,
      dmg ARM).
- [ ] T06 Próba na prawdziwym ini (migracja klucza usera) — po instalacji wersji z CI
      SPRAWDZENIE: GOTOWE GDY 1–2 na prawdziwym pliku; kopia: `inwentaryzacja.ini.bak-20261023-przed-sejfem`.
