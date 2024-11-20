package UI;

public class WifiThread extends Thread {

    DataComponent dataComponent;
    Client client;
    MapComponent mapComponent;

    WifiThread(DataComponent dataComponent, MapComponent mapComponent, Client client){
        this.dataComponent = dataComponent;
        this.mapComponent = mapComponent;
        this.client = client;
    }

    public void run() {
        while (true) {
			int identifier = client.receiveByte();
            //System.out.println(start_identifier);
			if (identifier == 200) {
				var msg = client.receiveMessage(13);
				//This would be so much cleaner if java had an unsigned byte type...
				dataComponent.set_sensor_data(
					msg[0] & 0xFF | ((msg[1] & 0xFF) << 8),
					msg[2] & 0xFF | ((msg[3] & 0xFF) << 8),
					msg[4] & 0xFF | ((msg[5] & 0xFF) << 8),
					msg[6] & 0xFF | ((msg[7] & 0xFF) << 8)
				);
				dataComponent.set_steering_data(
                    msg[8] & 0xFF | ((msg[9] & 0xFF) << 8),
                    msg[10] & 0xFF | ((msg[11] & 0xFF) << 8),
					0 != msg[12]
				);
			}
            if(identifier == 201){
                var msg_length = client.receiveMessage(1);
                var msg = client.receiveMessage(msg_length[0]);

                char[] path = new char[msg_length[0]];
                for (int i = 0; i < msg_length[0]; i++) {
                    path[i] = (char)msg[i];
                }
                mapComponent.setPath(path);
            }
            if(identifier == 202){
                var msg = client.receiveMessage(1);
                dataComponent.set_manual(msg[0]!=0);
            }

        }
    }
}

