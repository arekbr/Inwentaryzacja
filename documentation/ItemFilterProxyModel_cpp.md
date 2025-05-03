# Dokumentacja dla pliku `ItemFilterProxyModel.cpp`

## Metadane
- **Plik**: ItemFilterProxyModel.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `ItemFilterProxyModel.cpp` zawiera implementację metod klasy `ItemFilterProxyModel`, która umożliwia dynamiczne filtrowanie listy eksponatów w aplikacji inwentaryzacyjnej na podstawie atrybutów takich jak typ, producent, model, status i miejsce przechowywania. Klasa korzysta z mechanizmu `QSortFilterProxyModel`, zapewniając filtrowanie wierszy modelu źródłowego (np. `QSqlRelationalTableModel`) z ignorowaniem wielkości liter.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje model proxy i konfiguruje filtrowanie bez uwzględnienia wielkości liter.

2. **Sekcja metod ustawiania filtrów**  
   - Metody `setTypeFilter`, `setVendorFilter`, `setModelFilter`, `setStatusFilter`, `setStorageFilter` zapisują wartości filtrów i odświeżają widok.

3. **Sekcja metody filterAcceptsRow**  
   - Implementuje logikę filtrowania, sprawdzając zgodność wierszy modelu źródłowego z filtrami.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia model proxy jako dziecko podanego rodzica i konfiguruje filtrowanie z ignorowaniem wielkości liter (`Qt::CaseInsensitive`).

2. **Ustawianie filtrów**:
   - Każda metoda ustawiania filtru (`setTypeFilter`, `setVendorFilter` itp.) zapisuje wartość filtru w odpowiedniej zmiennej prywatnej (`m_type`, `m_vendor` itp.) i wywołuje `invalidateFilter()`, co odświeża widok z uwzględnieniem nowego filtru.
   - Pusty ciąg jako wartość filtru oznacza brak ograniczeń dla danego atrybutu.

3. **Filtrowanie**:
   - Metoda `filterAcceptsRow` sprawdza, czy wartości w kolumnach modelu źródłowego (2: typ, 3: producent, 4: model, 9: status, 10: miejsce przechowywania) odpowiadają ustawionym filtrom.
   - Używa lambdy `pasuje`, która porównuje wartość w kolumnie z wartością filtru. Jeśli filtr jest pusty, wszystkie wartości dla tej kolumny są akceptowane.
   - Wiersz jest widoczny tylko wtedy, gdy spełnia wszystkie aktywne filtry.

## Zależności
- **Biblioteki Qt**:
  - `QSortFilterProxyModel`: Bazowa klasa dla filtrowania i sortowania.
  - `QModelIndex`, `QAbstractItemModel`: Obsługa indeksów i danych modelu źródłowego.
- **Nagłówek**:
  - `ItemFilterProxyModel.h`: Definicja klasy.
- **Zasoby**:
  - Brak zależności od zasobów Qt.

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Filtrowanie ignoruje wielkość liter, co ułatwia dopasowanie wartości.
- Struktura kolumn (2, 3, 4, 9, 10) jest zgodna z modelem `QSqlRelationalTableModel` używanym w aplikacji.
- Wywołanie `invalidateFilter()` po każdej zmianie filtru zapewnia natychmiastowe odświeżenie widoku.