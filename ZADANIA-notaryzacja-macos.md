# ZADANIA — notaryzacja paczek macOS

## CO

Wydania macOS są podpisane Developer ID i przechodzą notaryzację Apple, więc otwierają się bez `xattr` i bez ostrzeżenia Gatekeepera.

## GOTOWE GDY

1. Pobrana z internetu paczka Intel i ARM: `spctl` zwraca `accepted`, `source=Notarized Developer ID`, a `stapler validate` przechodzi dla aplikacji i obrazu DMG.
2. Aplikacja startuje na maku bez polecenia `xattr` i łączy się z bazą MySQL (działa wtyczka QMYSQL przy wzmocnionym uruchamianiu).
3. Odrzucenie przez Apple przerywa budowanie i wypisuje dziennik błędów notaryzacji.
4. Wypchnięcia na `dev` i pull requesty budują się bez notaryzacji. Tag wydania bez sekretów kończy się błędem, a nie paczką bez notaryzacji.

## SPRAWDZĘ TAK

| Punkt | Sprawdzenie |
|---|---|
| 1 | `gh workflow run` na tej gałęzi, pobranie artefaktu z flagą kwarantanny, `spctl` i `stapler` na maku |
| 2 | Uruchomienie pobranej paczki bez `xattr`, połączenie z bazą robi użytkownik w oknie konfiguracji |
| 3 | Dziennik kroku w CI; ścieżka błędu czytana w skrypcie, celowego odrzucenia nie wymuszam |
| 4 | Zapis warunku w workflow, kontrola po wypchnięciu na `dev` |

## BRZEGI

- Skladniki w `Contents/Frameworks` i `Contents/PlugIns` trzeba podpisać od środka na zewnątrz, `--deep` nie daje znacznika czasu i wzmocnienia każdemu składnikowi w przewidywalny sposób.
- Biblioteka `libmariadb` może ładować wtyczki uwierzytelniania spoza paczki; wzmocnione uruchamianie by to zablokowało. Konta bazy używają `mysql_native_password`, wbudowanego w bibliotekę.
- Wpisy Pęku kluczy z wersji 1.7.0 mają partycję `cdhash`, więc pierwsza wersja z Team ID zapyta raz.

## POZA

- Minimalna wersja macOS paczek.
- Program instalacyjny `.pkg`.
- Pomiar „zero pytań Pęku kluczy” wymaga dwóch kolejnych wydań z Team ID, zostaje na następne wydanie.

## ZAŁOŻENIA

- Sekrety repo: `MACOS_CODESIGN_P12_*` zawierają Developer ID Application, `APPLE_NOTARY_*` to klucz App Store Connect z rolą Developer.
- Bez dodatkowych uprawnień (entitlements). Dojdą tylko wtedy, gdy zmierzymy, że są potrzebne.

## Warstwy dodawane

| CO dodaję | PO CO dziś | PROSTSZE ODRZUCONE, BO |
|---|---|---|
| Notaryzacja tylko przy tagu i ręcznym starcie | Dwie wysyłki do Apple na architekturę wydłużają każde budowanie | Notaryzacja przy każdym wypchnięciu nic nie daje paczkom roboczym |

## Przyrosty

| # | Zakres | Sprawdzenie |
|---|---|---|
| P1 | Podpis od środka, wzmocnione uruchamianie, znacznik czasu | `codesign --verify --strict` w CI |
| P2 | Notaryzacja i `stapler` dla aplikacji i DMG | Punkt 1 i 3 |
| P3 | Test paczek na maku | Punkt 2 |
| P4 | Wydanie po scaleniu | Tag, kontrola opublikowanych plików |
