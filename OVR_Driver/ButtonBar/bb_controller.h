#pragma once
#include <hidapi.h>
#include <thread>
#include <mutex>
#include <map>
#include "driverlog.h"
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
	mutex button_lock;
	unsigned short vid;
	unsigned short pid;
	hid_device *bb_hid_device;
	bool running;
	thread poller_thread;
	map<int, int> button_map;

	void report(unsigned char report[2]);

	void setup();

	void close();

	void poller();

	void set_button_state(int button_id, bool state);

public:

	ButtonBarController();

	bool Get_button_state(int button_id);
	void Get_button_states(bool* states);

	void Start();

	void Stop();

	void PollForChanges();

};