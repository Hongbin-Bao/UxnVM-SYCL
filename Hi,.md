Hi,

When I use SYCL, I have encountered a problem, when there is an infinite loop in the SYCL kernel code, it will go wrong
The following is a reproduction of the problem

The reason for the problem is that there is an infinite loop in the SYCL kernel code. Even if a certain state of the infinite loop will stop, the same error will occur. 



The environment needs to be GPU

Create a test file:

infinite_loop.cpp



```
#include <CL/sycl.hpp>

int main() {
    cl::sycl::queue queue;

   
    std::vector<int> data(1, 42);
    cl::sycl::buffer<int, 1> buffer(data.data(), data.size());

    
    queue.submit([&](cl::sycl::handler& cgh) {
        auto acc = buffer.get_access<cl::sycl::access::mode::read_write>(cgh);

        cgh.parallel_for<class infinite_loop>(
            cl::sycl::range<1>(data.size()), 
            [=](cl::sycl::id<1> idx) {
                for(;;) { 
                    
                }
            });
    });

    queue.wait_and_throw();

    return 0;
}


```



compile and run

```
icpx -fsycl infinite_loop.cpp -o infinite_loop
```

```
./infinite_loop
```



When I test on devcloud:
If I use: Intel(R) UHD Graphics [0x9a60]
The following error will appear:
terminate called after throwing an instance of 'sycl::_V1::runtime_error'
   what(): Native API failed. Native API returns: -1 (PI_ERROR_DEVICE_NOT_FOUND) -1 (PI_ERROR_DEVICE_NOT_FOUND)
Aborted

If I use: Intel(R) UHD Graphics P630 [0x3e96]
The program still fails to run SYCL queue tasks, and there is no prompt.

[The cause of the problem seems to be due to: A workload that takes more than four seconds for GPU hardware to execute is a long-running workload. By default, individual threads that qualify as long-running workloads are considered hung and are terminated.](https://www.intel.com/content/www/us/en/docs/oneapi/installation-guide-linux/2023-2/gpu-disable-hangcheck.html)

Of course, under normal circumstances, few people will try to let the GPU not do any output for 4s

But if it is because of this reason, I think the error throwing prompt is not obvious, and this output is more like not finding the GPU, which will cause a lot of trouble, and I don’t know why some of them appear on different graphics cards Error prompts, some do not.

For this situation, I think it should throw a more appropriate exception.

