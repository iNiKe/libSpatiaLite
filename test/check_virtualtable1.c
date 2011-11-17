/*

 check_virtualtable1.c -- SpatiaLite Test Case

 Author: Brad Hards <bradh@frogmouth.net>

 ------------------------------------------------------------------------------
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2011
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards <bradh@frogmouth.net>
Ahmadou Dicko <dicko.ahmadou@gmail.com>

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.
 
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"

int main (int argc, char *argv[])
{
    sqlite3 *db_handle = NULL;
    char *sql_statement;
    int ret;
    char *err_msg = NULL;
    int i;
    char **results;
    int rows;
    int columns;

    spatialite_init (0);

    ret = sqlite3_open_v2 (":memory:", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "cannot open in-memory db: %s\n", sqlite3_errmsg (db_handle));
	sqlite3_close (db_handle);
	db_handle = NULL;
	return -1;
    }
    
    ret = sqlite3_exec (db_handle, "create VIRTUAL TABLE places USING VirtualText(\"testcase1.csv\", UTF-8, 0, POINT, DOUBLEQUOTE);", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "VirtualText error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -2;
    }
    
    asprintf(&sql_statement, "select col003, col005, col006, col008 from places WHERE col003 = \"Canal Creek\";");
    ret = sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns, &err_msg);
    free(sql_statement);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "Error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -10;
    }
    if ((rows != 2) || (columns != 4)) {
	fprintf (stderr, "Unexpected error: select columns bad result: %i/%i.\n", rows, columns);
	return  -11;
    }
    if (strcmp(results[0], "COL003") != 0) {
	fprintf (stderr, "Unexpected error: header() bad result: %s.\n", results[0]);
	return  -12;
    }
    if (strcmp(results[4], "Canal Creek") != 0) {
	fprintf (stderr, "Unexpected error: name4() bad result: %s.\n", results[4]);
	return  -13;
    }
    if (strncmp(results[5], "-27.86667", 9) != 0) {
	fprintf (stderr, "Unexpected error: lat1() bad result: %s.\n", results[5]);
	return  -14;
    }
    if (strncmp(results[6], "151.51667", 9) != 0) {
	fprintf (stderr, "Unexpected error: lon2() bad result: %s.\n", results[6]);
	return  -15;
    }
    if (strcmp(results[8], "Canal Creek") != 0) {
	fprintf (stderr, "Unexpected error: name8() bad result: %s.\n", results[8]);
	return  -16;
    }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "BEGIN;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "BEGIN error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -21;
    }

    ret = sqlite3_exec (db_handle, "DELETE FROM places WHERE col015 > 100000;", NULL, NULL, &err_msg);
    if (ret != SQLITE_READONLY) {
	fprintf (stderr, "UPDATE error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -21;
    }
    sqlite3_free (err_msg);

    ret = sqlite3_exec (db_handle, "ROLLBACK;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "ROLLBACK error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -22;
    }
 
    ret = sqlite3_get_table (db_handle, "SELECT col003, col015 FROM places WHERE col015 > 100000", &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "Error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -23;
    }
    if ((rows != 1) || (columns != 2)) {
	fprintf (stderr, "Unexpected error: select columns bad result2: %i/%i.\n", rows, columns);
	return  -24;
    }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "DROP TABLE places;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
	fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	sqlite3_free (err_msg);
	return -25;
    }

    sqlite3_close (db_handle);
    spatialite_cleanup();
    sqlite3_reset_auto_extension();
    
    return 0;
}