/*
	Copyright (c) 2024 Mark Narain Enzinger

	MIT License (https://github.com/marknarain/image-encrypt/blob/main/LICENSE)
*/

#include "image-encrypt.h"
#include "BMP.cpp"

int main()
{
	std::string pictureName = "input-image.bmp";
	std::string fileName = "input-text.txt";
	std::string outputName = "output-text.txt";
	int choice;

	std::cout << "\033[1m\033[4m" << "> image-encrypt <" << "\033[0m\033[24m" << "\n";
	std::cout << "This program encrypts a file into a picture or decrypts a file from a picture" << "\n";
	std::cout << "Choose a option from the following menu: " << "\n"; 
	std::cout << "1: Encrypt" << "\n";
	std::cout << "2: Decrypt" << "\n";
	std::cout << "3: Guide" << "\n";
	std::cout << "4: Exit" << "\n\n";

	std::cin >> choice;
	int encryption_type;

	system("cls");
	switch (choice)
	{
		case 1:
			std::cout << "Enter the name of your picture: ";
			std::cin >> pictureName;
			{
				BMP bmp(pictureName);
				std::cout << "Enter the name of the file you want to encrypt: ";
				std::cin >> fileName;

				std::cout << "Which encryption do you want to use?" << "\n";
				std::cout << "1: AES" << "\n";
				std::cout << "2: XOR" << "\n";
				std::cout << "3: None" << "\n";

				std::cin >> encryption_type;

				bmp.encrypt(fileName, encryption_type);
			}
			break;

		case 2:
			std::cout << "Enter the name of your picture: ";
			std::cin >> pictureName;
			{
				BMP bmp(pictureName);
				std::cout << "How should the output be named: ";
				std::cin >> fileName;

				std::cout << "Which encryption do you want to use?" << "\n";
				std::cout << "1: AES" << "\n";
				std::cout << "2: XOR" << "\n";
				std::cout << "3: None" << "\n";

				std::cin >> encryption_type;

				bmp.decrypt(fileName, encryption_type);
			}
			break;

		case 3:
			std::cout << "\033[1m\033[4m" << "Guide" << "\033[0m\033[24m" << "\n";
			std::cout << "To encrypt a file into a picture, place the image and the file in the same dirctory with this program." << "\n";
			std::cout << "To decrypt an image, place the image and the key file in the same directory with this program." << "\n";
			std::cout << "The key file is generated when you encrypt a file." << "\n";
			break; 

		case 4:
			std::cout << "Exiting program ..." << "\n";
			break;

		default:
			std::cout << "\aInvalid input" << "\n";
			break;
	}

	return 0;
}

