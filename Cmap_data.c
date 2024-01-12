#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAP_DATA_JSON_PATH "./MapData.json"

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

typedef struct
{
    int OutlineDataOffset;
    int MiddleLineDataOffset;
    int ParkingSpaceDataOffset;
    int PathDataOffset;
} Offset;

typedef struct
{
    int StartVertex[2];
    int EndVertex[2];
} Outline, MiddleLine;

typedef struct
{
    char ParkingSpaceID[4];
    int BottomLeftVertex[2];
    int BottomRightVertex[2];
    int TopRightVertex[2];
    int TopLeftVertex[2];
    bool IsParkingAvailable;
    bool IsHandicappedParkingSpace;
} ParkingSpace;

typedef struct
{
    char NodeID[4];
    int StartVertex[2];
    int NodeVertex[2];
    int LinkedNodes[4];
    int LinkedIndex[4];
} PathData;

int readAndProcessJSON(const char *jsonFileName)
{
    Header header = {0};
    Offset offset = {0};
    FILE *file;
    int i;

    FILE *jsonFile = fopen(jsonFileName, "r");
    if (!jsonFile)
    {
        printf("Failed to open JSON file.\n");
        return -1;
    }

    char line[1024];
    int currentSection = -1;
    while (fgets(line, sizeof(line), jsonFile))
    {
        if (strstr(line, "outline"))
            currentSection = 0;
        else if (strstr(line, "middleLine"))
            currentSection = 1;
        else if (strstr(line, "parkingSpace"))
            currentSection = 2;
        else if (strstr(line, "PathData"))
            currentSection = 3;

        if (currentSection == -1)
            continue;

        if (strstr(line, "{"))
        {
            if (currentSection == 0)
                header.OutlineDataCount++;
            else if (currentSection == 1)
                header.MiddleLineDataCount++;
            else if (currentSection == 2)
                header.ParkingSpaceDataCount++;
            else if (currentSection == 3)
                header.PathDataCount++;
        }
    }

    file = fopen("MapData.bin", "wb");
    if (!file)
    {
        perror("Unable to create file");
        fclose(jsonFile);
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

    fseek(jsonFile, 0, SEEK_SET);
    currentSection = -1;
    Outline outline;
    MiddleLine middleLine;
    ParkingSpace parkingSpace;
    PathData pathData;
    char buffer[1024];
    while (fgets(line, sizeof(line), jsonFile))
    {
        if (strstr(line, "outline"))
            currentSection = 0;
        else if (strstr(line, "middleLine"))
            currentSection = 1;
        else if (strstr(line, "parkingSpace"))
            currentSection = 2;
        else if (strstr(line, "PathData"))
            currentSection = 3;

        if (currentSection == -1)
            continue;

        if (strstr(line, "{"))
        {
            memset(buffer, 0, sizeof(buffer));
            if (currentSection == 0)
                memset(&outline, 0, sizeof(Outline));
            else if (currentSection == 1)
                memset(&middleLine, 0, sizeof(MiddleLine));
            else if (currentSection == 2)
                memset(&parkingSpace, 0, sizeof(ParkingSpace));
            else if (currentSection == 3)
                memset(&pathData, 0, sizeof(PathData));
        }

        if (currentSection == 0)
        {
            if (strstr(line, "StartVertex"))
            {
                sscanf(line, " \"StartVertex\" : [%d, %d],", &outline.StartVertex[0], &outline.StartVertex[1]);
            }
            else if (strstr(line, "EndVertex"))
            {
                sscanf(line, " \"EndVertex\" : [%d, %d]", &outline.EndVertex[0], &outline.EndVertex[1]);
                fwrite(&outline, sizeof(Outline), 1, file);
            }
        }
        else if (currentSection == 1)
        {
            if (strstr(line, "StartVertex"))
            {
                sscanf(line, " \"StartVertex\" : [%d, %d],", &middleLine.StartVertex[0], &middleLine.StartVertex[1]);
            }
            else if (strstr(line, "EndVertex"))
            {
                sscanf(line, " \"EndVertex\" : [%d, %d]", &middleLine.EndVertex[0], &middleLine.EndVertex[1]);
                fwrite(&middleLine, sizeof(MiddleLine), 1, file);
            }
        }
        else if (currentSection == 2)
        {
            if (strstr(line, "ParkingSpaceID"))
            {
                sscanf(line, " \"ParkingSpaceID\" : \"%5[^\"]\",", parkingSpace.ParkingSpaceID);
            }
            else if (strstr(line, "BottomLeftVertex"))
            {
                sscanf(line, " \"BottomLeftVertex\" : [%d, %d],", &parkingSpace.BottomLeftVertex[0], &parkingSpace.BottomLeftVertex[1]);
            }
            else if (strstr(line, "BottomRightVertex"))
            {
                sscanf(line, " \"BottomRightVertex\" : [%d, %d],", &parkingSpace.BottomRightVertex[0], &parkingSpace.BottomRightVertex[1]);
            }
            else if (strstr(line, "TopRightVertex"))
            {
                sscanf(line, " \"TopRightVertex\" : [%d, %d],", &parkingSpace.TopRightVertex[0], &parkingSpace.TopRightVertex[1]);
            }
            else if (strstr(line, "TopLeftVertex"))
            {
                sscanf(line, " \"TopLeftVertex\" : [%d, %d],", &parkingSpace.TopLeftVertex[0], &parkingSpace.TopLeftVertex[1]);
            }
            else if (strstr(line, "IsParkingAvailable"))
            {
                sscanf(line, " \"IsParkingAvailable\" : \"%5[^\"]\",", buffer);
                parkingSpace.IsParkingAvailable = (strcmp(buffer, "true") == 0);
                fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);
            }
            else if (strstr(line, "IsHandicappedParkingSpace"))
            {
                sscanf(line, " \"IsHandicappedParkingSpace\" : \"%5[^\"]\",", buffer);
                parkingSpace.IsHandicappedParkingSpace = (strcmp(buffer, "true") == 0);
            }
        }

        else if (currentSection == 3)
        {
            if (strstr(line, "NodeID"))
            {
                sscanf(line, " \"NodeID\" : \"%4[^\"]\",", pathData.NodeID);
            }
            else if (strstr(line, "NodeVertex"))
            {
                sscanf(line, " \"NodeVertex\" : [%d, %d],", &pathData.NodeVertex[0], &pathData.NodeVertex[1]);
            }
            else if (strstr(line, "LinkedNodes"))
            {
                // LinkedNodes 값을 정수 배열로 읽어들임
                sscanf(line, " \"LinkedNodes\" : [ \"%d\", \"%d\", \"%d\", \"%d\" ],",
                       &pathData.LinkedNodes[0], &pathData.LinkedNodes[1],
                       &pathData.LinkedNodes[2], &pathData.LinkedNodes[3]);
            }
            else if (strstr(line, "LinkedIndex"))
            {
                sscanf(line, " \"LinkedIndex\" : [%d, %d, %d, %d],", &pathData.LinkedIndex[0], &pathData.LinkedIndex[1], &pathData.LinkedIndex[2], &pathData.LinkedIndex[3]);
                fwrite(&pathData, sizeof(PathData), 1, file);
            }
        }
    }
    fclose(file);
    fclose(jsonFile);

    return 0;
}

int main()
{
    const char *jsonFileName = MAP_DATA_JSON_PATH;

    if (readAndProcessJSON(jsonFileName) == 0)
    {
        printf("JSON 파일을 성공적으로 처리하여 바이너리 파일로 변환했습니다.\n");
    }

    return 0;
}