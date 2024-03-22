#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <random>

#pragma pack(push, 1)

using namespace std;

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

void error(const string& message)
{
	int err;
    cout << message << "\n" << "Press any key and then enter to exit ...";
    cin >> err;
    cout << "Exiting...\n";
    exit(1);
}

struct BMP 
{
    // Constructor
    BMP(string fname)
    {
        ifstream file(fname, ios_base::binary);
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

    // Encrypt text into image
    void encrypt(string fname)
    {
        read_text_from_file(fname);
        generate_key();
        // Here the text is encrypted using the key from 'key'
        encrypt_decrypt_data();
        write_text_to_img_data();
        write_image_out("encrypted.bmp");
    }

    // Decrypt text from image
    void decrypt(string fname)
    {
		read_key();
		read_text_from_img_data();
        // Here the text is decrypted using the key from 'key' 
        encrypt_decrypt_data();
        write_text_out("decrypted.txt");
	}

    // Read text from file
    void read_text_from_file(string fname)
    {
		ifstream file(fname, ios_base::binary);
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

    // Write text to file
    void write_text_out(string fname)
    {
        ofstream file(fname, ios_base::binary);
        if (!file)
        {
			error("Unable to open the output file.");
		}

        file.write((const char*)text.data(), text.size());

        file.close();
    }

    // Write the text to the image data bitwise
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
        for (int i = 32; i < text_size + 32; i++)
        {
            uint8_t byte = text[i - 32];
            for (int j = 0; j < 8; j++)
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

    void read_text_from_img_data()
    {
        int text_size = 0;
		int data_size = img_data.size();

        // Read the first 32 bits from the image data to determin how long the stored data in the image is.
        for (int i = 0; i < 32; i++)
        {
			text_size = text_size | ((img_data[i] & 1) << i);
		}

		text.resize(text_size);

        for (int i = 32, keyCount = 0; i < text_size + 32; i++)
        {
            for (int j = 0; j < 8; j++, keyCount = (keyCount + 1) % 64)
            {
                text[i - 32] = text[i - 32] | (((img_data[i * 8 + j] & 1)) << j);
			}
		}
    }
    
    void write_image_out(string fname)
    {
        ofstream file(fname, ios_base::binary);
		if (!file)
		{
			error("Unable to open the output image file.");
		}

		file.write((const char*)&file_header, sizeof(file_header));
		file.write((const char*)&info_header, sizeof(info_header));
        file.write((const char*)img_data.data(), img_data.size());

        file.close();
    }

    void generate_key()
    {
		srand(time(0));
        ofstream file("key", ios_base::binary);
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

    void read_key()
    {
        ifstream file("key", ios_base::binary);
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

    void encrypt_decrypt_data()
    {
        for (int i = 0; i < text.size(); i++)
        {
            text[i] = text[i] ^ (key & 0xFF);
            key = (key >> 8) | (key << 56);
		}
    }
   
    private:
        BMPFileHeader file_header;
        BMPInfoHeader info_header;
        vector<uint8_t> img_data;
        vector<uint8_t> text;
        uint64_t key;

};