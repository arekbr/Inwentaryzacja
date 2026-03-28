# Inwentaryzacja – Program do zarządzania kolekcją retro komputerów

Witaj w projekcie **Inwentaryzacja**! To narzędzie stworzone z myślą o maniakach retro komputerów, którzy chcą uporządkować swoje zbiory – od Amigi po ZX Spectrum. Program powstaje jako eksperyment z użyciem AI, ale z sercem i pasją do oldschoolowego sprzętu!

## Po co to wszystko?

Chciałem mieć soft skrojony pod moje potrzeby – prosty, ale funkcjonalny, do zarządzania kolekcją retro komputerów. Sam jestem „marnym programistą” (doba ma tylko 24 godziny!), więc pisanie tego od zera zajęłoby mi pewnie z 5 lat. Zamiast tego postawiłem na eksperyment: tworzę ten program z pomocą AI – **ChatGPT** i **GROK**. 

Jedyna rzecz, którą zrobiłem sam, to początkowe GUI, ale i tak AI zrobiło poprawioną wersję. 

Dlaczego tak? Bo to frajda, wyzwanie i sposób, żeby mieć narzędzie, jakiego nie znajdę na półce. A przy okazji uczę się C++ – powoli, w zrywach, ale z planem na emeryturę, kiedy ogarnę C jak mistrz! 😄

## Co potrafi program?

Inwentaryzacja to więcej niż prosta baza danych. Aktualna wersja (1.2.4) oferuje:
- **Zarządzanie elementami**: Dodawaj, edytuj, usuwaj komputery i akcesoria z kolekcji.
- **types i producenci**: Kategoryzuj sprzęt (np. Amiga, Atari) i przypisuj producentów.
- **Statusy i miejsca przechowywania**: Oznacz, czy sprzęt jest „sprawny”, „do naprawy”, i gdzie leży („strych”, „piwnica”).
- **Obsługa zdjęć**: Dodawaj fotki swoich retro maszyn i przeglądaj je w pełnym ekranie.
- **Elastyczna baza danych**: Wybierz między SQLite3 (prosta, lokalna) a MySQL (dla bardziej zaawansowanych).
- **UUID**: Unikalne identyfikatory dla każdego rekordu, gotowe na duże kolekcje.
- **Skórki**: Można definiować swoje skórki, żeby można było wpisywać swoje eksponaty w klimacie ulubionego retro kompuera.

Program jest w pełni funkcjonalny, ale wciąż się rozwija – kolejne funkcje już w planach!

## Jak zacząć?

### Wymagania
- **Qt**: Używamy Qt 6.9.0 (testowane z Qt Creator 16.0.1).
- **Baza danych**: Potrzebujesz SQLite3 (wbudowane) lub MySQL (jeśli chcesz serwer).
- **System**: Działa na Windows, macOS i Linux.

### Instalacja
1. Zainstaluj [Qt 6.9.0](https://www.qt.io/download) i Qt Creator.
2. Pobierz kod źródłowy projektu:

git clone https://github.com/arekbr/Inwentaryzacja.git

### uruchamianie skryptów 
W osobnych katalogach są skrypty dla systemów Windows/Linux/Macos. Testowałem to na Windows 11/Debian 12/Macos Ventura 13.75. Pliki trzeba skopiować do głównego katalogu projektu i uruchamiać w kolejności. Wynikiem powinien być gotowy program oraz paczka w zależności od systemu. Instalator exe, plik DEB oraz DMG.

### GitHub Actions – zdalne buildy instalatorów
Repo ma też workflow GitHub Actions w `.github/workflows/build-installers.yml`, który buduje zdalnie:
- pakiet `Debian .deb`
- instalator `Windows x64`
- paczkę `macOS Intel`
- paczkę `macOS ARM`

Workflow można uruchomić ręcznie z zakładki `Actions` albo automatycznie po pushu na wybrane gałęzie i tagi `v*`.

Wyniki lądują jako:
- `Artifacts` w danym uruchomieniu workflow
- pliki release, jeśli workflow został uruchomiony z taga `v*`

Skrypty pakujące używane przez CI są w katalogu `ci/`.

Masz problemy z kompilacją? Napisz, spróbujemy ogarnąć – wiem, jak to jest, gdy „nic nie działa”! 😜 Jak wyszło z doświadczeń i pracy, szczególnie chodzi o system macos, to musi być wspierany, żeby skrypty działały. Sama apka się kompiluje, gdyby ktoś chciał, ale CMakelist i skrypty będą działać tylko tam, gdzie brew może w pełni zarządzać systemem. Czyli Ventura to minimum.

## Chcesz dołączyć?

To projekt dla pasjonatów! Jeśli jesteś maniakiem retro, programistą, albo po prostu lubisz eksperymenty z AI, zapraszam do współpracy. Możesz:
- **Być operatorem AI**: Testuj modele, generuj kod, poprawiaj moje próby.
- **Kodować**: Jeśli ogarniasz C++ i Qt, pomóż rozwinąć projekt.
- **Projektować GUI**: Można się pokusić o lifting!
- **Dawać pomysły**: Masz wizję nowej funkcji? Dawaj znać!

Pisz na do mnie, albo dołącz do repozytorium. Razem stworzymy soft, który uszczęśliwi każdego kolekcjonera retro!

## Dlaczego AI?

Bo to przyszłość, a ja lubię testować granice. ChatGPT i GROK piszą kod szybciej, niż ja nadążam sprawdzać. To jak mieć zespół programistów w kieszeni – tylko czasem trzeba ich poprawić. 😄 Eksperyment trwa, a efekty? Sami oceńcie – Inwentaryzacja działa i ma się dobrze!

## Plany na przyszłość

- Lepsze GUI – coś, co odda klimat retro komputerów.
- Eksport/import danych (np. CSV, JSON).
- Statystyki kolekcji (np. ile maszyn, w jakim stanie).
- Może apka mobilna? (Ale to daleka przyszłość!)

## Podziękowania

Specjalne wirtualne ukłony dla **ChatGPT** i **GROK** – bez nich ten kod by nie powstał!

---

Wszystkiego dobrego!  
**Arek "Yugorin" Bronowicki**  
SAMAR

Licencja
Kod źródłowy Inwentaryzacji jest udostępniony na licencji WTFPL (Do What The F*ck You Want To Public License) – róbcie z nim, co chcecie! 😄 Zobacz plik LICENSE dla szczegółów.
Zależności
Projekt używa następujących bibliotek, każda z własną licencją:

Qt: Na licencji LGPLv3. Kod źródłowy i szczegóły licencji dostępne na qt.io. Przy dystrybucji binarnej (np. InwentaryzacjaInstaller.exe) należy zapewnić dostęp do kodu źródłowego Qt.
SQLite: W domenie publicznej, bez ograniczeń. Zobacz sqlite.org.
MySQL Connector/C: Na licencji GPLv2 z wyjątkiem FOSS, zgodny z LGPL w tym projekcie. Zobacz mysql.com.
MinGW (biblioteki runtime GCC): Na licencjach permissywnych (GPL z wyjątkami). Zobacz mingw.org.
Inne zależności (np. OpenSSL, jeśli używane): Sprawdź ich licencje w dystrybucji źródłowej.
Fonty TTF: C64 - Copyright Filip Blazek 2008, ZX Spectrum: Copyright (c) 2016, Jorge Ferrer, Berkelium64 GEOS: Created by Rebecca Bettencourt with FontForge 2.0 (http://fontforge.sf.net), EightBitAtari-Ataripl : Outlines Copyright (c) 2018, Steve Boswell (https://choccyhobnob.com), Amiga Topaz: https://github.com/rewtnull/amigafonts/blob/master/README

Aby być w zgodzie z licencjami, upewnij się, że przy dystrybucji aplikacji zapewniasz dostęp do kodu źródłowego bibliotek LGPL/GPL.
Podziękowania
Wielkie dzięki dla ChatGPT i Grok za napisanie kodu, oraz dla Qt, SQLite, MySQL i MinGW za napędzanie tego projektu! 😎
