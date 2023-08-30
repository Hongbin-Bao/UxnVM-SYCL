# UxnVM
**SYCL implementation of the Uxn VM**

https://github.com/Hongbin-Bao/UxnVM-SYCL

Document Introduction



[UxnSourceCode/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSourceCode/uxn)

It is mainly some analysis of the source code of the Uxn VM project. Until of 2022.07.09, some comments to help understanding are added to some c files under src, and a CMake file is added to the project to facilitate the use of development tools for compilation,You can ignore this file as it is just the source code of UxnVM and is not used for doing any development. Just added some comments to help me understand at the beginning of the project. But I don't want to delete it, because the original UxnVM project is still under development, so it is necessary to keep a self-developed version. The UxnVM version developed by the project is July 1, 2023

[UxnSyclImp/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSyclImp/uxn)

Mainly for the SYCL implementation of Uxn VM

Recommended Development Tools：Clion https://www.jetbrains.com/clion/

### 1.Some necessary study materials

​	a.Uxn VM sources: https://git.sr.ht/~rabbits/uxn

​	b.Uxn Tutorial

​				uxn tutorial: https://compudanzas.net/uxn_tutorial.html

​				urn project introduce: https://wiki.xxiivv.com/site/uxn.html

​	c.SYCL Tutorial: https://github.com/codeplaysoftware/syclacademy

​	d. SDL API: https://www.libsdl.org/

​	e.CMake Tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/index.html

[UxnSyclImp/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSyclImp/uxn)

In addition, it is necessary to learn the features of the language after C++11, as well as knowledge of assembly language and the principles of computer composition.



### 2.Environment build

Compile tool download for MacOS with Apple chip:

https://github.com/OpenSYCL/OpenSYCL  (I personally recommend)

https://github.com/triSYCL/triSYCL

(It took me a lot of time to set up the environment and configure the development tools.)



If you use windows or linux environment to develop(things will become a lot easier):

https://www.intel.com/content/www/us/en/developer/tools/oneapi/data-parallel-c-plus-plus.html

https://www.intel.com/content/www/us/en/developer/articles/technical/compiling-sycl-with-different-gpus.html

https://developer.codeplay.com/products/oneapi/nvidia/2023.0.0/guides/get-started-guide-nvidia



If you want to use remote development, Intel Devcloud is very convenient

https://devcloud.intel.com/oneapi/documentation/connect-with-vscode/

You just need to clone the code and go to[UxnSyclImp/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSyclImp/uxn)



Find build.sh,You need to modify  CC="${CC:-syclcc}" in the script
 to CC="${CC:-icpx -fsycl}"

and execute the build script

```
 ./build.sh
```



### 3.Some necessary introductions

1.The project comprises three executables: uxnasm, uxncli, and uxnemu. Uxnasm converts an assembly language tal file into a rom file, while uxncli and uxnemu are virtual machine implementations, the latter being the version with graphical user interface (GUI).

2.In order to facilitate development, the Cmake file is added to the project

3.For test cases that have no output, you may see errors:terminate called after throwing an instance of 'sycl::_V1::runtime_error'
what(): Native API failed. Native API returns: -1 (PI_ERROR_DEVICE_NOT_FOUND) -1 (PI_ERROR_DEVICE_NOT_FOUND)
Aborted

This is because:

A workload that takes more than four seconds for GPU hardware to execute is a long-running workload. By default, individual threads that qualify as long-running workloads are considered hung and are terminated.https://www.intel.com/content/www/us/en/docs/oneapi/installation-guide-linux/2023-2/gpu-disable-hangcheck.html

However, due to the current imperfect development of SYCL, this error looks more like the GPU device was not found. It has been fed back to the intel/llvm open source community.https://github.com/intel/llvm/issues/10944

4.Some test case, you can find these places:

https://codeberg.org/wimvanderbauwhede/nito/src/branch/main/demos

https://git.sr.ht/~rabbits/uxn/tree/main/item/projects/examples/exercises/fib.tal

https://git.sr.ht/~rabbits/uxn/tree/main/item/projects/software/piano.tal
