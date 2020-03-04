SELECT
    ChangedToAt,
    Name
FROM
    NameHistory
WHERE
    Uuid = '%1'
ORDER BY
    ChangedToAt ASC;
