#include <stdbool.h>

typedef enum REMOTE_RESULT {
    REMOTE_RESULT_OK,
    REMOTE_RESULT_UNAUTHORIZED,
    REMOTE_RESULT_INVALID_RESPONSE,
    REMOTE_RESULT_INTERNAL_ERROR,
    REMOTE_RESULT_UNEXPECTED_RESPONSE,
    REMOTE_RESULT_INVALID_PARAMETER,
	REMOTE_RESULT_INCORRECT_CREDS,
} REMOTE_RESULT;

REMOTE_RESULT getToken(char **token, char *user, char *password);
REMOTE_RESULT getDoors(char* token, int doorIndex, char **doorName);
REMOTE_RESULT openDoor(char *token, char *doorName, char *action);
REMOTE_RESULT getDoorStatus(char *token, char *doorName, bool *isOpen);
