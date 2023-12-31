#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string.h>
#include <typeinfo>
#include <chrono>  //计时头文件

#include "bm3d.h"
#include "utilities.h"

#define YUV       0
#define YCBCR     1
#define OPP       2
#define RGB       3
#define DCT       4
#define BIOR      5
#define HADAMARD  6
#define NONE      7

using namespace std;

/**
 * @file   main.cpp
 * @brief  Main executable file. Do not use lib_fftw to
 *         process DCT.
 *
 * @author MARC LEBRUN  <marc.lebrun@cmla.ens-cachan.fr>
 */


int main(int argc, char **argv)
{
    //! Check if there is the right call for the algorithm
	if (argc < 16)
	{
		cout << "usage: BM3D image sigma add_noise noisy basic denoised difference bias \
                 difference_bias computeBias tau_2d_hard useSD_hard \
                 tau_2d_wien useSD_wien color_space" << endl;
		return EXIT_FAILURE;
	}

	//! Declarations  作为中间变量存储 起始数据 加噪后数据 去噪后数据 作为对比的图片数据
	vector<float> img, img_noisy, img_basic, img_denoised, img_bias, img_diff;
	vector<float> img_basic_bias;
	vector<float> img_diff_bias;
    unsigned width, height, chnls;//宽 高 通道

    //! Load image
	if(load_image(argv[1], img, &width, &height, &chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

	//! Variables initialization
	float  fSigma       = atof(argv[2]);//所加噪声值
	int    useSD_1      = atoi(argv[12]);//
	int    useSD_2      = atoi(argv[14]);//
	const unsigned tau_2D_hard  = (strcmp(argv[11], "dct" ) == 0 ? DCT :
                                  (strcmp(argv[11], "bior") == 0 ? BIOR : NONE));
                                                    
          
     
                           
    if (tau_2D_hard == NONE)
    {
        cout << "tau_2d_hard is not known. Choice is :" << endl;
        cout << " -dct" << endl;
        cout << " -bior" << endl;
        return EXIT_FAILURE;
    }
	const unsigned tau_2D_wien  = (strcmp(argv[13], "dct" ) == 0 ? DCT :
                                  (strcmp(argv[13], "bior") == 0 ? BIOR : NONE));
                                                                    
                                  
                               
    if (tau_2D_wien == NONE)
    {
        cout << "tau_2d_wien is not known. Choice is :" << endl;
        cout << " -dct" << endl;
        cout << " -bior" << endl;
        return EXIT_FAILURE;
    };//颜色空间种类 标记
	const unsigned color_space  = (strcmp(argv[15], "rgb"  ) == 0 ? RGB   :
                                  (strcmp(argv[15], "yuv"  ) == 0 ? YUV   :
                                  (strcmp(argv[15], "ycbcr") == 0 ? YCBCR :
                                  (strcmp(argv[15], "opp"  ) == 0 ? OPP   : NONE))));
    if (color_space == NONE)
    {
        cout << "color_space is not known. Choice is :" << endl;
        cout << " -rgb" << endl;
        cout << " -yuv" << endl;
        cout << " -opp" << endl;
        cout << " -ycbcr" << endl;
        return EXIT_FAILURE;
    };
	unsigned       wh           = (unsigned) width * height;//输入图像的大小  按像素点个数来计
	unsigned       whc          = (unsigned) wh * chnls;//输入图像的存储大小 每个像书店由颜色空间值标记
	int           compute_bias = atoi(argv[10]);//偏移？
	int addnoise = atoi(argv[3]);//加噪值
	
	

	img_noisy.resize(whc);//
	img_diff.resize(whc);
	if (compute_bias)
	{
	    img_bias.resize(whc);
	    img_basic_bias.resize(whc);
	    img_diff_bias.resize(whc);
	}

	//! Add noise
	cout << endl << "Add noise [sigma = " << fSigma << "] ...";
			
	if (addnoise == 1)
		add_noise(img, img_noisy, fSigma); 	
	else 
		img_noisy = img; 
		
    cout << "done." << endl;


    //计时区函数
    auto start = std::chrono::high_resolution_clock::now();
    //! Denoising
    if (run_bm3d(fSigma, img_noisy, img_basic, img_denoised, width, height, chnls,
                 useSD_1, useSD_2, tau_2D_hard, tau_2D_wien, color_space)
        != EXIT_SUCCESS)
        return EXIT_FAILURE;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "bm3d run time:  " << duration.count() << " ms" << std::endl;


    //! Compute PSNR and RMSE
    float psnr, rmse, psnr_bias, rmse_bias;
    float psnr_basic, rmse_basic, psnr_basic_bias, rmse_basic_bias;
    if(compute_psnr(img, img_basic, &psnr_basic, &rmse_basic) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    if(compute_psnr(img, img_denoised, &psnr, &rmse) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    cout << endl << "For noisy image :" << endl;
    cout << "PSNR: " << psnr << endl;
    cout << "RMSE: " << rmse << endl << endl;
    cout << "(basic image) :" << endl;
    cout << "PSNR: " << psnr_basic << endl;
    cout << "RMSE: " << rmse_basic << endl << endl;

    if (compute_bias)
    {
        if (run_bm3d(fSigma, img, img_basic_bias, img_bias, width, height, chnls,
                     useSD_1, useSD_2, tau_2D_hard, tau_2D_wien, color_space)
            != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (compute_psnr(img, img_bias, &psnr_bias, &rmse_bias) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (compute_psnr(img, img_basic_bias, &psnr_basic_bias, &rmse_basic_bias)
            != EXIT_SUCCESS) return EXIT_FAILURE;

        cout << "For bias image :" << endl;
        cout << "PSNR: " << psnr_bias << endl;
        cout << "RMSE: " << rmse_bias << endl << endl;
        cout << "(basic bias image) :" << endl;
        cout << "PSNR: " << psnr_basic_bias << endl;
        cout << "RMSE: " << rmse_basic_bias << endl << endl;
    }

    //! writing measures
    char path[13] = "measures.txt";
    ofstream file(path, ios::out | ios::trunc);
    if(file)
    {
        file << endl << "************" << endl;
        file << "-sigma           = " << fSigma << endl;
        file << "-PSNR_basic      = " << psnr_basic << endl;
        file << "-RMSE_basic      = " << rmse_basic << endl;
        file << "-PSNR            = " << psnr << endl;
        file << "-RMSE            = " << rmse << endl << endl;
        if (compute_bias)
        {
            file << "-PSNR_basic_bias = " << psnr_basic_bias << endl;
            file << "-RMSE_basic_bias = " << rmse_basic_bias << endl;
            file << "-PSNR_bias       = " << psnr_bias << endl;
            file << "-RMSE_bias       = " << rmse_bias << endl;
        }
        cout << endl;
        file.close();
    }
    else
    {
        cout << "Can't open measures.txt !" << endl;
        return EXIT_FAILURE;
    }

	//! Compute Difference
	cout << endl << "Compute difference...";
	if (compute_diff(img, img_denoised, img_diff, fSigma) != EXIT_SUCCESS)
        return EXIT_FAILURE;
	if (compute_bias)
        if (compute_diff(img, img_bias, img_diff_bias, fSigma) != EXIT_SUCCESS)
            return EXIT_FAILURE;
    cout << "done." << endl;

	//! save noisy, denoised and differences images
	cout << endl << "Save images...";

	if (addnoise) {
		if (save_image(argv[4], img_noisy, width, height, chnls) != EXIT_SUCCESS)
			return EXIT_FAILURE; }

    if (save_image(argv[5], img_basic, width, height, chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

	if (save_image(argv[6], img_denoised, width, height, chnls) != EXIT_SUCCESS)
		return EXIT_FAILURE;

    if (save_image(argv[7], img_diff, width, height, chnls) != EXIT_SUCCESS)
		return EXIT_FAILURE;

    if (compute_bias)
    {
        if (save_image(argv[8], img_bias, width, height, chnls) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (save_image(argv[9], img_diff_bias, width, height, chnls) != EXIT_SUCCESS)
            return EXIT_FAILURE;
    }

    cout << "done." << endl;
    
 

	return EXIT_SUCCESS;
}
