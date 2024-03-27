#include "pch.h"
#include "CppUnitTest.h"
#include "../image-encrypt/BMP.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace imageencrypttests
{
	TEST_CLASS(imageencrypttests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			//Write a test, that encrypts a file, reads the encrypted file, and compares it to the original file
			std::string pictureName = "tree.bmp";
			std::string fileName = "test.txt";
			std::string outputName = "output.bmp";
			int choice = 0;

			BMP bmp(pictureName);
			bmp.encrypt(fileName, 1);

			std::ifstream file(fileName, std::ios::binary);

			bmp.decrypt("encrypted.bmp", 1);

			std::ifstream output(outputName, std::ios::binary);

			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			std::string outputContents((std::istreambuf_iterator<char>(output)), std::istreambuf_iterator<char>());

			Assert::AreEqual(fileContents, outputContents);

			file.close();
			output.close();
		}
	};
}
