package UI;

import javax.swing.*;
import java.awt.*;
import static UI.InterfaceWindow.LENGTH_UNIT;

public class MapComponent extends JComponent{

    public int WINDOW_WIDTH = 4 * LENGTH_UNIT;
    public int WINDOW_HEIGHT = 5 * LENGTH_UNIT;
    public int TRACK_LENGTH = LENGTH_UNIT;
    public int TRACK_WIDHT = LENGTH_UNIT / 25;
    public int NODE_RADIUS = LENGTH_UNIT / 5;
    public int CROSSING_SIZE = LENGTH_UNIT / 5;
    public int TEXT_SIZE = LENGTH_UNIT / 10;

    // Position
    char last_pos;
    char next_pos;

    String path = "";

    public Dimension getPreferredSize() {
        return new Dimension(WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    public void setPath(char[] path){
        this.path = "";
        for (int i = 0; i < path.length; i++) {
            this.path += path[i];
            if (i != path.length -1) {
                this.path += " - ";
            }
        }
        setPosition(path);
        repaint();
    }

    public void setPosition (char[] path) {
        this.last_pos = path[0];
        this.next_pos = path[0];
        if (path.length > 1){
            this.next_pos = path[1];
        }
    }

    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        final Graphics2D g2d = (Graphics2D) g;
        // paint background
        g2d.setColor(Color.lightGray);
        g2d.fillRect(0, 0, (int)getPreferredSize().getWidth(), (int)getPreferredSize().getHeight());

        // paint tracks
        paintTrack(g2d, TRACK_LENGTH, 2*TRACK_LENGTH, 2*TRACK_LENGTH, TRACK_LENGTH, '1','A','B', '1'); // left upper
        paintTrack(g2d, TRACK_LENGTH, 2*TRACK_LENGTH, TRACK_LENGTH, 3*TRACK_LENGTH, 'A','2','B','1'); // left middle
        paintTrack(g2d, TRACK_LENGTH, 3*TRACK_LENGTH, 2*TRACK_LENGTH, 4*TRACK_LENGTH, '2','B','A','2'); // left lower
        paintTrack(g2d, 2*TRACK_LENGTH, 2*TRACK_LENGTH, 2*TRACK_LENGTH, TRACK_LENGTH, 'F','1','1','E'); // middle upper
        paintTrack(g2d, 2*TRACK_LENGTH, 2*TRACK_LENGTH, 2*TRACK_LENGTH, 3*TRACK_LENGTH, 'F','1','E','2'); // middle middle
        paintTrack(g2d, 2*TRACK_LENGTH, 3*TRACK_LENGTH, 2*TRACK_LENGTH, 4*TRACK_LENGTH, '2','F','E','2'); // middle lower
        paintTrack(g2d, 3*TRACK_LENGTH, 2*TRACK_LENGTH, 2*TRACK_LENGTH, 1*TRACK_LENGTH, 'D','1','1','C'); // right upper
        paintTrack(g2d, 3*TRACK_LENGTH, 2*TRACK_LENGTH, 3*TRACK_LENGTH, 3*TRACK_LENGTH, '2','D','1','C'); // right middle
        paintTrack(g2d, 3*TRACK_LENGTH, 3*TRACK_LENGTH, 2*TRACK_LENGTH, 4*TRACK_LENGTH, '2','D','C','2'); // right lower
        // paint crossings
        paintCrossing(g2d, 2*TRACK_LENGTH, TRACK_LENGTH, '1');
        paintCrossing(g2d, 2*TRACK_LENGTH, 4*TRACK_LENGTH, '2');
        // paint nodes
        paintNode(g2d, TRACK_LENGTH, 2*TRACK_LENGTH,false, 'A');
        paintNode(g2d, TRACK_LENGTH, 3*TRACK_LENGTH, true, 'B');
        paintNode(g2d, 2*TRACK_LENGTH, 2*TRACK_LENGTH, false, 'E');
        paintNode(g2d, 2*TRACK_LENGTH, 3*TRACK_LENGTH, true,'F');
        paintNode(g2d, 3*TRACK_LENGTH, 2*TRACK_LENGTH, true, 'D');
        paintNode(g2d, 3*TRACK_LENGTH, 3*TRACK_LENGTH,false, 'C');
        // paint strings
        paintString(g2d, TRACK_LENGTH, 2*TRACK_LENGTH, "A", false);
        paintString(g2d, TRACK_LENGTH, 3*TRACK_LENGTH, "B", true);
        paintString(g2d, 2*TRACK_LENGTH, 2*TRACK_LENGTH, "E", false);
        paintString(g2d, 2*TRACK_LENGTH, 3*TRACK_LENGTH, "F", true);
        paintString(g2d, 3*TRACK_LENGTH, 2*TRACK_LENGTH, "D", true);
        paintString(g2d, 3*TRACK_LENGTH, 3*TRACK_LENGTH, "C", false);
        paintString(g2d, 2*TRACK_LENGTH, TRACK_LENGTH, "1", true);
        paintString(g2d, 2*TRACK_LENGTH, 4*TRACK_LENGTH, "2", true);
        // paint path string
        if (path != null) {
            paintPath(g2d);
        }
    }

    // Takes a color and the start and end coordinates of the track.
    public void paintTrack(Graphics2D g2d, int start_x, int start_y, int end_x, int end_y, char last_id1, char next_id1, char last_id2, char next_id2) {
        if ((last_id1 == last_pos && next_id1 == next_pos) || (last_id2 == last_pos && next_id2 == next_pos)) {
            g2d.setColor(Color.green);
        }
        else {
            g2d.setColor(Color.white);
        }
        g2d.setStroke(new BasicStroke(TRACK_WIDHT));
        g2d.drawLine(start_x, start_y, end_x, end_y);
    }

    // Takes a color and the centre coordinates of the node.
    public void paintNode(Graphics2D g2d, int x, int y, boolean right, char id) {
        if (id == last_pos) {
            g2d.setColor(Color.green);
        }
        else if (id == next_pos) {
            g2d.setColor(Color.orange);
        }
        else {
            g2d.setColor(Color.white);
        }
        g2d.fillOval(x - (NODE_RADIUS / 2), y - (NODE_RADIUS / 2), NODE_RADIUS, NODE_RADIUS);
        g2d.setColor(Color.black);
        if (right){
            g2d.fillOval(x + (NODE_RADIUS / 4), y - (NODE_RADIUS / 4), NODE_RADIUS/2, NODE_RADIUS/2);
        }
        else {
            g2d.fillOval(x - (NODE_RADIUS *3/ 4), y - (NODE_RADIUS / 4), NODE_RADIUS/2, NODE_RADIUS/2);
        }
    }

    // Draws the letter start_identifier for the drop off nodes
    public void paintString(Graphics2D g2d, int x, int y, String s, boolean right) {
        g2d.setColor(Color.black);
        if(right){
            g2d.drawString(s, x + (NODE_RADIUS / 4), y - (NODE_RADIUS / 4));
        } else {
            g2d.drawString(s, x - (NODE_RADIUS *3/ 4),  y - (NODE_RADIUS / 4));
        }
    }

    public void paintPath(Graphics2D g2d){
        g2d.setColor(Color.black);
        g2d.drawString(path, TEXT_SIZE, TEXT_SIZE);
    }

    // Takes a color and the centre coordinates of the crossing.
    public void paintCrossing(Graphics2D g2d, int x, int y, char id) {
        if (last_pos == id) {
            g2d.setColor(Color.green);
        }
        else if (next_pos == id) {
            g2d.setColor(Color.orange);
        }
        else {
            g2d.setColor(Color.white);
        }
        g2d.fillRect(x - (CROSSING_SIZE / 2), y - (CROSSING_SIZE / 2), CROSSING_SIZE, CROSSING_SIZE);
    }
}
