To locate the project in a webserver the directory "web" should nesessarily contain 3 files, together with all release code:

1. "config.json" - config file in a format JSON with the following fields:

{
"name": "<Project name>",
"description": "<Short project description>",
"author": "<Author name>",
"affiliation": "<Author affiliation>",
"inputs": "<Description data format>",
"command": "<Matlab script returning an html-string, example: main_SSAForecasting.m>",
"testfile": "<Name of test data file, example: test.csv>",
"date": "<Date of release>",
"course": "<Name of a course>",
"presentation": "<Link on a presentation located at SF>",
"systemdocs": "<Link on system documentation located at SF>",
"probstatement": "<Link on problem statement description located at SF>"
}

2. Matlab main script, field "command" in config (example: main_SSAForecasting.m).
The script should accept only one data file (test data file) and return only an html string. 

3. Test data file, field "testfile" in config, an input for the main web script.
Your project will take as input this test file if not determined by user.

Attention! Together with this three files, all supplementary files: 
code, algorithm parameters, auxiliary files using by the main script should be also located in a directory "web".
