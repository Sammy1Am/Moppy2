package com.moppy.control;

import com.moppy.control.gui.MainWindow;
import com.moppy.core.comms.bridge.MultiBridge;
import com.moppy.core.events.mapper.MIDIEventMapper;
import com.moppy.core.events.mapper.MapperCollection;
import com.moppy.core.midi.MoppyMIDIReceiverSender;
import com.moppy.core.midi.MoppyMIDISequencer;
import com.moppy.core.status.StatusBus;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.MidiUnavailableException;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

public class MoppyControlGUI {

    /**
     * @param args the command line arguments
     * @throws java.io.IOException
     * @throws javax.sound.midi.MidiUnavailableException
     */
    public static void main(String[] args) throws IOException, MidiUnavailableException {
        
        //
        //// Initialize Moppy System components
        //

        // Create components
        final StatusBus statusBus = new StatusBus();
        final MultiBridge networkBridge = new MultiBridge();
        final MapperCollection mappers = new MapperCollection();
        final MoppyMIDIReceiverSender receiverSender = new MoppyMIDIReceiverSender(mappers, networkBridge);
        final MoppyMIDISequencer midiSequencer = new MoppyMIDISequencer(statusBus, receiverSender);
        
        // Setup shutdown hook to properly close everything down.
        Runtime.getRuntime().addShutdownHook(new Thread(){
            @Override
            public void run() {
                try {
                    midiSequencer.close();
                } catch (IOException ex) {
                    Logger.getLogger(MoppyControlGUI.class.getName()).log(Level.WARNING, null, ex);
                }
                receiverSender.close();
                try {
                    networkBridge.close();
                } catch (IOException ex) {
                    Logger.getLogger(MoppyControlGUI.class.getName()).log(Level.WARNING, null, ex);
                }
            }
        });
        
        //
        //// Load Settings / Defaults
        //
        
        mappers.addMapper(MIDIEventMapper.defaultMapper((byte)0x01));
        
        //
        //// Initialize and start the UI
        //
        
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (ClassNotFoundException | InstantiationException | IllegalAccessException | UnsupportedLookAndFeelException ex) {
            Logger.getLogger(MoppyControlGUI.class.getName()).log(Level.SEVERE, null, ex);
        }
        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            @Override
            public void run() {
                new MainWindow(statusBus, midiSequencer, networkBridge).setVisible(true);
            }
        });
    }

    
}
