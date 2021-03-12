#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void error(char *message)
{
    perror(message);
    perror("\n");
    exit(1); 
}



MYSQL* connection = NULL;
MYSQL_STMT* statementLocal = NULL;
MYSQL_STMT* statementRemote = NULL;

void Connect()
{
    connection = mysql_init(NULL); 
    if (connection == NULL) error("Could not initialize connection.");
    connection = mysql_real_connect(connection, "<database.host>", "<database.username>", "<database.password>", "<database.database>", 3306, NULL, 0);
    if (connection == NULL) error("Could not connect to the database.");
}

void Disconnect()
{
    mysql_close(connection);
}

void PrepareStatements()
{
    int result;


    statementLocal = mysql_stmt_init(connection);
    if (statementLocal == NULL) error("Could not create prepared statement.");
    result = mysql_stmt_prepare(statementLocal, "SELECT DISTINCT host, port FROM SshTunnel WHERE server = ? AND (entity = ? OR entity = IN (SELECT organization FROM Employee WHERE user = ?) OR entity = IN (SELECT team FROM Member WHERE user = ?)) AND (type = 1 OR type = 3)", 224);
    if (result != 0) error("Could not prepare statement.");


    statementRemote = mysql_stmt_init(connection);
    if (statementRemote == NULL) error("Could not create prepared statement.");
    result = mysql_stmt_prepare(statementRemote, "SELECT DISTINCT host, port FROM SshTunnel WHERE server = ? AND (entity = ? OR entity = IN (SELECT organization FROM Employee WHERE user = ?) OR entity = IN (SELECT team FROM Member WHERE user = ?)) AND (type = 2 OR type = 3)", 224);
    if (result != 0) error("Could not prepare statement.");
}

void DestroyStatements()
{
    mysql_stmt_close(statementLocal);
    mysql_stmt_close(statementRemote);
}



void getAccessInformation(MYSQL_STMT *statement, long long server, long long user, int account)
{
    int result;
    char host[65];
    int port;


    MYSQL_BIND params[5];
    MYSQL_BIND fields[2];
    MYSQL_RES *metaData;


    memset(params, 0, 5 * sizeof(MYSQL_BIND));
    memset(fields, 0, 2 * sizeof(MYSQL_BIND));


    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &server;

    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &account;

    params[2].buffer_type = MYSQL_TYPE_LONGLONG;
    params[2].buffer = &user;

    params[3].buffer_type = MYSQL_TYPE_LONGLONG;
    params[3].buffer = &user;

    params[4].buffer_type = MYSQL_TYPE_LONGLONG;
    params[4].buffer = &user;

    fields[0].buffer_type = MYSQL_TYPE_STRING;
    fields[0].buffer = &host;
    fields[0].buffer_length = 64;

    fields[1].buffer_type = MYSQL_TYPE_LONG;
    fields[1].buffer = &port;


    metaData = mysql_stmt_result_metadata(statement); 
    if (metaData == NULL) error("Could not get statement metadata"); 
    result = mysql_stmt_bind_param(statement, params);
    if (result != 0) error("Could not bind params to statement.");
    result = mysql_stmt_bind_result(statement, fields);
    if (result != 0) error("Could not bind fields to statement.");
    result = mysql_stmt_execute(statement); 
    if (result != 0) error("Could not execute statement.");


    int found = 0;
    while (1) {
        result = mysql_stmt_fetch(statement); 
        if (result == MYSQL_NO_DATA) break; 
        if (result != 0) error("Could not parse statement result.");
        
        found = 1;
        host[fields[0].length_value] = '\0';
        
        if (found == 1) {
            printf(",%s:%d", host, port);
        } else {
            printf("%s:%d", host, port);
        }
    }


    mysql_free_result(metaData);
}





int main(int argc, char **argv)
{
    if (argc != 4) error("Incorrect amount of arguments.");


    long long server = atol(argv[1]);
    long long user = atol(argv[2]);
    int account = atoi(argv[3]);


    Connect();
    PrepareStatements();

    printf("permitlisten=\"");
    getAccessInformation(statementRemote, server, user, account);
    printf("\" permitopen=\"");
    getAccessInformation(statementLocal, server, user, account);
    printf("\" command=\"/bin/true\" %lld", user);
    
    DestroyStatements();
    Disconnect();

    return 0;
}