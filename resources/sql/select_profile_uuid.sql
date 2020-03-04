SELECT
    Uuid,
    DateTime,
    FirstName,
    CurrentName,
    SkinUrl,
    Skin,
    SkinModel,
    CapeUrl,
    Cape,
    Legacy,
    Demo,
    Comments,
    NameHistory
FROM
    Profiles
WHERE
    Uuid = '%1';

