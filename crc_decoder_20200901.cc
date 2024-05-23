#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// �ִ� ��Ʈ ����
#define MAX_BIT 210

FILE* open_file(char* filename, char* mode, char* error_message); // ���� ���� �Լ� ����
void input_file_to_dataword(char*); // �Է� ������ ������ ����� ��ȯ�ϴ� �Լ� ����
void codeword_to_dataword(char*, unsigned char*, unsigned char*); // �ڵ� ���带 ������ ����� ��ȯ�ϴ� �Լ� ����
unsigned char* modulo_2_division(char*, unsigned char*); // ���� 2 ������ ���� �Լ� ����

// ���� ����
FILE* input_file, * output_file, * result_file;
int shift, dataword_size, error_count = 0, codeword_count = 0, output_file_size = 0;
unsigned char padding_len = 0;

// file open �Լ� 
FILE* open_file(char* filename, char* mode, char* error_message) {
    FILE* file = fopen(filename, mode); // ���� ����
    if (file == NULL) { // ���� ���� ���� ��
        printf("%s\n", error_message); // ���� �޼��� ���
    }
    return file; // ���� ������ ��ȯ
}

// main �Լ�
int main(int argc, char* argv[]) {
    // command ���� ���� Ȯ��
    if (argc != 6) {
        printf("Usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        return -1; // ���α׷� ����
    }

    // command ���ڿ��� ���ϸ�� ������, ������ ���� ũ�� ����
    char* input_file_name = argv[1];
    char* output_file_name = argv[2];
    char* result_file_name = argv[3];
    char* generator = argv[4];
    dataword_size = atoi(argv[5]); // ���ڿ��� ������ ��ȯ

    // �Է� ���� ����
    input_file = open_file(input_file_name, "rb", "input file open error.");
    if (input_file == NULL) {
        return -1; // ���α׷� ����
    }

    // ��� ���� ����
    output_file = open_file(output_file_name, "wb", "output file open error.");
    if (output_file == NULL) {
        fclose(input_file); // �Է� ���� �ݱ�
        return -1; // ���α׷� ����
    }

    // ��� ���� ����
    result_file = open_file(result_file_name, "w", "result file open error.\n");
    if (result_file == NULL) {
        fclose(input_file); // �Է� ���� �ݱ�
        fclose(output_file); // ��� ���� �ݱ�
        return -1; // ���α׷� ����
    }

    // ������ ���� ũ�� ��ȿ�� Ȯ��
    if (dataword_size != 4 && dataword_size != 8) {
        printf("Dataword size must be 4 or 8.\n");
        fclose(result_file); // ��� ���� �ݱ�
        fclose(output_file); // ��� ���� �ݱ�
        fclose(input_file); // �Է� ���� �ݱ�
        return -1; // ���α׷� ����
    }

    // �Է� ���Ϸκ��� �е� ���� �б�
    fread(&padding_len, sizeof(unsigned char), 1, input_file);

    // �Է� ������ ������ ����� ��ȯ
    input_file_to_dataword(argv[4]);

    // ��ü �Լ� ���� �� ��� ����, ��� ����, �Է� ���� ���� �� ���α׷� ����
    fclose(result_file);
    fclose(output_file);
    fclose(input_file);

    return 0;
}

// �Է� ������ ������ ����� ��ȯ�ϴ� �Լ�
void input_file_to_dataword(char* generator) {
    shift = strlen(generator) - 1; // ����Ʈ �� ����
    // �ӽ� ���� �� ���� �ʱ�ȭ, �ڵ� ���� ī��Ʈ �ʱ�ȭ, �ӽ� ���� �ʱ�ȭ, ���� ī��Ʈ �ʱ�ȭ
    unsigned char temp, dataword1[MAX_BIT] = { 0 }, dataword2[MAX_BIT] = { 0 }, read_buff[8] = { 0 };
    int codeword1_count = dataword_size + shift, codeword2_count = (dataword_size == 4) ? (dataword_size + shift) : 0;
    unsigned char tmp = 0;
    int buff_count = (output_file_size == 0) ? 8 : read_buff[8];

    while (1) {
        // ���۰� ���� á�� �� 
        if (buff_count == 8) {
            // ���Ϸκ��� 1����Ʈ �б�
            if (fread(&tmp, sizeof(unsigned char), 1, input_file) != 1) break;

            // 8��Ʈ �ݺ�
            for (int i = 0; i < buff_count; i++) {
                read_buff[buff_count - 1 - i] = (tmp % 2) ? '1' : '0'; // �������� ��ȯ�Ͽ� ���ۿ� ����
                tmp /= 2; // ���� ��Ʈ�� �̵�
            }

            // ���� ī��Ʈ �ʱ�ȭ
            buff_count = 0;

            // ��� ���� ũ�Ⱑ 0�� ���
            if (output_file_size == 0) {
                // �е� ���̸�ŭ �ݺ�
                for (int i = 0; i < padding_len; i++) {
                    buff_count++; // ���� ī��Ʈ ����
                    output_file_size++; // ��� ���� ũ�� ����
                }
            }
        }
        // ���۰� ���� ���� ���� ���
        else {
            // �ڵ� ���� 1�� ó���� ������ ���� ���
            if (codeword1_count != 0) {
                // ������ ���� 1�� ��Ʈ ����
                dataword1[--codeword1_count] = read_buff[buff_count++];
            }
            // �ڵ� ���� 1�� ó���� ���� ���
            else {
                // ������ ���� 2�� ��Ʈ ����
                dataword2[--codeword2_count] = read_buff[buff_count++];
            }
            // �ڵ� ���� ó���� ���� ���
            if (codeword1_count == 0 && codeword2_count == 0) {
                codeword_to_dataword(generator, dataword1, dataword2); // �ڵ� ���带 ������ ����� ��ȯ
                codeword1_count = dataword_size + shift; // �ڵ� ���� 1 ī��Ʈ �ʱ�ȭ
                codeword2_count = (dataword_size == 4) ? (dataword_size + shift) : 0; // �ڵ� ���� 2 ī��Ʈ �ʱ�ȭ
            }
        }
    }
    // ��� ���Ͽ� �ڵ� ���� ���� ���� �� ���
    fprintf(result_file, "%d %d", codeword_count, error_count);
}

// �ڵ� ���带 ������ ����� ��ȯ�ϴ� �Լ�
void codeword_to_dataword(char* generator, unsigned char* dataword1, unsigned char* dataword2) {
    // ���� 2 ������ ����
    dataword1 = modulo_2_division(generator, dataword1);

    // ������ ���� ũ�Ⱑ 4�� ���
    if (dataword_size == 4) {
        dataword2 = modulo_2_division(generator, dataword2); // ���� 2 ������ ����
    }

    int remainder = 0, pos = 1; // �������� ��ġ �ʱ�ȭ
    // ����Ʈ��ŭ �ݺ�
    for (int i = 0; i < shift; i++) {
        remainder += (dataword1[i] == '1') ? pos : 0; // ������ ���
        pos *= 2; // ���� ��ġ�� �̵�
    }
    codeword_count++; // �ڵ� ���� ī��Ʈ ����
    // �������� 0�� �ƴ� ���
    if (remainder != 0) { 
        error_count++; // ���� ī��Ʈ ����
    }

    // ������ ���� ũ�Ⱑ 4�� ���
    if (dataword_size == 4) {
        remainder = 0; pos = 1; // �������� ��ġ �ʱ�ȭ
        for (int i = 0; i < shift; i++) { // ����Ʈ��ŭ �ݺ�
            remainder += (dataword2[i] == '1') ? pos : 0; // ������ ���
            pos *= 2; // ���� ��ġ�� �̵�
        }
        codeword_count++; // �ڵ� ���� ī��Ʈ ����
        if (remainder != 0) { // �������� 0�� �ƴ� ���
            error_count++; // ���� ī��Ʈ ����
        }
    }

    // ���ڿ� ��ġ �ʱ�ȭ
    char ch = 0; pos = 1; 
    // ������ ���� ũ�Ⱑ 4�� ���
    if (dataword_size == 4) {  
        for (int i = shift; i < dataword_size + shift; i++) {
            ch += (dataword2[i] == '1') ? pos : 0; // ���� ���
            pos *= 2; // ���� ��ġ�� �̵�
        }
    }
    for (int i = shift; i < dataword_size + shift; i++) { // ������ ���� ũ�⸸ŭ �ݺ�
        ch += (dataword1[i] == '1') ? pos : 0; // ���� ���
        pos *= 2; // ���� ��ġ�� �̵�
    }
    fputc(ch, output_file); // ���ڸ� ��� ���Ͽ� ���
}

// ���� 2 ������ ���� �Լ�
unsigned char* modulo_2_division(char* generator, unsigned char* dataword) {
    unsigned char dataword_tmp[dataword_size + shift]; // �ӽ� ������ ���� �迭 
    memcpy(dataword_tmp, dataword, dataword_size + shift); // ������ ���� ����

    for (int i = dataword_size + shift - 1; i >= shift; i--) { // ������ ���� ũ�⸸ŭ �ݺ�
        if (dataword_tmp[i] == '1') { // ���� ��Ʈ�� 1�� ���
            for (int j = 0; j <= shift; j++) { // ����Ʈ��ŭ �ݺ�
                dataword_tmp[i - j] = (dataword_tmp[i - j] == generator[j]) ? '0' : '1'; //XOR ���� ����
            }
        }
    }

    memcpy(dataword, dataword_tmp, shift); // ��� ����
    return dataword; // ��� ��ȯ
}