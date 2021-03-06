// Session 2 of IVT exercices
// Léo Moulin

#include "stdafx.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <random>
#include <stdlib.h>

#define M_PI 3.141592654

using namespace std;

const int W = 256;
const int H = 256;

float *loadImage(string filename, int N)
// Function to load 32 bit per pixel image of size N*N

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
			std::cout << "All characters read successfully: " << length << "bytes." << endl << endl; 
		else
			std::cout << "Error: only " << imageFile.gcount() << " bytes could be read" << endl << endl; 
		imageFile.close(); 
		return I;
	}
	else
	{
		cout << endl << "cannot open the file: ./files/" << endl; 
		return I;
	}
}

void store(string name, float image[])
{
	// Store an image containint floats to a raw file
	ofstream file(name, ios::out | ios::binary);
	file.write((const char *)image, H*W * sizeof(float));
	cout << "Image stored" << endl;
}


float normalize8bpp(float gray_pixel)
// Maps a 8-bit value to a 32-bit value of range [0,1]
{
	return gray_pixel / 256;
}

float psnr_calc(float image1[], float image2[]) 
// Compute the PSNR between 2 images
{
	int max = 1;
	float mse = 0;
	float psnr;
	for (int x = 0; x < 256; x++) {
		for (int y = 0; y < 256; y++) {
			mse += pow((image1[x+W*y] - image2[x+W*y]), 2);
		}
	}
	mse = mse / (W*H);
	psnr = 10 * log10((max*max) / mse);
	return psnr;
}



//Creation of Gaussian kernel + normalization ////
float *blur(float I_norm[], float sigma)
{
	float *kernel = new float[3*3];
	float mean = 3/2;
	float sum = 0.0; // For accumulating the kernel values
	float *image_blur = new float[256 * 256];
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y) {
			kernel[3*x + y] = exp(-0.5 * (pow((x - mean) / sigma, 2.0) + pow((y - mean) / sigma, 2.0)))
				/ (2 * M_PI * sigma * sigma);

			// Accumulate the kernel values
			sum += kernel[3*x + y];
		}

	// Normalize the kernel
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			kernel[3*x + y] /= sum;

	for (int i = 1; i<W-1; i++) {
		for (int j = 1; j<H-1; j++) {
			float sum = 0.0;
			for (int m = -1; m<2; m++) {
				for (int n = -1; n<2; n++) {
					sum += kernel[3*(1 + m) + (1 + n)] * I_norm[256 * (i + m) + (j + n)];
				}
			}
			image_blur[256 * i + j] = sum;
		}
	}
	// Assign values to borders
	for (int i = 0; i<W; i++) {
		for (int j = 0; j<W; j++) {
			image_blur[i] = 0.5;
			image_blur[W + i] = 0.5;
			image_blur[W * i] = 0.5;
			image_blur[W * i + 1] = 0.5;
			image_blur[W * W - 255 + i] = 0.5;
			image_blur[W * W - 255 - (i + 1)] = 0.5;
			image_blur[(i + 1) * 255 + i] = 0.5;
			image_blur[(i + 1) * 255 + i - 1] = 0.5;
		}
	}
	return image_blur;
}

float *borders(float I_norm[])
// Handles the border pixels so that the image is 256x256
{
	for (int i = 0; i < W; i++) {
		for (int j = 0; j < H; j++) {
			I_norm[i] = 0.5; 
			I_norm[W + i] = 0.5; 
			I_norm[W * i] = 0.5; 
			I_norm[W * i + 1] = 0.5;
			I_norm[W * W - W + 1 + i] = 0.5; 
			I_norm[W * W - W + 1 - (i + 1)] = 0.5;
			I_norm[(i + 1) * (W-1) + i] = 0.5; 
			I_norm[(i + 1) * (W-1) + i - 1] = 0.5;
		}
	}
	return I_norm;
}

float *unsharp_masking(float image_norm[], float image_blur[])
// Applies a sharpening mask to an image
{
	float edges[W * H];
	float *image_sharp = new float[W * W];
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			// Get the edges 
			edges[W * x + y] = image_norm[W * x + y] - image_blur[W * x + y];
		}
	}
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < W; y++) {
			// Unsharp masking
			image_sharp[W * x + y] = image_norm[W * x + y] + 0.4 * edges[W * x + y];
		}
	}
	return image_sharp;
}

int main()
{	
	// Load the "Lena" image
	string lena = "lena.raw";
	float*original_image = loadImage(lena,W);

	// Convert integer pixel values to real values and add Gaussian noise
	float std_dev = 0.08;
	float* noisy_image = new float[H*W];
	float* original_image_norm = new float[H*W];
	std::default_random_engine generator;
	std::normal_distribution<float> distribution(0, std_dev);
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			float noise = distribution(generator);
			float pix32b = normalize8bpp(original_image[x+W*y]);
			original_image_norm[x + W * y] = pix32b;
			noisy_image[x + W*y] = pix32b + noise;
		}
	}
	// Store normalized original image
	store("original_image_norm.raw", original_image_norm);

	// Store noisy image
	store("noisy_image03.raw", noisy_image);

	// Calculate PSNR
	float psnr;
	psnr = psnr_calc(noisy_image, original_image_norm);
	cout << "PSNR = " << psnr << " between original image and noisy image with std_dev = " << std_dev << endl;

	// Apply Gaussian blur (Vary sigma from 0 to 1)
	float sigma;
	for (float kk = 0; kk < 11; kk++) {
		sigma = kk/10;
		float* image_blur = blur(original_image_norm, sigma);
		psnr = psnr_calc(image_blur, original_image_norm);
		cout << "PSNR = " << psnr << " between original image and blurred image with sigma = " << sigma << endl;
	}

	// Apply a sharpening mask to the sigma = 0.5 blurred image
	sigma = 0.5;
	float* image_blur = blur(original_image_norm, sigma);
	float* sharp_image = unsharp_masking(original_image_norm, image_blur);

	// Store sharpened image
	store("sharp_image.raw",sharp_image);

	// Calculate PSNR. The PSNR indeed increases when the blurred image is sharpened.
	psnr = psnr_calc(sharp_image, original_image_norm);
	cout << "PSNR = " << psnr << " between original image and sharpened blurred image with sigma = " << sigma << endl;
	

	// Image Capture artefacts (changed the noise variance to 0.08 so that the PSNR is the same as the blur with sigma = 0.5)
	float* imagenoisy_blur = blur(noisy_image, sigma);
	psnr = psnr_calc(imagenoisy_blur, original_image_norm);
	cout << "PSNR = " << psnr << " between original image and noisy image + blur" << endl;

	store("blur_noise.raw", imagenoisy_blur);

	// Blur then noise (Add the noise to the already calucated blurred original image)
	float* imageblur_noise = new float[W*H];
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			float noise = distribution(generator);
			imageblur_noise[x + W * y] = image_blur[x + W * y] + noise;
		}
	}
	psnr = psnr_calc(imageblur_noise, original_image_norm);
	cout << "PSNR = " << psnr << " between original image and blur + noise" << endl;

	store("noise_blur.raw", imageblur_noise);

	return 0;
}

