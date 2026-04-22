#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <setupapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wrl.h>
#include <string>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "webview2_sdk/build/native/include/WebView2.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "webview2_sdk/build/native/x64/WebView2Loader.dll.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
using namespace Microsoft::WRL;

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
#define uint64 unsigned long long int
#define int8 signed char
#define int16 signed short int
#define int32 signed int
#define int64 signed long long int
#define float32 float
#define float64 double
struct MotorController
{
    #pragma pack(push, 1)
    struct ModbusFrameReadTX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint16 crc;
    };
    struct ModbusFrameReadRX
    {
        uint8 device;
        uint8 command;
        uint8 bytes;
        int32 data;
        uint16 crc;
    };
    struct ModbusFrameWriteTX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint8 bytes;
        int32 data;
        uint16 crc;
    };
    struct ModbusFrameWriteRX
    {
        uint8 device;
        uint8 command;
        uint16 address;
        uint16 count;
        uint16 crc;
    };
    #pragma pack(pop)

    struct Modbus
    {
        ModbusFrameReadTX framereadtx;
        ModbusFrameReadRX framereadrx;
        ModbusFrameWriteTX framewritetx;
        ModbusFrameWriteRX framewriterx;
        HANDLE hSerial;
        bool connected;

        void CrcCalc(uint8* buf, uint32 len, uint16* out_crc)
        {
            uint16 crc = 0xFFFF;
            for (uint32 i = 0; i < len; i++)
            {
                crc ^= (uint16)buf[i];
                for (int j = 8; j != 0; j--)
                {
                    if ((crc & 0x0001) != 0)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else crc >>= 1;
                }
            }
            *out_crc = crc;
        }

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
            temp = data[0]; data[0] = data[3]; data[3] = temp;
            temp = data[1]; data[1] = data[2]; data[2] = temp;
        }
        void EndianTransform(int32* _data)
        {
            uint8* data = (uint8*)_data;
            uint8 temp;
            temp = data[0]; data[0] = data[3]; data[3] = temp;
            temp = data[1]; data[1] = data[2]; data[2] = temp;
        }

        Modbus()
        {
            hSerial = INVALID_HANDLE_VALUE;
            connected = false;
            memset(&framereadtx, 0, sizeof(framereadtx));
            memset(&framereadrx, 0, sizeof(framereadrx));
            memset(&framewritetx, 0, sizeof(framewritetx));
            memset(&framewriterx, 0, sizeof(framewriterx));
        }

        int32 Initialize(const char* com_port)
        {
            char port_path[64];
            sprintf_s(port_path, sizeof(port_path), "\\\\.\\%s", com_port);

            wchar_t wport[64];
            MultiByteToWideChar(CP_ACP, 0, port_path, -1, wport, 64);

            hSerial = CreateFileW(wport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hSerial == INVALID_HANDLE_VALUE)
                return -1;

            DCB dcb;
            memset(&dcb, 0, sizeof(dcb));
            dcb.DCBlength = sizeof(DCB);
            if (!GetCommState(hSerial, &dcb))
            {
                CloseHandle(hSerial);
                hSerial = INVALID_HANDLE_VALUE;
                return -1;
            }
            dcb.BaudRate = 9600;
            dcb.ByteSize = 8;
            dcb.Parity = NOPARITY;
            dcb.StopBits = ONESTOPBIT;
            dcb.fBinary = TRUE;
            dcb.fParity = FALSE;
            dcb.fOutxCtsFlow = FALSE;
            dcb.fOutxDsrFlow = FALSE;
            dcb.fDtrControl = DTR_CONTROL_ENABLE;
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fOutX = FALSE;
            dcb.fInX = FALSE;
            if (!SetCommState(hSerial, &dcb))
            {
                CloseHandle(hSerial);
                hSerial = INVALID_HANDLE_VALUE;
                return -1;
            }

            COMMTIMEOUTS timeouts;
            memset(&timeouts, 0, sizeof(timeouts));
            timeouts.ReadIntervalTimeout = 50;
            timeouts.ReadTotalTimeoutMultiplier = 10;
            timeouts.ReadTotalTimeoutConstant = 1000;
            timeouts.WriteTotalTimeoutMultiplier = 10;
            timeouts.WriteTotalTimeoutConstant = 1000;
            SetCommTimeouts(hSerial, &timeouts);

            connected = true;
            return 0;
        }

        void Close()
        {
            if (hSerial != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hSerial);
                hSerial = INVALID_HANDLE_VALUE;
            }
            connected = false;
        }

        int32 Read(uint8 device, uint16 address, int16& data)
        {
            if (!connected) return -1;

            framereadtx.device = device;
            framereadtx.command = 3;
            framereadtx.address = address; EndianTransform(&framereadtx.address);
            framereadtx.count = 2; EndianTransform(&framereadtx.count);
            CrcCalc((uint8*)&framereadtx, 6, &framereadtx.crc);

            PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

            DWORD bytes_written = 0;
            if (!WriteFile(hSerial, &framereadtx, sizeof(framereadtx), &bytes_written, NULL) || bytes_written != sizeof(framereadtx))
                return -1;

            memset(&framereadrx, 0, sizeof(framereadrx));
            DWORD bytes_read = 0;
            if (!ReadFile(hSerial, &framereadrx, sizeof(framereadrx), &bytes_read, NULL) || bytes_read != sizeof(framereadrx))
                return -1;

            // Verify CRC
            uint16 crc_check;
            CrcCalc((uint8*)&framereadrx, 3 + framereadrx.bytes, &crc_check);
            if (crc_check != framereadrx.crc)
                return -1;

            if (framereadrx.device != device || framereadrx.command != 3)
                return -1;

            EndianTransform((int16*)&framereadrx.data);
            data = (int16)framereadrx.data;

            return 0;
        }

        int32 Write(uint8 device, uint16 address, int16 data)
        {
            if (!connected) return -1;

            framewritetx.data = data;
            framewritetx.device = device;
            framewritetx.command = 16;
            framewritetx.address = address; EndianTransform(&framewritetx.address);
            framewritetx.count = 2; EndianTransform(&framewritetx.count);
            framewritetx.bytes = 4;

            EndianTransform((int16*)&framewritetx.data);
            CrcCalc((uint8*)&framewritetx, 7 + framewritetx.bytes, &framewritetx.crc);

            PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

            DWORD bytes_written = 0;
            if (!WriteFile(hSerial, &framewritetx, sizeof(framewritetx), &bytes_written, NULL) || bytes_written != sizeof(framewritetx))
                return -1;

            memset(&framewriterx, 0, sizeof(framewriterx));
            DWORD bytes_read = 0;
            if (!ReadFile(hSerial, &framewriterx, sizeof(framewriterx), &bytes_read, NULL) || bytes_read != sizeof(framewriterx))
                return -1;

            if (framewriterx.device != device || framewriterx.command != 16)
                return -1;

            return 0;
        }
    };

    Modbus modbus;
    #define MAX_RPM             1728
    #define MAX_FREQ_HZ         60
    #define VFD_SLAVE_ID        1
    #define VFD_ADDR_CONTROL    0x2000
    #define VFD_ADDR_FREQ_WRITE 0x2001
    #define VFD_ADDR_RESET      0x2002
    #define VFD_ADDR_FREQUENCY  0x2103
    #define VFD_ADDR_CURRENT    0x2104
    #define VFD_ADDR_VOLTAGE    0x2105
    #define VFD_CMD_RESET       (int16)(BIT01)
    #define VFD_CMD_START       (int16)(BIT04 | BIT01)
    #define VFD_CMD_STOP        (int16)(BIT00)

    int32 Initialize(const char* com_port)
    {
        return modbus.Initialize(com_port);
    }
    bool IsConnected()
    {
        return modbus.connected;
    }
    int32 Start()
    {
        return modbus.Write(VFD_SLAVE_ID, VFD_ADDR_CONTROL, VFD_CMD_START);
    }
    int32 Stop()
    {
        return modbus.Write(VFD_SLAVE_ID, VFD_ADDR_CONTROL, VFD_CMD_STOP);
    }
    int32 Reset()
    {
        return modbus.Write(VFD_SLAVE_ID, VFD_ADDR_CONTROL, VFD_CMD_RESET);
    }
    int32 SetRPM(float64 rpm)
    {
        if (rpm <= 0) return 0;
        if (rpm > MAX_RPM) rpm = MAX_RPM;
        float64 freq_hz = ((float64)rpm / (float64)MAX_RPM) * MAX_FREQ_HZ;
        float64 f = (freq_hz * 100.0);
        return modbus.Write(VFD_SLAVE_ID, VFD_ADDR_FREQ_WRITE, (int16)f);
    }
    float64 GetRPM()
    {
        int16 f;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_FREQUENCY, f) != 0)
			return -1.0;
        float64 freq_hz = ((float64)f / 100.0);
        float64 rpm = (freq_hz / MAX_FREQ_HZ) * MAX_RPM;
        return rpm;
    }
    float64 GetCurrent()
    {
        int16 c;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_CURRENT, c) != 0)
            return -1.0;
        float64 current = ((float64)c) / 100.0;
        return current;
    }
    float64 GetVoltage()
    {
        int16 v;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_VOLTAGE, v) != 0)
            return -1.0;
        float64 voltage = ((float64)v) / 10.0;
        return voltage;
    }
    void Release()
    {
        modbus.Close();
    }
}motorcontroller;

// ============================================================================
// Constants
// ============================================================================
#define MAX_STEPS           16
#define MAX_SOUND_PATH      260
#define TIMER_PROGRAM       1
#define TIMER_PROGRAM_MS    500
#define TIMER_STATUS        2
#define TIMER_STATUS_MS     100

// ============================================================================
// Data structures
// ============================================================================
struct ProfileStep
{
    int32 motor_rpm;
    int32 duration;
    char sound_file[MAX_SOUND_PATH];
};

#define MAX_LOGS 1000000
struct RideLog
{
    char date[16];
    char time[16];
    int32 num_people;
};

struct AppSettings
{
    char com_port[64];
    int32 num_seats;
    int32 difficulty;
    int32 total_rides;
    ProfileStep profiles[2][MAX_STEPS];
    
    int32 num_logs;
    RideLog logs[MAX_LOGS];
};

// ============================================================================
// Global state
// ============================================================================
static HWND g_hwnd;

// Audio state
static ma_engine g_audioEngine;
static ma_sound g_currentSound;
static bool g_isAudioEngineInitialized = false;
static bool g_isSoundLoaded = false;

// WebView2 pointers
static ComPtr<ICoreWebView2> g_webview;
static ComPtr<ICoreWebView2Controller> g_controller;

static AppSettings settings;

static bool program_running = false;
static int32 current_step = 0;
static DWORD program_start_tick = 0;
static DWORD step_start_tick = 0;

// ============================================================================
// Helper: Configuration Persistence
// ============================================================================
void SaveSettings()
{
    FILE* f = NULL;
    fopen_s(&f, "config.bin", "wb");
    if (f)
    {
        size_t offset = (char*)&settings.logs - (char*)&settings;
        size_t size_to_write = offset + settings.num_logs * sizeof(RideLog);
        fwrite(&settings, 1, size_to_write, f);
        fclose(f);
    }
}

void LoadSettings()
{
    FILE* f = NULL;
    fopen_s(&f, "config.bin", "rb");
    if (f)
    {
        fread(&settings, 1, sizeof(AppSettings), f);
        fclose(f);
    }
}

// ============================================================================
// Helper: COM port enumeration
// ============================================================================
struct ComPortInfo
{
    char port_name[32];
    char friendly_name[256];
};

static ComPortInfo g_com_ports[64];
static int g_com_port_count = 0;


void EnumerateComPorts()
{
    g_com_port_count = 0;

    static const GUID GUID_CLASS_PORTS = { 0x4D36E978, 0xE325, 0x11CE,{ 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };

    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_CLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) return;

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData) && g_com_port_count < 64; i++)
    {
        char friendly[256] = { 0 };
        if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendly, sizeof(friendly), NULL))
        {
            char port_name[32] = { 0 };
            char* p = strstr(friendly, "(COM");
            if (p)
            {
                p++;
                char* end = strchr(p, ')');
                if (end)
                {
                    int len = (int)(end - p);
                    if (len < 32)
                    {
                        strncpy_s(port_name, sizeof(port_name), p, len);
                    }
                }
            }

            if (port_name[0] != '\0')
            {
                strcpy_s(g_com_ports[g_com_port_count].port_name, sizeof(g_com_ports[g_com_port_count].port_name), port_name);
                strcpy_s(g_com_ports[g_com_port_count].friendly_name, sizeof(g_com_ports[g_com_port_count].friendly_name), friendly);
                g_com_port_count++;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
}

// ============================================================================
// Helper: Send data to WebView2
// ============================================================================
void SendComPortsToWeb()
{
    if (!g_webview) return;
    EnumerateComPorts();

    std::wstring json = L"{\"type\":\"COM_PORTS\",\"ports\":[";
    for (int i = 0; i < g_com_port_count; i++)
    {
        wchar_t wfriendly[256];
        MultiByteToWideChar(CP_ACP, 0, g_com_ports[i].friendly_name, -1, wfriendly, 256);
        
        wchar_t port_json[512];
        _snwprintf_s(port_json, 512, L"{\"name\":\"%S\",\"friendly\":\"%s\"}%s", 
            g_com_ports[i].port_name, wfriendly, (i == g_com_port_count - 1) ? L"" : L",");
        json += port_json;
    }
    json += L"],\"selected\":\"";
    json += std::wstring(settings.com_port, settings.com_port + strlen(settings.com_port));
    json += L"\"}";

    g_webview->PostWebMessageAsJson(json.c_str());
}

void SendProfileToWeb()
{
    if (!g_webview) return;
    int diff = settings.difficulty;

    std::wstring json = L"{\"type\":\"LOAD_PROFILE\",\"difficulty\":" + std::to_wstring(diff) + L",\"seats\":" + std::to_wstring(settings.num_seats) + L",\"total_rides\":" + std::to_wstring(settings.total_rides) + L",\"profile\":[";
    for (int i = 0; i < MAX_STEPS; i++)
    {
        wchar_t wsnd[MAX_SOUND_PATH];
        MultiByteToWideChar(CP_ACP, 0, settings.profiles[diff][i].sound_file, -1, wsnd, MAX_SOUND_PATH);
        
        // Escape backslashes for JSON
        std::wstring sound_path = wsnd;
        size_t start_pos = 0;
        while((start_pos = sound_path.find(L"\\", start_pos)) != std::string::npos) 
        {
             sound_path.replace(start_pos, 1, L"\\\\");
             start_pos += 2;
        }

        wchar_t step_json[1024];
        _snwprintf_s(step_json, 1024, L"{\"motor_rpm\":%d,\"duration\":%d,\"sound_file\":\"%s\"}%s",
            settings.profiles[diff][i].motor_rpm,
            settings.profiles[diff][i].duration,
            sound_path.c_str(),
            (i == MAX_STEPS - 1) ? L"" : L",");
        json += step_json;
    }
    json += L"]}";

    g_webview->PostWebMessageAsJson(json.c_str());
}

// ============================================================================
// Helper: Default profiles
// ============================================================================
void SetDefaultProfiles()
{
    memset(&settings, 0, sizeof(settings));
    settings.num_seats = 1;
    settings.difficulty = 0;
    settings.total_rides = 0;
    strcpy_s(settings.com_port, sizeof(settings.com_port), "");

    for (int d = 0; d < 2; d++) 
    {
        for (int i = 0; i < MAX_STEPS; i++) 
        {
            settings.profiles[d][i].motor_rpm = 0;
            settings.profiles[d][i].duration = 0;
            settings.profiles[d][i].sound_file[0] = '\0';
        }
    }
}

// ============================================================================
// Helper: Update status display
// ============================================================================
void UpdateStatusDisplay()
{
    if (!g_webview) return;

    wchar_t json[1024];
    
    // Send status update
    DWORD elapsed_sec = 0;
    if (program_running) elapsed_sec = (GetTickCount() - program_start_tick) / 1000;

    _snwprintf_s(json, 1024, L"{\"type\":\"UPDATE_STATUS\",\"data\":{\"connected\":%s,\"running\":%s,\"elapsedTime\":%d,\"currentStep\":%d}}",
        motorcontroller.IsConnected() ? L"true" : L"false",
        program_running ? L"true" : L"false",
        elapsed_sec,
        current_step);
    g_webview->PostWebMessageAsJson(json);

    // Send telemetry update
    float64 rpm = 0;
    float64 amps = 0;
    float64 volts = 0;

    if (motorcontroller.IsConnected())
    {
		rpm = motorcontroller.GetRPM();
		amps = motorcontroller.GetCurrent();
		volts = motorcontroller.GetVoltage();
		if (rpm < 0) rpm = 0;
		if (amps < 0) amps = 0;
		if (volts < 0) volts = 0;
    }

    _snwprintf_s(json, 1024, L"{\"type\":\"UPDATE_TELEMETRY\",\"data\":{\"rpm\":%.2f,\"current\":%.2f,\"voltage\":%.2f}}",
        rpm, amps, volts);
    g_webview->PostWebMessageAsJson(json);
}

// ============================================================================
// Helper: Browse for sound file
// ============================================================================
void BrowseSoundFile(HWND hwnd, int step_index)
{
    OPENFILENAMEW ofn;
    wchar_t filename[MAX_PATH] = L"";
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Audio Files (*.wav;*.mp3)\0*.wav;*.mp3\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"wav";

    if (GetOpenFileNameW(&ofn))
    {
        int diff = settings.difficulty;
        WideCharToMultiByte(CP_ACP, 0, filename, -1, settings.profiles[diff][step_index].sound_file, MAX_SOUND_PATH, NULL, NULL);

        // Notify webview
        wchar_t json[1024];
        const wchar_t* name_only = wcsrchr(filename, L'\\');
        if (name_only) name_only++; else name_only = filename;

        std::wstring sound_path = filename;
        size_t start_pos = 0;
        while((start_pos = sound_path.find(L"\\", start_pos)) != std::string::npos) 
        {
             sound_path.replace(start_pos, 1, L"\\\\");
             start_pos += 2;
        }

        _snwprintf_s(json, 1024, L"{\"type\":\"SOUND_SELECTED\",\"index\":%d,\"path\":\"%s\",\"filename\":\"%s\"}",
            step_index, sound_path.c_str(), name_only);
        g_webview->PostWebMessageAsJson(json);
        SaveSettings();
    }
}

// ============================================================================
// Program execution
// ============================================================================
void StopProgram();

void StopProfileSound()
{
    // Stop and uninit any currently playing miniaudio sound
    if (g_isSoundLoaded)
    {
        ma_sound_stop(&g_currentSound);
        ma_sound_uninit(&g_currentSound);
        g_isSoundLoaded = false;
    }
}

void PlayProfileSound(const char* sound_file)
{
    if (sound_file[0] == '\0')
    {
        OutputDebugStringA("PlayProfileSound: empty sound_file path, skipping.\n");
        return;
    }

    // Check if file exists
    wchar_t wsnd[MAX_SOUND_PATH];
    MultiByteToWideChar(CP_ACP, 0, sound_file, -1, wsnd, MAX_SOUND_PATH);
    if (GetFileAttributesW(wsnd) == INVALID_FILE_ATTRIBUTES)
    {
        char dbg[512];
        sprintf_s(dbg, sizeof(dbg), "PlayProfileSound: file not found: %s\n", sound_file);
        OutputDebugStringA(dbg);
        return;
    }

    StopProfileSound();

    if (!g_isAudioEngineInitialized)
    {
        OutputDebugStringA("PlayProfileSound error: miniaudio engine not initialized.\n");
        return;
    }

    // Play the sound using miniaudio high level API
    ma_result result = ma_sound_init_from_file(&g_audioEngine, sound_file, 0, NULL, NULL, &g_currentSound);

    char dbg[512];
    if (result == MA_SUCCESS)
    {
        g_isSoundLoaded = true;
        ma_sound_start(&g_currentSound);
        sprintf_s(dbg, sizeof(dbg), "PlayProfileSound: now playing: %s\n", sound_file);
    }
    else
    {
        sprintf_s(dbg, sizeof(dbg), "PlayProfileSound error: miniaudio failed to load (file: %s)\n", sound_file);
    }

    OutputDebugStringA(dbg);
}

void StartProgram()
{
    if (program_running) return;
    if (!motorcontroller.IsConnected()) return;

    int diff = settings.difficulty;

    current_step = -1;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        if (settings.profiles[diff][i].duration > 0)
        {
            current_step = i;
            break;
        }
    }

    if (current_step < 0) return;

	motorcontroller.Reset();
	motorcontroller.Start();
    
	motorcontroller.SetRPM(settings.profiles[diff][current_step].motor_rpm);

    PlayProfileSound(settings.profiles[diff][current_step].sound_file);

    program_running = true;
    program_start_tick = GetTickCount();
    step_start_tick = GetTickCount();

    SetTimer(g_hwnd, TIMER_PROGRAM, TIMER_PROGRAM_MS, NULL);
    UpdateStatusDisplay();
}

void StopProgram()
{
    if (!program_running) return;

    KillTimer(g_hwnd, TIMER_PROGRAM);

    if (motorcontroller.IsConnected())
    {
        motorcontroller.Stop();
    }

    program_running = false;
    current_step = 0;
    StopProfileSound(); // Stop any playing sound
    UpdateStatusDisplay();
}

void AdvanceToNextStep()
{
    int diff = settings.difficulty;
    int next = -1;
    for (int i = current_step + 1; i < MAX_STEPS; i++)
    {
        if (settings.profiles[diff][i].duration > 0) { next = i; break; }
    }

    if (next < 0)
    {
        StopProgram();
        return;
    }

    current_step = next;
    step_start_tick = GetTickCount();

	motorcontroller.SetRPM(settings.profiles[diff][current_step].motor_rpm);

    PlayProfileSound(settings.profiles[diff][current_step].sound_file);
}

void OnProgramTimer()
{
    if (!program_running) return;

    int diff = settings.difficulty;
    DWORD step_elapsed_ms = GetTickCount() - step_start_tick;
    DWORD step_duration_ms = (DWORD)(settings.profiles[diff][current_step].duration) * 1000;

    if (step_elapsed_ms >= step_duration_ms) AdvanceToNextStep();
}

// ============================================================================
// Win32
// ============================================================================
HRESULT OnWebMessageReceived(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args)
{
    LPWSTR messageRaw = nullptr;
    HRESULT hr = args->get_WebMessageAsJson(&messageRaw);
    if (FAILED(hr) || !messageRaw) return hr;

    std::wstring message = messageRaw;
    CoTaskMemFree(messageRaw);

    // Simple JSON dispatcher (looking for "type")
    if (message.find(L"\"type\":\"GET_INITIAL_DATA\"") != std::string::npos) 
    {
        SendComPortsToWeb();
        SendProfileToWeb();
    }
    else if (message.find(L"\"type\":\"CONNECT\"") != std::string::npos) 
    {
        if (motorcontroller.IsConnected())
        {
            StopProgram();
            KillTimer(g_hwnd, TIMER_STATUS);
			motorcontroller.Release();
        } 
        else 
        {
            // Extract port name (hacky but works for this controlled environment)
            size_t pos = message.find(L"\"port\":\"");
            if (pos != std::string::npos) 
            {
                pos += 8;
                size_t end = message.find(L"\"", pos);
                std::wstring portW = message.substr(pos, end - pos);
                
                char portA[64];
                WideCharToMultiByte(CP_ACP, 0, portW.c_str(), -1, portA, 64, NULL, NULL);
                
                if (motorcontroller.Initialize(portA) == 0)
                {
                    strcpy_s(settings.com_port, sizeof(settings.com_port), portA);
                    SaveSettings();
                    SetTimer(g_hwnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
                }
            }
        }
        UpdateStatusDisplay();
    }
    else if (message.find(L"\"type\":\"REFRESH_PORTS\"") != std::string::npos) 
    {
        SendComPortsToWeb();
    }
    else if (message.find(L"\"type\":\"START\"") != std::string::npos) 
    {
        std::wstring dateW = L"Unknown";
        size_t pos = message.find(L"\"date\":\"");
        if (pos != std::string::npos)
        {
            pos += 8;
            size_t end = message.find(L"\"", pos);
            if (end != std::string::npos)
                dateW = message.substr(pos, end - pos);
        }
        
        char dateA[16] = {0};
        WideCharToMultiByte(CP_ACP, 0, dateW.c_str(), -1, dateA, 15, NULL, NULL);

        SYSTEMTIME st;
        GetLocalTime(&st);
        char timeA[16] = {0};
        sprintf_s(timeA, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

        if (settings.num_logs < MAX_LOGS)
        {
            strcpy_s(settings.logs[settings.num_logs].date, dateA);
            strcpy_s(settings.logs[settings.num_logs].time, timeA);
            settings.logs[settings.num_logs].num_people = settings.num_seats;
            settings.num_logs++;
        }
        else
        {
            memmove(&settings.logs[0], &settings.logs[1], sizeof(RideLog) * (MAX_LOGS - 1));
            strcpy_s(settings.logs[MAX_LOGS - 1].date, dateA);
            strcpy_s(settings.logs[MAX_LOGS - 1].time, timeA);
            settings.logs[MAX_LOGS - 1].num_people = settings.num_seats;
        }

        settings.total_rides++;
        SaveSettings();
        
        SendProfileToWeb();
        StartProgram();
    }
    else if (message.find(L"\"type\":\"STOP\"") != std::string::npos) 
    {
        StopProgram();
    }
    else if (message.find(L"\"type\":\"GET_REPORTS\"") != std::string::npos) 
    {
        std::wstring json = L"{\"type\":\"REPORTS_DATA\",\"logs\":[";
        int32 start_idx = settings.num_logs > 1000 ? settings.num_logs - 1000 : 0;
        for (int32 i = start_idx; i < settings.num_logs; i++)
        {
            wchar_t wdate[16], wtime[16];
            MultiByteToWideChar(CP_ACP, 0, settings.logs[i].date, -1, wdate, 16);
            MultiByteToWideChar(CP_ACP, 0, settings.logs[i].time, -1, wtime, 16);
            
            wchar_t log_json[256];
            _snwprintf_s(log_json, 256, L"{\"date\":\"%s\",\"time\":\"%s\",\"people\":%d}%s",
                wdate, wtime, settings.logs[i].num_people,
                (i == settings.num_logs - 1) ? L"" : L",");
            json += log_json;
        }
        json += L"]}";
        if (g_webview) g_webview->PostWebMessageAsJson(json.c_str());
    }
    else if (message.find(L"\"type\":\"BROWSE_SOUND\"") != std::string::npos) 
    {
        size_t pos = message.find(L"\"index\":");
        if (pos != std::string::npos) 
        {
            pos += 8;
            int index = _wtoi(message.substr(pos).c_str());
            BrowseSoundFile(g_hwnd, index);
        }
    }
    else if (message.find(L"\"type\":\"SET_DIFFICULTY\"") != std::string::npos) 
    {
        size_t pos = message.find(L"\"value\":");
        if (pos != std::string::npos) 
        {
            pos += 8;
            settings.difficulty = _wtoi(message.substr(pos).c_str());
            SaveSettings();
            SendProfileToWeb();
        }
    }
    else if (message.find(L"\"type\":\"SET_SEATS\"") != std::string::npos) 
    {
        size_t pos = message.find(L"\"value\":");
        if (pos != std::string::npos) 
        {
            pos += 8;
            settings.num_seats = _wtoi(message.substr(pos).c_str());
            SaveSettings();
        }
    }
    else if (message.find(L"\"type\":\"UPDATE_STEP\"") != std::string::npos) 
    {
        // Simple extraction of index, field, and value
        size_t idxPos = message.find(L"\"index\":");
        size_t fldPos = message.find(L"\"field\":\"");
        size_t valPos = message.find(L"\"value\":");

        if (idxPos != std::string::npos && fldPos != std::string::npos && valPos != std::string::npos) 
        {
            int index = _wtoi(message.substr(idxPos + 8).c_str());
            int value = _wtoi(message.substr(valPos + 8).c_str());
            
            std::wstring field = message.substr(fldPos + 9);
            field = field.substr(0, field.find(L"\""));

            int diff = settings.difficulty;
            if (index >= 0 && index < MAX_STEPS) 
            {
                if (field == L"rpm") settings.profiles[diff][index].motor_rpm = value;
                else if (field == L"duration") settings.profiles[diff][index].duration = value;
                SaveSettings();
            }
        }
    }
    

    return S_OK;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_controller) 
        {
            RECT bounds;
            GetClientRect(hwnd, &bounds);
            g_controller->put_Bounds(bounds);
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_PROGRAM) OnProgramTimer();
        if (wParam == TIMER_STATUS) UpdateStatusDisplay();
        break;

    case WM_CLOSE:
        if (program_running) 
        {
            if (MessageBoxW(hwnd, L"Program is running. Stop and exit?", L"Confirm Exit", MB_YESNO | MB_ICONQUESTION) != IDYES) break;
            StopProgram();
        }
		motorcontroller.Release();
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, 
                       LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), 
                       (HBRUSH)(COLOR_WINDOW + 1), NULL, L"MotorControlApp", LoadIcon(NULL, IDI_APPLICATION) };
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"MotorControlApp", L"Delta MS300 VFD Control Dashboard", 
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, 
                            NULL, NULL, hInstance, NULL);

    if (!g_hwnd) return 1;

    SetDefaultProfiles();
    LoadSettings();

    // Initialize miniaudio
    if (ma_engine_init(NULL, &g_audioEngine) == MA_SUCCESS)
    {
        g_isAudioEngineInitialized = true;
    }

    // Initialize WebView2
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd = g_hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT 
            {
                env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT 
                    {
                        if (controller != nullptr) 
                        {
                            g_controller = controller;
                            g_controller->get_CoreWebView2(&g_webview);
                        }

                        ICoreWebView2Settings* Settings;
                        g_webview->get_Settings(&Settings);
                        Settings->put_IsScriptEnabled(TRUE);
                        Settings->put_AreDefaultContextMenusEnabled(FALSE);
                        Settings->put_IsStatusBarEnabled(FALSE);

                        RECT bounds;
                        GetClientRect(hwnd, &bounds);
                        g_controller->put_Bounds(bounds);

                        g_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(OnWebMessageReceived).Get(), nullptr);

                        // Load index.html
                        wchar_t path[MAX_PATH];
                        GetCurrentDirectoryW(MAX_PATH, path);
                        std::wstring url = L"file:///";
                        url += path;
                        url += L"\\web\\index.html";
                        g_webview->Navigate(url.c_str());

                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }

    // Cleanup miniaudio
    StopProfileSound();
    if (g_isAudioEngineInitialized)
    {
        ma_engine_uninit(&g_audioEngine);
    }

    return (int)msg.wParam;
}
