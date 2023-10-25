#!/bin/bash
#==========================================
# Copyright © Intel Corporation
#
# SPDX-License-Identifier: MIT
#==========================================
# Script to submit job in Intel(R) DevCloud
# Version: 0.71
#==========================================
#启用oneAPI环境
. /opt/intel/oneapi/setvars.sh  > /dev/null
if [ -z "$(icpx --version)" ]; then
  echo "oneAPI env has some problem! Please check your settings." 
  echo "oneAPI env has some problem! Please check your settings."
  echo "oneAPI env has some problem! Please check your settings."
else
  echo "oneAPI env is OK!"
  echo "oneAPI env is OK!"
  echo "oneAPI env is OK!"
fi

#检查sycl环境是否配置成功
sycl-ls

# 设置 ONEAPI_DEVICE_SELECTOR 和 SYCL_PI_TRACE 环境变量
 

# 先编译
make 

#运行
./BM3Ddenoising lena.png 25.0 0 ImNoisy.dat ImBasic.dat ImDenoised.dat ImDiff.dat ImBias.dat ImDiffBias.dat 0 dct 1 dct 1 rgb


vtune -collect cpugpu-concurrency -result-dir vtune_data $(pwd)/BM3Ddenoising lena.png 25.0 0 ImNoisy.dat ImBasic.dat ImDenoised.dat ImDiff.dat ImBias.dat ImDiffBias.dat 0 dct 1 dct 1 rgb


vtune -report summary -result-dir vtune_data -format html -report-output $(pwd)/summary.html

