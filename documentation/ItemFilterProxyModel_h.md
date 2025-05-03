# Dokumentacja dla pliku `ItemFilterProxyModel.h`

## Metadane
- **Plik**: ItemFilterProxyModel.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `ItemFilterProxyModel.h` definiuje klasę `ItemFilterProxyModel`, która dziedziczy po `QSortFilterProxyModel` i służy do dynamicznego filtrowania listy eksponatów w aplikacji inwentaryzacyjnej. Klasa umożliwia ustawienie filtrów dla atrybutów eksponatu, takich jak typ, producent, model, status i miejsce przechowywania, oraz kontroluje, które wiersze modelu źródłowego są wyświetlane w widoku na podstawie zadanych kryteriów.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `ItemFilterProxyModel`**  
   - Dziedziczy po `QSortFilterProxyModel`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje model proxy.
   - Metody ustawiania filtrów: `setTypeFilter`, `setVendorFilter`, `setModelFilter`, `setStatusFilter`, `setStorageFilter` – ustawiają filtry dla odpowiednich atrybutów.

3. **Metoda chroniona**  
   - `filterAcceptsRow`: Implementuje logikę filtrowania, decydując, które wiersze modelu źródłowego są widoczne.

4. **Zmienne prywatne**  
   - `m_type`, `m_vendor`, `m_model`, `m_status`, `m_storage`: Przechowują wartości filtrów dla każdego atrybutu.

## Opis działania
- **Inicjalizacja**: Konstruktor tworzy model proxy jako dziecko podanego rodzica, przygotowując go do filtrowania danych.
- **Ustawianie filtrów**: Dedykowane metody (`setTypeFilter`, `setVendorFilter` itp.) zapisują wartości filtrów dla odpowiednich atrybutów i odświeżają widok poprzez wywołanie `invalidateFilter()`.
- **Filtrowanie**: Metoda `filterAcceptsRow` sprawdza, czy wartości w wierszu modelu źródłowego odpowiadają ustawionym filtrom. Pusty filtr dla danego atrybutu oznacza akceptację wszystkich wartości dla tej kolumny.
- **Zastosowanie**: Klasa jest używana w aplikacji inwentaryzacyjnej do dynamicznego filtrowania danych w widoku (np. `QTableView`), współpracując z modelem źródłowym, takim jak `QSqlRelationalTableModel`.

## Zależności
- **Biblioteki Qt**:
  - `QSortFilterProxyModel`: Bazowa klasa dla filtrowania i sortowania danych.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Filtry są niezależne, co pozwala na elastyczne kombinacje kryteriów filtrowania.
- Klasa zakłada, że model źródłowy ma określoną strukturę kolumn (np. typ w kolumnie 2), co jest zgodne z modelem `QSqlRelationalTableModel` używanym w aplikacji.
- Filtrowanie jest dynamiczne i natychmiast odświeża widok po zmianie filtru.