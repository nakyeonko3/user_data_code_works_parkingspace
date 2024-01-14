#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdbool.h>

#define USER_DATA_BIN_PATH "./UserData.bin"
// #define USER_DATA_BIN_PATH "/home/tjchoi/Project/UserData.bin"

#define CAR_NUMBER_PATTERN "^[0-9]{2,3}[가-힣]{3}-[0-9]{4}$"

bool is_valid_car_number(const char *carNumber);

// 데이터 헤더를 위한 구조체 정의
typedef struct
{
    int TotalDataSize;         // 전체 데이터 크기
    int UserDataCount;         // 사용자 데이터 수
    int ParkingSpaceDataCount; // 주차 공간 데이터 수
    int UserDataSize;          // 사용자 데이터 크기
    int ParkingSpaceSize;      // 주차 공간 데이터 크기
} Header;

// 데이터 오프셋을 위한 구조체 정의
typedef struct
{
    int UserDataOffset;         // 사용자 데이터 오프셋
    int ParkingSpaceDataOffset; // 주차 공간 데이터 오프셋
} Offset;

// 사용자 데이터 구조체 정의
typedef struct
{
    char Name[13];      // 사용자 이름
    char CarType[20];   // 차종
    char CarNumber[12]; // 차량 번호
} UserData;

// 선택한 주차 공간 데이터 구조체 정의
typedef struct
{
    int ParkingSpace; // 주차장 번호
} ParkingSpace;

// 데이터 파일 초기화 및 헤더 작성 함수
void Initialize_bin_file(const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("파일 열기 실패");
        return;
    }

    Header header = {0, 0, 0, sizeof(UserData), sizeof(ParkingSpace)};
    fwrite(&header, sizeof(header), 1, file); // 파일 시작 부분에 헤더 저장
    printf("파일 초기화 완료\n");

    fclose(file);
}

bool is_valid_car_number(const char *carNumber)
{
    regex_t regex;
    int retC = 0;
    int retM = 0;

    // 정규 표현식 컴파일
    retC = regcomp(&regex, CAR_NUMBER_PATTERN, REG_EXTENDED);
    if (retC != 0)
    {
        fprintf(stderr, "정규 표현식 컴파일 실패: 반환 코드 %d\n", retC);
        return false;
    }

    // 정규 표현식 매칭
    retM = regexec(&regex, carNumber, 0, NULL, 0);
    if (retM == 0)
    {
        return true; // 차량번호가 올바른 형식임
    }

    if (retM == REG_NOMATCH)
    {
        fprintf(stderr, "차량번호가 정규 표현식과 일치하지 않음: %s\n", carNumber);
        return false; // 차량번호가 올바르지 않은 형식임
    }

    regfree(&regex);
}

// 사용자 데이터 검색 함수
int search_data(const char *filename, const char *CarNumber, int *outParkingSpace)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("파일 열기 실패");
        return 0;
    }

    Header header;
    fread(&header, sizeof(header), 1, file); // 파일에서 헤더 정보 읽기

    UserData userData;

    for (int i = 0; i < header.UserDataCount; ++i)
    {
        fseek(file, sizeof(header) + i * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET); // 오프셋 설정
        fread(&userData, sizeof(UserData), 1, file);                                           // 사용자 데이터 읽기

        // 차량 번호를 제공된 번호와 비교
        if (strcmp(userData.CarNumber, CarNumber) == 0)
        {
            // 일치하는 데이터를 찾으면 주차공간 데이터를 반환
            ParkingSpace parkingSpace;
            fseek(file, sizeof(header) + i * (sizeof(UserData) + sizeof(ParkingSpace)) + sizeof(UserData), SEEK_SET);
            fread(&parkingSpace, sizeof(ParkingSpace), 1, file);
            *outParkingSpace = parkingSpace.ParkingSpace;
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0; // 일치하는 데이터를 찾지 못한 경우
}

// 사용자 데이터 등록 함수
void register_data(const char *filename, const char *Name, const char *CarType, const char *CarNumber)
{
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        perror("파일 열기 실패");
        return;
    }

    Header header;
    Offset offset;

    // 헤더 정보 읽기
    fread(&header, sizeof(header), 1, file);

    // 주차 가능한지 확인
    if (header.UserDataCount >= 22)
    {
        printf("주차장이 가득 찼습니다. 더 이상 주차할 수 없습니다.\n");
        fclose(file);
        return;
    }

    // 현재 오프셋 계산
    offset.UserDataOffset = sizeof(Header) + (header.UserDataCount * sizeof(UserData)) + (header.UserDataCount * sizeof(ParkingSpace));

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

// 주차장 번호 업데이트 함수
int UpdateParkingSpace(const char *filename, const char *CarNumber, const char *NewParkingSpaceStr)
{
    int NewParkingSpace = atoi(NewParkingSpaceStr); // 문자열을 정수로 변환
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        perror("파일 열기 실패");
        return 1;
    }

    Header header;
    fread(&header, sizeof(header), 1, file);

    UserData userData;
    int found = 0;
    int i;

    for (i = 0; i < header.UserDataCount; i++)
    {
        fseek(file, sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
        fread(&userData, sizeof(UserData), 1, file);

        if (strcmp(userData.CarNumber, CarNumber) == 0)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        ParkingSpace parkingSpace;
        parkingSpace.ParkingSpace = NewParkingSpace; // 이미 정수로 변환된 값 사용

        int parkingSpaceOffset = sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)) + sizeof(UserData);
        fseek(file, parkingSpaceOffset, SEEK_SET);
        fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);

        fclose(file);
        return 0;
    }
    else
    {
        fclose(file);
        return 1;
    }
}

// 사용자 데이터 삭제 함수
void delete_data(const char *filename, const char *CarNumber)
{
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        perror("파일 열기 실패");
        return;
    }

    Header header;
    fread(&header, sizeof(header), 1, file);

    UserData userData;
    int deletedCount = 0;

    for (int i = 0; i < header.UserDataCount; ++i)
    {
        fseek(file, sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
        fread(&userData, sizeof(UserData), 1, file);

        if (strcmp(userData.CarNumber, CarNumber) == 0)
        {
            // 삭제된 데이터 수 증가
            deletedCount++;
        }
        else
        {
            // 삭제된 데이터가 아닌 경우, 현재 위치에 데이터를 다시 씀
            fseek(file, sizeof(Header) + (i - deletedCount) * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
            fwrite(&userData, sizeof(UserData), 1, file);
        }
    }

    // 파일 크기 조정
    if (deletedCount > 0)
    {
        long newSize = sizeof(Header) + (header.UserDataCount - deletedCount) * (sizeof(UserData) + sizeof(ParkingSpace));
        ftruncate(fileno(file), newSize);
        header.UserDataCount -= deletedCount;
        fseek(file, 0, SEEK_SET);
        fwrite(&header, sizeof(Header), 1, file);
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    const char *filename = USER_DATA_BIN_PATH;

    if (argc < 3)
    {
        fprintf(stderr, "사용 방법: <flag> <CarNumber> [Name] [CarType]\n");
        return 1;
    }

    int flag = atoi(argv[1]);

    if (flag == 1) // 검색
    {
        const char *CarNumberToSearch = argv[2];
        int parkingSpaceInfo;
        int userFound = search_data(filename, CarNumberToSearch, &parkingSpaceInfo);
        if (!userFound)
        {
            fprintf(stderr, "DB 안에 해당 차량 번호가 존재 하지 않음: %s\n", CarNumberToSearch);
            return 0;
        }
        // 추가됨
        printf("%d\n", parkingSpaceInfo);
    }

    if (flag == 2) // 등록
    {
        if (argc < 5)
        {
            fprintf(stderr, "등록을 위한 충분한 인수가 제공되지 않았습니다.");
            return 1;
        }

        const char *CarNumber = argv[2];

        if (!is_valid_car_number(CarNumber))
        {
            fprintf(stderr, " 올바른 차량번호 형식이 아닙니다.");
            return 1;
        }

        register_data(filename, argv[3], argv[4], CarNumber);
    }

    if (flag == 3)
    {
        const char *CarNumberToUpdate = argv[2];
        const char *NewParkingSpaceStr = argv[3]; // 이것은 문자열

        int updateResult = UpdateParkingSpace(filename, CarNumberToUpdate, NewParkingSpaceStr);

        if (updateResult == 0)
        {
            printf("주차장 번호가 업데이트되었습니다.");
        }
        else
        {
            printf("주차장 번호 업데이트 실패.");
        }
    }
    if (flag == 4) // 삭제
    {
        const char *CarNumberToDelete = argv[2];
        delete_data(filename, CarNumberToDelete);
    }
    return 0;
}
