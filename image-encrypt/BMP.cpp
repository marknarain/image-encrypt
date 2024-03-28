/*
    Copyright (c) 2024 Mark Narain Enzinger

    MIT License (https://github.com/marknarain/image-encrypt/blob/main/LICENSE)
*/

#include "image-encrypt.h"

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
    void encrypt(std::string fname, int encryption_type)
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
    void decrypt(std::string fname, int encryption_type)
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
            default:
                error("Invalid encryption type");
        }

        write_text_out(fname);
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
    void write_text_to_img_data()
    {
        uint32_t text_size = text.size();
        uint32_t data_size = img_data.size();
        uint32_t max_text_size = (data_size - 64) / 8;

        uint32_t i = 0;

        // We write the size of the text in the first 32 bytes of the image data
        for (; i < 32; i++)
        {
			uint8_t bit = (text_size >> i) & 1;
			img_data[i] = (img_data[i] & 0xFE) | bit;
		}

        if (text_size > max_text_size)
        {
			error("The text is to large for the image");
		}

        // Every bit of the text is written in the image data
        for (; i < text_size + 32; i++)
        {
            uint8_t byte = text[i - 32];
            for (uint32_t j = 0; j < 8; j++)
            {
                uint8_t bit = byte & 1;
                byte = byte >> 1;  

                img_data[i * 8 + j] = (img_data[i * 8 + j] & 0xFE) | bit;
            }
        }

        // Fill the rest of the image data with random data
        // Always change the lowest bit to ensure that the image data is different from the original image
        for (; i < data_size - 32; i++)
        {
            img_data[i] = (img_data[i] & 0xFE) | (rand() & 1);
        }

        // Calculate the CRC32 checksum of the image data until data_size - 32
        uint32_t crc = calculate_crc32(std::vector<uint8_t>(img_data.begin(), img_data.begin() + data_size - 32));

        // Write the CRC32 checksum to the last 32 bits of the image data
        for (; i < data_size; i++)
        {
			uint8_t bit = (crc >> (i - data_size + 32)) & 1;
			img_data[i] = (img_data[i] & 0xFE) | bit;
		}
    }

    /// <summary>
    /// Read the text from the image data bitwise
    /// </summary>
    void read_text_from_img_data()
    {
        uint32_t text_size = 0;
        uint32_t data_size = img_data.size();

        // Read the first 32 bits from the image data to determin how long the stored data in the image is.
        for (uint32_t i = 0; i < 32; i++)
        {
			text_size = text_size | ((img_data[i] & 1) << i);
		}

        uint32_t crc_read = calculate_crc32(std::vector<uint8_t>(img_data.begin(), img_data.begin() + data_size - 32));
        uint32_t crc_expected = 0;

        // Read the last 32 bits from the image data to get the CRC32 checksum
        for (uint32_t i = data_size - 32; i < data_size; i++)
        {
            crc_expected = crc_expected | ((img_data[i] & 1) << (i - data_size + 32));
        }

        if (crc_read != crc_expected)
        {
            error("The data in the image is corrupted");
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
    void read_aes_key()
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
    void aes_encrypt() 
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
    void aes_decrypt()
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
        uint8_t padding;

        // Text to encrypt/decrypt
        std::vector<uint8_t> text;

        // Keys for encryption/decryption
        uint64_t key;
        std::vector<uint8_t> aes_key;

};