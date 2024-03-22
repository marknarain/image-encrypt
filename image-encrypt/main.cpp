#include "image-encrypt.h"
#include "BMP.cpp"

int main()
{
	string pictureName;
	string fileName;
	string outputName;
	int choice;

	cout << "image-encrypt" << "\n";
	cout << "Enter picture name: ";
	cin >> pictureName;
	BMP bmp(pictureName);
	
	cout << "Enter 0 to encrypt or 1 to decrypt: ";
	cin >> choice;

	if (choice == 0)
	{
		cout << "Enter the name of the file you want to encrypt: ";
		cin >> fileName;
		bmp.encrypt(fileName);
	}
	else if (choice == 1)
	{
		cout << "Make sure there is a file named 'key' in the same directory" << "\n";
		bmp.decrypt(fileName);
	}
	else
	{
		cout << "Invalid input";
	}

	return 0;
}

