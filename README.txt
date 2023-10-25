使用说明 
1.需先安装fftw3f

2.启用oneAPI工具环境

3.在intel devcloud上提交run_sycl.sh
qsub -l nodes=1:gpu:ppn=2 -d . run_sycl.sh
