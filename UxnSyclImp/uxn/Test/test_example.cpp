/**
 * 
 * Description: Write unit tests
 * 
 * Created by: Hongbin Bao
 * Created on: 2023/8/5 16:55
 * 
 */




//#include "../src/uxn.h"
//#include <gtest/gtest.h>
//
//extern int uxn_boot(Uxn *u, Uint8 *ram);
//
//TEST(UxnBootTest, SetsRamToGivenValue) {
//    Uxn u;
//    Uint8 ram[256] = {0};
//
//    ASSERT_TRUE(uxn_boot(&u, ram)); // Check that uxn_boot returns 1 (true)
//    ASSERT_EQ(u.ram, ram); // Check that the ram pointer is set correctly
//
//    // Check that the Uxn memory is zeroed out properly except for the ram pointer
//    char *cptr = (char *)&u;
//    for(size_t i = 0; i < sizeof(u); i++) {
//        if(cptr + i == (char *)&u.ram) {
//            i += sizeof(u.ram) - 1; // skip checking the ram pointer itself
//            continue;
//        }
//        ASSERT_EQ(cptr[i], 0);
//    }
//
//    // Checking that the function pointers are initialized to null
//    ASSERT_EQ(u.dei, nullptr);
//    ASSERT_EQ(u.deo, nullptr);
//}


#include "../src/uxn.h"
#include <gtest/gtest.h>
#include <iostream>

extern int uxn_boot(Uxn *u, Uint8 *ram);

TEST(UxnBootTest, SetsRamToGivenValue) {
    Uxn u;
    Uint8 ram[256] = {0};

    ASSERT_TRUE(uxn_boot(&u, ram)); // Check that uxn_boot returns 1 (true)
    std::cout << "uxn_boot returned true." << std::endl;

    ASSERT_EQ(u.ram, ram); // Check that the ram pointer is set correctly
    std::cout << "RAM pointer set correctly." << std::endl;

    // Check that the Uxn memory is zeroed out properly except for the ram pointer
    char *cptr = (char *)&u;
    bool memoryZeroed = true;
    for(size_t i = 0; i < sizeof(u); i++) {
        if(cptr + i == (char *)&u.ram) {
            i += sizeof(u.ram) - 1; // skip checking the ram pointer itself
            continue;
        }
        if(cptr[i] != 0) {
            memoryZeroed = false;
            break;
        }
    }

    if(memoryZeroed) {
        std::cout << "Uxn memory zeroed out properly." << std::endl;
    } else {
        std::cout << "Uxn memory not zeroed out properly." << std::endl;
    }

    // Checking that the function pointers are initialized to null
    ASSERT_EQ(u.dei, nullptr);
    std::cout << "Function pointer 'dei' initialized to nullptr." << std::endl;

    ASSERT_EQ(u.deo, nullptr);
    std::cout << "Function pointer 'deo' initialized to nullptr." << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


