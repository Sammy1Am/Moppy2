package com.moppy.control.gui.netpanel;

import com.moppy.core.comms.bridge.MultiBridge;
import com.moppy.core.comms.bridge.NetworkBridge;
import java.awt.Color;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JCheckBox;
import javax.swing.JPanel;

/**
 *
 */
public abstract class NetPanel extends JPanel {
    protected final MultiBridge multiBridge;
    protected final NetworkBridge netBridge;
    
    public NetPanel(MultiBridge multiBridge, NetworkBridge netBridge) {
        this.multiBridge = multiBridge;
        this.netBridge = netBridge;
    }
    
    protected void initNetPanel(JCheckBox activeCheckbox) {
        showInactive(activeCheckbox);
    }
    
    private void showActive(JCheckBox activeCheckbox) {
        activeCheckbox.setForeground(Color.BLACK);
        activeCheckbox.setToolTipText(netBridge.getNetworkIdentifier());
    }
    
    private void showInactive(JCheckBox activeCheckbox) {
        activeCheckbox.setToolTipText(netBridge.getNetworkIdentifier());
        activeCheckbox.setForeground(Color.GRAY);
    }
    
    private void showError(JCheckBox activeCheckbox, Exception ex) {
        activeCheckbox.setToolTipText(ex.getMessage());
        activeCheckbox.setForeground(Color.RED);
        activeCheckbox.setSelected(false);
    }
    
    protected void activate(JCheckBox activeCheckbox) {
        try {
            netBridge.connect();
            multiBridge.addBridge(netBridge);
            showActive(activeCheckbox);
        } catch (IOException ex) {
            Logger.getLogger(NetPanel.class.getName()).log(Level.WARNING, null, ex);
            showError(activeCheckbox, ex);
        }
    }
    
    protected void deactivate(JCheckBox activeCheckbox) {
        try {
            multiBridge.removeBridge(netBridge);
            netBridge.close();
        } catch (IOException ex) {
            Logger.getLogger(NetPanel.class.getName()).log(Level.WARNING, null, ex);
        }
        showInactive(activeCheckbox);
    }
}
