// ButtonBarTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "bb_controller.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main()
{
    string s;
    ButtonBarController ctl = ButtonBarController();
    ctl.Start();
    while (true) {
        cout << ctl.Get_button_state(0) << endl;
        cout << ctl.Get_button_state(1) << endl;
        cout << ctl.Get_button_state(2) << endl;
        cout << ctl.Get_button_state(3) << endl;
        cout << ctl.Get_button_state(4) << endl;
        cout << ctl.Get_button_state(5) << endl;
        cout << ctl.Get_button_state(6) << endl;
        cout << ctl.Get_button_state(7) << endl;
        Sleep(1000);
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
