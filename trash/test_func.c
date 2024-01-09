#include <stdio.h>
#include <regex.h>
#include <stdbool.h>
// #include <string.h>
#define CAR_NUMBER_PATTERN "^[0-9]{2,3}[가-힣]{3}-[0-9]{4}$"
// #define CAR_NUMBER_PATTERN "^[가-힣]{3}$"

bool is_valid_car_number(const char *carNumber);


int main() {
    // !is_valid_car_number(const char *carNumber)는 false이거나 true인가?
    //  is_valid_car_number 에 올바른 값이 들어 갔을 때 false가 나오는가?
    const char *CarNumber = "15누-8762";

    printf("%d\n",!is_valid_car_number(CarNumber));

    if (!is_valid_car_number(CarNumber))
    {
        fprintf(stderr, "올바른 차량번호 형식이 아닙니다. 입력된 차량번호: %s\n", CarNumber);

        return 0;
    }

    
    // 정수 0은 false
    // 정수 1은 true로 취급된다.
    
    // is_valid_car_number(const char *carNumber 의 출력값이 1또는 0이다.
    return 0;
}




bool is_valid_car_number(const char *carNumber)
{
    regex_t regex;
    int retC;
    int retM;

    // 정규 표현식 컴파일
    retC = regcomp(&regex, CAR_NUMBER_PATTERN, REG_EXTENDED);
    if (retC !=0)
    {
        fprintf(stderr, "정규 표현식 컴파일 실패: 반환 코드 %d\n", retC);
        return false;
    }

    // 정규 표현식 매칭
    retM = regexec(&regex, carNumber, 0, NULL, 0);
    if (retM == 0) { 
        return true; // 차량번호가 올바른 형식임
    }

    if (retM == REG_NOMATCH)
    {
        fprintf(stderr, "차량번호가 정규 표현식과 일치하지 않음: %s\n", carNumber);
        return false; // 차량번호가 올바르지 않은 형식임
    }

    regfree(&regex);
}

