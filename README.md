# Program do inwentaryzacji zbiorów retro kompuerów



## Jak i po co?

Program powstaje wyłącznie przy użyciu ChatGPT (różne modele) oraz GROK. Niedługo dołączy pewnie DeepSeek. Jedyna rzecz którą zrobiłem sam, to GUI (pewnie dlatego wygląda tak ascetycznie). Jednak dopracuję to w swoim czasie. Na razie skupiam się na tym, żeby program działał tak jak tego potrzebuję. Dlaczego tak? Trochę jako eksperyment, ale głównie potrzeba posiaania takiego softu, skrojonego dla mnie. Sam jestem marnym programistą. Napisanie tego zajełoby pewnie z 5 lat o ile dałbym radę. Zajmuję się czymś innym, doba ma tylko 24h. Na emeryturze dam radę. Takie mam postanowienie, że się nauczę C. Dla własnej przyjemności. Teraz, mam zrywy, coś tam zrobię, potem porzucam na miesiące. Nie da się inaczej. Jeśli jednak ktoś z Was chce dołączyc do tego szaleństwa :) jako operator AI lub prawdziwy programista, który ogarnia tę kuwetę to zapraszam. Możemy stworzy soft który pomoże takim maniakom retro jak ja.

---
Mając na uwadze swoje doświadczenia z projektami, które nigdy nie działają :P przy próbie kompilacji. Używam tego QT. Na maku i pod linuxem.

  Qt Creator 15.0.1  
  Bazuje na Qt 6.8.1 (Clang 15.0 (Apple), x86_64)
  Wersja z Jan 22 2025 13:40:45  
  Z wersji 0855a2e14d  
---  
  
  

oraz 
  sqlite3  

zakładam bazę w ten sposób

+ sqlite3 "/ścieżka/do/bazy/muzeum.db"

wpisuję to

+ .read createTable.sql

potem robię

+ INSERT INTO types (name) VALUES ('Komputer'), ('Monitor'), ('Kabel');
+ INSERT INTO vendors (name) VALUES ('Commodore'), ('Atari');
+ INSERT INTO models (name, vendor_id) VALUES ('Amiga 500', 1), ('C64', 1), ('Atari 800XL', 2);
+ INSERT INTO statuses (name) VALUES ('Sprawny'), ('Uszkodzony'), ('W naprawie');
+ INSERT INTO storage_places (name) VALUES ('Magazyn 1'), ('Półka B3');
+ INSERT INTO eksponaty (name, type_id, vendor_id, model_id, serial_number, production_year, status_id, storage_place_id, description, value)
+ VALUES ('Amiga 500', 1, 1, 1, 'A500-1234', 1987, 1, 1, 'Klasyczny komputer Amiga', 2);

i tym sposobem wpisuję przykładowy rekord do bazy.  


---
  wszystkiego dobrego  
  Arek "Yugorin" Bronowicki  
  SAMAR 

