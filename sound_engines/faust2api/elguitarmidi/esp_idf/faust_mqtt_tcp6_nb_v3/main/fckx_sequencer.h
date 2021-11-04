/************************************************************************
 ************************************************************************
 FCKX_SEQUENCER API 
 Copyright (C) 2021 FCKX
 ---------------------------------------------------------------------
Sequencer functions for application with TTGO TAudio V1.6 board and
Faust DSP engines
 ************************************************************************
 ************************************************************************/


#ifndef __fckx_sequencer_api__
#define __fckx_sequencer_api__

class FCKXSequence {
    public:         //publicly accessible
        void setTempo (float tempoin);
        float getTempo();
        FCKXSequence(); //constructor
    private:        //only accessible for class member functions
        //jdksmidi trackstatus
        float tempobpm;    // current tempo in beats per minute
        int pg;      // current program change, or -1
        int volume;     // current volume controller value
        int timesig_numerator;  // numerator of current time signature
        int timesig_denominator; // denominator of current time signature
        int bender_value;   // last seen bender value
        char track_name[256];  // track name
        bool got_good_track_name; // true if we dont want to use generic text events for track name
        bool notes_are_on;   // true if there are any notes currently on

};


FCKXSequence::FCKXSequence(void){   //constructor definition
    tempobpm = 120;
}

void FCKXSequence::setTempo( float tempoin ) {
   tempobpm = tempoin;
}
float FCKXSequence::getTempo( void ) {
   return tempobpm;
}


void test_fckx_sequencer_lib();

#endif