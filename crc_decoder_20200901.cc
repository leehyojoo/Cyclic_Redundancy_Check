#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 최대 비트 길이
#define MAX_BIT 210

FILE* open_file(char* filename, char* mode, char* error_message); // 파일 열기 함수 선언
void input_file_to_dataword(char*); // 입력 파일을 데이터 워드로 변환하는 함수 선언
void codeword_to_dataword(char*, unsigned char*, unsigned char*); // 코드 워드를 데이터 워드로 변환하는 함수 선언
unsigned char* modulo_2_division(char*, unsigned char*); // 모듈로 2 나눗셈 수행 함수 선언

// 전역 변수
FILE* input_file, * output_file, * result_file;
int shift, dataword_size, error_count = 0, codeword_count = 0, output_file_size = 0;
unsigned char padding_len = 0;

// file open 함수 
FILE* open_file(char* filename, char* mode, char* error_message) {
    FILE* file = fopen(filename, mode); // 파일 열기
    if (file == NULL) { // 파일 열기 실패 시
        printf("%s\n", error_message); // 에러 메세지 출력
    }
    return file; // 파일 포인터 반환
}

// main 함수
int main(int argc, char* argv[]) {
    // command 인자 개수 확인
    if (argc != 6) {
        printf("Usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        return -1; // 프로그램 종료
    }

    // command 인자에서 파일명과 생성기, 데이터 워드 크기 추출
    char* input_file_name = argv[1];
    char* output_file_name = argv[2];
    char* result_file_name = argv[3];
    char* generator = argv[4];
    dataword_size = atoi(argv[5]); // 문자열을 정수로 변환

    // 입려 파일 열기
    input_file = open_file(input_file_name, "rb", "input file open error.");
    if (input_file == NULL) {
        return -1; // 프로그램 종료
    }

    // 출력 파일 열기
    output_file = open_file(output_file_name, "wb", "output file open error.");
    if (output_file == NULL) {
        fclose(input_file); // 입력 파일 닫기
        return -1; // 프로그램 종료
    }

    // 결과 파일 열기
    result_file = open_file(result_file_name, "w", "result file open error.\n");
    if (result_file == NULL) {
        fclose(input_file); // 입력 파일 닫기
        fclose(output_file); // 출력 파일 닫기
        return -1; // 프로그램 종료
    }

    // 데이터 워드 크기 유효성 확인
    if (dataword_size != 4 && dataword_size != 8) {
        printf("Dataword size must be 4 or 8.\n");
        fclose(result_file); // 결과 파일 닫기
        fclose(output_file); // 출력 파일 닫기
        fclose(input_file); // 입력 파일 닫기
        return -1; // 프로그램 종료
    }

    // 입력 파일로부터 패딩 길이 읽기
    fread(&padding_len, sizeof(unsigned char), 1, input_file);

    // 입력 파일을 데이터 워드로 변환
    input_file_to_dataword(argv[4]);

    // 전체 함수 수행 후 결과 파일, 출력 파일, 입력 파일 닫은 후 프로그램 종료
    fclose(result_file);
    fclose(output_file);
    fclose(input_file);

    return 0;
}

// 입력 파일을 데이터 워드로 변환하는 함수
void input_file_to_dataword(char* generator) {
    shift = strlen(generator) - 1; // 시프트 값 설정
    // 임시 변수 및 버퍼 초기화, 코드 워드 카운트 초기화, 임시 변수 초기화, 버퍼 카운트 초기화
    unsigned char temp, dataword1[MAX_BIT] = { 0 }, dataword2[MAX_BIT] = { 0 }, read_buff[8] = { 0 };
    int codeword1_count = dataword_size + shift, codeword2_count = (dataword_size == 4) ? (dataword_size + shift) : 0;
    unsigned char tmp = 0;
    int buff_count = (output_file_size == 0) ? 8 : read_buff[8];

    while (1) {
        // 버퍼가 가득 찼을 때 
        if (buff_count == 8) {
            // 파일로부터 1바이트 읽기
            if (fread(&tmp, sizeof(unsigned char), 1, input_file) != 1) break;

            // 8비트 반복
            for (int i = 0; i < buff_count; i++) {
                read_buff[buff_count - 1 - i] = (tmp % 2) ? '1' : '0'; // 이진수로 변환하여 버퍼에 저장
                tmp /= 2; // 다음 비트로 이동
            }

            // 버퍼 카운트 초기화
            buff_count = 0;

            // 출력 파일 크기가 0인 경우
            if (output_file_size == 0) {
                // 패딩 길이만큼 반복
                for (int i = 0; i < padding_len; i++) {
                    buff_count++; // 버퍼 카운트 증가
                    output_file_size++; // 출력 파일 크기 증가
                }
            }
        }
        // 버퍼가 가득 차지 않은 경우
        else {
            // 코드 워드 1의 처리가 끝나지 않은 경우
            if (codeword1_count != 0) {
                // 데이터 워드 1에 비트 저장
                dataword1[--codeword1_count] = read_buff[buff_count++];
            }
            // 코드 워드 1의 처리가 끝난 경우
            else {
                // 데이터 워드 2에 비트 저장
                dataword2[--codeword2_count] = read_buff[buff_count++];
            }
            // 코드 워드 처리가 끝난 경우
            if (codeword1_count == 0 && codeword2_count == 0) {
                codeword_to_dataword(generator, dataword1, dataword2); // 코드 워드를 데이터 워드로 변환
                codeword1_count = dataword_size + shift; // 코드 워드 1 카운트 초기화
                codeword2_count = (dataword_size == 4) ? (dataword_size + shift) : 0; // 코드 워드 2 카운트 초기화
            }
        }
    }
    // 결과 파일에 코드 워드 수와 오류 수 기록
    fprintf(result_file, "%d %d", codeword_count, error_count);
}

// 코드 워드를 데이터 워드로 변환하는 함수
void codeword_to_dataword(char* generator, unsigned char* dataword1, unsigned char* dataword2) {
    // 모듈로 2 나눗셈 수행
    dataword1 = modulo_2_division(generator, dataword1);

    // 데이터 워드 크기가 4인 경우
    if (dataword_size == 4) {
        dataword2 = modulo_2_division(generator, dataword2); // 모듈로 2 나눗셈 수행
    }

    int remainder = 0, pos = 1; // 나머지와 위치 초기화
    // 시프트만큼 반복
    for (int i = 0; i < shift; i++) {
        remainder += (dataword1[i] == '1') ? pos : 0; // 나머지 계산
        pos *= 2; // 다음 위치로 이동
    }
    codeword_count++; // 코드 워드 카운트 증가
    // 나머지가 0이 아닌 경우
    if (remainder != 0) { 
        error_count++; // 오류 카운트 증가
    }

    // 데이터 워드 크기가 4인 경우
    if (dataword_size == 4) {
        remainder = 0; pos = 1; // 나머지와 위치 초기화
        for (int i = 0; i < shift; i++) { // 시프트만큼 반복
            remainder += (dataword2[i] == '1') ? pos : 0; // 나머지 계산
            pos *= 2; // 다음 위치로 이동
        }
        codeword_count++; // 코드 워드 카운트 증가
        if (remainder != 0) { // 나머지가 0이 아닌 경우
            error_count++; // 오류 카운트 증가
        }
    }

    // 문자와 위치 초기화
    char ch = 0; pos = 1; 
    // 데이터 워드 크기가 4인 경우
    if (dataword_size == 4) {  
        for (int i = shift; i < dataword_size + shift; i++) {
            ch += (dataword2[i] == '1') ? pos : 0; // 문자 계산
            pos *= 2; // 다음 위치로 이동
        }
    }
    for (int i = shift; i < dataword_size + shift; i++) { // 데이터 워드 크기만큼 반복
        ch += (dataword1[i] == '1') ? pos : 0; // 문자 계산
        pos *= 2; // 다음 위치로 이동
    }
    fputc(ch, output_file); // 문자를 출력 파일에 기록
}

// 모듈로 2 나눗셈 수행 함수
unsigned char* modulo_2_division(char* generator, unsigned char* dataword) {
    unsigned char dataword_tmp[dataword_size + shift]; // 임시 데이터 워드 배열 
    memcpy(dataword_tmp, dataword, dataword_size + shift); // 데이터 워드 복사

    for (int i = dataword_size + shift - 1; i >= shift; i--) { // 데이터 워드 크기만큼 반복
        if (dataword_tmp[i] == '1') { // 현재 비트가 1인 경우
            for (int j = 0; j <= shift; j++) { // 시프트만큼 반복
                dataword_tmp[i - j] = (dataword_tmp[i - j] == generator[j]) ? '0' : '1'; //XOR 연산 수행
            }
        }
    }

    memcpy(dataword, dataword_tmp, shift); // 결과 복사
    return dataword; // 결과 반환
}