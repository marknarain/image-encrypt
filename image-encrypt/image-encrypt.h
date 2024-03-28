/*
    Copyright (c) 2024 Mark Narain Enzinger

    MIT License (https://github.com/marknarain/image-encrypt/blob/main/LICENSE)
*/

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <random>
#include <cstdint>
#include <cstdio>

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define AES_KEY_SIZE 32

#pragma pack(push, 1)

struct BMPFileHeader
{
    uint16_t file_type{ 0x4D42 };               // File type always BM which is 0x4D42
    uint32_t file_size{ 0 };                    // Size of the file (in bytes)
    uint16_t reserved1{ 0 };                    // Reserved, always 0
    uint16_t reserved2{ 0 };                    // Reserved, always 0
    uint32_t offset_data{ 0 };                  // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader
{
    uint32_t size{ 0 };                         // Size of this header (in bytes)
    int32_t width{ 0 };                         // width of bitmap in pixels
    int32_t height{ 0 };                        // width of bitmap in pixels
    //       (if positive, bottom-up, with origin in lower left corner)
    //       (if negative, top-down, with origin in upper left corner)
    uint16_t planes{ 1 };                       // No. of planes for the target device, this is always 1
    uint16_t bit_count{ 0 };                    // No. of bits per pixel
    uint32_t compression{ 0 };                  // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
    uint32_t size_image{ 0 };                   // 0 - for uncompressed images
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };                  // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t colors_important{ 0 };             // No. of colors used for displaying the bitmap. If 0 all colors are required
};

#pragma pack(pop)

struct BMP
{
    BMP(std::string fname);
    void encrypt(std::string fname, int encryption_type);
    void decrypt(std::string fname, int encryption_type);
    void read_text_from_file(std::string fname);
    void write_text_out(std::string fname);
    void write_text_to_img_data();
    void read_text_from_img_data();
    void write_image_out(std::string fname);
    void generate_key();
    void read_key();
    void encrypt_decrypt_data();
    void generate_aes_key();
    void read_aes_key();
    void aes_encrypt();
    void aes_decrypt();

    private:
        // Data from the BMP file
        BMPFileHeader file_header;
        BMPInfoHeader info_header;
        std::vector<uint8_t> img_data;
        uint8_t padding;

        // Text to encrypt/decrypt
        std::vector<uint8_t> text;

        // Keys for encryption/decryption
        uint64_t key;
        std::vector<uint8_t> aes_key;
};