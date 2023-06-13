#include <hidapi.h>
#include <thread>
#include "bb_controller.h"
#include <map>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

void ButtonBarController::report(unsigned char report[2])
{
	map<int, int> ::iterator i;
	bool temp = false;
	int flag = 0;
	int addr = 0;
	for (i= this->button_map.begin(); i != this->button_map.end(); i++)
	{
		flag = (*i).first;
		addr = (*i).second;
		this->buttons[addr] = ((report[1] & flag) == flag);
	}
};

void ButtonBarController::setup()
{
	hid_init();
	this->bb_hid_device = hid_open(this->vid, this->pid, NULL);
	hid_set_nonblocking(this->bb_hid_device, 1);
	this->running = false;
};

void ButtonBarController::close()
{
	hid_close(this->bb_hid_device);
};

void ButtonBarController::poller()
{
	while (this->running) {
		unsigned char device_report[2];
		int result = 0;
		result = hid_read(this->bb_hid_device, device_report, 2);
		if (result > 0) {
			this->report(device_report);
		}
		Sleep(10);
	};
};

ButtonBarController::ButtonBarController()
{
	this->vid = 12346;
	this->pid = 32963;
	this->button_map[1] = 0;
	this->button_map[2] = 1;
	this->button_map[4] = 2;
	this->button_map[8] = 3;
	this->button_map[16] = 4;
	this->button_map[32] = 5;
	this->button_map[64] = 6;
	this->button_map[128] = 7;
	for (short i = 0; i < 8; i++)
	{
		this->buttons[i] = false;
	}
	this->setup();
};

bool ButtonBarController::Get_button_state(int button_id)
{
	return this->buttons[button_id];
}

void ButtonBarController::Start()
{
	this->running = true;
	this->poller_thread = thread(&ButtonBarController::poller, this);
};

void ButtonBarController::Stop()
{
	this->running = false;
	this->poller_thread.join();
};