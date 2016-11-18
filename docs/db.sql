-- Schema: public

DROP TABLE users;
DROP TABLE modules;
DROP TABLE courses;


CREATE TABLE users (
	id	SERIAL PRIMARY KEY,
	username  varchar(50) NOT NULL,
	email varchar(50) NOT NULL,

	CONSTRAINT email_unique UNIQUE(email)
);

CREATE TABLE courses (
    id          SERIAL PRIMARY KEY ,
    name        varchar(100)    NOT NULL,
    description text
);


-- Table: modules
CREATE TABLE modules (
    id        SERIAL PRIMARY KEY,
    course_id INTEGER NOT NULL
                      REFERENCES courses (id),
   name        varchar(100)    NOT NULL,
    description text
);


