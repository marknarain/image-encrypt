#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <random>

#include <openssl/aes.h>
#include <openssl/rand.h>

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

void error(const std::string& message)
{
	int err;
    std::cout << message << "\n" << "Press any key and then enter to exit ...";
    std::cin >> err;
    std::cout << "Exiting...\n";
    exit(1);
}

struct BMP 
{
    /// <summary>
    /// Constructor for the BMP class. Reads the BMP file and the headers from the file.
    /// </summary>
    /// <param name="fname">: The name of the BMP file</param>
    BMP(std::string fname)
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
        // Each row must have an even number of bytes. Thats why there are 2 zeroes if the number of pixels per row is even and one if it is odd.
        int new_line_len = width % 2 == 0 ? 2 : 1;

        // For each row of pixels, there is a width and the number of zeroes at the end of the row. 
        int data_size = (width * height * byte_count) + (new_line_len * height);

        img_data.resize(data_size);

        file.seekg(file_header.offset_data, file.beg);
        file.read((char*)img_data.data(), img_data.size());

        file.close();

        // We initialize the key, so that we can check if it is generated.
        key = 0;
    }

    /// <summary>
    /// Encrypts the text from the file and writes it to the image
    /// </summary>
    /// <param name="fname">: The name of the BMP-File to encrypt</param>
    void encrypt(std::string fname)
    {
        read_text_from_file(fname);
        
        //generate_key();
        
        generate_aes_key();
        // Here the text is encrypted using the key from 'key'
        
        //encrypt_decrypt_data();
        
        aes_encrypt();
        
        write_text_to_img_data();
        write_image_out("encrypted.bmp");
    }

    /// <summary>
    /// Decrypts the text from the image and writes it to a file
    /// </summary>
    /// <param name="fname">: The Name of the BMP-File to be decrypted</param>
    void decrypt(std::string fname)
    {
		//read_key();
		
        read_aes_key();
        
        read_text_from_img_data();
        
        // Here the text is decrypted using the key from 'key' 
        
        //encrypt_decrypt_data();
        
        aes_decrypt();

        write_text_out("decrypted.txt");
	}

    /// <summary>
    /// Read text from file and store it in the text variable
    /// </summary>
    /// <param name="fname">The text file from which the text is to be read</param>
    void read_text_from_file(std::string fname)
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
    void write_text_out(std::string fname)
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
    /// <param name="fname">The name of the file to write the text to</param>
    void write_text_to_img_data()
    {
        uint32_t text_size = text.size();
        uint32_t data_size = img_data.size();

        // We write the size of the text in the first 32 bytes of the image data
        for (int i = 0; i < 32; i++)
        {
			uint8_t bit = (text_size >> i) & 1;
			img_data[i] = (img_data[i] & 0xFE) | bit;
		}

        // Every bit of the text is written in the image data
        for (uint32_t i = 32; i < text_size + 32; i++)
        {
            uint8_t byte = text[i - 32];
            for (uint32_t j = 0; j < 8; j++)
            {
                if ((i * 8 + j) > data_size - 1)
                {
                    error("The text is to large for the image");
                }

                uint8_t bit = byte & 1;
                byte = byte >> 1;  

                img_data[i * 8 + j] = (img_data[i * 8 + j] & 0xFE) | bit;
            }
        }
    }

    /// <summary>
    /// Read the text from the image data bitwise
    /// </summary>
    /// <param name="fname">The name of the file to write the text to</param>
    void read_text_from_img_data()
    {
        uint32_t text_size = 0;
        uint32_t data_size = img_data.size();

        // Read the first 32 bits from the image data to determin how long the stored data in the image is.
        for (uint32_t i = 0; i < 32; i++)
        {
			text_size = text_size | ((img_data[i] & 1) << i);
		}

		text.resize(text_size + 32);

        for (uint32_t i = 32; i < text_size + 32; i++)
        {
            for (uint32_t j = 0; j < 8; j++)
            {
                text[i - 32] = text[i - 32] | (((img_data[i * 8 + j] & 1)) << j);
			}
		}
    }
    
    /// <summary>
    /// Write the image data to a file
    /// </summary>
    /// <param name="fname">The name of the file to write the image data to</param>
    void write_image_out(std::string fname)
    {
        std::ofstream file(fname, std::ios_base::binary);
		if (!file)
		{
			error("Unable to open the output image file.");
		}

		file.write((const char*)&file_header, sizeof(file_header));
		file.write((const char*)&info_header, sizeof(info_header));
        file.write((const char*)img_data.data(), img_data.size());

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
    void generate_key()
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
    void read_key()
    {
        std::ifstream file("key", std::ios_base::binary);
        if (!file)
        {
			error("Unable to open the input text file.");
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
    void encrypt_decrypt_data()
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
    void generate_aes_key() 
    {       
        aes_key.resize(AES_KEY_SIZE);
        if (RAND_bytes(aes_key.data(), AES_KEY_SIZE) != 1) {
            // Handle error if random byte generation fails
            throw std::runtime_error("Failed to generate random key using OpenSSL RAND_bytes");
        }

        // Write the generated key to a file
        std::ofstream file("aes_key", std::ios_base::binary);
        if (!file) {
			throw std::runtime_error("Failed to open file for writing AES key");
		}
        file.write((const char*)aes_key.data(), aes_key.size());
		file.close();
    }

    /// <summary>
    /// Reads a 256-bit AES key from a file and stores it in the aes_key variable
    /// </summary>
    void read_aes_key()
    {
        std::ifstream file("aes_key", std::ios_base::binary);
        if (!file) {
			throw std::runtime_error("Failed to open file for reading AES key");
		}

		aes_key.resize(AES_KEY_SIZE);
		file.read((char*)aes_key.data(), aes_key.size());
		file.close();
	}

    /// <summary>
    /// Encrypts the text using the AES key stored in the aes_key variable using AES-256 encryption
    /// </summary>
    void aes_encrypt() 
    {
        // Check if the key length is appropriate
        if (aes_key.size() != 16 && aes_key.size() != 24 && aes_key.size() != 32)
        {
            throw std::invalid_argument("Key length must be 16, 24, or 32 bytes.");
        }

        // Initialize OpenSSL cipher context
        EVP_CIPHER_CTX* ctx;
        if (!(ctx = EVP_CIPHER_CTX_new()))
        {
            throw std::runtime_error("Error creating EVP cipher context.");
        }

        // Initialize AES-256 encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_key.data(), NULL) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error initializing AES encryption.");
        }

        // Prepare output buffer
        std::vector<uint8_t> ciphertext(text.size() + EVP_MAX_BLOCK_LENGTH);

        // Encrypt plaintext
        int len;
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, text.data(), text.size()) != 1) 
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error performing AES encryption.");
        }

        // Finalize encryption
        int ciphertext_len = len;
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) 
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error finalizing AES encryption.");
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
    void aes_decrypt()
    {
        // Check if the key length is appropriate
        if (aes_key.size() != 16 && aes_key.size() != 24 && aes_key.size() != 32)
        {
            throw std::invalid_argument("Key length must be 16, 24, or 32 bytes.");
        }

        // Initialize OpenSSL cipher context
        EVP_CIPHER_CTX* ctx;
        if (!(ctx = EVP_CIPHER_CTX_new()))
        {
            throw std::runtime_error("Error creating EVP cipher context.");
        }

        // Initialize AES-256 decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_key.data(), NULL) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error initializing AES decryption.");
        }

        // Prepare output buffer
        std::vector<uint8_t> plaintext(text.size() + EVP_MAX_BLOCK_LENGTH);

        // Decrypt ciphertext
        int len;
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, text.data(), text.size()) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error performing AES decryption.");
        }

        // Resize the output buffer to actual plaintext size
        plaintext.resize(len);

        // Clean up
        EVP_CIPHER_CTX_free(ctx);

        text = plaintext;
    }

   
    private:
        // Data from the BMP file
        BMPFileHeader file_header;
        BMPInfoHeader info_header;
        std::vector<uint8_t> img_data;

        // Text to encrypt/decrypt
        std::vector<uint8_t> text;

        // Keys for encryption/decryption
        uint64_t key;
        std::vector<uint8_t> aes_key;

};