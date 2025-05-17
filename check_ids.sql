-- Sprawdzenie ID w tabeli eksponaty
SELECT 'Eksponaty z nieprawidłowym status_id' as check_type,
       id, status_id
FROM eksponaty 
WHERE status_id LIKE '{%'
   OR status_id NOT LIKE '________-____-____-____-____________'
UNION ALL
SELECT 'Eksponaty z nieprawidłowym storage_place_id' as check_type,
       id, storage_place_id
FROM eksponaty
WHERE storage_place_id LIKE '{%'
   OR storage_place_id NOT LIKE '________-____-____-____-____________'
UNION ALL
SELECT 'Eksponaty z nieprawidłowym type_id' as check_type,
       id, type_id
FROM eksponaty
WHERE type_id LIKE '{%'
   OR type_id NOT LIKE '________-____-____-____-____________'
UNION ALL
SELECT 'Eksponaty z nieprawidłowym vendor_id' as check_type,
       id, vendor_id
FROM eksponaty
WHERE vendor_id LIKE '{%'
   OR vendor_id NOT LIKE '________-____-____-____-____________'
UNION ALL
SELECT 'Eksponaty z nieprawidłowym model_id' as check_type,
       id, model_id
FROM eksponaty
WHERE model_id LIKE '{%'
   OR model_id NOT LIKE '________-____-____-____-____________'; 