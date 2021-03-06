// Session 5 & 6 : IVT exercices
// Léo Moulin

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

#define M_PI 3.1415926535897932

using namespace std;

const int W = 256;
const int H = 256;

float *loadImage(string filename)
// Load the image
{
	ifstream imageFile;
	imageFile.open((filename), ofstream::in | ofstream::binary);
	if (imageFile.is_open())
	{
		imageFile.seekg(0, imageFile.end);
		int length = (int)imageFile.tellg();
		imageFile.seekg(0, imageFile.beg);
		float *I = new float[256 * 256]; 
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


float *encode_rle(float image[])
// RLE Encoder. As we do not know the size of the code yet, we initialize it to the maximum size (size of the image)
{
	int j = 1;
	float *encoded_image = new float[(W * H)+1];
	
	// First value is the color of the first pixel
	encoded_image[0] = image[0];

	// Initialize the encoded image to 0.5 everywhere
	
	for (int x = 1; x < W*H+1; x++) {
		encoded_image[x] = 0.5;
	}
	for (int i = 0; i < W * H; i++) {
		int sum = 1;
		while (i + 1<W*H+1 && image[i] == image[i + 1]) {
			// While the previous pixel and the current pixel are the same
			sum++;
			i++;
		}
		// Store the sum value at the j position
		encoded_image[j] = sum;
		j++;
	}
	return encoded_image;
}

float *remove(float encoded_image[], int M)
// Set the code to its right size. Remove the unused values of the code (0.5). Takes as input the code and the length of the code
{
	int length = W * H;
	float *encoded_image_final = new float[M + 1];

	// Initialize the encoded image to 0.5 everywhere
	for (int x = 0; x < M+1; x++) {
		encoded_image_final[x] = encoded_image[x];
	}
	return encoded_image_final;
}


float *decode_rle(float encoded_image[])
// RLE Decoding
{
	float *decoded_image = new float[W*H];
	int j = 0;
	int k = 1;
	for (int i = 1; i < W * H; ++i) {
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

		while (j<encoded_image[k] - 1) {
			decoded_image[i] = decoded_image[i - 1];
			i++;
			j++;
		}
		k++;
		i = i - 1;
	}
	return decoded_image;
}

int max_runlength(float encoded_image[]) {
// Finds the maximum of the encoded_image array
	int N = 0;
	for (int i = 0; i < W * H; i++)
	{
		if (encoded_image[i]>N)
			N = encoded_image[i];
	}
	return N;
}

int number_runlength(float encoded_image[]) {
// Finds the number of runlengths  
	int M = 0;
	for (int i = 0; i<W *H + 1; i++)
	{
		// If we do not encounter a 0.5
		if (encoded_image[i] != 0.5)
			M++;
	}
	M = M - 1; // First value = color of pixel
	return M;
}

float *normalization(float PDF[], int N) {
	float *normalize_code = new float[N+1];
	normalize_code[0] = 0;
	int sum = 0;

	for (int i = 1; i<N+1; i++) {
		sum = sum + PDF[i];
	}
	for (int i = 1; i<N+1; i++) {
		normalize_code[i] = PDF[i] / sum;
	}
	return normalize_code;
}

float* pdf(float encoded_image[], int N, int M) {
// Calculates the pdf with the encoded image, M and N.

	float *PDF = new float[N+1];
	for (int k = 0; k < N+1; k++) {
		PDF[k] = 0;
	}
	int sum = 0;
	for (int i = 1; i < M; ++i) {
		int a = encoded_image[i];
		for (int j = 1; j < M; ++j) {
			if (encoded_image[i] == encoded_image[j] && encoded_image[i] != 0) {
				PDF[a] += 1;
				encoded_image[j] = 0;
			}
		}
	}
	return PDF;
}

float entropy(float encoded_image_norm[]) {
	// Compute the Entropy H = minimum number of bits per run
	float entropy = 0.0;
	for (int i = 0; i < 374; ++i) {
		if (encoded_image_norm[i] != 0) {
			// Definition of the entropy : H = -p*log2(p)
			entropy += encoded_image_norm[i] * log2(encoded_image_norm[i]);
		}
	}
	entropy = -entropy;
	cout << "The entropy is : " << entropy << endl;
	return entropy;
}

float psnr_calc(float I1[], float I2[])
{
	double sum = 0.0;
	double MAX = 1.0;
	for (int x = 0; x < W; x++) {
		for (int y = 0; y <= H; y++) {
			sum = sum + ((I1[W * x + y] - I2[W * x + y])*(I1[W * x + y] - I2[W * x + y]));
		}
	}
	float MSE = sum / (W*W); 
	float PSNR = 10 * log10((MAX*MAX) / MSE); 
	return PSNR;
}

string golomb(int number) {
// Exp-Golomb encoding
	// Add 1 to the integer
	number += 1;
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
	int count_tot = 2*count - 1;
	final_binary = binary.substr(32 - count_tot, count_tot);
	cout << final_binary << endl;
	return final_binary;
}

int inverse_golomb(string final_binary) {
// Exp-golomb decoding
	string binary;
	int number_of_zeros = 0;	
	int size = final_binary.size();
	for (int i = 0; i < size; ++i) {
		if (final_binary[i] == '1') {	
			binary = final_binary.substr(number_of_zeros, number_of_zeros + 1);	
			i = size;
		}
		number_of_zeros++;	
	}
	unsigned int number = std::bitset<32>(binary).to_ulong();	
	number -= 1;
	return number;
}


int main()
{
	// PART 1 : Bilevel images and run-length encoding (RLE)
	// Read the 32bpp picture of lena
	string filename = "earth_binary.raw";
	float* original_image = loadImage(filename);

	// RLE Encoding
	float *encoded_image = encode_rle(original_image);
	
	int M = number_runlength(encoded_image);
	cout << "Total number of runlengths M = " << M << endl;
	store(encoded_image, "encoded_image.raw", W * H);

	// Final coded image
	float *encoded_image_final = remove(encoded_image,M);
	store(encoded_image_final, "encoded_image_final.raw", M+1);
	// RLE Decoding
	float *decoded_image = decode_rle(encoded_image_final);
	store(decoded_image, "decoded_image.raw", W * H);

	// Verification
	float psnr = psnr_calc(decoded_image, original_image);
	cout << "PSNR between the original and the encode + decoded image = " << psnr << endl; // infinite PSNR

	// PART 2 : Discrete probability density functions (PDF)
	// Maximum run length
	int N = max_runlength(encoded_image_final);
	cout << "Maximum run length N = " << N << endl;
	
	// Creates an array with the probabilities of occurency
	float* pdf1 = pdf(encoded_image, N, M);
	store(pdf1, "pdf.raw", N+1);

	// Normalize the PDF
	float* normalize_pdf = normalization(pdf1, N);
	store(normalize_pdf, "normalized_pdf.raw", N+1);
	
	// Calculate the entropy of the code
	float entropy1 = entropy(normalize_pdf);

	// PART 3 : Exp-Golomb variable-length code (VLC)
	// Test with a random number
	string binary = golomb(333);
	int a = inverse_golomb(binary);
	cout << a << endl;

	return 0;
}
