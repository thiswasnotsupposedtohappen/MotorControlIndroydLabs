#define BIT00 0b00000000000000000000000000000001
#define BIT01 0b00000000000000000000000000000010
#define BIT02 0b00000000000000000000000000000100
#define BIT03 0b00000000000000000000000000001000
#define BIT04 0b00000000000000000000000000010000
#define BIT05 0b00000000000000000000000000100000
#define BIT06 0b00000000000000000000000001000000
#define BIT07 0b00000000000000000000000010000000
#define BIT08 0b00000000000000000000000100000000
#define BIT09 0b00000000000000000000001000000000
#define BIT10 0b00000000000000000000010000000000
#define BIT11 0b00000000000000000000100000000000
#define BIT12 0b00000000000000000001000000000000
#define BIT13 0b00000000000000000010000000000000
#define BIT14 0b00000000000000000100000000000000
#define BIT15 0b00000000000000001000000000000000
#define BIT16 0b00000000000000010000000000000000
#define BIT17 0b00000000000000100000000000000000
#define BIT18 0b00000000000001000000000000000000
#define BIT19 0b00000000000010000000000000000000
#define BIT20 0b00000000000100000000000000000000
#define BIT21 0b00000000001000000000000000000000
#define BIT22 0b00000000010000000000000000000000
#define BIT23 0b00000000100000000000000000000000
#define BIT24 0b00000001000000000000000000000000
#define BIT25 0b00000010000000000000000000000000
#define BIT26 0b00000100000000000000000000000000
#define BIT27 0b00001000000000000000000000000000
#define BIT28 0b00010000000000000000000000000000
#define BIT29 0b00100000000000000000000000000000
#define BIT30 0b01000000000000000000000000000000
#define BIT31 0b10000000000000000000000000000000
#define uint8 unsigned char
#define uint16 unsigned short int
#define uint32 unsigned int
#define uint64 unsigned long int
#define int8 signed char
#define int16 signed short int
#define int32 signed int
#define int64 signed long int
#define float32 float
#define float64 double
#define float32pINF 0x7F800000
#define float32nINF 0xFF800000
#define float32NaN 0xFFC00000
#define float64pINF 0x7FF0000000000000
#define float64nINF 0xFFF0000000000000
#define float64NaN std::nan("")
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <algorithm>
using namespace std;

struct Modbus
{
    struct FrameReadTX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint16 crc;

        void CRC()
        {
            crc = 0xFFFF;
            crc ^= (uint16)(((uint8*)this)[0]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
            crc ^= (uint16)(((uint8*)this)[1]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
            crc ^= (uint16)(((uint8*)this)[2]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
            crc ^= (uint16)(((uint8*)this)[3]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
            crc ^= (uint16)(((uint8*)this)[4]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
            crc ^= (uint16)(((uint8*)this)[5]); for (int i = 8; i != 0; i--) { if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; } else crc >>= 1; }
        }
    }framereadtx;
    struct FrameReadRX
    {
        uint8 device;
        uint8 command;
        uint8 bytes;
		int32 data;
        uint16 crc;

        int32 CRC()
        {
            uint16 crctest = 0xFFFF;
            for (uint32 i = 0; i < (3); i++)
            {
                crctest ^= (uint16)(((uint8*)this)[i]);

                for (int i = 8; i != 0; i--)
                {
                    if ((crctest & 0x0001) != 0)
                    {
                        crctest >>= 1;
                        crctest ^= 0xA001;
                    }
                    else crctest >>= 1;
                }
            }
            for (uint32 i = 8; i < (uint32)(bytes + 8); i++)
            {
                crctest ^= (uint16)(((uint8*)this)[i]);

                for (int i = 8; i != 0; i--)
                {
                    if ((crctest & 0x0001) != 0)
                    {
                        crctest >>= 1;
                        crctest ^= 0xA001;
                    }
                    else crctest >>= 1;
                }
            }
            if (crctest == crc)
                return 0;
            else
                return -1;
        }
    }framereadrx;
    struct FrameWriteTX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint8 bytes;
        int32 data;
        uint16 crc;

        void CRC()
        {
            crc = 0xFFFF;
            for (uint32 i = 0; i < (7); i++)
            {
                crc ^= (uint16)(((uint8*)this)[i]);

                for (int i = 8; i != 0; i--)
                {
                    if ((crc & 0x0001) != 0)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else crc >>= 1;
                }
            }
            for (uint32 i = 8; i < (uint32)(bytes + 8); i++)
            {
                crc ^= (uint16)(((uint8*)this)[i]);

                for (int i = 8; i != 0; i--)
                {
                    if ((crc & 0x0001) != 0)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else crc >>= 1;
                }
            }
        }
    }framewritetx;
    struct FrameWriteRX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint16 crc;
    }framewriterx;

    void EndianTransform(uint16* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp = data[0];
        data[0] = data[1];
        data[1] = temp;
    }
    void EndianTransform(int16* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp = data[0];
        data[0] = data[1];
        data[1] = temp;
    }
    void EndianTransform(uint32* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp;
        temp = data[0];
        data[0] = data[3];
        data[3] = temp;
        temp = data[1];
        data[1] = data[2];
        data[2] = temp;
    }
    void EndianTransform(int32* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp;
        temp = data[0];
        data[0] = data[3];
        data[3] = temp;
        temp = data[1];
        data[1] = data[2];
        data[2] = temp;
    }
    void EndianTransform(uint64* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp;
        temp = data[0];
        data[0] = data[7];
        data[7] = temp;
        temp = data[1];
        data[1] = data[6];
        data[6] = temp;
        temp = data[2];
        data[2] = data[5];
        data[5] = temp;
        temp = data[3];
        data[3] = data[4];
        data[4] = temp;
    }
    void EndianTransform(int64* _data)
    {
        uint8* data = (uint8*)_data;
        uint8 temp;
        temp = data[0];
        data[0] = data[7];
        data[7] = temp;
        temp = data[1];
        data[1] = data[6];
        data[6] = temp;
        temp = data[2];
        data[2] = data[5];
        data[5] = temp;
        temp = data[3];
        data[3] = data[4];
        data[4] = temp;
    }

    void Initialize()
    {
        //UART Init Code
    }
    int32 Read(uint8 device, uint16 address,int16 &data)
    {
        framereadtx.device = device;
        framereadtx.command = 3;
        framereadtx.address = address; EndianTransform(&framereadtx.address);
        framereadtx.count = 4 / 2; EndianTransform(&framereadtx.count);
        framereadtx.CRC();

        //UART Code

        EndianTransform((int16*) & framereadrx.data);

        if (framereadrx.device != device || framereadrx.command != 3)
            return -1;

		data = framereadrx.data;

        return 0;
    }
    int32 ReadAnyways(uint8 device, uint16 address, int16& data)
    {
        framereadtx.device = device;
        framereadtx.command = 3;
        framereadtx.address = address; EndianTransform(&framereadtx.address);
        framereadtx.count = 4 / 2; EndianTransform(&framereadtx.count);
        framereadtx.CRC();

        //UART Code

        EndianTransform((int16*)&framereadrx.data);

        data = framereadrx.data;

        return 0;
    }
    int32 Write(uint8 device, uint16 address, int16 data)
    {
        framewritetx.data = data;

        framewritetx.device = device;
        framewritetx.command = 16;
        framewritetx.address = address; EndianTransform(&framewritetx.address);
        framewritetx.count = 4 / 2; EndianTransform(&framewritetx.count);
        framewritetx.bytes = 4;

        EndianTransform((int16*)&framewritetx.data);
        framewritetx.CRC();

        //UART Code

        if (framewriterx.device != device || framewriterx.command != 16)
            return -1;

        return 0;
    }

    int32 WriteReadback(uint8 device, uint16 address, int16 data)
    {
        if (Write(device, address, data) != 0)
            return -1;
        framereadrx.data = 0xCCCC;
        if (Read(device, address, data) != 0)
            return -1;
        if (framereadrx.data != data)
            return -1;
        return 0;
    }
};
uint8 slaveid = 1;
void main()
{
	while (1);
}