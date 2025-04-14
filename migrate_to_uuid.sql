BEGIN TRANSACTION;

----------------------------------------------------------------------
-- 1. Tabela types
----------------------------------------------------------------------
CREATE TABLE new_types (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

-- Tymczasowa tabela mapująca:  stare_types_id (INT) → new_types_id (TEXT).
CREATE TEMP TABLE temp_id_map_types (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_types(old_id, new_id)
    SELECT t.id, hex(randomblob(16)) FROM types t;

INSERT INTO new_types(id, name)
    SELECT m.new_id, t.name
    FROM types t
    JOIN temp_id_map_types m ON t.id = m.old_id;

----------------------------------------------------------------------
-- 2. Tabela vendors
----------------------------------------------------------------------
CREATE TABLE new_vendors (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TEMP TABLE temp_id_map_vendors (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_vendors(old_id, new_id)
    SELECT v.id, hex(randomblob(16)) FROM vendors v;

INSERT INTO new_vendors(id, name)
    SELECT m.new_id, v.name
    FROM vendors v
    JOIN temp_id_map_vendors m ON v.id = m.old_id;

----------------------------------------------------------------------
-- 3. Tabela models
----------------------------------------------------------------------
CREATE TABLE new_models (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    vendor_id TEXT NOT NULL
);

CREATE TEMP TABLE temp_id_map_models (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_models(old_id, new_id)
    SELECT mo.id, hex(randomblob(16)) FROM models mo;

INSERT INTO new_models(id, name, vendor_id)
    SELECT mm.new_id,
           mo.name,
           mv.new_id  -- Zastępujemy stare vendor_id nowym
    FROM models mo
    JOIN temp_id_map_models mm   ON mo.id       = mm.old_id
    JOIN temp_id_map_vendors mv  ON mo.vendor_id= mv.old_id;

----------------------------------------------------------------------
-- 4. Tabela statuses
----------------------------------------------------------------------
CREATE TABLE new_statuses (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TEMP TABLE temp_id_map_statuses (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_statuses(old_id, new_id)
    SELECT s.id, hex(randomblob(16)) FROM statuses s;

INSERT INTO new_statuses(id, name)
    SELECT ms.new_id, s.name
    FROM statuses s
    JOIN temp_id_map_statuses ms ON s.id = ms.old_id;

----------------------------------------------------------------------
-- 5. Tabela storage_places
----------------------------------------------------------------------
CREATE TABLE new_storage_places (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TEMP TABLE temp_id_map_storage_places (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_storage_places(old_id, new_id)
    SELECT sp.id, hex(randomblob(16)) FROM storage_places sp;

INSERT INTO new_storage_places(id, name)
    SELECT ms.new_id, sp.name
    FROM storage_places sp
    JOIN temp_id_map_storage_places ms ON sp.id = ms.old_id;

----------------------------------------------------------------------
-- 6. Tabela eksponaty
----------------------------------------------------------------------
CREATE TABLE new_eksponaty (
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
    value INTEGER CHECK(value >= 0)
);

CREATE TEMP TABLE temp_id_map_eksponaty (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_eksponaty(old_id, new_id)
    SELECT e.id, hex(randomblob(16)) FROM eksponaty e;

INSERT INTO new_eksponaty(
    id, name, type_id, vendor_id, model_id,
    serial_number, part_number, revision, production_year,
    status_id, storage_place_id,
    description, value
)
SELECT
    me.new_id,
    e.name,
    mt.new_id,  -- typ
    mv.new_id,  -- vendor
    mm.new_id,  -- model
    e.serial_number,
    e.part_number,
    e.revision,
    e.production_year,
    ms.new_id,  -- status
    mp.new_id,  -- storage place
    e.description,
    e.value
FROM eksponaty e
JOIN temp_id_map_eksponaty        me ON e.id           = me.old_id
JOIN temp_id_map_types            mt ON e.type_id       = mt.old_id
JOIN temp_id_map_vendors          mv ON e.vendor_id     = mv.old_id
JOIN temp_id_map_models           mm ON e.model_id      = mm.old_id
JOIN temp_id_map_statuses         ms ON e.status_id     = ms.old_id
JOIN temp_id_map_storage_places   mp ON e.storage_place_id = mp.old_id;

----------------------------------------------------------------------
-- 7. Tabela photos
----------------------------------------------------------------------
CREATE TABLE new_photos (
    id TEXT PRIMARY KEY,
    eksponat_id TEXT NOT NULL,
    photo BLOB NOT NULL
);

CREATE TEMP TABLE temp_id_map_photos (
    old_id INT,
    new_id TEXT
);

INSERT INTO temp_id_map_photos(old_id, new_id)
    SELECT p.id, hex(randomblob(16)) FROM photos p;

INSERT INTO new_photos(id, eksponat_id, photo)
SELECT mp.new_id,
       me.new_id,  -- nowe id eksponatu
       p.photo
FROM photos p
JOIN temp_id_map_photos mp     ON p.id = mp.old_id
JOIN temp_id_map_eksponaty me  ON p.eksponat_id = me.old_id;

----------------------------------------------------------------------
-- 8. Usunięcie starych tabel i zmiana nazw
----------------------------------------------------------------------
DROP TABLE photos;
DROP TABLE eksponaty;
DROP TABLE models;
DROP TABLE vendors;
DROP TABLE types;
DROP TABLE statuses;
DROP TABLE storage_places;

ALTER TABLE new_photos RENAME TO photos;
ALTER TABLE new_eksponaty RENAME TO eksponaty;
ALTER TABLE new_models RENAME TO models;
ALTER TABLE new_vendors RENAME TO vendors;
ALTER TABLE new_types RENAME TO types;
ALTER TABLE new_statuses RENAME TO statuses;
ALTER TABLE new_storage_places RENAME TO storage_places;

COMMIT;
