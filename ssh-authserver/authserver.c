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





struct IdList {

    long long value;
    struct IdList* next;

};





MYSQL* connection = NULL;
MYSQL_STMT* stmtGetOrganizations = NULL;
MYSQL_STMT* stmtGetTeams = NULL;
MYSQL_STMT* stmtGetAccess = NULL;





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


    stmtGetOrganizations = mysql_stmt_init(connection);
    if (stmtGetOrganizations == NULL) error("Could not create prepared statement.");
    result = mysql_stmt_prepare(stmtGetOrganizations, "SELECT organization FROM Employee WHERE user = ?", 48);
    if (result != 0) error("Could not prepare statement.");


    stmtGetTeams = mysql_stmt_init(connection);
    if (stmtGetTeams == NULL) error("Could not create prepared statement.");
    result = mysql_stmt_prepare(stmtGetTeams, "SELECT team FROM Member WHERE user = ?", 38);
    if (result != 0) error("Could not prepare statement.");


    stmtGetAccess = mysql_stmt_init(connection);
    if (stmtGetAccess == NULL) error("Could not create prepared statement.");
    result = mysql_stmt_prepare(stmtGetAccess, "SELECT account FROM SshAccount WHERE server = ? AND entity = ? AND account = ?", 78);
    if (result != 0) error("Could not prepare statement.");
}

void DestroyStatements()
{
    mysql_stmt_close(stmtGetOrganizations);
    mysql_stmt_close(stmtGetTeams);
    mysql_stmt_close(stmtGetAccess);
}





struct IdList** GetOrganizations(struct IdList** organizations, long user)
{
    int result;
    long long organization;

    MYSQL_BIND params[1];
    MYSQL_BIND fields[1];
    MYSQL_RES *metaData;


    memset(params, 0, sizeof(MYSQL_BIND));
    memset(fields, 0, sizeof(MYSQL_BIND));


    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &user;

    fields[0].buffer_type = MYSQL_TYPE_LONGLONG;
    fields[0].buffer = &organization;


    metaData = mysql_stmt_result_metadata(stmtGetOrganizations); 
    if (metaData == NULL) error("Could not get statement metadata"); 
    result = mysql_stmt_bind_param(stmtGetOrganizations, params);
    if (result != 0) error("Could not bind params to statement.");
    result = mysql_stmt_bind_result(stmtGetOrganizations, fields);
    if (result != 0) error("Could not bind fields to statement.");
    result = mysql_stmt_execute(stmtGetOrganizations); 
    if (result != 0) error("Could not execute statement.");


    while(1) {
        result = mysql_stmt_fetch(stmtGetOrganizations); 
        if (result == MYSQL_NO_DATA) break; 
        if (result != 0) error("Could not parse statement result.");

        organizations[0] = malloc(sizeof(struct IdList));
        organizations[0]->value = organization;
        organizations[0]->next = NULL;
        organizations = &(organizations[0]->next);
    }


    mysql_free_result(metaData);
    return organizations;
}

struct IdList** GetTeams(struct IdList** teams, long long user)
{
    int result;
    long long team;


    MYSQL_BIND params[1];
    MYSQL_BIND fields[1];
    MYSQL_RES *metaData;


    memset(params, 0, sizeof(MYSQL_BIND));
    memset(fields, 0, sizeof(MYSQL_BIND));


    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &user;

    fields[0].buffer_type = MYSQL_TYPE_LONGLONG;
    fields[0].buffer = &team;


    metaData = mysql_stmt_result_metadata(stmtGetTeams); 
    if (metaData == NULL) error("Could not get statement metadata"); 
    result = mysql_stmt_bind_param(stmtGetTeams, params);
    if (result != 0) error("Could not bind params to statement.");
    result = mysql_stmt_bind_result(stmtGetTeams, fields);
    if (result != 0) error("Could not bind fields to statement.");
    result = mysql_stmt_execute(stmtGetTeams); 
    if (result != 0) error("Could not execute statement.");


    while(1) {
        result = mysql_stmt_fetch(stmtGetTeams); 
        if (result == MYSQL_NO_DATA) break; 
        if (result != 0) error("Could not parse statement result.");

        teams[0] = malloc(sizeof(struct IdList));
        teams[0]->value = team;
        teams[0]->next = NULL;
        teams = &(teams[0]->next);
    }


    mysql_free_result(metaData);
    return teams;
}

int GetAccess(long long server, long long entity, int account)
{
    int result;
    int temp;


    MYSQL_BIND params[3];
    MYSQL_BIND fields[1];
    MYSQL_RES *metaData;


    memset(params, 0, 3 * sizeof(MYSQL_BIND));
    memset(fields, 0, sizeof(MYSQL_BIND));


    params[0].buffer_type = MYSQL_TYPE_LONGLONG;
    params[0].buffer = &server;

    params[1].buffer_type = MYSQL_TYPE_LONGLONG;
    params[1].buffer = &entity;

    params[2].buffer_type = MYSQL_TYPE_LONG;
    params[2].buffer = &account;

    fields[0].buffer_type = MYSQL_TYPE_LONG;
    fields[0].buffer = &temp;


    metaData = mysql_stmt_result_metadata(stmtGetAccess); 
    if (metaData == NULL) error("Could not get statement metadata"); 
    result = mysql_stmt_bind_param(stmtGetAccess, params);
    if (result != 0) error("Could not bind params to statement.");
    result = mysql_stmt_bind_result(stmtGetAccess, fields);
    if (result != 0) error("Could not bind fields to statement.");
    result = mysql_stmt_execute(stmtGetAccess); 
    if (result != 0) error("Could not execute statement.");


    int permitted = 0;
    while (1) {
        result = mysql_stmt_fetch(stmtGetAccess); 
        if (result == MYSQL_NO_DATA) break; 
        if (result != 0) error("Could not parse statement result.");
        permitted = 1;
    }


    mysql_free_result(metaData);
    return permitted;
}





int main(int argc, char **argv)
{
    if (argc != 4) error("Incorrect amount of arguments.");


    long long server = atol(argv[1]);
    long long user = atol(argv[2]);
    int account = atoi(argv[3]);


    struct IdList* entities = malloc(sizeof(struct IdList));
    entities->value = user;
    entities->next = NULL;
    struct IdList** entityList = &(entities->next);


    Connect();
    PrepareStatements();


    entityList = GetOrganizations(entityList, user);
    entityList = GetTeams(entityList, user);


    struct IdList* entity = entities;
    while (entity != NULL) {
        if (GetAccess(server, entity->value, account) == 1) {
            printf("%lli\n", user);
            break;
        }
        entity = entity->next;
    }


    DestroyStatements();
    Disconnect();

    return 0;
}