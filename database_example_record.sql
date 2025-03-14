INSERT INTO types (name) VALUES ('Komputer'), ('Monitor'), ('Kabel');
INSERT INTO vendors (name) VALUES ('Commodore'), ('Atari');
INSERT INTO models (name, vendor_id) VALUES ('Amiga 500', 1), ('C64', 1), ('Atari 800XL', 2);
INSERT INTO statuses (name) VALUES ('Sprawny'), ('Uszkodzony'), ('W naprawie');
INSERT INTO storage_places (name) VALUES ('Magazyn 1'), ('Półka B3');
INSERT INTO eksponaty (name, type_id, vendor_id, model_id, serial_number, production_year, status_id, storage_place_id, description, value)
VALUES ('Amiga 500', 1, 1, 1, 'A500-1234', 1987, 1, 1, 'Klasyczny komputer Amiga', 2);
