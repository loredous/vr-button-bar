//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "controller_device_driver.h"

#include "driverlog.h"
#include "bb_controller.h"

// Let's create some variables for strings used in getting settings.
// This is the section where all of the settings we want are stored. A section name can be anything,
// but if you want to store driver specific settings, it's best to namespace the section with the driver identifier
// ie "<my_driver>_<section>" to avoid collisions
static const char *my_controller_main_settings_section = "driver_buttonbar";

// Single settings 
static const char *my_buttonbar_settings_section = "driver_buttonbar_controller";

// These are the keys we want to retrieve the values for in the settings
static const char *my_controller_settings_key_model_number = "mycontroller_model_number";
static const char *my_controller_settings_key_serial_number = "mycontroller_serial_number";


MyControllerDeviceDriver::MyControllerDeviceDriver( vr::ETrackedControllerRole role )
{
	// Set a member to keep track of whether we've activated yet or not
	is_active_ = false;

	// The constructor takes a role argument, that gives us information about if our controller is a left or right hand.
	// Let's store it for later use. We'll need it.
	my_controller_role_ = role;

	// We have our model number and serial number stored in SteamVR settings. We need to get them and do so here.
	// Other IVRSettings methods (to get int32, floats, bools) return the data, instead of modifying, but strings are
	// different.
	char model_number[ 1024 ];
	vr::VRSettings()->GetString( my_controller_main_settings_section, my_controller_settings_key_model_number, model_number, sizeof( model_number ) );
	my_controller_model_number_ = model_number;

	// Get our serial number depending on our "handedness"
	char serial_number[ 1024 ];
	vr::VRSettings()->GetString( my_buttonbar_settings_section,
		my_controller_settings_key_serial_number, serial_number, sizeof( serial_number ) );
	my_controller_serial_number_ = serial_number;

	// Here's an example of how to use our logging wrapper around IVRDriverLog
	// In SteamVR logs (SteamVR Hamburger Menu > Developer Settings > Web console) drivers have a prefix of
	// "<driver_name>:". You can search this in the top search bar to find the info that you've logged.
	DriverLog( "ButtonBar Model Number: %s", my_controller_model_number_.c_str() );
	DriverLog( "ButtonBar Serial Number: %s", my_controller_serial_number_.c_str() );
}

void MyControllerDeviceDriver::button_poller()
{
	while (this->is_active_)
	{
		// update our inputs here
		bool states[8] = { false };
		buttonbar_controller_.Get_button_states(states);
		vr::VRDriverInput()->UpdateBooleanComponent(this->b1_handle, true, 50);
		vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[MyComponent_button2_click], true, 50);
		vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[MyComponent_button3_click], states[2], 50);
		vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[MyComponent_button4_click], states[3], 50);
		vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[MyComponent_button5_click], states[4], 50);
		vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[MyComponent_button6_click], states[5], 50);
	}
}

vr::DriverPose_t MyControllerDeviceDriver::GetPose()
{
	// Let's retrieve the Hmd pose to base our controller pose off.

	// First, initialize the struct that we'll be submitting to the runtime to tell it we've updated our pose.
	vr::DriverPose_t pose = { 0 };

	// The pose we provided is valid.
	// This should be set is
	pose.poseIsValid = true;

	// Our device is always connected.
	// In reality with physical devices, when they get disconnected,
	// set this to false and icons in SteamVR will be updated to show the device is disconnected
	pose.deviceIsConnected = true;

	// The state of our tracking. For our virtual device, it's always going to be ok,
	// but this can get set differently to inform the runtime about the state of the device's tracking
	// and update the icons to inform the user accordingly.
	pose.result = vr::TrackingResult_Running_OK;

	return pose;
}
//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver after our
//  IServerTrackedDeviceProvider calls IVRServerDriverHost::TrackedDeviceAdded.
//-----------------------------------------------------------------------------
vr::EVRInitError MyControllerDeviceDriver::Activate( uint32_t unObjectId )
{
	// Set an member to keep track of whether we've activated yet or not
	is_active_ = true;

	// Let's keep track of our device index. It'll be useful later.
	my_controller_index_ = unObjectId;

	// Properties are stored in containers, usually one container per device index. We need to get this container to set
	// The properties we want, so we call this to retrieve a handle to it.
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer( my_controller_index_ );

	// Let's begin setting up the properties now we've got our container.
	// A list of properties available is contained in vr::ETrackedDeviceProperty.

	// First, let's set the model number.
	vr::VRProperties()->SetStringProperty( container, vr::Prop_ModelNumber_String, my_controller_model_number_.c_str() );

	// Let's tell SteamVR our role which we received from the constructor earlier.
	vr::VRProperties()->SetInt32Property( container, vr::Prop_ControllerRoleHint_Int32, my_controller_role_ );


	// Now let's set up our inputs

	// This tells the UI what to show the user for bindings for this controller,
	// As well as what default bindings should be for legacy apps.
	// Note, we can use the wildcard {<driver_name>} to match the root folder location
	// of our driver.
	vr::VRProperties()->SetStringProperty( container, vr::Prop_InputProfilePath_String, "{buttonbar}/input/mycontroller_profile.json" );

	// Let's set up handles for all of our components.
	// Even though these are also defined in our input profile,
	// We need to get handles to them to update the inputs.
	vr::EVRInputError result;

	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/a/click", &b1_handle );
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/b/click", &input_handles_[MyComponent_button2_click]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/x/click", &input_handles_[MyComponent_button3_click]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/y/click", &input_handles_[MyComponent_button4_click]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/start/click", &input_handles_[MyComponent_button5_click]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/back/click", &input_handles_[MyComponent_button6_click]);

	buttonbar_controller_.Start();
	button_update_thread = thread(&MyControllerDeviceDriver::button_poller, this);
	DriverLog("Started ButtonBar tracking!");

	// We've activated everything successfully!
	// Let's tell SteamVR that by saying we don't have any errors.
	return vr::VRInitError_None;
}

//-----------------------------------------------------------------------------
// Purpose: If you're an HMD, this is where you would return an implementation
// of vr::IVRDisplayComponent, vr::IVRVirtualDisplay or vr::IVRDirectModeComponent.
//
// But this a simple example to demo for a controller, so we'll just return nullptr here.
//-----------------------------------------------------------------------------
void *MyControllerDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
	return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when a debug request has been made from an application to the driver.
// What is in the response and request is up to the application and driver to figure out themselves.
//-----------------------------------------------------------------------------
void MyControllerDeviceDriver::DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
	if ( unResponseBufferSize >= 1 )
		pchResponseBuffer[ 0 ] = 0;
}


//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when the device should enter standby mode.
// The device should be put into whatever low power mode it has.
// We don't really have anything to do here, so let's just log something.
//-----------------------------------------------------------------------------
void MyControllerDeviceDriver::EnterStandby()
{
	DriverLog( "Buttonbar requested to standby" );
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when the device should deactivate.
// This is typically at the end of a session
// The device should free any resources it has allocated here.
//-----------------------------------------------------------------------------
void MyControllerDeviceDriver::Deactivate()
{
	// Let's join our pose thread that's running
	// by first checking then setting is_active_ to false to break out
	// of the while loop, if it's running, then call .join() on the thread
	if ( is_active_.exchange( false ) )
	{
		buttonbar_controller_.Stop();
		button_update_thread.join();
	}

	// unassign our controller index (we don't want to be calling vrserver anymore after Deactivate() has been called
	my_controller_index_ = vr::k_unTrackedDeviceIndexInvalid;
}


//-----------------------------------------------------------------------------
// Purpose: This is called by our IServerTrackedDeviceProvider when its RunFrame() method gets called.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
void MyControllerDeviceDriver::MyRunFrame()
{

}


//-----------------------------------------------------------------------------
// Purpose: This is called by our IServerTrackedDeviceProvider when it pops an event off the event queue.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
void MyControllerDeviceDriver::MyProcessEvent( const vr::VREvent_t &vrevent )
{
	
}

//-----------------------------------------------------------------------------
// Purpose: Our IServerTrackedDeviceProvider needs our serial number to add us to vrserver.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
const std::string &MyControllerDeviceDriver::MyGetSerialNumber()
{
	return my_controller_serial_number_;
}