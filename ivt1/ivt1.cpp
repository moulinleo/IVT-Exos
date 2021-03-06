#include <cmath>
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <random>

#define M_PI 3.141592654

using namespace std;

const int W = 256; // Width of the image
const int H = 256; // Height of the image

void store(string name, float image[])
{
	// Store an image containint floats to a raw file
	ofstream file(name, ios::out | ios::binary);
	file.write((const char *)image, H*W * sizeof(float));
	cout << "Image stored" << endl;
}

float mse_calc(float image1[], float image2[])
{
	// Calculate the MSE between 2 images (256x256)
	float mse = 0.0;
	for (int x = 0; x < 256; x++) {
		for (int y = 0; y < 256; y++) {
			mse += pow((image1[x+W*y] - image2[x+W*y]), 2);
		}
	}
	mse = mse / (W*H);
	cout << "MSE : " << mse << endl;
	cout << "PSNR : " << 10 * log10(1 / mse) << endl;
	return mse;
}

int main() 
{
	// Cosines image
	float cos_image[W*H];
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			cos_image[x+W*y] = 0.5 + 0.5*cos(x*(M_PI / 32))*cos(y*(M_PI / 64));
		}
	}
	store("cos_image.raw", cos_image);

	// Uniform-distributed random image
	std::default_random_engine generator;
	
	std::uniform_real_distribution<float> unif_distribution(0, 1);
	//std::normal_distribution<float> unif_distribution(0.5, 0.25);
	float* random_image = new float[W*H];
	float mean_image[W*H];
	float mse, psnr;
	
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			random_image[x+W*y] = unif_distribution(generator);
			mean_image[x+W*y] = 0.5;
		}
	}
	store("random_image.raw", random_image);
	/*
	// Calculate the MSE between the random and the mean images
	cout << "Uniformly random image and mean image of 0.5 : " << endl;
	mse = mse_calc(random_image, mean_image);
	psnr = 10 * log10(1 / mse); // Maximum value of the grayscale image is 1

	
	// Gaussian-distributed image of mean 0.5
	float std_dev = 0.25;
	std::normal_distribution<float> gaussian_distribution(0.5, std_dev);

	float*gaussian_image = new float[W*H];
	
	for (int i = 0; i < W; i++) {
		for (int j = 0; j < H; j++) {
			gaussian_image[i+W*j] = gaussian_distribution(generator);
		}
	}
	
	store("gaussian_image025.raw", gaussian_image);

	// Calculate the MSE between the gaussian image and the mean images
	cout << "Gaussian-distributed random image and mean image of 0.5 : " << endl;
	mse = mse_calc(gaussian_image, mean_image);
	psnr = 10 * log10(1 / mse); 
	*/
    return 0;
}

