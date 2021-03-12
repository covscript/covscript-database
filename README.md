# Open Database Connectivity for Covariant Script

**Special Notice: It's not recommended to use SQLite3 via ODBC Driver while Covariant Script have native SQLite3 support.**

## Official Build & Tested Platform

|Platform|Data Source Administrator|ODBC Driver|
|----|----|----|
|Microsoft Windows|ODBC Data Sources|MariaDB ODBC Driver|
|Microsoft Windows|ODBC Data Sources|MySQL ODBC Driver|
|Apple macOS|iODBC|MariaDB ODBC Driver|
|Ubuntu 20.04|unixODBC|MariaDB ODBC Driver|

## Preparation

### Windows

Download and install [MariaDB ODBC Driver](https://mariadb.com/downloads/)

### macOS

1. Download [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/Downloads)
2. Download [MariaDB ODBC Driver](https://mariadb.com/downloads/)

### Linux

1. Install unixODBC: `sudo apt install unixodbc`
2. Install MariaDB ODBC Driver: `sudo apt install odbc-mariadb`

## Configuration

1. [MariaDB](https://mariadb.com/kb/en/creating-a-data-source-with-mariadb-connectorodbc/)
    + If it doesn't work properly on Ubuntu after following the official documentation, try modifying the configuration file: `Driver=/usr/lib/x86_64-linux-gnu/odbc/libmaodbc.so`
2. [MySQL](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration.html)
