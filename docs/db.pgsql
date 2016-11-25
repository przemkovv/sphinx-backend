-- Schema: public

DROP TABLE tests;
DROP TABLE files;
DROP TABLE solutions;
DROP TABLE tasks;
DROP TABLE modules;
DROP TABLE courses;
DROP TABLE users;
DROP TYPE user_role;
DROP TYPE language;

CREATE TYPE user_role AS ENUM('student', 'teacher', 'admin');
CREATE TYPE language AS ENUM('cpp');

CREATE TABLE users (
	id	SERIAL PRIMARY KEY,
	firstname  VARCHAR(50) NOT NULL,
	lastname  VARCHAR(50) NOT NULL,
	username  VARCHAR(50) NOT NULL,
	student_id VARCHAR(10),
	email VARCHAR(50) NOT NULL,
	role user_role,
	CONSTRAINT email_unique UNIQUE(email),
	CONSTRAINT username_unique UNIQUE(username),
	CONSTRAINT student_id_not_null CHECK ((role IN ('student') AND student_id
			IS NOT NULL) OR (role NOT IN ('student') AND student_id IS NULL))
);

CREATE TABLE courses (
	id          SERIAL PRIMARY KEY ,
	title        VARCHAR(100)    NOT NULL,
	description TEXT,
	owner_id INTEGER NOT NULL REFERENCES users(id)
);


-- Table: modules
CREATE TABLE modules (
	id        SERIAL PRIMARY KEY,
	course_id INTEGER NOT NULL REFERENCES courses (id),
	title        VARCHAR(100)    NOT NULL,
	description TEXT,
	"order" INTEGER
);

CREATE TABLE tasks (
	id SERIAL PRIMARY KEY,
	title VARCHAR(100) NOT NULL,
	description TEXT,
	module_id INTEGER NOT NULL REFERENCES modules(id),
	"order" INTEGER
);

CREATE TABLE solutions (
	id SERIAL primary key,
	task_id INTEGER NOT NULL REFERENCES tasks(id),
	author_id INTEGER NOT NULL REFERENCES users(id),
	date TIMESTAMP WITH TIME ZONE NOT NULL,
	lang language NOT NULL
);

CREATE TABLE files (
	id SERIAL PRIMARY KEY,
	solution_id INTEGER NOT NULL REFERENCES solutions(id),
	filename VARCHAR(50) NOT NULL,
	content TEXT
);

CREATE TABLE tests (
	id SERIAL PRIMARY KEY,
	task_id INTEGER NOT NULL REFERENCES tasks(id),
	stdin TEXT,
	expected_stdout TEXT,
	expected_stderr TEXT
);


SELECT * FROM users;
SELECT * FROM courses;

