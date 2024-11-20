package UI;
import javax.swing.*;
import java.awt.*;
import static UI.InterfaceWindow.LENGTH_UNIT;

public class DataComponent extends JComponent{

    public int WINDOW_WIDTH = 2 * LENGTH_UNIT;
    public int WINDOW_HEIGHT = 5 * LENGTH_UNIT;
    public int TEXT_SIZE = LENGTH_UNIT / 5;

    // steering data
    int intensity;
    int turn;
    boolean is_breaking;
    boolean manual;

    // sensor data
    int dist;
    int vel_r;
    int vel_l;
    int angle;

    public DataComponent () {
        this.intensity = 0;
        this.turn = 0;
        this.is_breaking = true;
        this.dist = 0;
        this.vel_r = 0;
        this.vel_l = 0;
        this.angle = 0;
        this.manual = true;
    }

    public Dimension getPreferredSize() {
        return new Dimension(WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    public void set_steering_data(int intensity, int turn, boolean is_breaking) {
        this.intensity = intensity;
        this.turn = turn;
        this.is_breaking = is_breaking;
        repaint();
    }

    public void set_sensor_data(int dist, int vel_r, int vel_l, int angle) {
        this.dist = dist;
        this.vel_r = vel_r;
        this.vel_l = vel_l;
        this.angle = angle;
        repaint();
    }

    public void set_manual(boolean b){
        this.manual = b;
    }

    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        final Graphics2D g2d = (Graphics2D) g;
        //paint background
        g2d.setColor(Color.WHITE);
        g2d.fillRect(0, 0, (int)getPreferredSize().getWidth(), (int)getPreferredSize().getHeight());
        // paint text
        String[] lines = {"Intensity: "+this.intensity, "Turn: "+this.turn, "Breaking: "+this.is_breaking, 
        "Distance: "+this.dist, "Velocity: "+((this.vel_r+this.vel_l)/2), "Angle: "+ this.angle, "Manual: " + this.manual,
         "Automatic: " + !this.manual};
        int y = TEXT_SIZE;
        int x = TEXT_SIZE;

        for (String line: lines){
            paintText(g2d, line, x, y);
            y += TEXT_SIZE*3/2;
        }
    }

    public void paintText(Graphics2D g2d, String string, int x, int y) {
        g2d.setColor(Color.BLACK);
        g2d.setFont(new Font ("TimesRoman", Font.PLAIN, TEXT_SIZE));
        g2d.drawString(string, x, y);
    }


    
}
