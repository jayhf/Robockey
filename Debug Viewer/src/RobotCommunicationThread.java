import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

import javax.swing.JOptionPane;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.PortInUseException;

public class RobotCommunicationThread extends Thread {
	private CommPort port;
	OutputStream out;
	public RobotCommunicationThread() throws PortInUseException, IOException {
		super.setDaemon(true);
		@SuppressWarnings("unchecked")
		Enumeration<CommPortIdentifier> ports = CommPortIdentifier.getPortIdentifiers();
		List<CommPortIdentifier> serialPorts = new ArrayList<>();
		while(ports.hasMoreElements()){
			CommPortIdentifier port = ports.nextElement();
			if(port.getPortType() == CommPortIdentifier.PORT_SERIAL && !port.isCurrentlyOwned()){
				serialPorts.add(port);
			}
		}
		if(serialPorts.size() == 1){
			this.port = serialPorts.get(0).open("Robockey", 2000);
		}
		else if(serialPorts.size() > 1){
			Object selection = JOptionPane.showInputDialog(null, "Select a COM port", "COM Port Selection", JOptionPane.QUESTION_MESSAGE, null, serialPorts.toArray(), serialPorts.get(0));
			port = (CommPort) selection;
		}
		else
			throw new IllegalArgumentException("No comm port found!");
		out = port.getOutputStream();
	}
	@Override
	public void finalize(){
		port.close();
	}
	@Override
	public void run(){
		while(true){
			try {
				InputStream in = port.getInputStream();
			
				while(true){
					if(super.isInterrupted()){
						out.close();
						in.close();
						port.close();
						return;
					}
					if(in.available()>12){
						if(in.read() == 0xFF && in.read() == 0x00){
							byte data[] = new byte[10];
							in.read(data);
							switch(data[0]){
								
							}
						}
					}
					else
						Thread.yield();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}