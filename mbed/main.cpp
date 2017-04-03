#include "mbed.h"
#include "utils.hpp"
#include "EthernetInterface.h"
#include "frdm_client.hpp"
#include "metronome.hpp"
#include <string>
#define IOT_ENABLED

Serial pc(USBTX, USBRX);
using namespace std;

namespace active_low
{
	const bool on = false;
	const bool off = true;
}

DigitalOut g_led_red(LED1);
DigitalOut g_led_green(LED2);
DigitalOut g_led_blue(LED3);

InterruptIn g_button_mode(SW3);
InterruptIn g_button_tap(SW2);

int temp = 0;
int localBPM = 0;

//create a metronome
metronome myMetronome(60, 0);

void on_mode()
{
	// Change modes
	if (myMetronome.isLearnMode()) {
		g_led_green = active_low::off;
		myMetronome.disableLearn();	
	}	
	else {
		g_led_green = active_low::off;
		myMetronome.enableLearn();
	}
}

void on_tap()
{
	// Receive a tempo tap
	if (myMetronome.isLearnMode()) {
		myMetronome.tap();
		myMetronome.blink(g_led_red);
	}
}

int main()
{	
	// Seed the RNG for networking purposes
    unsigned seed = utils::entropy_seed();
    srand(seed);

	// Button falling edge is on push (rising is on release)
 	g_button_mode.fall(&on_mode);   
    g_button_tap.fall(&on_tap);
    	
	// LEDs are active LOW - true/1 means off, false/0 means on
	// Use the constants for easier reading
    g_led_red = active_low::off;
    g_led_green = active_low::on;
    g_led_blue = active_low::off;
	
#ifdef IOT_ENABLED
	// Turn on the blue LED until connected to the network
    g_led_blue = active_low::on;

	// Need to be connected with Ethernet cable for success
    EthernetInterface ethernet;
    if (ethernet.connect() != 0)
        return 1;

	// Pair with the device connector
    frdm_client client("coap://api.connector.mbed.com:5684", &ethernet);
    if (client.get_state() == frdm_client::state::error)
        return 1;

	// The REST endpoints for this device
	// Add your own M2MObjects to this list with push_back before client.connect()
    M2MObject* this_object; 
	//M2MResource* expose_this_value;
	M2MResource *units, *bpm, *min_bpm, *max_bpm; 
    M2MObjectList objects; 
	
	this_object = M2MInterfaceFactory::create_object("3318");
    M2MObjectInstance* this_object_instance = this_object->create_object_instance();
    
    //Now expose the units_expose	
	bpm = this_object_instance->create_dynamic_resource("5900", "BPM Value", M2MResourceInstance::INTEGER, false);
	bpm->set_operation(M2MBase::GET_PUT_ALLOWED);
	bpm->set_value(localBPM);
	objects.push_back(this_object); 
	
	min_bpm = this_object_instance->create_dynamic_resource("5601", "Minimum BPM Value", M2MResourceInstance::INTEGER, false);
	min_bpm->set_operation(M2MBase::GET_ALLOWED);
	min_bpm->set_value(myMetronome.getMinBPM());
	objects.push_back(this_object); 
	
	max_bpm = this_object_instance->create_dynamic_resource("5602", "Maximum BPM Value", M2MResourceInstance::INTEGER, false);
	max_bpm->set_operation(M2MBase::GET_ALLOWED);
	max_bpm->set_value(myMetronome.getMaxBPM());
	objects.push_back(this_object);
		
	units = this_object_instance->create_dynamic_resource("5701", "Units", M2MResourceInstance::STRING, false);
    units->set_operation(M2MBase::GET_ALLOWED);
    char buffer[4] = "BPM";
    units->set_value((const uint8_t*)buffer, 4);
    objects.push_back(this_object);  
	
    M2MDevice* device = frdm_client::make_device();
    objects.push_back(device);

	// Publish the RESTful endpoints
    client.connect(objects);

	// Connect complete; turn off blue LED forever
    g_led_blue = active_low::off; //change here by Karan
#endif

    while (true)
    {
    	localBPM = myMetronome.getBPM();
    	
    	if (!myMetronome.isLearnMode()) 	
    		myMetronome.play(g_led_green);
	
		bpm->set_value(localBPM);
		min_bpm->set_value(myMetronome.getMinBPM());
		max_bpm->set_value(myMetronome.getMaxBPM());

#ifdef IOT_ENABLED
        if (client.get_state() == frdm_client::state::error)
           break;     
#endif
    }
#ifdef IOT_ENABLED
    client.disconnect();
#endif
    return 1;
}