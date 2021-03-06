// Session 3 & 4
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

#define M_PI 3.141592654

using namespace std;

// width and heigth of the images
const int W = 256;
const int H = 256;

float *loadImage(string filename, int N)
// Load the image
{
	float *I = new float[N*N]; 
	ifstream imageFile;
	imageFile.open((filename), ofstream::in | ofstream::binary);
	if (imageFile.is_open())
	{
		imageFile.seekg(0, imageFile.end); 
		int length = (int)imageFile.tellg();
		imageFile.seekg(0, imageFile.beg); 
		imageFile.read((char*)I, length); 
		if (imageFile)
			std::cout << "All characters read successfully: " << length << " bytes" << endl << endl; 
		else
			std::cout << "Error: only " << imageFile.gcount() << " bytes could be read" << endl << endl; 
	}
	else
	{
		cout << endl << "cannot open the file "<< endl; 
	}
	return I;
}

void store(float I[], string filename, int size)
{
	ofstream file(filename, ios::out | ios::binary); 
	file.write((const char *)I, size * sizeof(float));
}

float psnr_calc(int N, int max, float image1[], float image2[])
// Compute the PSNR between 2 images
{
	float err = 0;
	float psnr;
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			err = err+  pow((image1[x + N * y] - image2[x + N* y]), 2);
		}
	}
	float mse = err / (N*N);
	psnr = 10 * log10((max*max) / mse);
	return psnr;
}

float *normalize8bpp(float I[], int N)
// Normalize the image to [0,1]
{
	float *I_norm = new float[N*N];	
	for (int x = 0; x <= N - 1; x++) {
		for (int y = 0; y <= N - 1; y++) {
			I_norm[N * x + y] = I[N * x + y] / N; 
	}
}
return I_norm;
}


float *basis_vector(int N)
{
	cout << "Calculating basis vectors ..." << endl;
	float *b = new float[N * N];
	for (int k = 0; k<N; ++k) {
		for (int n = 0; n<N; ++n) {
			if (n == 0) {
				b[N * n + k] = (float)(1 / sqrt(N))*cos(M_PI*n*(2 * k + 1) / (N*2));
			}
			else {
				 b[N * n + k] = (float)(sqrt(2) / sqrt(N))*cos(M_PI*n*(2 * k + 1) / (N*2));
			}
		}
	}
	return b;
}

float *transform(float b[], float I[], int N)
{
	cout << "Calculating DCT coefficients ..." << endl;
	float *coeff = new float[N * N];
	for (int k = 0; k<N; k++) {
		for (int n = 0; n<N; ++n) {
			coeff[N * k + n] = 0;
			for (int i = 0; i<N; ++i) {
				for (int j = 0; j<N; ++j) {
					coeff[N * k + n] += b[N * i + n] * b[N * j + k] * I[N * i + j];

				}
			}
		}
	}
	return coeff;
}

float *threshold(float coeff[], int N, float th)
{
	float *coeff_tH = new float[N*N];
	float threshold = th;
	for (int k = 0; k<N; k++) {
		for (int n = 0; n<N; n++) {
			if (abs(coeff[N * k + n]) >= threshold) {
				coeff_tH[N * k + n] = coeff[N * k + n];
			}
			else {
				coeff_tH[N * k + n] = 0;
			}
		}
	}
	return coeff_tH;
}

float *transpose(float b[], float coeff[], int N)
{
	cout << "Calculating IDCT coefficients ..." << endl;
	float *s = new float[N * N];
	for (int i = 0; i<N; ++i) {
		for (int j = 0; j<N; ++j) {
			s[N * i + j] = 0;
			for (int k = 0; k<N; ++k) {
				for (int n = 0; n<N; ++n) {
					s[N* i + j] += b[N * i + n] * b[N * j + k] * coeff[N * k + n];
				}
			}
		}
	}
	return s;
}

float *approximate(float b[], float image[], float Q[], int N)

// Approximate function that apply DCT, Q, IQ, IDCT for each 8x8 block 
{
	float *coeff = new float[W * H];

	// Calculate DCT coefficients for 8x8 blocks
	for (int k = 0; k<256; ++k) {
		for (int n = 0; n<256; ++n) {
			coeff[256 * k + n] = 0;
			for (int i = 0; i<256; ++i) {
				for (int j = 0; j<256; ++j) {
					coeff[256 * k + n] += b[256 * i + n] * b[256 * j + k] * image[256 * i + j];
				}
			}
		}
	}

	// Store the result of the DCT
	store(coeff, "dct_coefficients.raw",N*N);

	//Apply quantization
	float *z = new float[256 * 256];
	for (int k = 0; k<32; ++k) {
		for (int n = 0; n<32; ++n) {
			for (int i = 0; i<8; ++i) {
				for (int j = 0; j<8; ++j) {
					z[256 * (k * 8 + i) + (n * 8 + j)] = abs((coeff[256 * (k * 8 + i) + (n * 8 + j)] + abs(Q[8 * i + j] / 2)) / Q[8 * i + j]);
				}
			}
		}
	}
	// Store the result of the Q
	store(z, "quantization.raw",N*N);

	// Inverse quantization
	float *z_approx = new float[256 * 256];
	for (int k = 0; k<32; ++k) {
		for (int n = 0; n<32; ++n) {
			for (int i = 0; i<8; ++i) {
				for (int j = 0; j<8; ++j) {
					z_approx[256 * (k * 8 + i) + (n * 8 + j)] = (coeff[256 * (k * 8 + i) + (n * 8 + j)] * Q[8 * i + j]);
				}
			}
		}
	}

	// Store the result of the IQ
	store(z_approx, "inv_quantization.raw",N*N);

	// IDCT
	float *s = new float[256 * 256];
	for (int i = 0; i<256; ++i) {
		for (int j = 0; j<256; ++j) {
			s[256 * i + j] = 1;
			for (int k = 0; k<256; ++k) {
				for (int n = 0; n<256; ++n) {
					s[256 * i + j] += b[256 * i + n] * b[256 * j + k] * z_approx[256 * k + n];

				}
			}
		}
	}
	// Store the result of the IDCT
	store(s, "idct.raw",N*N);
	return s;
}

int main()
{
	// Load and read the original image
	const int N = 256; // block size
	string lena = "lena.raw";
	float* original_image = loadImage(lena,N);
	
	float* original_image_norm = normalize8bpp(original_image, N);
	// Store the basis vectors
	float* b = basis_vector(N);
	store(b,"basis_vector.raw",N*N);

	// DCT coefficents of the original image
	float *coeff = transform(b, original_image_norm,N);
	store(coeff, "DCT_coefficients.raw",N*N);

	// DCT coefficient thresholded
	//for (float k = 1; k < 11; k++) {
		float th = 0.1;
		float *coeff_th = threshold(coeff, N, th);
		//store(coeff_th, "DCT_coefficients_th.raw",N*N);

		// IDCT
		float *s = transpose(b, coeff_th, N);
		store(s, "IDCT_image.raw", N*N);
		int max = 1;
		float psnr = psnr_calc(N, max, s, original_image_norm);
		cout << "PSNR between original image and DFT/IDFT image : " << psnr << " for th = " << th << endl;
	//}
	// Quantization matrix 50% quality
	float Q[8 * 8] = { 16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99 };

	//store(Q, "quantization_matrix.raw",N*N);

	float* idct = approximate(b, original_image, Q, N);

	store(idct, "result_approx.raw",N*N);

}
