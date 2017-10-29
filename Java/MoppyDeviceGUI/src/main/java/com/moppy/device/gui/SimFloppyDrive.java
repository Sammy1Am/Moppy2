/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.device.gui;

import com.jsyn.ports.UnitOutputPort;
import com.jsyn.unitgen.SquareOscillator;

/**
 * A simulated FloppyDrive for testing purposes
 */
public class SimFloppyDrive {
    public SquareOscillator so = new SquareOscillator();
    
    public SimFloppyDrive(){
        so.noteOff();
    }
    
    public void playFrequency(double frequency){
        so.noteOn(frequency, 0.05);
    }
    
    public void stopFrequency(){
        so.noteOff();
    }
    
    public void resetDrive(){
        so.noteOff();
    }
    
    public UnitOutputPort getOutput(){
        return so.getOutput();
    }
}
