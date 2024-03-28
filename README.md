# image-encrypt

By Mark Narain Enzinger

## Overview

This program can encrypt text and store it inside an image. It offers two encryption methods: XOR encryption and AES encryption. The text is stored inside the image by modifying the lowest byte of all three color channels (RGB) of each pixel. The text is structured within the pixel data as follows:

| Bytes                                            | Purpose                                                               |
| ------------------------------------------------ | --------------------------------------------------------------------- |
| 32                                               | Length of the text that follows                                       |
| text.len * 8                                     | All bits of the text                                                  |
| Remaining unused pixel bytes until img.len - 32  | Random bytes to fill the image                                        |
| 32                                               | Checksum (Consists of all bytes of the image data exept the checksum) |

Currently, the user can choose between XOR-linking the text with a randomly generated 64-bit key or using 256-bit AES encryption.

I started this project to start learning about cryptographic programming and to figure out the BMP file format. 
Perhaps I will again come back and continue with this project to broaden my encryption/decryption knowledge.

## Used Libraries

- OpenSSL 3.0.13: A precompiled version is included in the Solution directory. 
  
  This compilation is tailored for Visual Studio 2022 and may not be compatible with other environments. 

  I downloaded OpenSSL from https://www.openssl.org/source/.
  Instructions for installing OpenSSL can be found in the downloaded folder:
  INSTALL.md and NOTES-WINDOWS.md

## Used Environment

- Windows 11
- Visual Studio 2022 (17.9.3)
- Atlassian Sourcetree (optional)

## Links

[marknarain.com](https://www.marknarain.com)
