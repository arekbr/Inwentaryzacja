# ZADANIA — 1.6.1: pełny podgląd eksponatu, opis na liście, przycisk AI

MASZYNA: mak `Mac-Studio-Arek`, plik `~/projektyQT/Inwentaryzacja-2026/ZADANIA-podglad-opis-1.6.1.md`,
gałąź `feature/podglad-zdjecia-opis` (od `dev`). Zgłoszenie usera 23.10.2026 ~11:53 (zrzuty z 1.6.0 na maku).

**CO:**
1. Podgląd eksponatu (podwójne kliknięcie na liście) pokazuje miniatury zdjęć (podwójne kliknięcie
   miniatury = pełny ekran) i wszystkie pola tylko do odczytu: typ, producent, model, numer seryjny,
   part number, revision, rok, status, miejsce, oryginalne opakowanie — obok opis, przycisk AI, Edytuj.
2. Lista eksponatów pokazuje sformatowany opis zaznaczonego eksponatu obok zdjęć na dole.
3. Przycisk „Wzbogać opis AI” stoi w rzędzie przycisków podglądu, a nie ucięty w lewym górnym rogu.

**DIAGNOZA (pomiar, nie domysł):** 1 i 2 to braki projektu — formularze listy, podglądu i edycji są
w 1.6.0 identyczne z 1.5.0 (`git diff v1.5.0 v1.6.0 -- forms` = tylko okno konfiguracji), żadna
gałąź ani wersja nie miała opisu na liście ani zdjęć w podglądzie. 3 to błąd od 1.5.0:
`qobject_cast<QHBoxLayout *>(ui->editButton->parentWidget()->layout())` — rodzicem przycisku jest
okno z układem PIONOWYM, rzutowanie daje nullptr, przycisk wisi bez układu w (0,0).

**GOTOWE GDY:**
1. Test: w podglądzie rekordu z 2 zdjęciami lista miniatur ma 2 pozycje, pola pokazują wartości
   z bazy (S/N, rok, status, miejsce, „Tak” przy opakowaniu).
2. Test: przycisk AI (klucz z `ANTHROPIC_API_KEY`) jest w `buttonLayout` (indexOf ≥ 0) — CZERWONY na
   kodzie 1.6.0, zielony po poprawce.
3. Test: zaznaczenie wiersza na liście wstawia jego opis do pola opisu; brak zaznaczenia = puste.
4. Zrzuty (QWidget::grab na cocoa) podglądu i dołu listy — oglądane, nic nie ucięte.
5. CI 4 platformy zielone, release v1.6.1, instalacja na maku, nowa paczka dla Łukasza.

**SPRAWDZĘ TAK:** `ctest` (repository_tests offscreen + keychain_tests cocoa) + zrzuty z testów
(`INW_ZRZUTY=<katalog>`), na końcu ręcznie na maku po instalacji (user).

**BRZEGI:** eksponat bez zdjęć (lista miniatur ukryta, pola zostają) · bez opisu (pole puste) ·
zaznaczonych kilka wierszy na liście (opis czyszczony jak zdjęcia) · rok 0 / puste pola („—”) ·
bardzo długi opis (pole przewijane, nie rozpycha okna) · skórki ciemne (pole opisu z jawnymi
kolorami jak w podglądzie — mina § 4 skilla).

**POZA ZAKRESEM:** edycja w podglądzie (od tego jest Edytuj), pole `value` (wartość) — nie
pokazujemy, brak w zleceniu; zmiana kolejności kolumn tabeli; zapamiętywanie proporcji zdjęcia/opis.

**ZAŁOŻENIA:**
- Wersja 1.6.1 — decyzja usera („1.6.1 i nowa paczka”), mimo że to nowe funkcje.
- Miniatury w podglądzie: `QListWidget` w trybie ikon (przewijany, gotowe zaznaczanie i dwuklik),
  zdjęcia z `PhotoService::loadStoredPhotos` (to samo źródło co lista), pełny ekran
  `FullScreenPhotoViewer` (to samo co lista).
- Opis na liście z kolumny 11 modelu tabeli (pełny tekst już jest w modelu — bez dodatkowego SELECT).
- Lista: zdjęcia i opis obok siebie w `QHBoxLayout` 2:3 — bez `QSplitter` (zapamiętywanie
  proporcji poza zakresem).

**WARSTWY (CO | PO CO dziś | PROSTSZE ODRZUCONE, BO):**
- helper `zapiszZrzut()` w testach (zmienna `INW_ZRZUTY`) | dowód wizualny zmian UI bez klikania
  w aplikację (brak uprawnień Dostępności dla automatu) | zrzut całego ekranu aplikacji wymaga
  kliknięcia OK w oknie konfiguracji — nie do zautomatyzowania.

## Przyrosty

- [x] T1 Test-reprodukcja przycisku AI w tests/repository_tests.cpp — CZERWONY na 1.6.0:
      `FAIL! previewDialog_enrichButtonInButtonRow() 'buttonRow->indexOf(enrichButton) >= 0' returned FALSE`
      (przycisk znaleziony, ale poza układem — właściwy powód)
- [x] T2 Poprawka przycisku AI w src/PreviewDialog.cpp (`ui->buttonLayout->insertWidget(0, …)`) — ten sam test PASS
- [x] T3 Pełny podgląd: forms/PreviewDialog.ui + src/PreviewDialog.cpp + include/PreviewDialog.h —
      `previewDialog_showsPhotosAndFields` PASS (2 miniatury, 10 pól; brzeg bez zdjęć/roku: lista ukryta, „—”, „Nie”);
      zrzut cocoa obejrzany: miniatury | pola, opis, przycisk AI w rzędzie
- [x] T4 Opis na liście: forms/itemList.ui + src/itemList.cpp — `itemList_showsDescriptionOfSelectedRow` PASS
      (markdown wyrenderowany, po odznaczeniu pusto); zrzut obejrzany. Mina testu: model listy = INNER JOIN
      słowników ⇒ rekord bez słowników nie trafia na listę (test uzupełniony przed pierwszym uruchomieniem)
      Razem offscreen: 35 PASS + 3 SKIP (sejf), cocoa: 5/5
- [x] T4b Przegląd kodu (/code-review medium, 10 pozycji) — przyjęte i zrobione: test podglądu nie
      sięga do sejfu (`ScopedTestApiKey`; wcześniej przechodził TYLKO dzięki `ANTHROPIC_API_KEY`
      w powłoce — w czystym środowisku na macOS offscreen by zawisł; sprawdzone `env -u … perl alarm 120`),
      zaznaczenie z `selectedRows()` zamiast delty (test regresyjny + mutacja ⇒ FAIL), sygnał
      `descriptionChanged` po AI → lista odświeża wiersz, `refreshList()` czyści panel po resecie modelu,
      `openLinks=false` w panelu opisu listy, miniatury z bajtów (pełne zdjęcie dekodowane przy dwukliku),
      jedna ścieżka SQL zdjęć (`fetchPhotos(limit<=0 = wszystkie)`), „—” przy nieudanym odczycie,
      zamknięcie połączenia w teście listy. Odrzucone: podwójne meta (makieta zatwierdzona przez usera).
      Wynik: offscreen bez klucza 35 PASS + 3 SKIP, ctest 2/2, cocoa UI 5/5, sejf testowy czysty.
- [x] T5 Wersja 1.6.1, PR → dev → main, tag, release, paczki 4 platform sprawdzone od środka —
  PR #38 → dev, #39 → main, tag `v1.6.1` = `0b385bc`, 7/7 plików z sumami zgodnymi z GitHubem (23.10.2026)
- [x] T6 Instalacja 1.6.1 na maku + nowa paczka dla Łukasza (ZIP 1.6.0 do usunięcia) — zainstalowana
  23.10, ZIP 1.6.1 zbudowany i sprawdzony, ZIP 1.6.0 usunięty (odhaczone dopiero w 1.7.0)
