
--
-- File generated with SQLiteStudio v3.1.0 on Sun Oct 30 00:48:39 2016
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
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER NOT NULL,
    name      TEXT
);


-- Table: users
CREATE TABLE users (
    id       INTEGER NOT NULL
                     PRIMARY KEY AUTOINCREMENT,
    username TEXT    NOT NULL,
    email    TEXT    NOT NULL
);


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
