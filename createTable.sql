-- createTable_uuid.sql
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;

CREATE TABLE eksponaty (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    type_id TEXT NOT NULL,
    vendor_id TEXT NOT NULL,
    model_id TEXT NOT NULL,
    serial_number TEXT,
    part_number TEXT,
    revision TEXT,
    production_year INTEGER CHECK(production_year BETWEEN 1900 AND 2100),
    status_id TEXT NOT NULL,
    storage_place_id TEXT NOT NULL,
    description TEXT,
    value INTEGER CHECK(value >= 0),
    FOREIGN KEY (type_id) REFERENCES types(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (status_id) REFERENCES statuses(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (storage_place_id) REFERENCES storage_places(id) ON DELETE RESTRICT ON UPDATE CASCADE
);

CREATE TABLE types (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE vendors (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE models (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    vendor_id TEXT NOT NULL,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE statuses (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE storage_places (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE photos (
    id TEXT PRIMARY KEY,
    eksponat_id TEXT NOT NULL,
    photo BLOB NOT NULL,
    FOREIGN KEY (eksponat_id) REFERENCES eksponaty(id) ON DELETE CASCADE ON UPDATE CASCADE
);

-- Indeks po numerze seryjnym
CREATE INDEX idx_serial_number ON eksponaty(serial_number);
