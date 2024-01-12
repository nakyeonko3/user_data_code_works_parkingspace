#include <stdio.h>
#include <json-c/json.h>
#define MAP_DATA_JSON "./MapData.json"
// #define MAP_DATA_JSON "./test.json"

int main()
{
    FILE *jFile;
    char *jbuffer;

    json_object *forwards;
    long jFileSize;

    jFile = fopen(MAP_DATA_JSON, "r");
    if (jFile == NULL)
    {
        fputs("File error", stderr);
        exit(-1);
    }
    fseek(jFile, 0, SEEK_END);
    jFileSize = ftell(jFile);
    rewind(jFile);

    jbuffer = (char *)malloc(sizeof(char) * jFileSize);
    if (jbuffer == NULL)
    {
        fputs("Memory error", stderr);
        exit(2);
    }

    fread(jbuffer, jFileSize, 1, jFile);

    // printf("%ld\n", jFileSize);
    // printf("%s\n", jbuffer);
    json_object *parsed_obj = json_tokener_parse(jbuffer);
    json_object *medi_array = json_object_object_get(parsed_obj, "outline");

    // medi_array is an array of objects
    int arraylen = json_object_array_length(medi_array);

    for (int i = 0; i < arraylen; i++)
    {
        // get the i-th object in medi_array
        json_object *medi_array_obj = json_object_array_get_idx(medi_array, i);
        // printf("Name: %s\n", json_object_get_string(medi_array_obj));
        // get the name attribute in the i-th object
        json_object *medi_array_obj_name1 = json_object_object_get(medi_array_obj, "StartVertex");
        // print out the name attribute
        printf("StartVertex=%s\n", json_object_get_string(medi_array_obj_name1));

        // get the name attribute in the i-th object
        json_object *medi_array_obj_name2 = json_object_object_get(medi_array_obj, "EndVertex");
        // print out the name attribute
        printf("EndVertex=%s\n", json_object_get_string(medi_array_obj_name2));
    }

    // json_object *parsed_obj = json_tokener_parse(jbuffer);

    // json_object *tmp;
    // if (json_object_object_get_ex(parsed_obj, "outline", &tmp))
    // {
    //     // Key Name exists
    //     printf("Name: %s\n", json_object_get_string(tmp));
    //     // Name: xxxxx
    // }

    // json_object *parsed_obj = json_object_from_file(MAP_DATA_JSON);
    // if (!parsed_obj)
    // {
    //     fputs("File error", stderr);
    //     exit(-1);
    // }
    // // json_object *parsed_obj = json_object_from_file(buffer);

    // json_object *tmp;
    // json_object_object_get_ex(parsed_obj, "itemA", &tmp);

    // // int n_items;
    // // const char *json = "{ \"Name\": \"고낙연\", \"Id\": 101, \"Voting_eligible\": true }";
    // // json_object *root_obj = json_tokener_parse(json);

    fclose(jFile);
    return 0;
}