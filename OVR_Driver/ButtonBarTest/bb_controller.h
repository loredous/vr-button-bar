#pragma once
#include <hidapi.h>
#include <thread>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

class ButtonBarController
{
private:
	bool buttons[8];
	short vid;
	short pid;
	hid_device *bb_hid_device;
	bool running;
	thread poller_thread;
	map<int, int> button_map;

	void report(unsigned char report[2]);

	void setup();

	void close();

	void poller();

public:

	ButtonBarController();

	bool Get_button_state(int button_id);

	void Start();

	void Stop();

};