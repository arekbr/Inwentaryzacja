PRAGMA foreign_keys = ON;

-- Tabela główna: Eksponaty (przechowuje wpisy inwentarza)
CREATE TABLE eksponaty (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type_id INTEGER NOT NULL,
    vendor_id INTEGER NOT NULL,
    model_id INTEGER NOT NULL,
    serial_number TEXT UNIQUE,
    part_number TEXT,
    revision TEXT,
    production_year INTEGER CHECK(production_year BETWEEN 1900 AND 2100),
    status_id INTEGER NOT NULL,
    storage_place_id INTEGER NOT NULL,
    description TEXT,
    value INTEGER CHECK(value >= 0),
    FOREIGN KEY (type_id) REFERENCES types(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (status_id) REFERENCES statuses(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (storage_place_id) REFERENCES storage_places(id) ON DELETE RESTRICT ON UPDATE CASCADE
);

-- Tabela Typów (np. komputer, monitor, kabel)
CREATE TABLE types (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL
);

-- Tabela Producentów (np. Commodore, Atari)
CREATE TABLE vendors (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL
);

-- Tabela Modeli (np. Amiga 500, C64)
CREATE TABLE models (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    vendor_id INTEGER NOT NULL,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE CASCADE ON UPDATE CASCADE
);

-- Tabela Statusów (np. sprawny, uszkodzony, w naprawie)
CREATE TABLE statuses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL
);

-- Tabela Miejsc przechowywania (np. magazyn 1, półka B3)
CREATE TABLE storage_places (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL
);

-- Tabela Zdjęć (zdjęcia są przechowywane w formacie BLOB)
CREATE TABLE photos (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    eksponat_id INTEGER NOT NULL,
    photo BLOB NOT NULL,
    FOREIGN KEY (eksponat_id) REFERENCES eksponaty(id) ON DELETE CASCADE ON UPDATE CASCADE
);

-- Indeks dla szybszego wyszukiwania po numerze seryjnym
CREATE INDEX idx_serial_number ON eksponaty(serial_number);
