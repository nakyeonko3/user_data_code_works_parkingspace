#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAP_DATA_BIN_PATH "./MapData.bin"
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
    char ParkingSpaceID[4]; // 수정된 크기
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
    char NodeID[4]; // 수정된 크기
    int StartVertex[2];
    int NodeVertex[2];
    int LinkedNodes[4];
    int LinkedIndex[4];
} PathData;

// 바이너리 파일을 읽어서 데이터를 화면에 출력하는 함수
int readBinaryData(const char *binFileName)
{
    FILE *file = fopen(binFileName, "rb");
    if (!file)
    {
        perror("Unable to open binary file");
        return -1;
    }

    Header header;
    Offset offset;

    // 헤더와 오프셋 정보 읽기
    fread(&header, sizeof(Header), 1, file);
    fread(&offset, sizeof(Offset), 1, file);

    printf("Total Data Size: %d\n", header.TotalDataSize);

    // Reading and printing Outline data
    fseek(file, offset.OutlineDataOffset, SEEK_SET);
    printf("Outline Data:\n");
    for (int i = 0; i < header.OutlineDataCount; i++)
    {
        Outline outline;
        fread(&outline, sizeof(Outline), 1, file);
        printf("StartVertex: [%d, %d], EndVertex: [%d, %d]\n",
               outline.StartVertex[0], outline.StartVertex[1],
               outline.EndVertex[0], outline.EndVertex[1]);
    }

    // Reading and printing MiddleLine data
    fseek(file, offset.MiddleLineDataOffset, SEEK_SET);
    printf("MiddleLine Data:\n");
    for (int i = 0; i < header.MiddleLineDataCount; i++)
    {
        MiddleLine middleLine;
        fread(&middleLine, sizeof(MiddleLine), 1, file);
        printf("StartVertex: [%d, %d], EndVertex: [%d, %d]\n",
               middleLine.StartVertex[0], middleLine.StartVertex[1],
               middleLine.EndVertex[0], middleLine.EndVertex[1]);
    }

    fseek(file, offset.ParkingSpaceDataOffset, SEEK_SET);
    printf("ParkingSpace Data:\n");
    for (int i = 0; i < header.ParkingSpaceDataCount; i++)
    {
        ParkingSpace parkingSpace;
        fread(&parkingSpace, sizeof(ParkingSpace), 1, file);

        // Convert ParkingSpaceID to an integer for comparison
        int parkingSpaceID = atoi(parkingSpace.ParkingSpaceID);

        // Set IsParkingAvailable to true for all parking spaces
        parkingSpace.IsParkingAvailable = true;

        // Set IsHandicappedParkingSpace based on parking space ID
        if (parkingSpaceID >= 1 && parkingSpaceID <= 20)
        {
            parkingSpace.IsHandicappedParkingSpace = false;
        }
        else if (parkingSpaceID == 21 || parkingSpaceID == 22)
        {
            parkingSpace.IsHandicappedParkingSpace = true;
        }

        // Print parking space data
        printf("ParkingSpaceID: %s, BottomLeftVertex: [%d, %d], BottomRightVertex: [%d, %d], TopRightVertex: [%d, %d], TopLeftVertex: [%d, %d], IsParkingAvailable: %s, IsHandicappedParkingSpace: %s\n",
               parkingSpace.ParkingSpaceID,
               parkingSpace.BottomLeftVertex[0], parkingSpace.BottomLeftVertex[1],
               parkingSpace.BottomRightVertex[0], parkingSpace.BottomRightVertex[1],
               parkingSpace.TopRightVertex[0], parkingSpace.TopRightVertex[1],
               parkingSpace.TopLeftVertex[0], parkingSpace.TopLeftVertex[1],
               parkingSpace.IsParkingAvailable ? "true" : "false",
               parkingSpace.IsHandicappedParkingSpace ? "true" : "false");
    }

    // Reading and printing PathData
    fseek(file, offset.PathDataOffset, SEEK_SET);
    printf("PathData:\n");
    for (int i = 0; i < header.PathDataCount; i++)
    {
        PathData pathData;
        fread(&pathData, sizeof(PathData), 1, file);
        printf("NodeID: %s, NodeVertex: [%d, %d], LinkedNodes: [%d, %d, %d, %d], LinkedIndex: [",
               pathData.NodeID,
               pathData.NodeVertex[0], pathData.NodeVertex[1],
               pathData.LinkedNodes[0], pathData.LinkedNodes[1], pathData.LinkedNodes[2], pathData.LinkedNodes[3]);
        for (int j = 0; j < 4; j++)
        {
            if (pathData.LinkedIndex[j] != 0) // 값이 0이 아닌 경우에만 출력
            {
                printf("%d", pathData.LinkedIndex[j]);
                if (j < 3 && pathData.LinkedIndex[j + 1] != 0) // 다음 요소가 0이 아닐 경우에만 쉼표 출력
                {
                    printf(", ");
                }
            }
        }
        printf("]\n");
    }
    fclose(file);
    return 0;
}

int main()
{
    const char *binFileName = MAP_DATA_BIN_PATH;

    // 바이너리 파일을 읽어서 데이터 출력
    if (readBinaryData(binFileName) == 0)
    {
        printf("데이터를 성공적으로 읽고 출력했습니다.\n");
    }
    else
    {
        printf("바이너리 파일에서 데이터를 읽는 중 오류가 발생했습니다.\n");
    }

    return 0;
}