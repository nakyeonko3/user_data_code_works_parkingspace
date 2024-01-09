#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define USER_DATA_BIN_PATH "/home/tjchoi/Project/UserData.bin" // 사용자 데이터 파일의 경로를 상수로 정의
#define USER_DATA_BIN_PATH "./UserData.bin" // 사용자 데이터 파일의 경로를 상수로 정의

// 파일 구조와 데이터 구조체 정의
typedef struct
{
    int TotalDataSize;
    int UserDataCount;
    int ParkingSpaceDataCount;
    int UserDataSize;
    int ParkingSpaceSize;
} Header; // 파일의 헤더 정보를 담는 구조체

typedef struct
{
    int UserDataOffset;
    int ParkingSpaceDataOffset;
} Offset; // 사용자 데이터와 주차 공간 데이터의 오프셋 정보를 담는 구조체

typedef struct
{
    char Name[13];
    char CarType[20];
    char CarNumber[12];
} UserData; // 사용자 정보를 담는 구조체

typedef struct
{
    int ParkingSpace;
} ParkingSpace; // 주차 공간 정보를 담는 구조체

// 사용자 데이터와 주차 공간 데이터를 읽어오는 함수
void ReadUserData(const char *filename, int index, UserData *userData, ParkingSpace *parkingSpace)
{
    FILE *file = fopen(filename, "rb"); // 파일을 이진 읽기 모드로 열기
    if (!file)
    {
        perror("File opening failed"); // 파일 열기 실패 시 에러 메시지 출력
        exit(1);
    }

    Header header;
    fread(&header, sizeof(header), 1, file); // 파일의 헤더 읽기

    // 인덱스가 유효한 범위 내에 있는지 확인 후, 데이터 읽기
    if (index >= 0 && index < header.UserDataCount)
    {
        Offset offset = {
            .UserDataOffset = sizeof(Header) + (index * sizeof(UserData)) + (index * sizeof(ParkingSpace)),
            .ParkingSpaceDataOffset = sizeof(Header) + ((index + 1) * sizeof(UserData)) + (index * sizeof(ParkingSpace))};

        fseek(file, offset.UserDataOffset, SEEK_SET); // 사용자 데이터 위치로 이동
        fread(userData, sizeof(UserData), 1, file); // 사용자 데이터 읽기

        userData->Name[sizeof(userData->Name) - 1] = '\0'; // 문자열 마지막에 NULL 추가

        fseek(file, offset.ParkingSpaceDataOffset, SEEK_SET); // 주차 공간 데이터 위치로 이동
        fread(parkingSpace, sizeof(ParkingSpace), 1, file); // 주차 공간 데이터 읽기
    }

    fclose(file); // 파일 닫기
}

// 메인 함수
int main()
{
    const char *filename = USER_DATA_BIN_PATH;

    Header header;
    FILE *file = fopen(filename, "rb"); // 파일을 이진 읽기 모드로 열기
    if (!file)
    {
        perror("File opening failed"); // 파일 열기 실패 시 에러 메시지 출력
        return 1;
    }
    fread(&header, sizeof(header), 1, file); // 파일의 헤더 읽기

    int totalDataCount = header.UserDataCount; // 전체 데이터 개수

    // 사용자 데이터와 주차 공간 데이터를 저장할 배열 할당
    UserData *userDataArray = malloc(sizeof(UserData) * totalDataCount);
    ParkingSpace *parkingSpaceArray = malloc(sizeof(ParkingSpace) * totalDataCount);

    // 메모리 할당 실패 시 에러 처리
    if (!userDataArray || !parkingSpaceArray)
    {
        perror("Memory allocation failed");
        fclose(file);
        free(userDataArray);
        free(parkingSpaceArray);
        return 1;
    }

    // 전체 데이터를 읽어와서 출력
    for (int i = 0; i < totalDataCount; i++)
    {
        ReadUserData(filename, i, &userDataArray[i], &parkingSpaceArray[i]);
        printf("UserData %d\n", i);
        printf("Name: %s, CarType: %s, CarNumber: %s\n", userDataArray[i].Name, userDataArray[i].CarType, userDataArray[i].CarNumber);
        printf("ParkingSpace: %d\n", parkingSpaceArray[i].ParkingSpace);
    }

    // 할당된 메모리 해제 및 파일 닫기
    free(userDataArray);
    free(parkingSpaceArray);
    fclose(file);

    return 0;
}






























