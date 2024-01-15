#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAP_DATA_JSON "./MapData.json"

// 데이터 헤더를 위한 구조체 정의
typedef struct
{
    int TotalDataSize;
    int OutlineDataCount;
    int MiddleLineDataCount;
    int ParkingSpaceDataCount;
    int PathDataCount;
    int PathDataSize;
    int OutlineDataSize;
    int MiddleLineDataSize;
    int ParkingSpaceDataSize;
} Header;

// 데이터 오프셋을 위한 구조체 정의
typedef struct
{
    int OutlineDataOffset;
    int MiddleLineDataOffset;
    int ParkingSpaceDataOffset;
    int PathDataOffset;
} Offset;

// 외곽선 데이터를 위한 구조체 정의
typedef struct
{
    int StartVertex[2];
    int EndVertex[2];
} Outline, MiddleLine;

// 주차 공간 데이터를 위한 구조체 정의
typedef struct
{
    char ParkingSpaceID[3];
    int BottomLeftVertex[2];
    int BottomRightVertex[2];
    int TopRightVertex[2];
    int TopLeftVertex[2];
    bool IsParkingAvailable;
    bool IsHandicappedParkingSpace;
} ParkingSpace;

// 경로 데이터를 위한 구조체 정의
typedef struct
{
    char NodeID[1];
    int StartVertex[2];
    int NodeVertex[2];
    int LinkedNodes[4];
    int LinkedIndex[4];
} PathData;

// JSON 파일을 읽고 처리하는 함수
int readAndProcessJSON(const char *jsonFileName)
{
    Header header;
    Offset offset;
    FILE *file;
    json_object *jsonData, *outlineArray, *middleLineArray, *parkingSpaceArray, *pathArray;
    json_object *outlineObj, *middleLineObj, *parkingSpaceObj, *pathDataObj;
    json_object *nodeID, *startVertex, *endVertex, *vertex, *isParkingAvailable, *isHandicappedParkingSpace, *nodeVertex, *linkedNodes, *linkedIndex, *parkingSpaceId;
    int i;

    // JSON 파일을 읽어서 json_object를 생성
    jsonData = json_object_from_file(jsonFileName);
    if (!jsonData)
    {
        printf("Failed to read JSON file.\n");
        return -1;
    }

    // JSON에서 데이터 배열을 가져와서 각각의 데이터 수를 저장
    if (json_object_object_get_ex(jsonData, "outline", &outlineArray) &&
        json_object_object_get_ex(jsonData, "middleLine", &middleLineArray) &&
        json_object_object_get_ex(jsonData, "parkingSpace", &parkingSpaceArray) &&
        json_object_object_get_ex(jsonData, "PathData", &pathArray))
    {

        header.OutlineDataCount = json_object_array_length(outlineArray);
        header.MiddleLineDataCount = json_object_array_length(middleLineArray);
        header.ParkingSpaceDataCount = json_object_array_length(parkingSpaceArray);
        header.PathDataCount = json_object_array_length(pathArray);
    }
    else
    {
        printf("Failed to get data arrays from JSON.\n");
        json_object_put(jsonData);
        return -1;
    }

    // 바이너리 파일을 생성하고 헤더와 오프셋 정보를 기록
    file = fopen("MapData.bin", "wb");
    if (!file)
    {
        perror("Unable to create file");
        json_object_put(jsonData);
        return -1;
    }

    offset.OutlineDataOffset = sizeof(Header) + sizeof(Offset);
    offset.MiddleLineDataOffset = offset.OutlineDataOffset + header.OutlineDataCount * sizeof(Outline);
    offset.ParkingSpaceDataOffset = offset.MiddleLineDataOffset + header.MiddleLineDataCount * sizeof(MiddleLine);
    offset.PathDataOffset = offset.ParkingSpaceDataOffset + header.ParkingSpaceDataCount * sizeof(ParkingSpace);

    header.OutlineDataSize = header.OutlineDataCount * sizeof(Outline);
    header.MiddleLineDataSize = header.MiddleLineDataCount * sizeof(MiddleLine);
    header.ParkingSpaceDataSize = header.ParkingSpaceDataCount * sizeof(ParkingSpace);
    header.PathDataSize = header.PathDataCount * sizeof(PathData);

    header.TotalDataSize = sizeof(Header) + sizeof(Offset) + header.OutlineDataSize +
                           header.MiddleLineDataSize + header.ParkingSpaceDataSize + header.PathDataSize;

    fwrite(&header, sizeof(Header), 1, file);
    fwrite(&offset, sizeof(Offset), 1, file);

    // 외곽선 데이터를 읽어서 이진 파일에 기록
    for (i = 0; i < header.OutlineDataCount; i++)
    {
        outlineObj = json_object_array_get_idx(outlineArray, i);
        Outline outline;

        // JSON에서 필요한 데이터를 추출
        if (json_object_object_get_ex(outlineObj, "StartVertex", &startVertex))
        {
            outline.StartVertex[0] = json_object_get_int(json_object_array_get_idx(startVertex, 0));
            outline.StartVertex[1] = json_object_get_int(json_object_array_get_idx(startVertex, 1));
        }
        if (json_object_object_get_ex(outlineObj, "EndVertex", &endVertex))
        {
            outline.EndVertex[0] = json_object_get_int(json_object_array_get_idx(endVertex, 0));
            outline.EndVertex[1] = json_object_get_int(json_object_array_get_idx(endVertex, 1));
        }
        fwrite(&outline, sizeof(Outline), 1, file);
    }

    // 중간 선 데이터를 읽어서 이진 파일에 기록
    for (i = 0; i < header.MiddleLineDataCount; i++)
    {
        middleLineObj = json_object_array_get_idx(middleLineArray, i);
        MiddleLine middleLine;

        if (json_object_object_get_ex(middleLineObj, "StartVertex", &startVertex))
        {
            middleLine.StartVertex[0] = json_object_get_int(json_object_array_get_idx(startVertex, 0));
            middleLine.StartVertex[1] = json_object_get_int(json_object_array_get_idx(startVertex, 1));
        }
        if (json_object_object_get_ex(middleLineObj, "EndVertex", &endVertex))
        {
            middleLine.EndVertex[0] = json_object_get_int(json_object_array_get_idx(endVertex, 0));
            middleLine.EndVertex[1] = json_object_get_int(json_object_array_get_idx(endVertex, 1));
        }
        fwrite(&middleLine, sizeof(MiddleLine), 1, file);
    }

    // 주차 공간 데이터를 읽어서 이진 파일에 기록
    for (i = 0; i < header.ParkingSpaceDataCount; i++)
    {
        parkingSpaceObj = json_object_array_get_idx(parkingSpaceArray, i);
        ParkingSpace parkingSpace;

        if (json_object_object_get_ex(parkingSpaceObj, "ParkingSpaceID", &nodeID))
        {
            strncpy(parkingSpace.ParkingSpaceID, json_object_get_string(nodeID), 3);
        }
        if (json_object_object_get_ex(parkingSpaceObj, "BottomLeftVertex", &vertex))
        {
            parkingSpace.BottomLeftVertex[0] = json_object_get_int(json_object_array_get_idx(vertex, 0));
            parkingSpace.BottomLeftVertex[1] = json_object_get_int(json_object_array_get_idx(vertex, 1));
        }
        if (json_object_object_get_ex(parkingSpaceObj, "BottomRightVertex", &vertex))
        {
            parkingSpace.BottomRightVertex[0] = json_object_get_int(json_object_array_get_idx(vertex, 0));
            parkingSpace.BottomRightVertex[1] = json_object_get_int(json_object_array_get_idx(vertex, 1));
        }
        if (json_object_object_get_ex(parkingSpaceObj, "TopRightVertex", &vertex))
        {
            parkingSpace.TopRightVertex[0] = json_object_get_int(json_object_array_get_idx(vertex, 0));
            parkingSpace.TopRightVertex[1] = json_object_get_int(json_object_array_get_idx(vertex, 1));
        }
        if (json_object_object_get_ex(parkingSpaceObj, "TopLeftVertex", &vertex))
        {
            parkingSpace.TopLeftVertex[0] = json_object_get_int(json_object_array_get_idx(vertex, 0));
            parkingSpace.TopLeftVertex[1] = json_object_get_int(json_object_array_get_idx(vertex, 1));
        }

        // 주차 공간 ID에 따른 처리
        int parkingSpaceID = atoi(parkingSpace.ParkingSpaceID);
        // 21번과 22번 주차 공간에 대한 조건
        if (parkingSpaceID == 21 || parkingSpaceID == 22)
        {
            // 21번과 22번 주차 공간 처리
            if (json_object_object_get_ex(parkingSpaceObj, "IsHandicappedParkingSpace", &isHandicappedParkingSpace))
            {
                parkingSpace.IsHandicappedParkingSpace = json_object_get_boolean(isHandicappedParkingSpace);
            }
            parkingSpace.IsParkingAvailable = true;
        }
        else
        {
            // 나머지 주차 공간 처리
            if (json_object_object_get_ex(parkingSpaceObj, "IsParkingAvailable", &isParkingAvailable))
            {
                parkingSpace.IsParkingAvailable = json_object_get_boolean(isParkingAvailable);
            }
            parkingSpace.IsHandicappedParkingSpace = false;
        }
        fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);
    }

    // 경로 데이터를 읽어서 이진 파일에 기록
    for (int i = 0; i < header.PathDataCount; i++)
    {
        pathDataObj = json_object_array_get_idx(pathArray, i);
        PathData pathData;

        if (json_object_object_get_ex(pathDataObj, "NodeID", &nodeID))
        {
            strncpy(pathData.NodeID, json_object_get_string(nodeID), 4);
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
        fwrite(&pathData, sizeof(PathData), 1, file);
    }

    // 파일 닫고 메모리 해제
    fclose(file);
    json_object_put(jsonData);

    // return 0;
}

int main()
{
    const char *jsonFileName = MAP_DATA_JSON;
    return readAndProcessJSON(jsonFileName);
}