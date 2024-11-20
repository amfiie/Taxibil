package UI;

import java.io.*;
import java.net.*;
import java.util.Arrays;

public class Client{

    private String host_name;
    private Socket socket_out;
    private Socket socket_in;
    private DataOutputStream dout;
    private DataInputStream din;
    private int port_out;
    private int port_in;
    private boolean comunicate;

    public Client(int port_out, int port_in) {
        this.port_in = port_in;
        this.port_out = port_out;
        this.comunicate = false;
    }

    public void openSockets(){
        try{
            socket_out = new Socket(host_name, port_out);
            socket_in = new Socket(host_name, port_in);
            dout = new DataOutputStream((socket_out.getOutputStream()));
            din = new DataInputStream(socket_in.getInputStream());
            comunicate = true;
        } catch(Exception e) {System.out.println(e);}
    }

    public void sendMessage(byte[] data){
        try{
            if (comunicate) {
                //dout.writeByte(start_identifier);
                System.out.println(Arrays.toString(data));
                dout.write(data, 0, data.length);
                dout.flush();
            }
        }catch(Exception e) {System.out.println(e);}
    }

	public int receiveByte() {
		try {
			return din.readUnsignedByte();
		} catch (Exception e) {

		}
		return 0;
	}

    public byte[] receiveMessage(final int length) {
        byte[] msg = new byte[length];

        try{
            while(din.available() <= 0){
            }
            if (din.available() > 0) {
                din.readFully(msg);
            }

        }catch(Exception e) {System.out.println(e);}
        return msg;
    }

    public void closeSockets(){
        try{
            comunicate = false;
            dout.close();
            din.close();
            socket_out.close();
            socket_in.close();
        }catch(Exception e) {System.out.println(e);}
    }

    public void setHostname(String ip) {
        this.host_name = ip;
    }
}
