/*/ SerialTest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <windows.h>
#include <conio.h>

#include "Serial.h"
#include "stdafx.h"

using namespace std;

#define RX_BUFFSIZE 20

void printUsage(_TCHAR progName[]);


int _tmain(int argc, _TCHAR* argv[])
{
	char buffer[RX_BUFFSIZE];
	if(argc != 2)
	{
		printUsage(argv[0]);

		cout << "press any key and enter to quit" << endl;
		char temp;
		cin >> temp;

		return 10;
	}

		try
	{
		cout << "Opening com port"<< endl;
		tstring commPortName(argv[1]);
		Serial serial(commPortName);
		cout << "Port opened" << endl;

		cout << "writing something to the serial port" << endl;
		serial.flush();
		char hi[]="hi";
		while (1)
		{
			hi[0]= getch();
			int bytesWritten = serial.write(hi);
			cout << bytesWritten << " bytes were written to the serial port" << endl;
			for (int i = 0; i < 10; i++)
			{
				int charsRead = serial.read(buffer, RX_BUFFSIZE);
				cout << buffer;
				Sleep(10);
			}
			if (bytesWritten != sizeof(hi) - 1)
			{
				cout << "Writing to the serial port timed out" << endl;
			}
			Sleep(100);
		}

		

		cout << "Reading from the serial port: ";
		while (1)
		{
			if (int cha = serial.read(buffer, RX_BUFFSIZE) > 5)
			{
				for (int i = 0; i < 10; i++)
				{
					int charsRead = serial.read(buffer, RX_BUFFSIZE);
					cout << buffer;
					Sleep(100);
				}
			}
		}
		cout << endl;

	}catch(const char *msg)
	{
		cout << msg << endl;
	}

	cout << "press any key and enter to quit" << endl;
	char temp;
	cin >> temp;

	return 0;
}

void printUsage(_TCHAR progName[])
{
#if defined(UNICODE)
	wcout << progName << " <comm port>" << endl
		 << "e.g., " << progName << " COM1" << endl;
#else
	cout << progName << " <comm port>" << endl
		 << "e.g., " << progName << " COM1" << endl;
#endif
	
}*/