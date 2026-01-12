# 🕵️ Image Steganography using C (LSB Technique)

## 📌 Project Overview
This project implements **Image Steganography** using the **Least Significant Bit (LSB)** technique in **C language**.  
It allows users to **hide a secret text file inside a BMP image** and later **extract the hidden data** without visibly altering the image.

Steganography ensures **confidential communication** by concealing information within digital media such that its presence is undetectable to the human eye.

---

## 🎯 Objectives
- Understand the working of **LSB-based steganography**
- Implement **encoding and decoding** in C
- Learn **bit-level manipulation**
- Work with **BMP image file format**
- Strengthen knowledge in **file handling and structures**

---

## 🧠 Steganography Technique Used
### 🔹 Least Significant Bit (LSB)
- Each pixel byte in a BMP image has unused least significant bits
- Secret data bits are embedded into these LSBs
- Changes are visually imperceptible
- Original image quality is preserved

---

## 🗂️ Project Structure
├── encode.c
├── decode.c
├── test_encode.c
├── common.h
├── encode.h
├── decode.h
├── show.c
├── show.h
├── types.h
├── secret.txt
├── beautiful.bmp
├── stego.bmp
└── README.md

---

## 🛠️ Compilation & Execution

### 🔹 Encoding

gcc *.c

./a.out -e beautiful.bmp secret.txt stego.bmp

### 🔹 Decoding

./a.out -d stego.bmp output.txt

---

## 📥 Input
- **Cover Image**: BMP format
- **Secret File**: Text file

## 📤 Output
- **Stego Image**: Image containing hidden data
- **Extracted Text File**

---

## ✅ Features
- Supports BMP image format
- Secure and invisible data hiding
- Efficient bit manipulation
- Modular and well-structured code
- Command-line based interface

---

## 👨‍💻 Author
**Sugavelan G**  
Electronics and Communication Engineer  
Interested in Embedded Systems & Embedded C

