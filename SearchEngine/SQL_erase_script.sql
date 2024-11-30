-- Очистка содержимого таблиц и сброс ID для таблицы documents
DO $$
BEGIN
    -- Очищаем таблицу
    TRUNCATE TABLE documents CASCADE;
    
    -- Сбрасываем последовательность на 1
    PERFORM setval(pg_get_serial_sequence('documents', 'id'), 1, false);
END $$;

-- Очистка содержимого таблиц и сброс ID для таблицы words
DO $$
BEGIN
    -- Очищаем таблицу
    TRUNCATE TABLE words;
    
    -- Сбрасываем последовательность на 1
    PERFORM setval(pg_get_serial_sequence('words', 'id'), 1, false);
END $$;
