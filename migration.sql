-- Dodanie kolumny has_original_packaging do tabeli eksponaty
ALTER TABLE eksponaty ADD COLUMN has_original_packaging INTEGER DEFAULT 0; 