package UI;

import javax.swing.*;

public class InputWindow{
    private String text;

    public InputWindow(String text) {
        this.text = text;
    }

    public String showWindow(){
        //JFrame window = new JFrame();
        String input = JOptionPane.showInputDialog(text);
        return input;
    }
}
