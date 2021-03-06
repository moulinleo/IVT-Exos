// Session 5 & 6 : Project
// Léo Moulin & Alexis Rouvroy

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <stdio.h>      
#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <random>
#include <math.h>
#include "string"
#include "bitset"
#include <vector>

#define M_PI 3.1415926535897932

using namespace std;

const int N = 256;

float *loadImage(string filename, int N)
// Load the image
{
	float *I = new float[N * N];
	ifstream imageFile;
	imageFile.open((filename), ofstream::in | ofstream::binary);
	if (imageFile.is_open())
	{
		imageFile.seekg(0, imageFile.end);
		int length = (int)imageFile.tellg();
		imageFile.seekg(0, imageFile.beg);
		
		imageFile.read((char*)I, length);
		if (imageFile)
			std::cout << "All characters read successfully: " << length << "bytes" << endl << endl;
		else
			std::cout << "Error: only " << imageFile.gcount() << " bytes could be read" << endl << endl;
		imageFile.close();
		return I;
	}
	else
	{
		cout << endl << "cannot open the file" << endl;
	}
}

void store(float I[], string dataname, int size)
// Store function 
{
	ofstream file(dataname, ios::out | ios::binary);
	file.write((const char *)I, size * sizeof(float));
}


float *encode_rle(float image[], int *len_list)
// RLE Encoder. As we do not know the size of the code yet, we initialize it to the maximum size (size of the image)
{
	int j = 1;
	float *encoded_image = new float[(N * N) + 1];

	// First value is the color of the first pixel
	encoded_image[0] = image[0];

	// Initialize the encoded image to 0.5 everywhere

	for (int x = 1; x < N*N + 1; x++) {
		encoded_image[x] = 0.5;
	}
	for (int i = 0; i < N* N; i++) {
		int sum = 1;
		while (i + 1<N*N + 1 && image[i] == image[i + 1]) {
			// While the previous pixel and the current pixel are the same
			sum++;
			i++;
		}
		// Store the sum value at the j position
		encoded_image[j] = sum;
		j++;
	}
	// We keep the length of the code in memory
	*len_list = j;
	float *final_code = new float[j];
	for (int k = 0; k < j; k++) {
		final_code[k] = encoded_image[k];
	}
	return encoded_image;
}


float *decode_rle(float encoded_image[], int N, int size_code)
// RLE Decoding
{
	float *decoded_image = new float[N*N];
	int j = 0;
	int k = 1;
	for (int i = 1; i < N * N; ++i) {
		j = 0;
		if (i == 1) {
			decoded_image[0] = encoded_image[0];
		}
		else if (decoded_image[i - 1] == 1) {
			decoded_image[i] = 0;
			i++;
		}
		else {
			decoded_image[i] = 1;
			i++;
		}

		while (j<encoded_image[k] - 1 && k < size_code) {
			decoded_image[i] = decoded_image[i - 1];
			i++;
			j++;
		}
		k++;
		i = i - 1;
	}
	return decoded_image;
}


float psnr_calc(float I1[], float I2[])
{
	double sum = 0.0;
	double MAX = 1.0;
	for (int x = 0; x < N; x++) {
		for (int y = 0; y <= N; y++) {
			sum = sum + ((I1[N * x + y] - I2[N * x + y])*(I1[N * x + y] - I2[N * x + y]));
		}
	}
	float MSE = sum / (N*N);
	float PSNR = 10 * log10((MAX*MAX) / MSE);
	return PSNR;
}

string golomb(int number) {
// Exp-Golomb encoding
	// Add 1 to the integer
	number = number + 1;
	// Convert the integer to binary
	string binary = std::bitset<sizeof(int) * 8>(number).to_string();
	string final_binary;
	int count = 0;
	for (int i = 0; i < sizeof(int) * 8; i++) {
		if (binary[i] == '1') {
			// Counts the number of zeros before the encountering the start of the binary number
			count = 32 - i;
			i = sizeof(int) * 8;
		}
	}
	// Final size of the binary
	int count_tot = 2 * count - 1;
	final_binary = binary.substr(32 - count_tot, count_tot);
	return final_binary;
}

float* inverse_golomb(string final_binary, int size_list) {
// Exp-golomb decoding
	float *decode = new float[size_list];
	string binary;
	int k = 0;
	int number_of_zeros = 0;
	int size = final_binary.size();
	for (int i = 0; i < size; ++i) {
		if (final_binary[i] == '1') {
			binary = final_binary.substr(i, number_of_zeros + 1);
			unsigned int number = std::bitset<32>(binary).to_ulong();
			number -= 1;
			decode[k] = number;
			k++;
			i += number_of_zeros;
			number_of_zeros = 0;
		}
		else {
			number_of_zeros++;
		}
	}
	return decode;
}

int main()
{
	// ENCODING PART : Combine RLE and VLC to compress bilevel images
	string filename = "earth_binary.raw";
	float* original_image = loadImage(filename,N);
	
	// RLE Encoding
	int size_code = 0;
	float *code = encode_rle(original_image, &size_code);
	store(code, "encoded_image.raw", N * N);
	
	// Exp-Golomb encoding : creation of the output bit stream
	string binary_out;
	for (int i = 0; i < size_code; i++) {
		binary_out += golomb(code[i]);
	}
	// Output bit stream
	std::ofstream out("binary.txt");
	out << binary_out;
	out.close();

	// DECODING PART 
	// Input bit stream
	string binary_in;
	ifstream input;
	// Read txt file
	input.open("binary.txt");
	input >> binary_in;
	input.close();

	// Exp-golomb decoding
	float *decoded_binary = new float[size_code];
	decoded_binary = inverse_golomb(binary_in, size_code);
	store(decoded_binary, "decoded_binary.raw", size_code);

	// RLE Decoding
	float *decoded_image = decode_rle(decoded_binary, N, size_code);
	store(decoded_image, "decoded_image.raw", N * N);
	
	// Calculation of the PSNR
	float psnr = psnr_calc(decoded_image, original_image);
	cout << "Final PSNR between original and decoded image : " << psnr << endl;

	return 0;
}
