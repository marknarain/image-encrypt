#include "image-encrypt.h"
#include "BMP.cpp"

int main()
{
	std::string pictureName;
	std::string fileName;
	std::string outputName;
	int choice;

	std::cout << "image-encrypt" << "\n";
	std::cout << "Enter picture name: ";
	std::cin >> pictureName;
	BMP bmp(pictureName);
	
	std::cout << "Enter 0 to encrypt or 1 to decrypt: ";
	std::cin >> choice;

	if (choice == 0)
	{
		std::cout << "Enter the name of the file you want to encrypt: ";
		std::cin >> fileName;
		bmp.encrypt(fileName);
	}
	else if (choice == 1)
	{
		std::cout << "Make sure there is a file named 'key' in the same directory" << "\n";
		bmp.decrypt(fileName);
	}
	else
	{
		std::cout << "Invalid input";
	}

	return 0;
}

