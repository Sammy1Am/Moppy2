package com.moppy.control.gui;

import com.moppy.core.midi.MoppyMIDISequencer;
import com.moppy.core.status.StatusBus;

/**
 * 
 */
public class MainWindow extends javax.swing.JFrame {

    private final StatusBus statusBus;
    private final MoppyMIDISequencer midiSequencer;
    
    
    /**
     * Creates new form MainWindow
     */
    public MainWindow(StatusBus statusBus, MoppyMIDISequencer midiSequencer) {
        this.statusBus = statusBus;
        this.midiSequencer = midiSequencer;
        
        initComponents();
    }

    /**
     * This method is called from within the constructor to initialize the form. WARNING: Do NOT modify this code. The content of this method is always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        sequencerPanel = new SequencerPanel(midiSequencer);
        statusBus.registerConsumer(sequencerPanel);

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Moppy Control");

        sequencerPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(sequencerPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(324, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(sequencerPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(164, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private com.moppy.control.gui.SequencerPanel sequencerPanel;
    // End of variables declaration//GEN-END:variables
}
