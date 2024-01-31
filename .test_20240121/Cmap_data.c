#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAP_DATA_JSON "./MapData.json"
// #define MAP_DATA_JSON "/home/ctj/projects/Project/MapData.json"

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

int readAndProcessJSON(const char *jsonFileName)
{
    Header header;
    Offset offset;
    FILE *file;
    json_object *jsonData, *drawingArray, *parkingSpaceArray, *pathArray;
    json_object *drawingObj, *parkingSpaceObj, *pathDataObj;
    json_object *startVertex, *endVertex, *bottomLeftVertex, *bottomRightVertex, *topRightVertex, *topLeftVertex;
    json_object *isParkingAvailable, *isHandicappedParkingSpace, *nodeID, *nodeVertex, *linkedNodes, *linkedIndex;
    int i;

    jsonData = json_object_from_file(jsonFileName);
    if (!jsonData)
    {
        printf("Failed to read JSON file.\n");
        return -1;
    }

    if (!json_object_object_get_ex(jsonData, "drawingData", &drawingArray) ||
        !json_object_object_get_ex(jsonData, "parkingSpace", &parkingSpaceArray) ||
        !json_object_object_get_ex(jsonData, "PathData", &pathArray))
    {
        printf("Failed to get data arrays from JSON.\n");
        json_object_put(jsonData);
        return -1;
    }

    header.DrawingDataCount = json_object_array_length(drawingArray);
    header.ParkingSpaceDataCount = json_object_array_length(parkingSpaceArray);
    header.PathDataCount = json_object_array_length(pathArray);

    file = fopen("MapData.bin", "wb");
    if (!file)
    {
        perror("Unable to create file");
        json_object_put(jsonData);
        return -1;
    }

    offset.DrawingDataOffset = sizeof(Header) + sizeof(Offset);
    offset.ParkingSpaceDataOffset = offset.DrawingDataOffset + header.DrawingDataCount * sizeof(DrawingData);
    offset.PathDataOffset = offset.ParkingSpaceDataOffset + header.ParkingSpaceDataCount * sizeof(ParkingSpace);

    header.DrawingDataSize = header.DrawingDataCount * sizeof(DrawingData);
    header.ParkingSpaceDataSize = header.ParkingSpaceDataCount * sizeof(ParkingSpace);
    header.PathDataSize = header.PathDataCount * sizeof(PathData);

    header.TotalDataSize = sizeof(Header) + sizeof(Offset) + header.DrawingDataSize +
                           header.ParkingSpaceDataSize + header.PathDataSize;

    fwrite(&header, sizeof(Header), 1, file);
    fwrite(&offset, sizeof(Offset), 1, file);

    for (i = 0; i < header.DrawingDataCount; i++)
    {
        drawingObj = json_object_array_get_idx(drawingArray, i);
        DrawingData drawingData;
        memset(&drawingData, 0, sizeof(DrawingData));

        if (json_object_object_get_ex(drawingObj, "StartVertex", &startVertex))
        {
            drawingData.StartVertex[0] = json_object_get_int(json_object_array_get_idx(startVertex, 0));
            drawingData.StartVertex[1] = json_object_get_int(json_object_array_get_idx(startVertex, 1));
            if (json_object_object_get_ex(drawingObj, "EndVertex", &endVertex))
            {
                drawingData.EndVertex[0] = json_object_get_int(json_object_array_get_idx(endVertex, 0));
                drawingData.EndVertex[1] = json_object_get_int(json_object_array_get_idx(endVertex, 1));
            }
        }
        else if (json_object_object_get_ex(drawingObj, "BottomLeftVertex", &bottomLeftVertex))
        {
            drawingData.BottomLeftVertex[0] = json_object_get_int(json_object_array_get_idx(bottomLeftVertex, 0));
            drawingData.BottomLeftVertex[1] = json_object_get_int(json_object_array_get_idx(bottomLeftVertex, 1));
            if (json_object_object_get_ex(drawingObj, "BottomRightVertex", &bottomRightVertex))
            {
                drawingData.BottomRightVertex[0] = json_object_get_int(json_object_array_get_idx(bottomRightVertex, 0));
                drawingData.BottomRightVertex[1] = json_object_get_int(json_object_array_get_idx(bottomRightVertex, 1));
            }
            if (json_object_object_get_ex(drawingObj, "TopRightVertex", &topRightVertex))
            {
                drawingData.TopRightVertex[0] = json_object_get_int(json_object_array_get_idx(topRightVertex, 0));
                drawingData.TopRightVertex[1] = json_object_get_int(json_object_array_get_idx(topRightVertex, 1));
            }
            if (json_object_object_get_ex(drawingObj, "TopLeftVertex", &topLeftVertex))
            {
                drawingData.TopLeftVertex[0] = json_object_get_int(json_object_array_get_idx(topLeftVertex, 0));
                drawingData.TopLeftVertex[1] = json_object_get_int(json_object_array_get_idx(topLeftVertex, 1));
            }
        }

        fwrite(&drawingData, sizeof(DrawingData), 1, file);
    }

    for (i = 0; i < header.ParkingSpaceDataCount; i++)
    {
        parkingSpaceObj = json_object_array_get_idx(parkingSpaceArray, i);
        ParkingSpace parkingSpace;
        memset(&parkingSpace, 0, sizeof(ParkingSpace));

        if (json_object_object_get_ex(parkingSpaceObj, "ParkingSpaceID", &nodeID))
        {
            strncpy(parkingSpace.ParkingSpaceID, json_object_get_string(nodeID), 3);
        }
        if (json_object_object_get_ex(parkingSpaceObj, "IsParkingAvailable", &isParkingAvailable))
        {
            parkingSpace.IsParkingAvailable = json_object_get_boolean(isParkingAvailable);
        }
        if (json_object_object_get_ex(parkingSpaceObj, "IsHandicappedParkingSpace", &isHandicappedParkingSpace))
        {
            parkingSpace.IsHandicappedParkingSpace = json_object_get_boolean(isHandicappedParkingSpace);
        }

        fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);
    }

    for (i = 0; i < header.PathDataCount; i++)
    {
        pathDataObj = json_object_array_get_idx(pathArray, i);
        PathData pathData;
        memset(&pathData, 0, sizeof(PathData));

        if (json_object_object_get_ex(pathDataObj, "NodeID", &nodeID))
        {
            strncpy(pathData.NodeID, json_object_get_string(nodeID), 2);
        }
        if (json_object_object_get_ex(pathDataObj, "StartVertex", &startVertex))
        {
            pathData.StartVertex[0] = json_object_get_int(json_object_array_get_idx(startVertex, 0));
            pathData.StartVertex[1] = json_object_get_int(json_object_array_get_idx(startVertex, 1));
        }
        if (json_object_object_get_ex(pathDataObj, "NodeVertex", &nodeVertex))
        {
            pathData.NodeVertex[0] = json_object_get_int(json_object_array_get_idx(nodeVertex, 0));
            pathData.NodeVertex[1] = json_object_get_int(json_object_array_get_idx(nodeVertex, 1));
        }
        if (json_object_object_get_ex(pathDataObj, "LinkedNodes", &linkedNodes))
        {
            for (int j = 0; j < 4; j++)
            {
                if (json_object_array_get_idx(linkedNodes, j))
                {
                    pathData.LinkedNodes[j] = json_object_get_int(json_object_array_get_idx(linkedNodes, j));
                }
                else
                {
                    pathData.LinkedNodes[j] = -1;
                }
            }
        }
        if (json_object_object_get_ex(pathDataObj, "LinkedIndex", &linkedIndex))
        {
            for (int j = 0; j < 4; j++)
            {
                if (json_object_array_get_idx(linkedIndex, j))
                {
                    pathData.LinkedIndex[j] = json_object_get_int(json_object_array_get_idx(linkedIndex, j));
                }
                else
                {
                    pathData.LinkedIndex[j] = -1;
                }
            }
        }
        printf("-------------------------------------------------");
        printf("pathData.NodeID:%s\n", pathData.NodeID);
        printf("pathData.LinkedIndex: [%d, %d, %d, %d] \n", pathData.LinkedIndex[0], pathData.LinkedIndex[0], pathData.LinkedIndex[0], pathData.LinkedIndex[0]);
        printf("-------------------------------------------------");
        fwrite(&pathData, sizeof(PathData), 1, file);
    }

    fclose(file);
    json_object_put(jsonData);

    return 0;
}

int main()
{
    const char *jsonFileName = MAP_DATA_JSON;
    return readAndProcessJSON(jsonFileName);
}
