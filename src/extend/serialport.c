#ifdef WIN32

#include <stdio.h>
#include <windows.h>
#include "serialport.h"
#include "lua_ext_error.h"
#include "ymodem.h"

/**
	\brief Opens a new connection to a serial port
	\param portname		name of the serial port(COM1 - COM9 or \\\\.\\COM1-COM256)
	\param baudrate		the baudrate of this port (for example 9600)
	\param stopbits		th nuber of stoppbits (one, onePointFive or two)
	\param parity		the parity (even, odd, off or mark)
	\return			HANDLE to the serial port
	*/
HANDLE openSerialPort(LPCSTR portname,enum Baudrate baudrate, enum Stopbits stopbits, enum Paritycheck parity)
{
	DWORD  accessdirection =GENERIC_READ | GENERIC_WRITE;
	char full_path[25];

	sprintf(full_path,"\\\\.\\%s",portname);
	for (uint32_t i =0;i<strlen(full_path);++i) full_path[i] = (char)toupper((int)full_path[i]);

	HANDLE hSerial = CreateFile((LPCSTR)full_path,
		accessdirection,
		0,
		0,
		OPEN_EXISTING,
		0,
		0);
	if (hSerial == INVALID_HANDLE_VALUE) {
		error_log(__LINE__, __FILE__, 1, "CreateFile");
		return NULL;
	}
	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		error_log(__LINE__, __FILE__, 2, "GetCommState");
		return NULL;
	}
	dcbSerialParams.BaudRate=baudrate;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=stopbits;
	dcbSerialParams.Parity=parity;
	if(!SetCommState(hSerial, &dcbSerialParams)){
		error_log(__LINE__, __FILE__, 3, "SetCommState");
		return NULL;
	}
	COMMTIMEOUTS timeouts={0};
#if  (COM_DELAYS == 0)
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
#elif (COM_DELAYS == 1)
	// these are the port delays from CodeDownload application
	// in Visual Studio
	timeouts.ReadIntervalTimeout = 200; // 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 100; // 10;
	timeouts.WriteTotalTimeoutConstant = 1000; // 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
#else
	timeouts.ReadIntervalTimeout = 200; // 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 100; // 10;
	timeouts.WriteTotalTimeoutConstant = 1000; // 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
#endif

	if(!SetCommTimeouts(hSerial, &timeouts)){
		error_log(__LINE__, __FILE__, 4, "SetCommTimeouts");
		return NULL;
	}
	return hSerial;
}

/**
	\brief Read data from the serial port
	\param hSerial		File HANDLE to the serial port
	\param buffer		pointer to the area where the read data will be written
	\param buffersize	maximal size of the buffer area
	\return				amount of data that was read
	*/
DWORD readFromSerialPort(HANDLE hSerial, char * buffer, int buffersize)
{
    DWORD dwBytesRead = 0;
    if(!ReadFile(hSerial, buffer, buffersize, &dwBytesRead, NULL))
    {
    	error_log(__LINE__, __FILE__, 5, "ReadFile");
    }
    return dwBytesRead;
}

/**
	\brief write data to the serial port
	\param hSerial	File HANDLE to the serial port
	\param buffer	pointer to the area where the read data will be read
	\param length	amount of data to be read
	\return			amount of data that was written
	*/
DWORD writeToSerialPort(HANDLE hSerial, char * data, int length)
{
	DWORD dwBytesRead = 0;
	if(!WriteFile(hSerial, data, length, &dwBytesRead, NULL)){
		error_log(__LINE__, __FILE__, 6, "WriteFile");
	}
	return dwBytesRead;
}

void closeSerialPort(HANDLE hSerial)
{
	if (hSerial == NULL) {
		error_log(__LINE__, __FILE__, 7, "Null pointer passed to handle");
	}
	else CloseHandle(hSerial);
}

#endif
