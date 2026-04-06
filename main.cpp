// ============================================================================
// Windows includes (must be before custom type macros to avoid conflicts)
// ============================================================================
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
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

// ============================================================================
// Bit macros and type definitions
// ============================================================================
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

// ============================================================================
// Constants
// ============================================================================
#define MAX_STEPS           16
#define MAX_SOUND_PATH      260
#define MAX_RPM             1440
#define MAX_DURATION        600
#define MAX_FREQ_HZ         50.0
#define VFD_SLAVE_ID        1
#define VFD_ADDR_CONTROL    0x2000
#define VFD_ADDR_FREQ_WRITE 0x2001
#define VFD_ADDR_FREQ_READ  0x2103
#define VFD_ADDR_CURRENT    0x2104
#define VFD_ADDR_VOLTAGE    0x2105
#define VFD_CMD_START       (int16)(BIT04 | BIT01)
#define VFD_CMD_STOP        (int16)(BIT00)

#define TIMER_PROGRAM       1
#define TIMER_PROGRAM_MS    500
#define TIMER_STATUS        2
#define TIMER_STATUS_MS     100

// Control IDs
#define IDC_COM_PORT        1001
#define IDC_CONNECT         1002
#define IDC_START            1003
#define IDC_STOP             1004
#define IDC_SEATS            1005
#define IDC_DIFFICULTY       1006
#define IDC_SAVE             1007
#define IDC_LOAD             1008
#define IDC_REFRESH          1009

#define IDC_STATUS_CONN     1010
#define IDC_STATUS_PROG     1011
#define IDC_STATUS_TIME     1012
#define IDC_STATUS_STEP     1013
#define IDC_STATUS_RPM      1014
#define IDC_STATUS_CURRENT  1015
#define IDC_STATUS_VOLTAGE  1016

#define IDC_RPM_BASE        1100
#define IDC_DUR_BASE        1200
#define IDC_ABS_BASE        1300
#define IDC_SND_BASE        1400
#define IDC_BRW_BASE        1500

// ============================================================================
// Data structures
// ============================================================================
struct ProfileStep
{
    int32 motor_rpm;
    int32 duration;
    char sound_file[MAX_SOUND_PATH];
};

struct AppSettings
{
    char com_port[64];
    int32 num_seats;
    int32 difficulty;
    ProfileStep profiles[2][MAX_STEPS];
};

// ============================================================================
// Modbus frame structs (packed for serial communication)
// ============================================================================
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

// ============================================================================
// Modbus struct with UART implementation
// ============================================================================
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

// ============================================================================
// Global state
// ============================================================================
static HINSTANCE g_hinst;
static HWND g_hwnd;
static HWND g_hwnd_com_port;
static HWND g_hwnd_connect;
static HWND g_hwnd_start;
static HWND g_hwnd_stop;
static HWND g_hwnd_seats;
static HWND g_hwnd_difficulty;
static HWND g_hwnd_save;
static HWND g_hwnd_load;

static HWND g_hwnd_status_conn;
static HWND g_hwnd_status_prog;
static HWND g_hwnd_status_time;
static HWND g_hwnd_status_step;
static HWND g_hwnd_status_rpm;
static HWND g_hwnd_status_current;
static HWND g_hwnd_status_voltage;

static HWND g_hwnd_rpm[MAX_STEPS];
static HWND g_hwnd_dur[MAX_STEPS];
static HWND g_hwnd_abs[MAX_STEPS];
static HWND g_hwnd_snd[MAX_STEPS];
static HWND g_hwnd_brw[MAX_STEPS];
static HWND g_hwnd_step_label[MAX_STEPS];

static Modbus modbus;
static AppSettings settings;

static bool program_running = false;
static int32 current_step = 0;
static DWORD program_start_tick = 0;
static DWORD step_start_tick = 0;

static HFONT g_font;
static HFONT g_font_bold;

// ============================================================================
// Helper: RPM <-> VFD frequency value
// ============================================================================
int16 RpmToFreqValue(int32 rpm)
{
    if (rpm <= 0) return 0;
    if (rpm > MAX_RPM) rpm = MAX_RPM;
    float64 freq_hz = ((float64)rpm / (float64)MAX_RPM) * MAX_FREQ_HZ;
    return (int16)(freq_hz * 100.0);
}

int32 FreqValueToRpm(int16 freq_val)
{
    float64 freq_hz = (float64)freq_val / 100.0;
    return (int32)((freq_hz / MAX_FREQ_HZ) * (float64)MAX_RPM);
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

void PopulateComPortDropdown()
{
    SendMessage(g_hwnd_com_port, CB_RESETCONTENT, 0, 0);
    EnumerateComPorts();
    for (int i = 0; i < g_com_port_count; i++)
    {
        wchar_t wfriendly[256];
        MultiByteToWideChar(CP_ACP, 0, g_com_ports[i].friendly_name, -1, wfriendly, 256);
        SendMessage(g_hwnd_com_port, CB_ADDSTRING, 0, (LPARAM)wfriendly);
    }
    if (g_com_port_count > 0)
        SendMessage(g_hwnd_com_port, CB_SETCURSEL, 0, 0);
}

// ============================================================================
// Helper: Default profiles
// ============================================================================
void SetDefaultProfiles()
{
    memset(&settings, 0, sizeof(settings));
    settings.num_seats = 1;
    settings.difficulty = 0;
    strcpy_s(settings.com_port, sizeof(settings.com_port), "");

    // Easy profile
    for (int i = 0; i < MAX_STEPS; i++)
    {
        settings.profiles[0][i].motor_rpm = 0;
        settings.profiles[0][i].duration = 0;
        settings.profiles[0][i].sound_file[0] = '\0';
    }

    // Hard profile
    for (int i = 0; i < MAX_STEPS; i++)
    {
        settings.profiles[1][i].motor_rpm = 0;
        settings.profiles[1][i].duration = 0;
        settings.profiles[1][i].sound_file[0] = '\0';
    }
}

// ============================================================================
// Helper: Update profile table UI from settings
// ============================================================================
void UpdateProfileTableFromSettings()
{
    int diff = settings.difficulty;
    int32 abs_time = 0;
    wchar_t buf[32];

    for (int i = 0; i < MAX_STEPS; i++)
    {
        _snwprintf_s(buf, 32, L"%d", settings.profiles[diff][i].motor_rpm);
        SetWindowTextW(g_hwnd_rpm[i], buf);

        _snwprintf_s(buf, 32, L"%d", settings.profiles[diff][i].duration);
        SetWindowTextW(g_hwnd_dur[i], buf);

        abs_time += settings.profiles[diff][i].duration;
        _snwprintf_s(buf, 32, L"%d", abs_time);
        SetWindowTextW(g_hwnd_abs[i], buf);

        wchar_t wsnd[MAX_SOUND_PATH];
        MultiByteToWideChar(CP_ACP, 0, settings.profiles[diff][i].sound_file, -1, wsnd, MAX_SOUND_PATH);
        // Display only the filename, not the full path
        const wchar_t* filename_only = wcsrchr(wsnd, L'\\');
        if (filename_only)
            filename_only++;
        else
            filename_only = wsnd;
        SetWindowTextW(g_hwnd_snd[i], filename_only);
    }
}

// ============================================================================
// Helper: Read profile table UI into settings
// ============================================================================
void ReadProfileTableToSettings()
{
    int diff = settings.difficulty;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        wchar_t buf[32];
        GetWindowTextW(g_hwnd_rpm[i], buf, 32);
        settings.profiles[diff][i].motor_rpm = _wtoi(buf);
        if (settings.profiles[diff][i].motor_rpm < 0) settings.profiles[diff][i].motor_rpm = 0;
        if (settings.profiles[diff][i].motor_rpm > MAX_RPM) settings.profiles[diff][i].motor_rpm = MAX_RPM;

        GetWindowTextW(g_hwnd_dur[i], buf, 32);
        settings.profiles[diff][i].duration = _wtoi(buf);
        if (settings.profiles[diff][i].duration < 0) settings.profiles[diff][i].duration = 0;
        if (settings.profiles[diff][i].duration > MAX_DURATION) settings.profiles[diff][i].duration = MAX_DURATION;

        // Sound file path is stored directly in settings when browsing,
        // so we do not read it from the UI (which only shows the filename).
    }
}

// ============================================================================
// Helper: Recalculate absolute times
// ============================================================================
void RecalculateAbsoluteTime()
{
    int32 abs_time = 0;
    wchar_t buf[32];

    for (int i = 0; i < MAX_STEPS; i++)
    {
        wchar_t dur_buf[32];
        GetWindowTextW(g_hwnd_dur[i], dur_buf, 32);
        int32 dur = _wtoi(dur_buf);
        abs_time += dur;
        _snwprintf_s(buf, 32, L"%d", abs_time);
        SetWindowTextW(g_hwnd_abs[i], buf);
    }
}

// ============================================================================
// Helper: Enable/disable profile controls
// ============================================================================
void SetProfileControlsEnabled(bool enabled)
{
    for (int i = 0; i < MAX_STEPS; i++)
    {
        EnableWindow(g_hwnd_rpm[i], enabled);
        EnableWindow(g_hwnd_dur[i], enabled);
        EnableWindow(g_hwnd_snd[i], enabled);
        EnableWindow(g_hwnd_brw[i], enabled);
    }
    EnableWindow(g_hwnd_save, enabled);
    EnableWindow(g_hwnd_load, enabled);
    EnableWindow(g_hwnd_difficulty, enabled);
    EnableWindow(g_hwnd_seats, enabled);
}

// ============================================================================
// Helper: Update status display
// ============================================================================
void UpdateStatusDisplay()
{
    wchar_t buf[64];

    SetWindowTextW(g_hwnd_status_conn, modbus.connected ? L"Connected" : L"Disconnected");
    SetWindowTextW(g_hwnd_status_prog, program_running ? L"Running" : L"Stopped");

    if (program_running)
    {
        DWORD elapsed_ms = GetTickCount() - program_start_tick;
        int32 elapsed_sec = (int32)(elapsed_ms / 1000);

        _snwprintf_s(buf, 64, L"%d s", elapsed_sec);
        SetWindowTextW(g_hwnd_status_time, buf);

        _snwprintf_s(buf, 64, L"%d", current_step + 1);
        SetWindowTextW(g_hwnd_status_step, buf);
    }
    else
    {
        SetWindowTextW(g_hwnd_status_time, L"0 s");
        SetWindowTextW(g_hwnd_status_step, L"0");
    }

    if (modbus.connected)
    {
        int16 freq_val = 0;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_FREQ_READ, freq_val) == 0)
        {
            float64 rpm = ((float64)freq_val / 100.0 / MAX_FREQ_HZ) * (float64)MAX_RPM;
            _snwprintf_s(buf, 64, L"%.2f RPM", rpm);
            SetWindowTextW(g_hwnd_status_rpm, buf);
        }
        else
        {
            SetWindowTextW(g_hwnd_status_rpm, L"N/A");
		}

        int16 current_val = 0;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_CURRENT, current_val) == 0)
        {
            float64 amps = (float64)current_val / 100.0;
            _snwprintf_s(buf, 64, L"%.2f A", amps);
            SetWindowTextW(g_hwnd_status_current, buf);
        }
        else
        {
            SetWindowTextW(g_hwnd_status_current, L"N/A");
        }

        int16 voltage_val = 0;
        if (modbus.Read(VFD_SLAVE_ID, VFD_ADDR_VOLTAGE, voltage_val) == 0)
        {
            float64 volts = (float64)voltage_val / 10.0;
            _snwprintf_s(buf, 64, L"%.2f V", volts);
            SetWindowTextW(g_hwnd_status_voltage, buf);
        }
        else
        {
            SetWindowTextW(g_hwnd_status_voltage, L"N/A");
		}
    }
    else
    {
        SetWindowTextW(g_hwnd_status_rpm, L"0.00 RPM");
        SetWindowTextW(g_hwnd_status_current, L"0.00 A");
        SetWindowTextW(g_hwnd_status_voltage, L"0.00 V");
    }
}

// ============================================================================
// Helper: Save/Load settings
// ============================================================================
void SaveSettings(HWND hwnd)
{
    ReadProfileTableToSettings();

    int sel = (int)SendMessage(g_hwnd_com_port, CB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel < g_com_port_count)
        strcpy_s(settings.com_port, sizeof(settings.com_port), g_com_ports[sel].port_name);

    settings.num_seats = (int32)SendMessage(g_hwnd_seats, CB_GETCURSEL, 0, 0) + 1;
    settings.difficulty = (int32)SendMessage(g_hwnd_difficulty, CB_GETCURSEL, 0, 0);

    OPENFILENAMEW ofn;
    wchar_t filename[MAX_PATH] = L"profile.bin";
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"bin";

    if (GetSaveFileNameW(&ofn))
    {
        char afilename[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, filename, -1, afilename, MAX_PATH, NULL, NULL);
        FILE* f = NULL;
        fopen_s(&f, afilename, "wb");
        if (f)
        {
            fwrite(&settings, sizeof(AppSettings), 1, f);
            fclose(f);
            MessageBoxW(hwnd, L"Settings saved successfully.", L"Save", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(hwnd, L"Failed to save file.", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void LoadSettings(HWND hwnd)
{
    OPENFILENAMEW ofn;
    wchar_t filename[MAX_PATH] = L"";
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = L"bin";

    if (GetOpenFileNameW(&ofn))
    {
        char afilename[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, filename, -1, afilename, MAX_PATH, NULL, NULL);
        FILE* f = NULL;
        fopen_s(&f, afilename, "rb");
        if (f)
        {
            AppSettings loaded;
            if (fread(&loaded, sizeof(AppSettings), 1, f) == 1)
            {
                settings = loaded;

                SendMessage(g_hwnd_seats, CB_SETCURSEL, settings.num_seats - 1, 0);
                SendMessage(g_hwnd_difficulty, CB_SETCURSEL, settings.difficulty, 0);
                UpdateProfileTableFromSettings();

                for (int i = 0; i < g_com_port_count; i++)
                {
                    if (strcmp(g_com_ports[i].port_name, settings.com_port) == 0)
                    {
                        SendMessage(g_hwnd_com_port, CB_SETCURSEL, i, 0);
                        break;
                    }
                }

                MessageBoxW(hwnd, L"Settings loaded successfully.", L"Load", MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                MessageBoxW(hwnd, L"Invalid file format.", L"Error", MB_OK | MB_ICONERROR);
            }
            fclose(f);
        }
        else
        {
            MessageBoxW(hwnd, L"Failed to open file.", L"Error", MB_OK | MB_ICONERROR);
        }
    }
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
    ofn.lpstrFilter = L"WAV Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = L"wav";

    if (GetOpenFileNameW(&ofn))
    {
        // Store full path in settings
        int diff = settings.difficulty;
        WideCharToMultiByte(CP_ACP, 0, filename, -1, settings.profiles[diff][step_index].sound_file, MAX_SOUND_PATH, NULL, NULL);

        // Display only the filename
        const wchar_t* name_only = wcsrchr(filename, L'\\');
        if (name_only)
            name_only++;
        else
            name_only = filename;
        SetWindowTextW(g_hwnd_snd[step_index], name_only);
    }
}

// ============================================================================
// Program execution
// ============================================================================
void StopProgram();

void StartProgram()
{
    if (program_running) return;
    if (!modbus.connected)
    {
        MessageBoxW(g_hwnd, L"Not connected to VFD. Please connect first.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    ReadProfileTableToSettings();

    int diff = settings.difficulty;

    // Find first step with duration > 0
    current_step = -1;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        if (settings.profiles[diff][i].duration > 0)
        {
            current_step = i;
            break;
        }
    }

    if (current_step < 0)
    {
        MessageBoxW(g_hwnd, L"All steps have zero duration. Nothing to run.", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    // Send VFD start command
    if (modbus.Write(VFD_SLAVE_ID, VFD_ADDR_CONTROL, VFD_CMD_START) != 0)
    {
        MessageBoxW(g_hwnd, L"Failed to send start command to VFD.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Send frequency for first step
    int16 freq_val = RpmToFreqValue(settings.profiles[diff][current_step].motor_rpm);
    modbus.Write(VFD_SLAVE_ID, VFD_ADDR_FREQ_WRITE, freq_val);

    // Play sound for first step
    if (settings.profiles[diff][current_step].sound_file[0] != '\0')
    {
        wchar_t wsnd[MAX_SOUND_PATH];
        MultiByteToWideChar(CP_ACP, 0, settings.profiles[diff][current_step].sound_file, -1, wsnd, MAX_SOUND_PATH);
        PlaySoundW(wsnd, NULL, SND_FILENAME | SND_ASYNC);
    }

    program_running = true;
    program_start_tick = GetTickCount();
    step_start_tick = GetTickCount();

    SetProfileControlsEnabled(false);
    EnableWindow(g_hwnd_start, FALSE);
    EnableWindow(g_hwnd_stop, TRUE);
    EnableWindow(g_hwnd_connect, FALSE);

    SetTimer(g_hwnd, TIMER_PROGRAM, TIMER_PROGRAM_MS, NULL);
    UpdateStatusDisplay();
}

void StopProgram()
{
    if (!program_running) return;

    KillTimer(g_hwnd, TIMER_PROGRAM);

    if (modbus.connected)
    {
        modbus.Write(VFD_SLAVE_ID, VFD_ADDR_CONTROL, VFD_CMD_STOP);
    }

    program_running = false;
    current_step = 0;

    SetProfileControlsEnabled(true);
    EnableWindow(g_hwnd_start, TRUE);
    EnableWindow(g_hwnd_stop, FALSE);
    EnableWindow(g_hwnd_connect, TRUE);

    UpdateStatusDisplay();
}

void AdvanceToNextStep()
{
    int diff = settings.difficulty;

    int next = -1;
    for (int i = current_step + 1; i < MAX_STEPS; i++)
    {
        if (settings.profiles[diff][i].duration > 0)
        {
            next = i;
            break;
        }
    }

    if (next < 0)
    {
        StopProgram();
        MessageBoxW(g_hwnd, L"Program cycle complete.", L"Info", MB_OK | MB_ICONINFORMATION);
        return;
    }

    current_step = next;
    step_start_tick = GetTickCount();

    int16 freq_val = RpmToFreqValue(settings.profiles[diff][current_step].motor_rpm);
    modbus.Write(VFD_SLAVE_ID, VFD_ADDR_FREQ_WRITE, freq_val);

    if (settings.profiles[diff][current_step].sound_file[0] != '\0')
    {
        wchar_t wsnd[MAX_SOUND_PATH];
        MultiByteToWideChar(CP_ACP, 0, settings.profiles[diff][current_step].sound_file, -1, wsnd, MAX_SOUND_PATH);
        PlaySoundW(wsnd, NULL, SND_FILENAME | SND_ASYNC);
    }
}

void OnProgramTimer()
{
    if (!program_running) return;

    int diff = settings.difficulty;
    DWORD step_elapsed_ms = GetTickCount() - step_start_tick;
    DWORD step_duration_ms = (DWORD)(settings.profiles[diff][current_step].duration) * 1000;

    if (step_elapsed_ms >= step_duration_ms)
    {
        AdvanceToNextStep();
    }
}

// ============================================================================
// Window procedure
// ============================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_font_bold = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        int y;

        // ---- Connection Section ----
        CreateWindowW(L"BUTTON", L"Connection", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            5, 2, 785, 50, hwnd, NULL, g_hinst, NULL);

        CreateWindowW(L"STATIC", L"COM Port:", WS_CHILD | WS_VISIBLE,
            15, 22, 65, 20, hwnd, NULL, g_hinst, NULL);

        g_hwnd_com_port = CreateWindowW(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            85, 19, 490, 300, hwnd, (HMENU)IDC_COM_PORT, g_hinst, NULL);

        g_hwnd_connect = CreateWindowW(L"BUTTON", L"Connect",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            585, 18, 100, 26, hwnd, (HMENU)IDC_CONNECT, g_hinst, NULL);

        CreateWindowW(L"BUTTON", L"\x21BB",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            695, 18, 30, 26, hwnd, (HMENU)IDC_REFRESH, g_hinst, NULL);

        // ---- Mode Section ----
        CreateWindowW(L"BUTTON", L"Mode", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            5, 55, 785, 50, hwnd, NULL, g_hinst, NULL);

        CreateWindowW(L"STATIC", L"No of Seats:", WS_CHILD | WS_VISIBLE,
            15, 75, 85, 20, hwnd, NULL, g_hinst, NULL);

        g_hwnd_seats = CreateWindowW(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            105, 72, 55, 120, hwnd, (HMENU)IDC_SEATS, g_hinst, NULL);
        SendMessage(g_hwnd_seats, CB_ADDSTRING, 0, (LPARAM)L"1");
        SendMessage(g_hwnd_seats, CB_ADDSTRING, 0, (LPARAM)L"2");
        SendMessage(g_hwnd_seats, CB_ADDSTRING, 0, (LPARAM)L"3");
        SendMessage(g_hwnd_seats, CB_ADDSTRING, 0, (LPARAM)L"4");
        SendMessage(g_hwnd_seats, CB_SETCURSEL, 0, 0);

        CreateWindowW(L"STATIC", L"Difficulty:", WS_CHILD | WS_VISIBLE,
            180, 75, 70, 20, hwnd, NULL, g_hinst, NULL);

        g_hwnd_difficulty = CreateWindowW(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            255, 72, 90, 80, hwnd, (HMENU)IDC_DIFFICULTY, g_hinst, NULL);
        SendMessage(g_hwnd_difficulty, CB_ADDSTRING, 0, (LPARAM)L"Easy");
        SendMessage(g_hwnd_difficulty, CB_ADDSTRING, 0, (LPARAM)L"Hard");
        SendMessage(g_hwnd_difficulty, CB_SETCURSEL, 0, 0);

        // ---- Control & Status Section ----
        CreateWindowW(L"BUTTON", L"Control && Status", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            5, 110, 785, 80, hwnd, NULL, g_hinst, NULL);

        g_hwnd_start = CreateWindowW(L"BUTTON", L"START",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            15, 130, 80, 26, hwnd, (HMENU)IDC_START, g_hinst, NULL);

        g_hwnd_stop = CreateWindowW(L"BUTTON", L"STOP",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
            100, 130, 80, 26, hwnd, (HMENU)IDC_STOP, g_hinst, NULL);

        // Status table headers
        {
            int sx = 190;
            int sh = 18;
            int sy_hdr = 128;
            int sy_val = 148;
            int col_w[] = { 85, 70, 55, 45, 90, 65, 75 };

            const wchar_t* headers[] = { L"Connection", L"Program", L"Time", L"Step", L"Motor RPM", L"Current", L"Voltage" };
            for (int i = 0; i < 7; i++)
            {
                CreateWindowW(L"STATIC", headers[i], WS_CHILD | WS_VISIBLE | SS_CENTER,
                    sx, sy_hdr, col_w[i], sh, hwnd, NULL, g_hinst, NULL);
                sx += col_w[i] + 5;
            }

            sx = 190;
            g_hwnd_status_conn = CreateWindowW(L"EDIT", L"Disconnected",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[0], 22, hwnd, (HMENU)IDC_STATUS_CONN, g_hinst, NULL);
            sx += col_w[0] + 5;

            g_hwnd_status_prog = CreateWindowW(L"EDIT", L"Stopped",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[1], 22, hwnd, (HMENU)IDC_STATUS_PROG, g_hinst, NULL);
            sx += col_w[1] + 5;

            g_hwnd_status_time = CreateWindowW(L"EDIT", L"0 s",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[2], 22, hwnd, (HMENU)IDC_STATUS_TIME, g_hinst, NULL);
            sx += col_w[2] + 5;

            g_hwnd_status_step = CreateWindowW(L"EDIT", L"0",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[3], 22, hwnd, (HMENU)IDC_STATUS_STEP, g_hinst, NULL);
            sx += col_w[3] + 5;

            g_hwnd_status_rpm = CreateWindowW(L"EDIT", L"0.00 RPM",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[4], 22, hwnd, (HMENU)IDC_STATUS_RPM, g_hinst, NULL);
            sx += col_w[4] + 5;

            g_hwnd_status_current = CreateWindowW(L"EDIT", L"0.00 A",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[5], 22, hwnd, (HMENU)IDC_STATUS_CURRENT, g_hinst, NULL);
            sx += col_w[5] + 5;

            g_hwnd_status_voltage = CreateWindowW(L"EDIT", L"0.00 V",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
                sx, sy_val, col_w[6], 22, hwnd, (HMENU)IDC_STATUS_VOLTAGE, g_hinst, NULL);
        }

        // ---- Profile Section ----
        CreateWindowW(L"BUTTON", L"Profile", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            5, 195, 785, 485, hwnd, NULL, g_hinst, NULL);

        g_hwnd_save = CreateWindowW(L"BUTTON", L"Save",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            15, 213, 80, 26, hwnd, (HMENU)IDC_SAVE, g_hinst, NULL);

        g_hwnd_load = CreateWindowW(L"BUTTON", L"Load",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            105, 213, 80, 26, hwnd, (HMENU)IDC_LOAD, g_hinst, NULL);

        // Table headers
        y = 245;
        CreateWindowW(L"STATIC", L"Step", WS_CHILD | WS_VISIBLE | SS_CENTER,
            15, y, 35, 18, hwnd, NULL, g_hinst, NULL);
        CreateWindowW(L"STATIC", L"Motor RPM", WS_CHILD | WS_VISIBLE | SS_CENTER,
            55, y, 90, 18, hwnd, NULL, g_hinst, NULL);
        CreateWindowW(L"STATIC", L"Duration(s)", WS_CHILD | WS_VISIBLE | SS_CENTER,
            150, y, 100, 18, hwnd, NULL, g_hinst, NULL);
        CreateWindowW(L"STATIC", L"Abs Time(s)", WS_CHILD | WS_VISIBLE | SS_CENTER,
            255, y, 95, 18, hwnd, NULL, g_hinst, NULL);
        CreateWindowW(L"STATIC", L"Sound File", WS_CHILD | WS_VISIBLE | SS_CENTER,
            355, y, 355, 18, hwnd, NULL, g_hinst, NULL);

        // Table rows
        y = 265;
        for (int i = 0; i < MAX_STEPS; i++)
        {
            int row_y = y + i * 25;
            wchar_t buf[8];
            _snwprintf_s(buf, 8, L"%d", i + 1);

            g_hwnd_step_label[i] = CreateWindowW(L"STATIC", buf,
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                15, row_y + 2, 35, 20, hwnd, NULL, g_hinst, NULL);

            g_hwnd_rpm[i] = CreateWindowW(L"EDIT", L"0",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER | ES_CENTER,
                55, row_y, 90, 22, hwnd, (HMENU)(INT_PTR)(IDC_RPM_BASE + i), g_hinst, NULL);

            g_hwnd_dur[i] = CreateWindowW(L"EDIT", L"0",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER | ES_CENTER,
                150, row_y, 100, 22, hwnd, (HMENU)(INT_PTR)(IDC_DUR_BASE + i), g_hinst, NULL);

            g_hwnd_abs[i] = CreateWindowW(L"STATIC", L"0",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                255, row_y + 2, 95, 20, hwnd, (HMENU)(INT_PTR)(IDC_ABS_BASE + i), g_hinst, NULL);

            g_hwnd_snd[i] = CreateWindowW(L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
                355, row_y, 330, 22, hwnd, (HMENU)(INT_PTR)(IDC_SND_BASE + i), g_hinst, NULL);

            g_hwnd_brw[i] = CreateWindowW(L"BUTTON", L"...",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                690, row_y, 30, 22, hwnd, (HMENU)(INT_PTR)(IDC_BRW_BASE + i), g_hinst, NULL);
        }

        // Set font on all child controls
        EnumChildWindows(hwnd, [](HWND child, LPARAM lParam) -> BOOL {
            SendMessage(child, WM_SETFONT, (WPARAM)lParam, TRUE);
            return TRUE;
        }, (LPARAM)g_font);

        // Populate COM ports and load defaults
        PopulateComPortDropdown();
        SetDefaultProfiles();
        UpdateProfileTableFromSettings();

        EnableWindow(g_hwnd_stop, FALSE);

        break;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == IDC_CONNECT)
        {
            if (modbus.connected)
            {
                if (program_running) StopProgram();
                KillTimer(hwnd, TIMER_STATUS);
                modbus.Close();
                SetWindowTextW(g_hwnd_connect, L"Connect");
                EnableWindow(g_hwnd_com_port, TRUE);
                UpdateStatusDisplay();
            }
            else
            {
                int sel = (int)SendMessage(g_hwnd_com_port, CB_GETCURSEL, 0, 0);
                if (sel < 0 || sel >= g_com_port_count)
                {
                    MessageBoxW(hwnd, L"Please select a COM port.", L"Error", MB_OK | MB_ICONWARNING);
                    break;
                }

                if (modbus.Initialize(g_com_ports[sel].port_name) == 0)
                {
                    strcpy_s(settings.com_port, sizeof(settings.com_port), g_com_ports[sel].port_name);
                    SetWindowTextW(g_hwnd_connect, L"Disconnect");
                    EnableWindow(g_hwnd_com_port, FALSE);
                    SetTimer(hwnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
                    UpdateStatusDisplay();
                }
                else
                {
                    MessageBoxW(hwnd, L"Failed to open COM port.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }

        if (id == IDC_REFRESH)
        {
            PopulateComPortDropdown();
        }

        if (id == IDC_START)
        {
            StartProgram();
        }

        if (id == IDC_STOP)
        {
            StopProgram();
        }

        if (id == IDC_SAVE)
        {
            SaveSettings(hwnd);
        }

        if (id == IDC_LOAD)
        {
            LoadSettings(hwnd);
        }

        if (id == IDC_DIFFICULTY && code == CBN_SELCHANGE)
        {
            ReadProfileTableToSettings();
            settings.difficulty = (int32)SendMessage(g_hwnd_difficulty, CB_GETCURSEL, 0, 0);
            UpdateProfileTableFromSettings();
        }

        if (id >= IDC_DUR_BASE && id < IDC_DUR_BASE + MAX_STEPS && code == EN_CHANGE)
        {
            RecalculateAbsoluteTime();
        }

        if (id >= IDC_BRW_BASE && id < IDC_BRW_BASE + MAX_STEPS)
        {
            int step_index = id - IDC_BRW_BASE;
            BrowseSoundFile(hwnd, step_index);
        }

        break;
    }

    case WM_TIMER:
    {
        if (wParam == TIMER_PROGRAM)
        {
            OnProgramTimer();
        }
        if (wParam == TIMER_STATUS)
        {
            UpdateStatusDisplay();
        }
        break;
    }

    case WM_CLOSE:
    {
        if (program_running)
        {
            int result = MessageBoxW(hwnd, L"Program is running. Stop and exit?", L"Confirm Exit", MB_YESNO | MB_ICONQUESTION);
            if (result != IDYES) break;
            StopProgram();
        }
        KillTimer(hwnd, TIMER_STATUS);
        modbus.Close();
        DestroyWindow(hwnd);
        break;
    }

    case WM_DESTROY:
    {
        if (g_font) DeleteObject(g_font);
        if (g_font_bold) DeleteObject(g_font_bold);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================================
// WinMain
// ============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hinst = hInstance;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"MotorControlApp";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        MessageBoxW(NULL, L"Failed to register window class.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT rc = { 0, 0, 800, 690 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    int win_w = rc.right - rc.left;
    int win_h = rc.bottom - rc.top;

    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int win_x = (screen_w - win_w) / 2;
    int win_y = (screen_h - win_h) / 2;

    g_hwnd = CreateWindowExW(0,
        L"MotorControlApp",
        L"Delta MS300 VFD Motor Control",
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
        win_x, win_y, win_w, win_h,
        NULL, NULL, hInstance, NULL);

    if (!g_hwnd)
    {
        MessageBoxW(NULL, L"Failed to create window.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        if (!IsDialogMessageW(g_hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return (int)msg.wParam;
}