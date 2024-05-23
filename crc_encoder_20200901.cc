#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BIT 210 // �ִ� ��Ʈ ����

FILE* open_file(char* filename, char* mode, char* error_message); // ������ ���� ���� �����͸� ��ȯ�ϴ� �Լ�
int get_file_size(FILE* file); // ���� ũ�⸦ ���ϴ� �Լ�
unsigned char calculate_padding_length(int file_size, int dataword_size, int generator_length); // �е� ���̸� ����ϴ� �Լ�
void write_binary(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2); // ���� �����͸� ���Ͽ� ���� �Լ�
void write_codewords(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2, int* buff_count); // �ڵ� ���带 ���Ͽ� ���� �Լ�

// ������ ó�� ���� �Լ�
void fill_dataword(unsigned char* dataword, unsigned char value, int size, int shift_bits);
void process_byte_to_datawords(char ch, unsigned char* dataword1, unsigned char* dataword2, int dataword_size);
unsigned char* modulo_2_division(const char* generator, unsigned char* dataword, int size);
void dataword_to_codeword(const char* generator, unsigned char* dataword1, unsigned char* dataword2);

// ���� ���ڵ� �Լ�
void input_file_to_codeword(char* generator);

FILE* input_file; // �Է� ���� ������ ����
FILE* output_file; // ��� ���� ������ ����

int shift; // ����Ʈ ��Ʈ �� ����
int dataword_size; // ������ ���� ũ�� ����
int input_file_size; // �Է� ���� ũ�� ����
int output_file_size = 0; // ��� ���� ũ�� �ʱ�ȭ
unsigned char padding_len = 0; // �е� ���� �ʱ�ȭ

// ������ ���� ���� �����͸� ��ȯ�ϴ� �Լ�
FILE* open_file(char* filename, char* mode, char* error_message) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        printf("%s\n", error_message); // ���� �޼��� ���
    }
    return file; // ���� ������ ��ȯ
}

// ���� ũ�⸦ ����ϴ� ��ƿ��Ƽ �Լ�
int get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END); // ���� ������ �̵�
    int size = ftell(file); // ���� ũ�� ���
    rewind(file);
    return size; // ���� ũ�� ��ȯ
}

// �е� ���̸� ����ϴ� ��ƿ��Ƽ �Լ�
unsigned char calculate_padding_length(int file_size, int dataword_size, int generator_length) {
    int bits_per_codeword; // �ڵ� ����� ��Ʈ ��

    // ������ ���� ũ�Ⱑ 4�� ���
    if (dataword_size == 4) {
        bits_per_codeword = 2 * (dataword_size + generator_length - 1); // �ڵ� ����� ��Ʈ �� ���
    }
    // �� ���� ���
    else {
        bits_per_codeword = dataword_size + generator_length - 1; // �ڵ� ����� ��Ʈ �� ���
    }
    // �е� ���� ��� �� ��ȯ
    return (8 - (bits_per_codeword * file_size) % 8) % 8;
}

int main(int argc, char* argv[]) {
    // command ���� ���� Ȯ��
    if (argc != 5) {
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        return -1; // ���� �޼��� ��� �� ���α׷� ����
    }

    char* input_file_name = argv[1]; // �Է� ���ϸ�
    char* output_file_name = argv[2]; // ��� ���ϸ�
    char* generator = argv[3]; // ������
    dataword_size = atoi(argv[4]); // ������ ���� ũ�⸦ ������ ��ȯ

    // �Է� ����
    input_file = open_file(input_file_name, "rb", "input file open error.");
    if (input_file == NULL) {
        return -1;
    }

    // ��� ����
    output_file = open_file(output_file_name, "wb", "output file open error.");
    if (output_file == NULL) {
        fclose(input_file);
        return -1;
    }

    // ������ ���� ũ�� ��ȿ�� �˻�
    if (dataword_size != 4 && dataword_size != 8) {
        printf("dataword size must be 4 or 8.\n");
        fclose(input_file);
        fclose(output_file);
        return -1;
    }

    // �Է� ���� ũ��
    int file_size = get_file_size(input_file);
    // �е� ���� ���
    padding_len = calculate_padding_length(file_size, dataword_size, (int)strlen(generator));
    // �е� ���̸� ��� ���Ͽ� ��
    fwrite(&padding_len, sizeof(unsigned char), 1, output_file);

    // �Է� ������ �ڵ� ����� ��ȯ
    input_file_to_codeword(generator);

    // ��� ����, �Է� ���� ���� �� ���α׷� ����
    fclose(output_file);
    fclose(input_file);

    return 0;
}

// ������ ���带 ä��� �Լ�
void fill_dataword(unsigned char* dataword, unsigned char value, int size, int shift_bits) {
    for (int i = 0; i < size; i++) { // ������ ���� ũ�⸸ŭ �ݺ�
        dataword[i] = ((value & 0b1) == 0b1 ? '1' : '0'); // ������ ���忡 ��Ʈ ä���
        value = value >> 1; // ���� ��Ʈ�� �̵�
    }
    for (int i = size; i < size + shift_bits; i++) { // ����Ʈ ��Ʈ��ŭ 0���� ä��
        dataword[i] = '0';
    }
}

// ����Ʈ�� ������ ����� ��ȯ�ϴ� �Լ�
void process_byte_to_datawords(char ch, unsigned char* dataword1, unsigned char* dataword2, int dataword_size) {
    unsigned char tmp = (unsigned char)ch;

    // ������ ���� ũ�Ⱑ 4�� ���
    if (dataword_size == 4) {
        fill_dataword(dataword1, tmp >> 4, dataword_size, shift); // ���� 4��Ʈ�� ������ ���忡 ä��
        fill_dataword(dataword2, tmp & 0x0F, dataword_size, shift); // ���� 4��Ʈ�� ������ ���忡 ä��
    }
    // �� ���� ���
    else {
        fill_dataword(dataword1, tmp, dataword_size, shift); // ������ ���忡 ����Ʈ ä��
    }
}

// ���� 2 �������� �����ϴ� �Լ�
unsigned char* modulo_2_division(const char* generator, unsigned char* dataword, int size) {
    unsigned char dataword_tmp[MAX_BIT] = { 0 }; // �ӽ� ������ ���� �迭 �ʱ�ȭ

    memcpy(dataword_tmp, dataword, size + shift); // ������ ���� ����

    for (int i = size + shift - 1; i >= shift; i--) { // ������ ���� ũ�⸸ŭ �ݺ�
        if (dataword_tmp[i] == '1') { // ���� ��Ʈ�� 1�� ���
            for (int j = 0; j <= shift; j++) { // ����Ʈ��ŭ �ݺ�
                dataword_tmp[i - j] = (dataword_tmp[i - j] == generator[j]) ? '0' : '1'; // XOR ���� ����
            }
        }
    }

    memcpy(dataword, dataword_tmp, shift); // ��� ����
    return dataword; // ��� ��ȯ
}

// ������ ���带 �ڵ� ����� ��ȯ�ϴ� �Լ�
void dataword_to_codeword(const char* generator, unsigned char* dataword1, unsigned char* dataword2) {
    for (int i = dataword_size - 1; i >= 0; i--) { // ������ ���� ũ�⸸ŭ �ݺ�
        dataword1[i + shift] = dataword1[i]; // ������ ���� 1�� ����Ʈ
    }
    for (int i = 0; i < shift; i++) { // ����Ʈ ��Ʈ��ŭ 0���� �ʱ�ȭ
        dataword1[i] = '0';
    }
    dataword1 = modulo_2_division(generator, dataword1, dataword_size); // ���� 2 ������ ����

    if (dataword_size == 4) { // ������ ���� ũ�Ⱑ 4�� ���
        for (int i = dataword_size - 1; i >= 0; i--) { // ������ ���� ũ�⸸ŭ �ݺ�
            dataword2[i + shift] = dataword2[i]; // ������ ���� 2�� ����Ʈ
        }
        for (int i = 0; i < shift; i++) { // ����Ʈ ��Ʈ��ŭ 0���� �ʱ�ȭ
            dataword2[i] = '0';
        }
        dataword2 = modulo_2_division(generator, dataword2, dataword_size); // ���� 2 ������ ����
    }
}

// ���� �����͸� ���Ͽ� ���� �Լ�
void write_codewords(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2, int* buff_count) {
    int remaining_codeword1 = strlen((char*)codeword1); // �ڵ� ���� 1�� ����
    int remaining_codeword2 = strlen((char*)codeword2); // �ڵ� ���� 2�� ����

    while (remaining_codeword1 > 0 || remaining_codeword2 > 0) { // �ڵ� ���尡 ���� �ִ� ���� �ݺ�
        if (remaining_codeword1 > 0) { // �ڵ� ���� 1�� ���� �ִ� ���
            write_buff[(*buff_count)++] = codeword1[--remaining_codeword1]; // ���ۿ� �ڵ� ���� 1 �߰�
        }
        else if (remaining_codeword2 > 0) { // �ڵ� ���� 2�� ���� �ִ� ���
            write_buff[(*buff_count)++] = codeword2[--remaining_codeword2]; // ���ۿ� �ڵ� ���� 2 �߰�
        }

        if (*buff_count == 8) { // ���۰� �� �� ���
            unsigned char tmp = 0; // �ӽ� ���� �ʱ�ȭ
            for (int i = 0, pos = 1; i < 8; i++, pos *= 2) { // 8��Ʈ�� ���� �ݺ�
                tmp += (write_buff[7 - i] == '1') ? pos : 0; // ���� �����͸� ������ ��ȯ�Ͽ� �ӽ� ������ ����
            }

            fwrite(&tmp, sizeof(unsigned char), 1, output_file); // ���Ͽ� ���� ������ ����
            *buff_count = 0; // ���� ī��Ʈ �ʱ�ȭ
        }
    }
}

// ���� �����͸� ���Ͽ� ���� �Լ�
void write_binary(unsigned char* write_buff, unsigned char* codeword1, unsigned char* codeword2) {
    static int buff_count = 0; // ���� ���� ���� �� �ʱ�ȭ

    if (output_file_size == 0) { // ��� ���� ũ�Ⱑ 0�� ���
        for (int i = 0; i < padding_len; i++) { // �е� ���̸�ŭ �ݺ�
            write_buff[buff_count++] = '0'; // ���ۿ� 0 �߰�
            output_file_size++; // ��� ���� ũ�� ����
        }
    }

    write_codewords(write_buff, codeword1, codeword2, &buff_count); // �ڵ� ���带 ���Ͽ� ���� �Լ� ȣ��
    write_buff[8] = buff_count; // ���ۿ� �ִ� ������ �� ����
}

// �Է� ������ �ڵ� ����� ��ȯ�ϴ� �Լ�
void input_file_to_codeword(char* generator) {
    shift = strlen(generator) - 1; // ����Ʈ ��Ʈ �� ���
    char ch; // ���� ���� ����
    unsigned char dataword1[MAX_BIT] = { 0 }; // ������ ���� 1 �迭 �ʱ�ȭ
    unsigned char dataword2[MAX_BIT] = { 0 }; // ������ ���� 2 �迭 �ʱ�ȭ
    unsigned char write_buff[9] = { 0 }; // ���� ���� �迭 �ʱ�ȭ

    // ���� ������ �ݺ�
    while ((ch = fgetc(input_file)) != EOF) {
        // ����Ʈ�� ������ ����� ��ȯ
        process_byte_to_datawords(ch, dataword1, dataword2, dataword_size);
        // ������ ���带 �ڵ� ����� ��ȯ
        dataword_to_codeword(generator, dataword1, dataword2);
        // ���� �����͸� ���Ͽ� ����
        write_binary(write_buff, dataword1, dataword2);
    }
}