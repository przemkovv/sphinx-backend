--
-- File generated with SQLiteStudio v3.1.0 on Mon Oct 31 20:20:09 2016
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: courses
CREATE TABLE courses (
    id          INTEGER NOT NULL
                        PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL,
    description TEXT
);


-- Table: modules
CREATE TABLE modules (
    id        INTEGER PRIMARY KEY AUTOINCREMENT
                      NOT NULL,
    course_id INTEGER NOT NULL
                      REFERENCES courses (id),
    name      TEXT
);


-- Table: users
CREATE TABLE users (
    id       INTEGER NOT NULL
                     PRIMARY KEY AUTOINCREMENT,
    username TEXT    NOT NULL,
    email    TEXT    NOT NULL
                     UNIQUE
);


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
