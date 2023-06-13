#include "bb_controller.h"
#include <string>

using namespace std;

void ButtonBarController::report(unsigned char report[2])
{
	map<int, int> ::iterator i;
	bool temp = false;
	int flag = 0;
	int addr = 0;
	DriverLog("------------ START REPORT ---------------");
	for (i= this->button_map.begin(); i != this->button_map.end(); i++)
	{
		flag = (*i).first;
		addr = (*i).second;
		this->set_button_state(addr, ((report[1] & flag) == flag));
	}
	DriverLog("------------ END REPORT ---------------");
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
		this->PollForChanges();
	};
};

void ButtonBarController::PollForChanges()
{
	unsigned char device_report[2];
	int result = 1;
	while (result > 0) {
		result = hid_read(this->bb_hid_device, device_report, 2);
		if (result > 0) {
			this->report(device_report);	
		};
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
	lock_guard<mutex> lock(button_lock);
	return (bool)this->buttons[button_id];
}

void ButtonBarController::Get_button_states(bool *states)
{
	bool button_state[8];
	{
		lock_guard<mutex> lock(button_lock);
		copy(begin(this->buttons), end(this->buttons), begin(button_state));
	}
	for (int button_id = 0; button_id < 8; button_id++)
	{
		states[button_id] = button_state[button_id];
	}
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

void ButtonBarController::set_button_state(int button_id, bool state)
{
	{
		lock_guard<mutex> lock(button_lock);
		this->buttons[button_id] = state;
	}
	DriverLog("Button %s: %s", to_string(button_id), to_string(state));
};