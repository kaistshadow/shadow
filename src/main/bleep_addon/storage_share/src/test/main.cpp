#include "file_control.h"
#include "share/sharing_tracker.h"

//#define _TEST_NORMAL

#include <cstring>
#include <map>
#include <iostream>

#include <cassert>

#include <fcntl.h>

void clean_files() {
    char buffer[10];
    for(int i = 0; i < 100; i++) {
        sprintf(buffer, "r%d.txt", i);
        remove(buffer);
    }
}

int fopen_test() {
//    file_control fc;
//    FILE* f = fc.fopen("read_test.txt", "r");
//    assert(f == nullptr);
//    FILE* f = fc.fopen("read_test.txt", "r");
    return 0;
}
int fread_test() {
    // initial setup
    file_control fc;
    FILE* f = fc.fopen("read_test.txt", "w");
    assert(fc.ftell(f) == 0);
    fc.fwrite("1234567890", sizeof(char), 10, f);
    assert(fc.ftell(f) == 10);
    fc.fclose(f);

    char res[10];
    f = fc.fopen("read_test.txt", "r");
    assert(fc.ftell(f) == 0);
    fc.ftell(f);
    for(int i=0; i<9; i++) {
        assert(fc.fread(res, sizeof(char), 1, f) == 1);
        assert(fc.ftell(f) == (i+1));
        assert(res[0] == ('1'+i));
    }
    assert(fc.fread(res, sizeof(char), 1, f) == 1);
    assert(fc.ftell(f) == 10 && res[0] == '0');
    // read at eof
    assert(fc.fread(res, sizeof(char), 1, f) == 0);
    assert(fc.ftell(f) == 10);
    assert(fc.fread(res, sizeof(char), 2, f) == 0);
    assert(fc.ftell(f) == 10);
    assert(fc.fread(res, sizeof(char), 3, f) == 0);
    assert(fc.ftell(f) == 10);

    // go to start position
    fc.fseek(f, 0, SEEK_SET);
    assert(fc.ftell(f) == 0);
    assert(fc.fread(res, sizeof(char), 4, f) == 4);
    res[4] = 0;
    assert(memcmp(res, "1234", 4) == 0);
    assert(fc.ftell(f) == 4);
    assert(fc.fread(res, sizeof(char), 4, f) == 4);
    assert(memcmp(res, "5678", 4) == 0);
    assert(fc.ftell(f) == 8);
    assert(fc.fread(res, sizeof(char), 4, f) == 2);
    assert(memcmp(res, "9078", 4) == 0);
    assert(fc.ftell(f) == 10);

    // go out of boundary
    fc.fseek(f, 12, SEEK_SET);
    assert(fc.ftell(f) == 12);
    assert(fc.fread(res, sizeof(char), 4, f) == 0);
    assert(memcmp(res, "9078", 4) == 0);
    assert(fc.ftell(f) == 12);

}

size_t parse_bitcoin_block(unsigned char* data, size_t size) {
    size_t idx = 0;
    // magic byte (4 byte)
    unsigned char magic0[4] = {0xf9, 0xbe, 0xb4, 0xd9};
    unsigned char magic1[4] = {0x0b, 0x11, 0x09, 0x07};
    unsigned char magic2[4] = {0xfa, 0xbf, 0xb5, 0xda};
    if (!(memcmp(magic0, &data[idx], 4) || memcmp(magic1, &data[idx], 4) || memcmp(magic2, &data[idx], 4))) {
        // error case
        return 0;
    }
    idx += 4;
    // size (4 byte)
    size_t block_length = 0;
    for (int i = 3; i >= 0; i--) {
        block_length = block_length * 256 + (data[idx + i]);
    }
    printf("%ld\n", block_length + 8);
    return block_length + 8;
}

void bitcoin_block_parse_test() {
    FILE* f = fopen("blk00000.dat", "rb");
    unsigned char b[1024];
    size_t readbyte = fread(b, sizeof(unsigned char), 1024, f);
    fclose(f);
    size_t s = parse_bitcoin_block(b, readbyte);

    parse_bitcoin_block(&b[s], readbyte - s);
}

int main() {
//    clean_files();
//    fread_test();
//    bitcoin_block_parse_test();
    FILE* f = fopen("t1.txt", "w");
    int res = posix_fallocate(fileno(f), 0, 10);
    printf("res: %d\n", ftell(f));
    fclose(f);

    f = fopen("t2.txt", "w+");
    fwrite("11", sizeof(char), 2, f);
    res = posix_fallocate(fileno(f), 4, 10);
    printf("res: %d\n", ftell(f));
    fclose(f);

    f = fopen("t3.txt", "w+");
    fwrite("11111111111111111111", sizeof(char), 20, f);
    res = posix_fallocate(fileno(f), 4, 10);
    printf("res: %d\n", ftell(f));
    fclose(f);

    f = fopen("t4.txt", "w+");
    fwrite("1111", sizeof(char), 4, f);
    res = posix_fallocate(fileno(f), 4, 10);
    printf("res: %d\n", ftell(f));
    fclose(f);

//#ifndef _TEST_NORMAL
//	file_control file_controller;
//    printf("dataseg\n");
//    FILE* f = file_controller.fopen("test.txt", "w+");
//    file_controller.fwrite("test", sizeof(char), 4, f);
//    printf("ftell after write 'test':%d\n\n", file_controller.ftell(f));
//    file_controller.fflush(f);
//
//    printf("fseek returns %d\n", file_controller.fseek(f, 1, SEEK_SET));
//    printf("fseek 1 from start ftell:%d\n\n", file_controller.ftell(f));
//
//    printf("fseek returns %d\n", file_controller.fseek(f, -1, SEEK_CUR));
//    printf("fseek -1 from current ftell:%d\n\n", file_controller.ftell(f));
//
//    printf("fseek returns %d\n", file_controller.fseek(f, 4, SEEK_END));
//    printf("fseek 4 from end of file ftell:%d\n\n", file_controller.ftell(f));
//
//    char read_buffer[15];
//    memset(read_buffer, 'a', sizeof(char)*15);
//    read_buffer[14] = 0;
//    file_controller.fread(read_buffer, sizeof(char), 4, f);
//    printf("fread:%s\n", read_buffer);
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("fseek:%d\n", file_controller.fseek(f, -1, SEEK_END));
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("fseek:%d\n", file_controller.fseek(f, 2, SEEK_END));
//    printf("ftell:%d\n", file_controller.ftell(f));
//    file_controller.fwrite("bbbb", sizeof(char), 4, f);
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("fseek:%d\n", file_controller.fseek(f, 2, SEEK_END));
//    printf("ftell:%d\n", file_controller.ftell(f));
//
//    memset(read_buffer, 'a', sizeof(char)*15);
//    read_buffer[14] = 0;
//    printf("fseek:%d\n", file_controller.fseek(f, 0, SEEK_SET));
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("freadcnt:%d\n", file_controller.fread(read_buffer, sizeof(char), 14, f));
//    printf("fread:\n");
//    for(int i=0; i<14; i++) {
//        printf("%d:%c\n", i, read_buffer[i]);
//    }
//    printf("ftell:%d\n", file_controller.ftell(f));
//
//    file_controller.fclose(f);
//
//
//    f = file_controller.fopen("a.txt", "a+");
//    file_controller.fwrite("1234", sizeof(char), 4, f);
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("fseek:%d\n", file_controller.fseek(f, 0, SEEK_SET));
//    printf("ftell:%d\n", file_controller.ftell(f));
//    file_controller.fwrite("5", sizeof(char), 1, f);
//    printf("ftell:%d\n", file_controller.ftell(f));
//    printf("fseek:%d\n", file_controller.fseek(f, 7, SEEK_SET));
//    printf("ftell:%d\n", file_controller.ftell(f));
//    file_controller.fwrite("6", sizeof(char), 1, f);
//    printf("ftell:%d\n", file_controller.ftell(f));
//    file_controller.fclose(f);
//#else
//    printf("original\n");
//    FILE* f = fopen("test.txt", "w+");
//    fwrite("test", sizeof(char), 4, f);
//    printf("ftell:%d\n", ftell(f));
//    fflush(f);
//    printf("fseek:%d\n", fseek(f, 1, SEEK_SET));
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, -1, SEEK_CUR));
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, 4, SEEK_END));
//    printf("ftell:%d\n", ftell(f));
//    char read_buffer[15];
//    memset(read_buffer, 'a', sizeof(char)*15);
//    read_buffer[14] = 0;
//    fread(read_buffer, sizeof(char), 4, f);
//    printf("fread:%s\n", read_buffer);
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, -1, SEEK_END));
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, 2, SEEK_END));
//    printf("ftell:%d\n", ftell(f));
//    fwrite("bbbb", sizeof(char), 4, f);
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, 2, SEEK_END));
//    printf("ftell:%d\n", ftell(f));
//
//    memset(read_buffer, 'a', sizeof(char)*15);
//    read_buffer[14] = 0;
//    printf("fseek:%d\n", fseek(f, 0, SEEK_SET));
//    printf("ftell:%d\n", ftell(f));
//    printf("freadcnt:%d\n", fread(read_buffer, sizeof(char), 14, f));
//    printf("fread:\n");
//    for(int i=0; i<14; i++) {
//        printf("%d:%c\n", i, read_buffer[i]);
//    }
//    printf("ftell:%d\n", ftell(f));
//
//    fclose(f);
//
//    remove("a.txt");
//    f = fopen("a.txt", "a+");
//    fwrite("1234", sizeof(char), 4, f);
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, 0, SEEK_SET));
//    printf("ftell:%d\n", ftell(f));
//    fwrite("5", sizeof(char), 1, f);
//    printf("ftell:%d\n", ftell(f));
//    printf("fseek:%d\n", fseek(f, 7, SEEK_SET));
//    printf("ftell:%d\n", ftell(f));
//    fwrite("6", sizeof(char), 1, f);
//    printf("ftell:%d\n", ftell(f));
//    fclose(f);
//
//#endif
    return 0;
}
