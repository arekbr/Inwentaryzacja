# Inwentaryzacja – Program do zarządzania kolekcją retro komputerów

Witaj w projekcie **Inwentaryzacja**! To narzędzie stworzone z myślą o maniakach retro komputerów, którzy chcą uporządkować swoje zbiory – od Amigi po ZX Spectrum. Program powstaje jako eksperyment z użyciem AI, ale z sercem i pasją do oldschoolowego sprzętu!

## Po co to wszystko?

Chciałem mieć soft skrojony pod moje potrzeby – prosty, ale funkcjonalny, do zarządzania kolekcją retro komputerów. Sam jestem „marnym programistą” (doba ma tylko 24 godziny!), więc pisanie tego od zera zajęłoby mi pewnie z 5 lat. Zamiast tego postawiłem na eksperyment: tworzę ten program z pomocą AI – **ChatGPT** i **GROK**. 

Jedyna rzecz, którą zrobiłem sam, to początkowe GUI, ale i tak AI zrobiło poprawioną wersję. 

Dlaczego tak? Bo to frajda, wyzwanie i sposób, żeby mieć narzędzie, jakiego nie znajdę na półce. A przy okazji uczę się C++ – powoli, w zrywach, ale z planem na emeryturę, kiedy ogarnę C jak mistrz! 😄

## Co potrafi program?

Inwentaryzacja to więcej niż prosta baza danych. Aktualna wersja (1.2.0) oferuje:
- **Zarządzanie elementami**: Dodawaj, edytuj, usuwaj komputery i akcesoria z kolekcji.
- **Typy i producenci**: Kategoryzuj sprzęt (np. Amiga, Atari) i przypisuj producentów.
- **Statusy i miejsca przechowywania**: Oznacz, czy sprzęt jest „sprawny”, „do naprawy”, i gdzie leży („strych”, „piwnica”).
- **Obsługa zdjęć**: Dodawaj fotki swoich retro maszyn i przeglądaj je w pełnym ekranie.
- **Elastyczna baza danych**: Wybierz między SQLite3 (prosta, lokalna) a MySQL (dla bardziej zaawansowanych).
- (TODO) **Filtrowanie i wyszukiwanie**: Szybko znajdź sprzęt po nazwie, typie czy statusie.
- **UUID**: Unikalne identyfikatory dla każdego rekordu, gotowe na duże kolekcje.

Program jest w pełni funkcjonalny, ale wciąż się rozwija – kolejne funkcje już w planach!

## Jak zacząć?

### Wymagania
- **Qt**: Używamy Qt 6.8.1 (testowane z Qt Creator 15.0.1).
- **Baza danych**: Potrzebujesz SQLite3 (wbudowane) lub MySQL (jeśli chcesz serwer).
- **System**: Działa na Windows, macOS i Linux.

### Instalacja
1. Zainstaluj [Qt 6.8.1](https://www.qt.io/download) i Qt Creator.
2. Pobierz kod źródłowy projektu:

git clone https://github.com/arekbr/Inwentaryzacja.git

Masz problemy z kompilacją? Napisz, spróbujemy ogarnąć – wiem, jak to jest, gdy „nic nie działa”! 😜

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
