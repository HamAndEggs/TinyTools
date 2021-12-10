#!/usr/bin/seabang --debug

#include <iostream>
#include <thread>
#include <vector>
#include <assert.h>
#include <bitset>

#include "../TinyTools.h"
#include "../TinyTools.cpp"

static void TestBitPacking(int pNumValues)
{
    std::cout << pNumValues << " -> ";
    std::vector<uint8_t> randomData;

    for( int n = 0 ; n < pNumValues ; n++ )
    {
        randomData.push_back(rand()&255);
    }

    uint8_t* out = nullptr;
    const size_t outSize = tinytools::network::Encode7Bit(randomData.data(),randomData.size(),&out);
    assert(out);

    std::cout << outSize << " -> ";

    for( size_t n = 0 ; n < outSize ; n++ )
    {
        assert( out[n] < 128 );
    }

    // Now rebuild data.
    uint8_t* out8 = nullptr;
    const size_t out8Size = tinytools::network::Decode7Bit(out,outSize,&out8);
    assert(out8);
    assert( out8Size == randomData.size() );

    std::cout << out8Size << "\r";

    for( size_t n = 0 ; n < randomData.size() ; n++ )
    {
        if( out8[n] != randomData[n] )
        {
            std::cerr << "Error at " << n << "\n";
            std::cout << std::bitset<8>(out8[n]) << " " << std::bitset<8>(randomData[n]) << "\n";
        }
        assert( out8[n] == randomData[n] );
    }

    delete []out;
    delete []out8;
}

int main(int argc, char *argv[])
{
    // Run as two threads to ensure no threading issues.

    std::thread otherThread = std::thread([]()
    {
        for( int z = 1 ; z < 100000 ; z++ )
        {
            TestBitPacking(z);
        }
    });

    for( int z = 1 ; z < 100000 ; z++ )
    {
        TestBitPacking(z);
    }

    if( otherThread.joinable() )
    {
        otherThread.join();
    }

    std::cout << "\nAll good\n";

    return EXIT_SUCCESS;
}
