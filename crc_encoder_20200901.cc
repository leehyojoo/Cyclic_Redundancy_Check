#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BIT 210 // 최대 비트 길이

FILE* open_file(char* filename, char* mode, char* error_message); // 파일을 열고 파일 포인터를 반환하는 함수
int get_file_size(FILE* file); // 파일 크기를 구하는 함수
unsigned char calculate_padding_length(int file_size, int dataword_size, int generator_length); // 패딩 길이를 계산하는 함수
void write_binary(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2); // 이진 데이터를 파일에 쓰는 함수
void write_codewords(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2, int* buff_count); // 코드 워드를 파일에 쓰는 함수

// 데이터 처리 관련 함수
void fill_dataword(unsigned char* dataword, unsigned char value, int size, int shift_bits);
void process_byte_to_datawords(char ch, unsigned char* dataword1, unsigned char* dataword2, int dataword_size);
unsigned char* modulo_2_division(const char* generator, unsigned char* dataword, int size);
void dataword_to_codeword(const char* generator, unsigned char* dataword1, unsigned char* dataword2);

// 메인 인코딩 함수
void input_file_to_codeword(char* generator);

FILE* input_file; // 입력 파일 포인터 선언
FILE* output_file; // 출력 파일 포인터 선언

int shift; // 시프트 비트 수 선언
int dataword_size; // 데이터 워드 크기 선언
int input_file_size; // 입력 파일 크기 선언
int output_file_size = 0; // 출력 파일 크기 초기화
unsigned char padding_len = 0; // 패딩 길이 초기화

// 파일을 열고 파일 포인터를 반환하는 함수
FILE* open_file(char* filename, char* mode, char* error_message) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        printf("%s\n", error_message); // 에러 메세지 출력
    }
    return file; // 파일 포인터 반환
}

// 파일 크기를 계산하는 유틸리티 함수
int get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END); // 파일 끝으로 이동
    int size = ftell(file); // 파일 크기 계싼
    rewind(file);
    return size; // 파일 크기 반환
}

// 패딩 길이를 계산하는 유틸리티 함수
unsigned char calculate_padding_length(int file_size, int dataword_size, int generator_length) {
    int bits_per_codeword; // 코드 워드당 비트 수

    // 데이터 워드 크기가 4인 경우
    if (dataword_size == 4) {
        bits_per_codeword = 2 * (dataword_size + generator_length - 1); // 코드 워드당 비트 수 계산
    }
    // 그 외의 경우
    else {
        bits_per_codeword = dataword_size + generator_length - 1; // 코드 워드당 비트 수 계산
    }
    // 패딩 길이 계산 후 반환
    return (8 - (bits_per_codeword * file_size) % 8) % 8;
}

int main(int argc, char* argv[]) {
    // command 인자 개수 확인
    if (argc != 5) {
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        return -1; // 에러 메세지 출력 후 프로그램 종료
    }

    char* input_file_name = argv[1]; // 입력 파일명
    char* output_file_name = argv[2]; // 출력 파일명
    char* generator = argv[3]; // 생성기
    dataword_size = atoi(argv[4]); // 데이터 워드 크기를 정수로 반환

    // 입력 파일
    input_file = open_file(input_file_name, "rb", "input file open error.");
    if (input_file == NULL) {
        return -1;
    }

    // 출력 파일
    output_file = open_file(output_file_name, "wb", "output file open error.");
    if (output_file == NULL) {
        fclose(input_file);
        return -1;
    }

    // 데이터 워드 크기 유효성 검사
    if (dataword_size != 4 && dataword_size != 8) {
        printf("dataword size must be 4 or 8.\n");
        fclose(input_file);
        fclose(output_file);
        return -1;
    }

    // 입력 파일 크기
    int file_size = get_file_size(input_file);
    // 패딩 길이 계산
    padding_len = calculate_padding_length(file_size, dataword_size, (int)strlen(generator));
    // 패딩 길이를 출력 파일에 씀
    fwrite(&padding_len, sizeof(unsigned char), 1, output_file);

    // 입력 파일을 코드 워드로 변환
    input_file_to_codeword(generator);

    // 출력 파일, 입력 파일 닫은 후 프로그램 종료
    fclose(output_file);
    fclose(input_file);

    return 0;
}

// 데이터 워드를 채우는 함수
void fill_dataword(unsigned char* dataword, unsigned char value, int size, int shift_bits) {
    for (int i = 0; i < size; i++) { // 데이터 워드 크기만큼 반복
        dataword[i] = ((value & 0b1) == 0b1 ? '1' : '0'); // 데이터 워드에 비트 채우기
        value = value >> 1; // 다음 비트로 이동
    }
    for (int i = size; i < size + shift_bits; i++) { // 시프트 비트만큼 0으로 채움
        dataword[i] = '0';
    }
}

// 바이트를 데이터 워드로 변환하는 함수
void process_byte_to_datawords(char ch, unsigned char* dataword1, unsigned char* dataword2, int dataword_size) {
    unsigned char tmp = (unsigned char)ch;

    // 데이터 워드 크기가 4인 경우
    if (dataword_size == 4) {
        fill_dataword(dataword1, tmp >> 4, dataword_size, shift); // 상위 4비트를 데이터 워드에 채움
        fill_dataword(dataword2, tmp & 0x0F, dataword_size, shift); // 하위 4비트를 데이터 워드에 채움
    }
    // 그 외의 경우
    else {
        fill_dataword(dataword1, tmp, dataword_size, shift); // 데이터 워드에 바이트 채움
    }
}

// 모듈로 2 나눗셈을 수행하는 함수
unsigned char* modulo_2_division(const char* generator, unsigned char* dataword, int size) {
    unsigned char dataword_tmp[MAX_BIT] = { 0 }; // 임시 데이터 워드 배열 초기화

    memcpy(dataword_tmp, dataword, size + shift); // 데이터 워드 복사

    for (int i = size + shift - 1; i >= shift; i--) { // 데이터 워드 크기만큼 반복
        if (dataword_tmp[i] == '1') { // 현재 비트가 1인 경우
            for (int j = 0; j <= shift; j++) { // 시프트만큼 반복
                dataword_tmp[i - j] = (dataword_tmp[i - j] == generator[j]) ? '0' : '1'; // XOR 연산 수행
            }
        }
    }

    memcpy(dataword, dataword_tmp, shift); // 결과 복사
    return dataword; // 결과 반환
}

// 데이터 워드를 코드 워드로 변환하는 함수
void dataword_to_codeword(const char* generator, unsigned char* dataword1, unsigned char* dataword2) {
    for (int i = dataword_size - 1; i >= 0; i--) { // 데이터 워드 크기만큼 반복
        dataword1[i + shift] = dataword1[i]; // 데이터 워드 1을 시프트
    }
    for (int i = 0; i < shift; i++) { // 시프트 비트만큼 0으로 초기화
        dataword1[i] = '0';
    }
    dataword1 = modulo_2_division(generator, dataword1, dataword_size); // 모듈로 2 나눗셈 수행

    if (dataword_size == 4) { // 데이터 워드 크기가 4인 경우
        for (int i = dataword_size - 1; i >= 0; i--) { // 데이터 워드 크기만큼 반복
            dataword2[i + shift] = dataword2[i]; // 데이터 워드 2를 시프트
        }
        for (int i = 0; i < shift; i++) { // 시프트 비트만큼 0으로 초기화
            dataword2[i] = '0';
        }
        dataword2 = modulo_2_division(generator, dataword2, dataword_size); // 모듈로 2 나눗셈 수행
    }
}

// 이진 데이터를 파일에 쓰는 함수
void write_codewords(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2, int* buff_count) {
    int remaining_codeword1 = strlen((char*)codeword1); // 코드 워드 1의 길이
    int remaining_codeword2 = strlen((char*)codeword2); // 코드 워드 2의 길이

    while (remaining_codeword1 > 0 || remaining_codeword2 > 0) { // 코드 워드가 남아 있는 동안 반복
        if (remaining_codeword1 > 0) { // 코드 워드 1이 남아 있는 경우
            write_buff[(*buff_count)++] = codeword1[--remaining_codeword1]; // 버퍼에 코드 워드 1 추가
        }
        else if (remaining_codeword2 > 0) { // 코드 워드 2가 남아 있는 경우
            write_buff[(*buff_count)++] = codeword2[--remaining_codeword2]; // 버퍼에 코드 워드 2 추가
        }

        if (*buff_count == 8) { // 버퍼가 꽉 찬 경우
            unsigned char tmp = 0; // 임시 변수 초기화
            for (int i = 0, pos = 1; i < 8; i++, pos *= 2) { // 8비트에 대해 반복
                tmp += (write_buff[7 - i] == '1') ? pos : 0; // 이진 데이터를 정수로 변환하여 임시 변수에 저장
            }

            fwrite(&tmp, sizeof(unsigned char), 1, output_file); // 파일에 이진 데이터 쓰기
            *buff_count = 0; // 버퍼 카운트 초기화
        }
    }
}

// 이진 데이터를 파일에 쓰는 함수
void write_binary(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2) {
    static int buff_count = 0; // 정적 변수 선언 및 초기화

    if (output_file_size == 0) { // 출력 파일 크기가 0인 경우
        for (int i = 0; i < padding_len; i++) { // 패딩 길이만큼 반복
            write_buff[buff_count++] = '0'; // 버퍼에 0 추가
            output_file_size++; // 출력 파일 크기 증가
        }
    }

    write_codewords(write_buff, codeword1, codeword2, &buff_count); // 코드 워드를 파일에 쓰는 함수 호출
    write_buff[8] = buff_count; // 버퍼에 있는 데이터 수 저장
}

// 입력 파일을 코드 워드로 변환하는 함수
void input_file_to_codeword(char* generator) {
    shift = strlen(generator) - 1; // 시프트 비트 수 계산
    char ch; // 문자 변수 선언
    unsigned char dataword1[MAX_BIT] = { 0 }; // 데이터 워드 1 배열 초기화
    unsigned char dataword2[MAX_BIT] = { 0 }; // 데이터 워드 2 배열 초기화
    unsigned char write_buff[9] = { 0 }; // 쓰기 버퍼 배열 초기화

    // 파일 끝까지 반복
    while ((ch = fgetc(input_file)) != EOF) {
        // 바이트를 데이터 워드로 변환
        process_byte_to_datawords(ch, dataword1, dataword2, dataword_size);
        // 데이터 워드를 코드 워드로 변환
        dataword_to_codeword(generator, dataword1, dataword2);
        // 이진 데이터를 파일에 쓰기
        write_binary(write_buff, dataword1, dataword2);
    }
}