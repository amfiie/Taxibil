package UI;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import org.freedesktop.gstreamer.Gst;
import org.freedesktop.gstreamer.Pipeline;
import org.freedesktop.gstreamer.State;
import org.freedesktop.gstreamer.elements.AppSink;
import org.freedesktop.gstreamer.swing.GstVideoComponent;

public class InterfaceWindow
{
    public final static int LENGTH_UNIT = 100;

    private Client client;
    private MapComponent mapComponent;
    private DataComponent dataComponent;
    private Pipeline pipeline;
    private GstVideoComponent vc;

    public InterfaceWindow() {
        client = new Client(12345, 54321);
        mapComponent = new MapComponent();
        dataComponent = new DataComponent();

        Gst.init();

        // Create a named pipeline
        pipeline = (Pipeline) Gst.parseLaunch(" udpsrc ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,width=620,height=390 ! appsink");
        if (pipeline == null){
            System.out.println("Pipeline null");
        }

        // pipeline with videotestsrc for TESTING
        AppSink sink = (AppSink) pipeline.getElementByName("appsink0");

        // Create component from appsink
        if (sink != null) {
            vc = new GstVideoComponent(sink);
        }
        // Check pipeline state
        pipeline.play();
        if (pipeline.getState().equals(State.PLAYING)){
            System.out.println("Pipeline is playing");
        }



    }

    private void addKeys(JFrame frame) {
        final int DRIVE_BUTTON = 0, LEFT_BUTTON = 1, RIGHT_BUTTON = 2, BRAKE_BUTTON = 3, REALEASE = 4;
        JComponent paneMove = frame.getRootPane();
        final InputMap inMove = paneMove.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
        inMove.put(KeyStroke.getKeyStroke("UP"), "drive_press");
        inMove.put(KeyStroke.getKeyStroke("DOWN"), "brake_press");
        inMove.put(KeyStroke.getKeyStroke("LEFT"), "left_press");
        inMove.put(KeyStroke.getKeyStroke("RIGHT"), "right_press");
        inMove.put(KeyStroke.getKeyStroke("released UP"), "drive_release");
        inMove.put(KeyStroke.getKeyStroke("released DOWN"), "brake_release");
        inMove.put(KeyStroke.getKeyStroke("released LEFT"), "left_release");
        inMove.put(KeyStroke.getKeyStroke("released RIGHT"), "right_release");

        final ActionMap actMove = paneMove.getActionMap();
        actMove.put("drive_press", new ButtonAction(DRIVE_BUTTON));
        actMove.put("left_press", new ButtonAction(LEFT_BUTTON));
        actMove.put("right_press", new ButtonAction(RIGHT_BUTTON));
        actMove.put("brake_press", new ButtonAction(BRAKE_BUTTON));
        actMove.put("drive_release", new ButtonAction(DRIVE_BUTTON + REALEASE));
        actMove.put("left_release", new ButtonAction(LEFT_BUTTON + REALEASE));
        actMove.put("right_release", new ButtonAction(RIGHT_BUTTON + REALEASE));
        actMove.put("brake_release", new ButtonAction(BRAKE_BUTTON + REALEASE));
    }

    private class ButtonAction extends AbstractAction {
        private final int button_action;
        private ButtonAction(int button_action) {
            this.button_action = button_action;
        }
        @Override public void actionPerformed(final ActionEvent e) {
            client.sendMessage(new byte[]{(byte)101, (byte) button_action});
        }
    }

    public void showWindow() {
        JFrame frame = new JFrame("interface");
        addKeys(frame);
        frame.setLayout(new BorderLayout());

        // receive data from socket
        WifiThread wifiThread = new WifiThread(dataComponent, mapComponent, client);
        new Thread(wifiThread).start();

        // if more than 5 components!
        // JPanel panel = new JPanel();
        // panel.setLayout(new BorderLayout());

        // IP button
        JPanel IP_panel = new JPanel();
        JButton ip_button = new JButton("Update ip-address");
        ip_button.addActionListener(e -> {
            InputWindow inputWindow = new InputWindow("insert ip address:");
            String ip = inputWindow.showWindow();
            if (ip != null) {
                client.setHostname(ip);
                client.openSockets();
            }
        });

        // Manual button
        JButton manual_button = new JButton("Switch steering");
        manual_button.addActionListener(e -> {
            client.sendMessage(new byte[]{102});
        });

        // Jobs button
        JPanel button_panel = new JPanel();
        JButton job_button = new JButton("Update path");
        job_button.addActionListener(e -> {
            InputWindow inputWindow = new InputWindow("insert start and stop node:");
            String job = inputWindow.showWindow();
            if ((job != null) && (job.length() != 0)) {
                    byte[] jobs = new byte[2];
                    for (int i = 0; i < 2; i++) {
                        switch (job.charAt(i)) {
                            case 'A':
                                jobs[i] = 0;
                                break;
                            case 'B':
                                jobs[i] = 1;
                                break;
                            case 'C':
                                jobs[i] = 2;
                                break;
                            case 'D':
                                jobs[i] = 3;
                                break;
                            case 'E':
                                jobs[i] = 4;
                                break;
                            case 'F':
                                jobs[i] = 5;
                                break;

                            default:
                                break;
                        }
                    }
                    client.sendMessage(new byte[] { (byte) 103, jobs[0], jobs[1] });
                }
        });

        // container panel for multiple buttons
        button_panel.add(job_button);
        IP_panel.add(ip_button);
        JPanel container_panel = new JPanel(new GridLayout(1,2));
        container_panel.add(button_panel);
        container_panel.add(IP_panel);

        //IP button component
        frame.getContentPane().add(container_panel, BorderLayout.SOUTH);
        //Manual button component
        frame.getContentPane().add(manual_button, BorderLayout.NORTH);
        // mapComponent
        frame.getContentPane().add(mapComponent, BorderLayout.WEST);
        // dataComponent
        frame.getContentPane().add(dataComponent, BorderLayout.CENTER);

        // gstreamer
        if (vc != null) {
            frame.getContentPane().add(vc, BorderLayout.EAST);
        }
        // Close program on window close
        class CloseListener extends WindowAdapter {
            @Override public void windowClosing(WindowEvent we) {
                frame.setVisible(false);
                frame.dispose();
                client.closeSockets();
                System.exit(0);
            }
        }
        frame.addWindowListener(new CloseListener());

        // end
        frame.pack();
        frame.setVisible(true);
    }
    public void close() {
        client.closeSockets();
    }
}
