# UxnVM
**SYCL implementation of the Uxn VM**

Document Introduction



[UxnSourceCode/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSourceCode/uxn)

It is mainly some analysis of the source code of the Uxn VM project. Until of 2022.07.09, some comments to help understanding are added to some c files under src, and a CMake file is added to the project to facilitate the use of development tools for compilation



[UxnSyclImp/uxn](https://github.com/Hongbin-Bao/UxnVM-SYCL/tree/main/UxnSyclImp/uxn)

Mainly for the SYCL implementation of Uxn VM, the project currently carries the source code of Uxn VM

**What is currently being done**: Reconstruct the `c` files in the scr directory into `cpp` files, because `syclcc` can only compile cpp files.



Recommended Development Tools：Clion https://www.jetbrains.com/clion/

### 1.Some necessary study materials

​	a.Uxn VM sources: https://git.sr.ht/~rabbits/uxn

​	b.Uxn Tutorial

​				uxn tutorial: https://compudanzas.net/uxn_tutorial.html

​				urn project introduce: https://wiki.xxiivv.com/site/uxn.html

​	c.SYCL Tutorial: https://github.com/codeplaysoftware/syclacademy

​	d. SDL API: https://www.libsdl.org/

​	e.CMake Tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/index.html



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



