#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#define USER_DATA_BIN_PATH "./UserData.bin"

// 데이터 헤더를 위한 구조체 정의
typedef struct
{
    int TotalDataSize;
    int UserDataCount;
    int SelectParkingSpaceDataCount;
    int UserDataSize;
    int SelectParkingSpaceSize;
} Header;

// 데이터 오프셋을 위한 구조체 정의
typedef struct
{
    int UserDataOffset;
    int SelectParkingSpaceDataOffset;
} Offset;

// 사용자 데이터 구조체 정의
typedef struct
{
    char Name[13];
    char CarType[20];
    char CarNumber[12];
} UserData;

// 선택한 주차 공간 데이터 구조체 정의
typedef struct
{
    int ParkingSpace;
} ParkingSpace;

FILE *read_byte_binfile(const char *filename)
{
    FILE *file = fopen(filename, "rb+");
    if (!file)
    {
        perror("File opening failed");
        exit(1);
    }
    return file;
}

// 사용자 데이터 등록 함수
void register_data(FILE *file, const char *CarNumber, const char *Name, const char *CarType)
{

    Header header = {0};
    Offset offset = {0};

    // 현재 오프셋 계산
    offset.UserDataOffset = sizeof(Header) + (header.UserDataCount * sizeof(UserData)) + (header.UserDataCount * sizeof(ParkingSpace));
    //   offset.SelectParkingSpaceDataOffset = offset.UserDataOffset + (header.SelectParkingSpaceDataCount * sizeof(ParkingSpace));

    // 오프셋으로 이동 후 사용자 데이터 쓰기
    fseek(file, offset.UserDataOffset, SEEK_SET);

    UserData userData;
    strncpy(userData.Name, Name, sizeof(userData.Name));
    strncpy(userData.CarType, CarType, sizeof(userData.CarType));
    strncpy(userData.CarNumber, CarNumber, sizeof(userData.CarNumber));

    fwrite(&userData, sizeof(UserData), 1, file);

    // 헤더 업데이트
    header.TotalDataSize += sizeof(UserData);
    header.UserDataCount++;

    // 파일 시작으로 이동 후 헤더 정보 업데이트
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, file);
    printf("데이터 추가: 이름 - %s, 차종 - %s, 차량번호 - %s\n", userData.Name, userData.CarType, userData.CarNumber);

    fclose(file);
}
int UpdateParkingSpace(FILE *file, const char *CarNumberToUpdate, const char *NewParkingSpace)
{
    Header header = {0};
    UserData userData = {0};
    int found = 0; // Flag to track if the CarNumberToUpdate is found
    int count = 0; // 바이너리 파일 안의 carNumber가 저장된 위치

    fread(&header, sizeof(header), 1, file);

    for (int i = 0; i < header.UserDataCount; ++i)
    {
        int size = sizeof(header) + i * sizeof(ParkingSpace) + i * sizeof(UserData);
        fseek(file, size, SEEK_SET);
        fread(&userData, sizeof(UserData), 1, file);
        count++;
        if (strcmp(userData.CarNumber, CarNumberToUpdate) == 0)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        ParkingSpace parkingSpace;
        parkingSpace.ParkingSpace = atoi(NewParkingSpace);

        int parkingSpaceOffset = sizeof(Header) + (count * sizeof(UserData)) + ((count - 1) * sizeof(ParkingSpace));

        fseek(file, parkingSpaceOffset, SEEK_SET);

        fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);

        printf("차량번호 : %s ", CarNumberToUpdate);
        printf("주차공간번호: %d\n", parkingSpace.ParkingSpace);
    }
    else
    {
        printf("Car Number %s not found.\n", CarNumberToUpdate);
        exit(1);
    }

    fclose(file);
    return 0;
}

// bin파일이 없다면 파일을 새로하나 만든다. bin 파일이 있다면 이 함수는 작동 하지 않난다.
void create_binfile_if_init(const char *filename)
{
    FILE *file1 = fopen(filename, "r");
    if (!file1)
    {
        FILE *file = fopen(filename, "w");
        fclose(file);
    }
    else
    {
        fclose(file1);
    }
}

int main(int argc, char *argv[])
{
    create_binfile_if_init(USER_DATA_BIN_PATH);
    FILE *file = read_byte_binfile(USER_DATA_BIN_PATH);
    // register_data(file, "고낙연", "superX", "1234-567");
    if (argc < 3)
    {
        // 매개변수가 충분하지 않을 경우 사용 방법 출력
        fprintf(stderr, "사용 방법: <flag> <CarNumber> [Name] [CarType]\n");
        return 1;
    }

    int flag = atoi(argv[1]);

    if (flag == 2)
    {
        if (argc != 5)
        {
            printf("argc =! 5, argments is not 4\n");
            return 1;
        }
        register_data(file, argv[2], argv[3], argv[4]);
        // 1, 차량번호, 이름, 차종
        return 0;
    }

    if (flag == 3)
    {
        if (argc != 4)
        {
            printf("argc =! 3, argments is not 3\n");
            return 1;
        }
        UpdateParkingSpace(file, argv[2], argv[3]);
        // 2, 차량번호, 주차번호
        return 0;
    }
    return 0;
}