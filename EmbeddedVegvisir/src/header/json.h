#ifndef JSON_H
#define JSON_H

#include <string.h>
#include <stdlib.h>

typedef char*                                   string;
#define TRUE                                    true
#define FALSE                                   false


#define new(x)                                  (x *) malloc(sizeof(x))
#define newWithSize(x, y)                       (x *) malloc(y * sizeof(x))
#define renewWithSize(x, y, z)                  (y *) realloc(x, z * sizeof(y))
#define isWhitespace(x)                         x == '\r' || x == '\n' || x == '\t' || x == ' '
#define removeWhitespace(x)                     while(isWhitespace(*x)) x++
#define removeWhitespaceCalcOffset(x, y)        while(isWhitespace(*x)) { x++; y++; }

typedef char                                    character;

struct _jsonobject;
struct _jsonpair;
union _jsonvalue;

typedef enum {
    JSON_STRING = 0,
    JSON_OBJECT
} JSONValueType;

typedef struct _jsonobject {
    struct _jsonpair *pairs;
    int count;
} JSONObject;

typedef struct _jsonpair {
    string key;
    union _jsonvalue *value;
    JSONValueType type;
} JSONPair;

typedef union _jsonvalue {
    string stringValue;
    struct _jsonobject *jsonObject;
} JSONValue;

JSONObject *parseJSON(string);
void freeJSONFromMemory(JSONObject *);
int strNextOccurence(string, char);
JSONObject * _parseJSON(string, int *);



#endif