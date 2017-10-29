/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.device.gui;

import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;

/**
 * FXML Controller class
 *
 * @author Sam
 */
public class MainWindowController implements Initializable {

    DeviceImpl device;
    
    @FXML
    private Button connectButton;
    
    @FXML
    private Button disconnectButton;
    
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }    
    
    public void doConnect() throws IOException {
        connectButton.setDisable(true);
        device = new DeviceImpl();
        device.connect();
        disconnectButton.setDisable(false);
    }
    
    public void doDisconnect() throws IOException {
        disconnectButton.setDisable(true);
        device.close();
        connectButton.setDisable(false);
    }
}
