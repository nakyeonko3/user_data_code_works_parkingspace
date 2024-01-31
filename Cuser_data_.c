#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdbool.h>

// #define USER_DATA_BIN_PATH "./UserData.bin"
#define USER_DATA_BIN_PATH "./UserData.bin"

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

// 데이터 파일 초기화 및 헤더 작성 함수 정의
void Initialize_bin_file(const char *filename)
{
    // 파일을 바이너리 쓰기 모드로 열기
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        // 파일 열기에 실패했을 경우 오류 메시지 출력
        perror("파일 열기 실패");
        return;
    }

    // Header 구조체 초기화
    Header header = {0, 0, 0, sizeof(UserData), sizeof(ParkingSpace)};
    // 파일의 시작 부분에 Header 구조체 저장
    fwrite(&header, sizeof(header), 1, file);
    // 파일 초기화 완료 메시지 출력
    printf("파일 초기화 완료\n");

    // 파일 닫기
    fclose(file);
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

// 사용자 데이터 등록 함수 정의
void register_data(const char *filename, const char *Name, const char *CarType, const char *CarNumber)
{
    // 파일을 업데이트 모드로 열기
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        // 파일 열기에 실패한 경우 오류 메시지 출력 후 함수 종료
        perror("파일 열기 실패");
        return;
    }

    // Header와 Offset 구조체 선언
    Header header;
    Offset offset;

    // 파일에서 Header 구조체 읽기
    fread(&header, sizeof(header), 1, file);

    // 주차 공간이 가득 찼는지 확인
    if (header.UserDataCount >= 22)
    {
        // 주차 공간이 가득 찼다면 메시지 출력 후 함수 종료
        printf("주차장이 가득 찼습니다. 더 이상 주차할 수 없습니다.\n");
        fclose(file);
        return;
    }

    // 파일 내의 현재 UserData의 오프셋 계산
    offset.UserDataOffset = sizeof(Header) + (header.UserDataCount * sizeof(UserData)) + (header.UserDataCount * sizeof(ParkingSpace));

    // 계산된 오프셋으로 파일 포인터 이동
    fseek(file, offset.UserDataOffset, SEEK_SET);

    // UserData 구조체 선언 및 초기화
    UserData userData;

    // 파라미터로 받은 이름, 차종, 차량번호를 userData에 복사
    strncpy(userData.Name, Name, sizeof(userData.Name));
    strncpy(userData.CarType, CarType, sizeof(userData.CarType));
    strncpy(userData.CarNumber, CarNumber, sizeof(userData.CarNumber));

    // 파일에 userData 쓰기
    fwrite(&userData, sizeof(UserData), 1, file);

    // Header 정보 업데이트
    header.TotalDataSize += sizeof(UserData);
    header.UserDataCount++;

    // 파일 시작으로 이동 후 업데이트된 Header 정보 쓰기
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, file);
    // 사용자 데이터 추가 메시지 출력
    printf("데이터 추가: 이름 - %s, 차종 - %s, 차량번호 - %s\n", userData.Name, userData.CarType, userData.CarNumber);

    // 파일 닫기
    fclose(file);
}

// 주차장 번호 업데이트 함수 정의
int UpdateParkingSpace(const char *filename, const char *CarNumber, const char *NewParkingSpaceStr)
{
    // 문자열을 정수로 변환
    int NewParkingSpace = atoi(NewParkingSpaceStr);

    // 파일을 업데이트 모드로 열기
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        // 파일 열기 실패 시 오류 메시지 출력
        perror("파일 열기 실패");
        return 1;
    }

    // Header 구조체 선언
    Header header;
    // 파일에서 Header 구조체 읽기
    fread(&header, sizeof(header), 1, file);

    // UserData 구조체 선언 및 찾기 위한 변수 초기화
    UserData userData;
    int found = 0;
    int i;

    // 파일에서 UserData 검색
    for (i = 0; i < header.UserDataCount; i++)
    {
        // 파일 내에서 UserData 위치로 이동
        fseek(file, sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
        // UserData 구조체 읽기
        fread(&userData, sizeof(UserData), 1, file);

        // 차량 번호가 일치하는지 확인
        if (strcmp(userData.CarNumber, CarNumber) == 0)
        {
            // 일치하는 차량 발견
            found = 1;
            break;
        }
    }

    // 차량 번호가 발견되었을 경우
    if (found)
    {
        // ParkingSpace 구조체 선언 및 업데이트
        ParkingSpace parkingSpace;
        parkingSpace.ParkingSpace = NewParkingSpace;

        // ParkingSpace 구조체의 오프셋 계산
        int parkingSpaceOffset = sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)) + sizeof(UserData);
        // 해당 오프셋으로 이동
        fseek(file, parkingSpaceOffset, SEEK_SET);
        // 업데이트된 ParkingSpace 구조체 쓰기
        fwrite(&parkingSpace, sizeof(ParkingSpace), 1, file);

        // 파일 닫기 및 함수 성공 리턴
        fclose(file);
        return 0;
    }
    // 차량 번호가 발견되지 않았을 경우
    else
    {
        // 파일 닫기 및 함수 실패 리턴
        fclose(file);
        return 1;
    }
}

// 사용자 데이터 삭제 함수 정의
void delete_data(const char *filename, const char *CarNumber)
{
    // 파일을 업데이트 모드로 열기
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        // 파일 열기 실패 시 오류 메시지 출력
        perror("파일 열기 실패");
        return;
    }

    // Header 구조체 선언
    Header header;
    // 파일에서 Header 구조체 읽기
    fread(&header, sizeof(header), 1, file);

    // UserData 구조체 선언 및 삭제된 데이터 수를 위한 변수 초기화
    UserData userData;
    int deletedCount = 0;

    // 파일 내의 모든 UserData 검색
    for (int i = 0; i < header.UserDataCount; ++i)
    {
        // 파일 내에서 UserData 위치로 이동
        fseek(file, sizeof(Header) + i * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
        // UserData 구조체 읽기
        fread(&userData, sizeof(UserData), 1, file);

        // 차량 번호가 일치하는지 확인
        if (strcmp(userData.CarNumber, CarNumber) == 0)
        {
            // 일치하는 데이터 발견 시 삭제된 데이터 수 증가
            deletedCount++;
        }
        else
        {
            // 일치하지 않는 데이터의 경우, 데이터를 새 위치에 다시 씀
            fseek(file, sizeof(Header) + (i - deletedCount) * (sizeof(UserData) + sizeof(ParkingSpace)), SEEK_SET);
            fwrite(&userData, sizeof(UserData), 1, file);
        }
    }

    // 삭제된 데이터가 있을 경우 파일 크기를 조정하는 구문 시작
    if (deletedCount > 0)
    {
        // 새 파일 크기를 계산: 헤더 크기 + (전체 데이터 수 - 삭제된 데이터 수) * (UserData와 ParkingSpace의 크기 합)
        long newSize = sizeof(Header) + (header.UserDataCount - deletedCount) * (sizeof(UserData) + sizeof(ParkingSpace));

        // ftruncate 함수를 사용하여 파일의 크기를 newSize로 조정
        ftruncate(fileno(file), newSize);

        // Header의 UserData 개수에서 삭제된 데이터 수만큼 빼서 업데이트
        header.UserDataCount -= deletedCount;

        // 파일 포인터를 파일의 시작점으로 이동
        fseek(file, 0, SEEK_SET);

        // 변경된 Header 정보를 파일에 다시 쓰기
        fwrite(&header, sizeof(Header), 1, file);
    }
    // 파일 닫기
    fclose(file);
}

// main 함수 정의
int main(int argc, char *argv[])
{
    // 파일 이름 정의
    const char *filename = USER_DATA_BIN_PATH;

    // 입력된 매개변수 개수 확인
    if (argc < 3)
    {
        // 매개변수가 충분하지 않을 경우 사용 방법 출력
        fprintf(stderr, "사용 방법: <flag> <CarNumber> [Name] [CarType]\n");
        return 1;
    }

    // 첫 번째 매개변수를 정수로 변환하여 flag에 저장
    int flag = atoi(argv[1]);

    // flag가 1인 경우 검색 기능 수행
    if (flag == 1)
    {
        // 검색할 차량 번호를 매개변수에서 읽어옴
        const char *CarNumberToSearch = argv[2];
        // 주차 공간 정보를 저장할 변수 선언
        int parkingSpaceInfo;
        // search_data 함수 호출로 차량 번호 검색
        int userFound = search_data(filename, CarNumberToSearch, &parkingSpaceInfo);
        if (!userFound)
        {
            // 차량 번호가 데이터베이스에 없을 경우 오류 메시지 출력
            fprintf(stderr, "DB 안에 해당 차량 번호가 존재 하지 않음: %s\n", CarNumberToSearch);
            return 0;
        }
        // 찾은 주차 공간 정보 출력
        printf("%d\n", parkingSpaceInfo);
        return -1;
    }

    // 플래그 값이 2인 경우 차량 등록 기능 수행
    if (flag == 2)
    {
        // 적절한 인수의 개수를 확인
        if (argc < 5)
        {
            // 인수가 충분하지 않은 경우 오류 메시지 출력 후 함수 종료
            fprintf(stderr, "등록을 위한 충분한 인수가 제공되지 않았습니다.\n");
            return 1;
        }

        // argv 배열에서 차량 번호를 가져옴
        const char *CarNumber = argv[2];

        // register_data 함수 호출하여 차량 등록
        // argv[3]과 argv[4]에는 사용자 이름과 차종이 포함됨
        register_data(filename, argv[3], argv[4], CarNumber);
    }

    // 플래그 값이 3인 경우, 주차 공간 번호 업데이트 기능 수행
    if (flag == 3)
    {
        // 업데이트할 차량 번호와 새 주차 공간 번호를 매개변수에서 가져옴
        const char *CarNumberToUpdate = argv[2];
        const char *NewParkingSpaceStr = argv[3]; // 새 주차 공간 번호 (문자열)

        // UpdateParkingSpace 함수 호출하여 주차 공간 번호 업데이트
        int updateResult = UpdateParkingSpace(filename, CarNumberToUpdate, NewParkingSpaceStr);

        // 업데이트 결과에 따른 메시지 출력
        if (updateResult == 0)
        {
            printf("주차장 번호가 업데이트되었습니다.\n");
        }
        else
        {
            printf("주차장 번호 업데이트 실패.\n");
        }
    }

    // 플래그 값이 4인 경우, 차량 삭제 기능 수행
    if (flag == 4)
    {
        // 삭제할 차량 번호를 매개변수에서 가져옴
        const char *CarNumberToDelete = argv[2];

        // delete_data 함수 호출하여 차량 삭제
        delete_data(filename, CarNumberToDelete);
    }
}