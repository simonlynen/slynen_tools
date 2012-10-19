/*
 *   Written 2010 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   Minimalist (and dogslow) Tausworthe RNG
 */
#ifndef __RNG_H__
    #define __RNG_H__

    #include <stdint.h>

    struct RNG
    {
    private:
        uint32_t z0;
        uint32_t z1;
        uint32_t z2;
        uint32_t z3;

        uint32_t step()
        {
            uint32_t b0 = (((z0<< 6)^z0)>>13); z0 = (((z0 & 0xFFFFFFFE)<<18) ^b0);
            uint32_t b1 = (((z1<< 2)^z1)>>27); z1 = (((z1 & 0xFFFFFFF8)<< 2) ^b1);
            uint32_t b2 = (((z2<<13)^z2)>>21); z2 = (((z2 & 0xFFFFFFF0)<< 7) ^b2);
            uint32_t b3 = (((z3<< 3)^z3)>>12); z3 = (((z3 & 0xFFFFFF80)<<13) ^b3);
            return z0 ^ z1 ^ z2 ^ z3;
        }

    public:
        RNG(
            uint32_t seed = 0
        )
        {
            z0 = 0xAA0FC731;
            z1 = 0x4D72B1C4;
            z2 = 0xF82FD7D6;
            z3 = 0xA148D4E4;
            for(int32_t i=0; i<10; ++i) step();

            z0 ^= ( 3*seed + 7);
            z1 ^= ( 5*seed + 11);
            z2 ^= ( 7*seed + 13);
            z3 ^= (11*seed + 17);
            for(int32_t i=0; i<10; ++i) step();
        }

        uint32_t sample32() { return step();            }
        float    sampleF()  { return step()*0x1.0p-32;  }
    };

#endif // __RNG_H__

