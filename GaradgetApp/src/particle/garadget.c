#include "garadget.h"
#include <glob.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <net_connection.h>
#include <json-glib/json-glib.h>
#include <string.h>

typedef struct MemoryStruct {
  char *memory;
  size_t size;
} MemoryStruct;

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

REMOTE_RESULT validateResponse(struct MemoryStruct data) {
	JsonParser *jsonParser  =  NULL;
	GError *error  =  NULL;
	jsonParser = json_parser_new ();

	json_parser_load_from_data(jsonParser, data.memory, data.size, &error);

	if (error) {
		g_error_free(error);
	}
	else {
		return false;
	}

    JsonNode *root;
    root = json_parser_get_root (jsonParser);

    if (root) {
        JsonReader *reader = json_reader_new(root);

        if (reader) {
        	json_reader_read_member(reader, "error");
	        const char *error_response = json_reader_get_string_value(reader);
            bool isAutorized = strcmp("invalid_token", error_response) != 0;

            json_reader_end_member(reader);

            g_object_unref(reader);                      
            g_object_unref(jsonParser);

            if (!isAutorized) {
                return REMOTE_RESULT_OK;
            }

            return REMOTE_RESULT_UNAUTHORIZED;            
        }
    }

    g_object_unref(jsonParser);

    return REMOTE_RESULT_INVALID_RESPONSE;
}

REMOTE_RESULT performGet(char *url, MemoryStruct *data) {
    CURL *curl;
	CURLcode curl_err;

    curl = curl_easy_init();

    connection_h connection;
    int conn_err;
    conn_err = connection_create(&connection);

	if (conn_err != CONNECTION_ERROR_NONE)
	{
		return REMOTE_RESULT_INTERNAL_ERROR;
	}

    curl_easy_setopt(curl, CURLOPT_URL, url);

	// Callback for data
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)data);

	/* getting data */
	curl_err = curl_easy_perform(curl);

	if (curl_err != CURLE_OK) {
    	curl_easy_cleanup(curl);
    	connection_destroy(connection);
		return REMOTE_RESULT_INTERNAL_ERROR;
	}

    connection_destroy(connection);

    return validateResponse(*data);
}

REMOTE_RESULT performPost(char *url, char *postData, MemoryStruct *data) {
    CURL *curl;
	CURLcode curl_err;

    curl = curl_easy_init();

    connection_h connection;
    int conn_err;
    conn_err = connection_create(&connection);

	if (conn_err != CONNECTION_ERROR_NONE)
	{
		return REMOTE_RESULT_INTERNAL_ERROR;
	}

    curl_easy_setopt(curl, CURLOPT_URL, url);

	// Callback for data
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)data);

    /* post data */
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postData));
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

	/* getting data */
	curl_err = curl_easy_perform(curl);

	if (curl_err != CURLE_OK) {
    	curl_easy_cleanup(curl);
    	connection_destroy(connection);
		return REMOTE_RESULT_INTERNAL_ERROR;
	}

    connection_destroy(connection);

    return validateResponse(*data);
}

REMOTE_RESULT getToken(char **token, char *user, char *password) {
	// Post data
	int postLength = strlen("username=&password=&grant_type=password&expires_in=2628000&client_id=_podKeys5db47ca19f1da96a97740db1d7baaf86&client_secret=_podKeys1a06c62aaa643af1fb75286280c70551") + strlen(user) + strlen(password) + 1;
	char *postData = (char *)calloc(postLength, sizeof(char));
	snprintf(postData, postLength, "grant_type=password&username=%s&password=%s&expires_in=2628000&client_id=_podKeys5db47ca19f1da96a97740db1d7baaf86&client_secret=_podKeys1a06c62aaa643af1fb75286280c70551", user, password);

    // Create structure at a reasonable size. It will grow if needed.
	struct MemoryStruct data;
    data.memory = malloc(1);
	data.size = 0;

    REMOTE_RESULT result = performPost("https://api.particle.io/oauth/token", postData, &data);

    cfree(postData);

    if (result != REMOTE_RESULT_OK) {
        return result;
    }

	// JSON Parser
	JsonParser *jsonParser = NULL;
	GError *error = NULL;
	jsonParser = json_parser_new();

	json_parser_load_from_data(jsonParser, data.memory, data.size, &error);

	if (error)
	{
	    g_error_free(error);
        g_object_unref(jsonParser);
        free(data.memory);

        return REMOTE_RESULT_INVALID_RESPONSE;
	}

    JsonNode *root;
    root = json_parser_get_root (jsonParser);

    bool status = false;

    if (root) {
        JsonReader *reader = json_reader_new(root);

        if (reader)
        {
    	    status = json_reader_read_member(reader, "access_token");

	        const char *tmpToken = json_reader_get_string_value(reader);
        	*token = (char *)calloc(strlen(tmpToken) + 1, sizeof(char));
            strcpy(*token, tmpToken);

            g_object_unref(reader);
        }
	}

    g_object_unref(jsonParser);
    free(data.memory);

    if (!status) {
        return REMOTE_RESULT_UNEXPECTED_RESPONSE;
    } else {
        return REMOTE_RESULT_OK;
    }
}

REMOTE_RESULT getDoors(char* token, int doorIndex, char **doorName) {
    if (token == NULL || doorName == NULL) {
        return REMOTE_RESULT_INVALID_PARAMETER;
    }

	int urlLength = strlen("https://api.particle.io/v1/devices?access_token=%s") + strlen(token) + 1;
	char *urlData = (char *)calloc(urlLength, sizeof(char));
	snprintf(urlData, urlLength, "https://api.particle.io/v1/devices?access_token=%s", token);

    // Create structure at a reasonable size. It will grow if needed.
	struct MemoryStruct data;
    data.memory = malloc(1);
	data.size = 0;

    REMOTE_RESULT result = performGet(urlData, &data);
    cfree(urlData);

    if (result != REMOTE_RESULT_OK) {
        return result;
    }

	// JSON Parser
	JsonParser *jsonParser = NULL;
	GError *error = NULL;
	jsonParser = json_parser_new();

	json_parser_load_from_data(jsonParser, data.memory, data.size, &error);

	if (error)
	{
	    g_error_free(error);
        g_object_unref(jsonParser);
        free(data.memory);

        return REMOTE_RESULT_INVALID_RESPONSE;
	}

    JsonNode *root;
    root = json_parser_get_root (jsonParser);

    bool status = false;

    if (root) {
        JsonReader *reader = json_reader_new(root);

        if (reader)
        {
            int numGarages = json_reader_count_elements(reader);

            if (numGarages - 1 < doorIndex) {
                g_object_unref(jsonParser);
                g_object_unref(reader);
                free(data.memory);

                return REMOTE_RESULT_INVALID_PARAMETER;
            }

            status = json_reader_read_element(reader, doorIndex);
    
            if (status) {
                status = json_reader_read_member(reader, "name");

                if (status) {
        	        const char *tmpDoorName = json_reader_get_string_value(reader);
                    *doorName = calloc(strlen(tmpDoorName) + 1, sizeof(char));
                    strcpy(*doorName, tmpDoorName);
                }

                json_reader_end_member(reader);
            }

            json_reader_end_element(reader);    
            g_object_unref(reader);
        }
    }

    g_object_unref(jsonParser);
    free(data.memory);

    if (!status) {
        return REMOTE_RESULT_UNEXPECTED_RESPONSE;
    } else {
        return REMOTE_RESULT_OK;
    }
};

REMOTE_RESULT getDoorStatus(char *token, char *doorName, bool *isOpen) {
	int urlLength = strlen("https://api.particle.io/v1/devices//doorStatus?access_token=") + strlen(doorName) + strlen(token) + 1;
	char *urlData = (char *)calloc(urlLength, sizeof(char));
	snprintf(urlData, urlLength, "https://api.particle.io/v1/devices/%s/doorStatus?access_token=%s", doorName, token);

    // Create structure at a reasonable size. It will grow if needed.
	struct MemoryStruct data;
    data.memory = malloc(1);
	data.size = 0;

    REMOTE_RESULT result = performGet(urlData, &data);
    cfree(urlData);

    if (result != REMOTE_RESULT_OK) {
        return result;
    }

	// JSON Parser
	JsonParser *jsonParser = NULL;
	GError *error = NULL;
	jsonParser = json_parser_new();

	json_parser_load_from_data(jsonParser, data.memory, data.size, &error);

	if (error)
	{
	    g_error_free(error);
        g_object_unref(jsonParser);
        free(data.memory);

        return REMOTE_RESULT_INVALID_RESPONSE;
	}

    JsonNode *root;
    root = json_parser_get_root (jsonParser);

    bool status = false;

    if (root) {
        JsonReader *reader = json_reader_new(root);

        if (reader)
        {
            status = json_reader_read_member(reader, "result");

            if (status) {
                const char* doorStatus = json_reader_get_string_value(reader);

                if (strstr(doorStatus, "closed") == NULL) {
                    *isOpen = true;
                } else {
                    *isOpen = false;
                }

                json_reader_end_member(reader);
            }

            g_object_unref(reader);
        }
    }

    g_object_unref(jsonParser);
    free(data.memory);

    if (!status) {
        return REMOTE_RESULT_UNEXPECTED_RESPONSE;
    } else {
        return REMOTE_RESULT_OK;
    }
}

REMOTE_RESULT openDoor(char *token, char *doorName, char *action) {

	int urlLength = strlen("https://api.particle.io/v1/devices/%s/setState") + strlen(doorName) + 1;
	char *urlData = (char *)calloc(urlLength, sizeof(char));
	snprintf(urlData, urlLength, "https://api.particle.io/v1/devices/%s/setState", doorName);

	// Post data
	int postLength = strlen("access_token=&arg=") + strlen(token) +strlen(action) + 1;
	char *postData = (char *)calloc(postLength, sizeof(char));
	snprintf(postData, postLength, "access_token=%s&arg=%s", token, action);

    // Create structure at a reasonable size. It will grow if needed.
	struct MemoryStruct data;
    data.memory = malloc(1);
	data.size = 0;

    REMOTE_RESULT result = performPost(urlData, postData, &data);
    cfree(urlData);
    cfree(postData);

    free(data.memory);

    return result;
}
