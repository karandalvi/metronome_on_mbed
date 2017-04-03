#pragma once
#include "Timer.h"
#include <limits>
class metronome
{

public:
    metronome(float i, float b);

public:

	//-----------------------------------------------------------------------------------
	
	// Getter Setter functions for Interval, BPI, WaitTime & learn_mode
	
	void updateTempo(float i, float b) {
		if ((b==0) || (i==0)) {
			setInterval(b);
			setBpi(i);
		}
		else {
			setInterval(i);
			setBpi(b);
			if (getBPM() > max)
				max = getBPM();
			if ((getBPM() < min) || (min == 0))
				min = getBPM();
		}
	}
	
    float getInterval() {
        return interval;
    }

    void setInterval (float i) {
        interval = i;
        updateWaitTime();
    }

    float getBpi () {
        return bpi;
    }

    void setBpi (float b) {
		bpi = b;
        updateWaitTime();
    }

    void updateWaitTime() {
        if (bpi == 0)
        	waitTime = 0;
        else
        	waitTime = interval / (2*bpi);
    }
	
  	bool isLearnMode() {
    	return learn_mode;
  	}
  	
  	int getMinBPM() {
  		return min;
  	}
  	
  	int getMaxBPM() {
  		return max;
  	}

  	void setMinBPM(int newMin ) {
  		min = newMin;
  	}
  	
  	void setMaxBPM(int newMax) {
  		max = newMax;
  	}  	
  	
  	int getBPM() {
		return round((bpi / interval) * 60);
	}

	//Generic round method
	int round(float f)
	{
  		return floor(f + 0.5);
	}
	
	//-----------------------------------------------------------------------------------

	//Method to run one iteration of play mode
    void play(DigitalOut playLED) {
        if (waitTime != 0) {
	        playLED = !playLED;
	        wait(waitTime);
	        if (isLearnMode())
	    		turnOff(playLED);
    	}
    	else 
    		turnOff(playLED);
    }

	//Method to blink LED once
    void blink(DigitalOut playLED) {
        playLED = !playLED;
        wait(0.1f);
        playLED = !playLED;
        wait(0.1f);        
    }
    
    //Methods to turn on / off a LED
    void turnOff(DigitalOut playLED) {
        playLED = true;
    }
    
    void turnOn(DigitalOut playLED) {
        playLED = false;
    }
    
	//-----------------------------------------------------------------------------------
	
    //Methods to enable or disable learn mode
    
    void enableLearn() {
		learn_mode = true;
		start_timing();   
    }
    
    void start_timing() {
              m_beat_count = 0;
              timer.reset();
              timer.start();     
    };
    
    void disableLearn() {
		stop_timing();
    	learn_mode = false;        
    }
        
    void stop_timing() {
    	if (m_beat_count >= 4) {
			updateTempo(timer.read(), m_beat_count);
			m_beat_count = 0; 
			timer.stop();
			timer.reset();
        }
    }

	//-----------------------------------------------------------------------------------

    void tap() {
    	m_beat_count++;
    };    

	//-----------------------------------------------------------------------------------
	
private:
    Timer timer;
    bool learn_mode;
    float waitTime;
    float interval;
    float bpi;
    float m_beat_count;
    int max;
    int min;   
};


	//-----------------------------------------------------------------------------------
	//Constructor for metronome
	
	metronome::metronome(float i, float b)
	{
	    interval = 0;
	    bpi = 0;
	    updateTempo(i,b);
	    learn_mode = false;
	    m_beat_count = 0;
	    max = 0;
	    min = 0;
	}
