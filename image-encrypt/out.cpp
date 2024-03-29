/*
    Copyright (c) 2024 Mark Narain Enzinger

    MIT License (https://github.com/marknarain/image-encrypt/blob/main/LICENSE)
*/

#include "image-encrypt.h"

// CRC32 Table
static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// Function to calculate CRC32 checksum
uint32_t calculate_crc32(const std::vector<uint8_t>& data)
{
    uint32_t crc = 0xFFFFFFFF; // Initialize CRC with all bits set to 1

    for (uint8_t byte : data)
    {
        crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ byte];
    }

    return crc ^ 0xFFFFFFFF; // Final XOR operation
}

void error(const std::string& message) 
{
    std::cout << message << "\a\n" << "Press enter to exit ...";

    char ch;

    // Read a character using cin.get()
    std::cin.get(ch);

    // Consume any remaining newline 
    if (std::cin.peek() == '\n') 
    {
        std::cin.get(); // Discard the newline character
    }

    std::cout << "Exiting...\n";
    exit(1);
}

/// <summary>
/// Constructor for the BMP struct. Reads the BMP file and the headers from the file.
/// </summary>
/// <param name="fname">: The name of the BMP file</param>
BMP::BMP(std::string fname)
{
    std::ifstream file(fname, std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the input image file.");
    }

    // Read the file and the info headers
    file.read((char*)&file_header, sizeof(file_header));
    file.read((char*)&info_header, sizeof(info_header));

    if (info_header.bit_count != 24 && info_header.bit_count != 32)
    {
        error("The program can treat only 24 or 32 bits per pixel BMP files");
    }

    // Look for the number of colors in the color table
    if (info_header.colors_used != 0)
    {
        error("The program can read only BMP files with 24-bit or 32-bit color depth without color table");
    }

    if (info_header.width < 0 || info_header.height < 0)
    {
        error("The program can treat only BMP files with the origin in the bottom left corner");
    }

    if (file_header.offset_data != 54)
    {
        error("The program can read only BMP files with header size of 54 bytes");
    }

    int width = info_header.width;
    int height = info_header.height;
    int byte_count = info_header.bit_count / 8;

    // Calculate the padding for each row in the image. There are zeroes at the end of each row for line break. 
    // The padding is necessary to ensure that the data of each row starts at a multiple of 4 bytes.
    padding = ((-width & byte_count));

    // For each row of pixels, there is a width and the number of zeroes at the end of the row. 
    int data_size = (width * height * byte_count) + (padding * height);

    img_data.resize(data_size);

    file.seekg(file_header.offset_data, file.beg);

    for (int i = 0; i < height; i++)
    {
        // Read the data row by row
        file.read((char*)(img_data.data() + (width * byte_count + padding) * i), width * byte_count);

        // Skip the padding. Cur means that the seekg should happen relative to the current position in the file.
        file.seekg(padding, std::ios_base::cur);
    }

    file.close();

    // We initialize the key, so that we can check if it was generated later.
    key = 0;
}

/// <summary>
/// Encrypts the text from the file and writes it to the image
/// </summary>
/// <param name="fname">: The name of the BMP-File to encrypt</param>
/// <param name="encryption_type">: The type of encryption to use</param>
void BMP::encrypt(std::string fname, int encryption_type)
{
    read_text_from_file(fname);

    switch (encryption_type)
    {
    case 1:
        generate_aes_key();
        aes_encrypt();
        break;
    case 2:
        generate_key();
        encrypt_decrypt_data();
        break;
    case 3:
		break;
    default:
        error("Invalid encryption type");
    }

    write_text_to_img_data();
    write_image_out("encrypted.bmp");
}

/// <summary>
/// Decrypts the text from the image and writes it to a file
/// </summary>
/// <param name="fname">: The Name of the BMP-File to be decrypted</param>
/// <param name="encryption_type">: The type of encryption to use</param>
void BMP::decrypt(std::string fname, int encryption_type)
{
    read_text_from_img_data();

    switch (encryption_type)
    {
    case 1:
        read_aes_key();
        aes_decrypt();
        break;
    case 2:
        read_key();
        encrypt_decrypt_data();
        break;
    case 3:
        break;
    default:
        error("Invalid encryption type");
    }

    write_text_out(fname);
}

/// <summary>
/// Read text from file and store it in the text variable
/// </summary>
/// <param name="fname">The text file from which the text is to be read</param>
void BMP::read_text_from_file(std::string fname)
{
    std::ifstream file(fname, std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the input file.");
    }

    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    text.resize(length);
    file.read((char*)text.data(), text.size());

    file.close();
}

/// <summary>
/// Write the text to a file
/// </summary>
/// <param name="fname">The name of the file to write the text to</param>
void BMP::write_text_out(std::string fname)
{
    std::ofstream file(fname, std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the output file.");
    }

    file.write((const char*)text.data(), text.size());

    file.close();
}

/// <summary>
/// Write the text to the image data bitwise
/// </summary>
void BMP::write_text_to_img_data()
{
    uint32_t text_size = text.size();
    uint32_t data_size = img_data.size();
    uint32_t max_text_size = (data_size - 64) / 8;

    // We write the size of the text in the first 32 bytes of the image data
    for (uint32_t i = 0; i < 32; i++)
    {
        uint8_t bit = (text_size >> i) & 1;

        img_data[i] &= ~1;
        img_data[i] |= bit;
    }

    if (text_size > max_text_size)
    {
        error("The text is to large for the image");
    }

    // Every bit of the text is written in the image data
    for (uint32_t i = 0; i < text_size; i++)
    {
        uint8_t byte = text[i];
        for (uint32_t j = 0; j < 8; j++)
        {
            uint8_t bit = byte & 1;
            byte = byte >> 1;

            img_data[i * 8 + j + 32] &= ~1;
            img_data[i * 8 + j + 32] |= bit;
        }
    }

    // Fill the rest of the image data with random data
    // Always change the lowest bit to ensure that the image data is different from the original image
    for (uint32_t i = text_size * 8 + 32; i < data_size - 32; i++)
    {
        img_data[i] |= (rand() & 1);
    }

    // Calculate the CRC32 checksum of the image data until data_size - 32
    uint32_t crc = calculate_crc32(std::vector<uint8_t>(img_data.begin(), img_data.begin() + data_size - 32));

    // Write the CRC32 checksum to the last 32 bits of the image data
    for (uint32_t i = data_size - 32; i < data_size; i++)
    {
        uint8_t bit = (crc >> (i - data_size + 32)) & 1;

        img_data[i] &= ~1;
        img_data[i] |= bit;
    }
}

/// <summary>
/// Read the text from the image data bitwise
/// </summary>
void BMP::read_text_from_img_data()
{
    uint32_t text_size = 0;
    uint32_t data_size = img_data.size();

    // Read the first 32 bits from the image data to determin how long the stored data in the image is.
    for (uint32_t i = 0; i < 32; i++)
    {
        text_size |= ((img_data[i] & 1) << i);
    }

    uint32_t crc_read = calculate_crc32(std::vector<uint8_t>(img_data.begin(), img_data.begin() + data_size - 32));
    uint32_t crc_expected = 0;

    // Read the last 32 bits from the image data to get the CRC32 checksum
    for (uint32_t i = data_size - 32; i < data_size; i++)
    {
        crc_expected |= ((img_data[i] & 1) << (i - data_size + 32));
    }

    if (crc_read != crc_expected)
    {
        error("The data in the image is corrupted");
    }

    text.resize(text_size);

    for (uint32_t i = 0; i < text_size; i++)
    {
        uint8_t byte = 0;
        for (uint32_t j = 0; j < 8; j++)
        {
            byte |= ((img_data[i * 8 + j + 32] & 1) << j);
        }
        text[i] = byte;
    }
}

/// <summary>
/// Write the image data to a file
/// </summary>
/// <param name="fname">The name of the file to write the image data to</param>
void BMP::write_image_out(std::string fname)
{
    std::ofstream file(fname, std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the output image file.");
    }

    file.write((const char*)&file_header, sizeof(file_header));
    file.write((const char*)&info_header, sizeof(info_header));

    for (int i = 0; i < info_header.height; i++)
    {
        file.write((const char*)(img_data.data() + (info_header.width * 3 + padding) * i), info_header.width * 3);
        file.seekp(padding, std::ios_base::cur);
    }

    file.close();
}

/**********************************************************************
*
* Functions for generating/reading keys and encrypting/decrypting
* vector<uint8_t> using encryption with XOR
*
**********************************************************************/

/// <summary>
/// Generates a random 64-bit key and writes it to a file
/// </summary>
void BMP::generate_key()
{
    srand(time(0));
    std::ofstream file("key", std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the output text file.");
    }

    uint8_t mask = 0xFF;
    for (int i = 0; i < 64; i += 8)
    {
        uint16_t current = rand();
        uint8_t byte = (current >> 8) & mask;
        key = (key << 8) | byte;
        file.write((const char*)&byte, sizeof(byte));
    }

    file.close();
}

/// <summary>
/// Reads a 64-bit key from a file and stores it in the key variable
/// </summary>
void BMP::read_key()
{
    std::ifstream file("key", std::ios_base::binary);
    if (!file)
    {
        error("Unable to open the key file.");
    }

    uint8_t mask = 0xFF;
    for (int i = 0; i < 64; i += 8)
    {
        uint8_t byte;
        file.read((char*)&byte, sizeof(byte));
        key = (key << 8) | byte;
    }

    file.close();
}

/// <summary>
/// Encrypts/decrypts the text using the key variable using XOR
/// </summary>
void BMP::encrypt_decrypt_data()
{
    for (int i = 0; i < text.size(); i++)
    {
        text[i] = text[i] ^ (key & 0xFF);
        key = (key >> 8) | (key << 56);
    }
}

/**********************************************************************
*
* Functions for generating/reading keys and encrypting/decrypting
* vector<uint8_t> using encryption with AES algorithm
*
**********************************************************************/

/// <summary>
/// Generates a random 256-bit AES key and writes it to a file
/// </summary>
void BMP::generate_aes_key()
{
    aes_key.resize(AES_KEY_SIZE);
    if (RAND_bytes(aes_key.data(), AES_KEY_SIZE) != 1)
    {
        error("Failed to generate random key using OpenSSL RAND_bytes");
    }

    // Write the generated key to a file
    std::ofstream file("aes_key", std::ios_base::binary);
    if (!file)
    {
        error("Failed to open file for writing AES key");
    }
    file.write((const char*)aes_key.data(), aes_key.size());
    file.close();
}

/// <summary>
/// Reads a 256-bit AES key from a file and stores it in the aes_key variable
/// </summary>
void BMP::read_aes_key()
{
    std::ifstream file("aes_key", std::ios_base::binary);
    if (!file)
    {
        error("Failed to open file for reading AES key");
    }

    aes_key.resize(AES_KEY_SIZE);
    file.read((char*)aes_key.data(), aes_key.size());
    file.close();
}

/// <summary>
/// Encrypts the text using the AES key stored in the aes_key variable using AES-256 encryption
/// </summary>
void BMP::aes_encrypt()
{
    // Check if the key length is appropriate
    if (aes_key.size() != 16 && aes_key.size() != 24 && aes_key.size() != 32)
    {
        error("Key length must be 16, 24, or 32 bytes.");
    }

    // Initialize OpenSSL cipher context
    EVP_CIPHER_CTX* ctx;
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        error("Error creating EVP cipher context.");
    }

    // Initialize AES-256 encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_key.data(), NULL) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error initializing AES encryption.");
    }

    // Prepare output buffer
    std::vector<uint8_t> ciphertext(text.size() + EVP_MAX_BLOCK_LENGTH);

    // Encrypt plaintext
    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, text.data(), text.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error performing AES encryption.");
    }

    // Finalize encryption
    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error finalizing AES encryption.");
    }

    ciphertext_len += len;

    // Resize the output buffer to actual ciphertext size
    ciphertext.resize(ciphertext_len);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    text = ciphertext;
}

/// <summary>
/// Decrypts the text using the AES key stored in the aes_key variable using AES-256 decryption
/// </summary>
void BMP::aes_decrypt()
{
    // Check if the key length is appropriate
    if (aes_key.size() != 16 && aes_key.size() != 24 && aes_key.size() != 32)
    {
        error("Key length must be 16, 24, or 32 bytes.");
    }

    // Initialize OpenSSL cipher context
    EVP_CIPHER_CTX* ctx;
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        error("Error creating EVP cipher context.");
    }

    // Initialize AES-256 decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_key.data(), NULL) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error initializing AES decryption.");
    }

    // Prepare output buffer
    std::vector<uint8_t> plaintext(text.size() + EVP_MAX_BLOCK_LENGTH);

    // Decrypt ciphertext
    int len;
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, text.data(), text.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error performing AES decryption.");
    }

    // Get the length of the plaintext produced by the update operation
    int plaintext_len = len;

    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        error("Error finalizing AES decryption.");
    }

    // Update the total plaintext length
    plaintext_len += len;

    // Resize the output buffer to actual plaintext size
    plaintext.resize(plaintext_len);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    text = plaintext;
}