// Cmap_data.c

#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAP_DATA_BIN "./MapData.bin"

typedef struct
{
    int TotalDataSize;
    int DrawingDataCount;
    int ParkingSpaceDataCount;
    int PathDataCount;
    int DrawingDataSize;
    int ParkingSpaceDataSize;
    int PathDataSize;
} Header;

typedef struct
{
    int DrawingDataOffset;
    int ParkingSpaceDataOffset;
    int PathDataOffset;
} Offset;

typedef struct
{
    int StartVertex[2];
    int EndVertex[2];
    int BottomLeftVertex[2];
    int BottomRightVertex[2];
    int TopRightVertex[2];
    int TopLeftVertex[2];
} DrawingData;

typedef struct
{
    char ParkingSpaceID[3];
    bool IsParkingAvailable;
    bool IsHandicappedParkingSpace;
} ParkingSpace;

typedef struct
{
    char NodeID[3];
    int StartVertex[2];
    int NodeVertex[2];
    int LinkedNodes[4];
    int LinkedIndex[4];
} PathData;

json_object *create_json_vertex_array(int *array, int size)
{
    json_object *jarray = json_object_new_array();
    for (int i = 0; i < size; ++i)
    {
        json_object_array_add(jarray, json_object_new_int(array[i]));
    }
    return jarray;
}

json_object *convertToJSON(FILE *file, Header header, Offset offset)
{
    json_object *jRoot = json_object_new_object();
    json_object *jDrawingData = json_object_new_array();
    json_object *jParkingSpaces = json_object_new_array();
    json_object *jPathData = json_object_new_array();

    fseek(file, offset.DrawingDataOffset, SEEK_SET);

    for (int i = 0; i < header.DrawingDataCount; i++)
    {
        DrawingData drawingData;
        fread(&drawingData, sizeof(DrawingData), 1, file);
        json_object *jDrawing = json_object_new_object();

        if (drawingData.StartVertex[0] != 0 || drawingData.StartVertex[1] != 0)
        {
            json_object_object_add(jDrawing, "StartVertex", create_json_vertex_array(drawingData.StartVertex, 2));
            json_object_object_add(jDrawing, "EndVertex", create_json_vertex_array(drawingData.EndVertex, 2));
        }
        else
        {
            json_object_object_add(jDrawing, "BottomLeftVertex", create_json_vertex_array(drawingData.BottomLeftVertex, 2));
            json_object_object_add(jDrawing, "BottomRightVertex", create_json_vertex_array(drawingData.BottomRightVertex, 2));
            json_object_object_add(jDrawing, "TopRightVertex", create_json_vertex_array(drawingData.TopRightVertex, 2));
            json_object_object_add(jDrawing, "TopLeftVertex", create_json_vertex_array(drawingData.TopLeftVertex, 2));
        }

        json_object_array_add(jDrawingData, jDrawing);
    }

    fseek(file, offset.ParkingSpaceDataOffset, SEEK_SET);
    for (int i = 0; i < header.ParkingSpaceDataCount; i++)
    {
        ParkingSpace parkingSpace;
        fread(&parkingSpace, sizeof(ParkingSpace), 1, file);
        json_object *jParking = json_object_new_object();
        json_object_object_add(jParking, "ParkingSpaceID", json_object_new_string(parkingSpace.ParkingSpaceID));
        json_object_object_add(jParking, "IsParkingAvailable", json_object_new_boolean(parkingSpace.IsParkingAvailable));
        json_object_object_add(jParking, "IsHandicappedParkingSpace", json_object_new_boolean(parkingSpace.IsHandicappedParkingSpace));
        json_object_array_add(jParkingSpaces, jParking);
    }

    fseek(file, offset.PathDataOffset, SEEK_SET);
    for (int i = 0; i < header.PathDataCount; i++)
    {
        PathData pathData;
        fread(&pathData, sizeof(PathData), 1, file);
        json_object *jPath = json_object_new_object();

        // NodeID를 문자열로 처리
        json_object_object_add(jPath, "NodeID", json_object_new_string(pathData.NodeID));
        json_object_object_add(jPath, "NodeVertex", create_json_vertex_array(pathData.NodeVertex, 2));

        json_object *jLinkedNodes = json_object_new_array();
        for (int j = 0; j < 4; ++j)
        {
            if (pathData.LinkedNodes[j] != -1)
            {
                json_object_array_add(jLinkedNodes, json_object_new_int(pathData.LinkedNodes[j]));
            }
        }
        json_object_object_add(jPath, "LinkedNodes", jLinkedNodes);

        json_object *jLinkedIndex = json_object_new_array();
        for (int j = 0; j < 4; ++j)
        {
            if (pathData.LinkedIndex[j] != -1)
            {
                json_object_array_add(jLinkedIndex, json_object_new_int(pathData.LinkedIndex[j]));
            }
        }
        json_object_object_add(jPath, "LinkedIndex", jLinkedIndex);

        json_object_array_add(jPathData, jPath);
    }

    json_object_object_add(jRoot, "drawingData", jDrawingData);
    json_object_object_add(jRoot, "parkingSpace", jParkingSpaces);
    json_object_object_add(jRoot, "PathData", jPathData);

    return jRoot;
}

json_object *readBinaryFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Unable to open file");
        return NULL;
    }

    Header header;
    Offset offset;
    fread(&header, sizeof(Header), 1, file);
    fread(&offset, sizeof(Offset), 1, file);

    json_object *jData = convertToJSON(file, header, offset);

    fclose(file);
    return jData;
}

int main()
{
    const char *binaryFileName = MAP_DATA_BIN;
    json_object *jData = readBinaryFile(binaryFileName);

    if (jData)
    {
        printf("%s\n", json_object_to_json_string_ext(jData, JSON_C_TO_STRING_PRETTY));
        json_object_put(jData);
    }

    return 0;
}
