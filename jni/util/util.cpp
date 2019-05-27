#include "../main.h"

uintptr_t FindLibrary(const char* library)
{
    char filename[0xFF] = {0},
    buffer[2048] = {0};
    FILE *fp = 0;
    uintptr_t address = 0;

    sprintf( filename, "/proc/%d/maps", getpid() );

    fp = fopen( filename, "rt" );
    if(fp == 0)
    {
        Log("ERROR: can't open file %s", filename);
        goto done;
    }

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if( strstr( buffer, library ) )
        {
            address = (uintptr_t)strtoul( buffer, 0, 16 );
            break;
        }
    }

    done:

    if(fp)
      fclose(fp);

    return address;
}
            
void iso_8859_11_to_utf8(char *out, const char *in, unsigned int len)
{
    static const int table[128] = 
    {    
        // 80
        0,     0,     0,   0,     0,   0,   0,   0,
        0,   0,   0,     0,   0,     0,     0,     0,
        // 90
        0,     0,   0,   0,   0,   0,   0,   0,
        0,      0,   0,     0,   0,     0,     0,     0,                
        // A0
        0x80b8e0,0x81b8e0,0x82b8e0,0x83b8e0,0x84b8e0,0x85b8e0,0x86b8e0,0x87b8e0,
        0x88b8e0,0x89b8e0,0x8ab8e0,0x8bb8e0,0x8cb8e0,0x8db8e0,0x8eb8e0,0x8fb8e0,
        // B0
        0x90b8e0,0x91b8e0,0x92b8e0,0x93b8e0,0x94b8e0,0x95b8e0,0x96b8e0,0x97b8e0,
        0x98b8e0,0x99b8e0,0x9ab8e0,0x9bb8e0,0x9cb8e0,0x9db8e0,0x9eb8e0,0x9fb8e0,
        // C0
        0xa0b8e0,0xa1b8e0,0xa2b8e0,0xa3b8e0,0xa4b8e0,0xa5b8e0,0xa6b8e0,0xa7b8e0,
        0xa8b8e0,0xa9b8e0,0xaab8e0,0xabb8e0,0xacb8e0,0xadb8e0,0xaeb8e0,0xafb8e0,
        // D0
        0xb0b8e0,0xb1b8e0,0xb2b8e0,0xb3b8e0,0xb4b8e0,0xb5b8e0,0xb6b8e0,0xb7b8e0,
        0xb8b8e0,0xb9b8e0,0xbab8e0,0       ,0       ,0       ,0       ,0xbfb8e0,
        // E0
        0x80b9e0,0x81b9e0,0x82b9e0,0x83b9e0,0x84b9e0,0x85b9e0,0x86b9e0,0x87b9e0,
        0x88b9e0,0x89b9e0,0x8ab9e0,0x8bb9e0,0x8cb9e0,0x8db9e0,0x8eb9e0,0x8fb9e0,
        // F0
        0x90b9e0,0x91b9e0,0x92b9e0,0x93b9e0,0x94b9e0,0x95b9e0,0x96b9e0,0x97b9e0,
        0x98b9e0,0x99b9e0,0x9ab9e0,0x9bb9e0
    };

    int count = 0;

    while (*in)
    {
        if(len && (count >= len)) break;

        // if (*in > 0x7e)
        if (*in > 0x80)
        {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;

            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            *out++ = (char)(v >> 16);
            /*char v = (int)&in++[count];

            if (((0xa1 <= v) && (v <= 0xda))
            || ((0xdf <= v) && (v <= 0xfb))) {
                char utf8Char = 0x0e00 + v - 0xa0;

                char utf8Byte1 = (char)(0xe0 | (utf8Char >> 12));
                *out++ = utf8Byte1;
                char utf8Byte2 = (char)(0x80 | ((utf8Char >> 6) & 0x3f));
                *out++ = utf8Byte2;
                char utf8Byte3 = (char)(0x80 | (utf8Char & 0x3f));
                *out++ = utf8Byte3;
            }*/

        }
        else // ASCII
            *out++ = *in++;

        count++;
    }

    *out = 0;
}

void cp1251_to_utf8(char *out, const char *in, unsigned int len)
{
    static const int table[128] = 
    {                    
        // 80
        0x82D0,     0x83D0,     0x9A80E2,   0x93D1,     0x9E80E2,   0xA680E2,   0xA080E2,   0xA180E2,
        0xAC82E2,   0xB080E2,   0x89D0,     0xB980E2,   0x8AD0,     0x8CD0,     0x8BD0,     0x8FD0,
        // 90
        0x92D1,     0x9880E2,   0x9980E2,   0x9C80E2,   0x9D80E2,   0xA280E2,   0x9380E2,   0x9480E2,
        0,          0xA284E2,   0x99D1,     0xBA80E2,   0x9AD1,     0x9CD1,     0x9BD1,     0x9FD1,
        // A0
        0xA0C2,     0x8ED0,     0x9ED1,     0x88D0,     0xA4C2,     0x90D2,     0xA6C2,     0xA7C2,              
        0x81D0,     0xA9C2,     0x84D0,     0xABC2,     0xACC2,     0xADC2,     0xAEC2,     0x87D0,
        // B0
        0xB0C2,     0xB1C2,     0x86D0,     0x96D1,     0x91D2,     0xB5C2,     0xB6C2,     0xB7C2,              
        0x91D1,     0x9684E2,   0x94D1,     0xBBC2,     0x98D1,     0x85D0,     0x95D1,     0x97D1,
        // C0
        0x90D0,     0x91D0,     0x92D0,     0x93D0,     0x94D0,     0x95D0,     0x96D0,     0x97D0,
        0x98D0,     0x99D0,     0x9AD0,     0x9BD0,     0x9CD0,     0x9DD0,     0x9ED0,     0x9FD0,
        // D0
        0xA0D0,     0xA1D0,     0xA2D0,     0xA3D0,     0xA4D0,     0xA5D0,     0xA6D0,     0xA7D0,
        0xA8D0,     0xA9D0,     0xAAD0,     0xABD0,     0xACD0,     0xADD0,     0xAED0,     0xAFD0,
        // E0
        0xB0D0,     0xB1D0,     0xB2D0,     0xB3D0,     0xB4D0,     0xB5D0,     0xB6D0,     0xB7D0,
        0xB8D0,     0xB9D0,     0xBAD0,     0xBBD0,     0xBCD0,     0xBDD0,     0xBED0,     0xBFD0,
        // F0
        0x80D1,     0x81D1,     0x82D1,     0x83D1,     0x84D1,     0x85D1,     0x86D1,     0x87D1,
        0x88D1,     0x89D1,     0x8AD1,     0x8BD1,     0x8CD1,     0x8DD1,     0x8ED1,     0x8FD1
    };

    int count = 0;

    while (*in)
    {
        if(len && (count >= len)) break;

        if (*in & 0x80)
        {
            register int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;   
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        }
        else // ASCII
            *out++ = *in++;

        count++;
    }

    *out = 0;
}