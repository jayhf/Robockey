import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.function.Function;

import javax.swing.JOptionPane;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.PortInUseException;

public class RobotCommunicationThread extends Thread {
	private CommPort port;
	private OutputStream out;
	private Function<Byte,Robot> robotFunction;
	private ConcurrentLinkedDeque<byte[]> sendBuffer = new ConcurrentLinkedDeque<>();
	public RobotCommunicationThread(Function<Byte,Robot> robotFunction) throws PortInUseException, IOException {
		this.robotFunction = robotFunction;
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
			System.out.println("Connected to "+this.port);
		}
		else if(serialPorts.size() > 1){
			Object selection = JOptionPane.showInputDialog(null, "Select a COM port", "COM Port Selection", JOptionPane.QUESTION_MESSAGE, null, serialPorts.toArray(), serialPorts.get(0));
			port = ((CommPortIdentifier) selection).open("Robockey", 2000);
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
							//Logger.getLogger().write(data);
							ByteBuffer buffer = ByteBuffer.wrap(data);
							byte id = buffer.get();
							Robot robot = robotFunction.apply(id);
							if(robot!=null)
								robot.receivedDebugMessage(buffer);
						}
					}
					else if(!sendBuffer.isEmpty()){
						byte[] buffer = sendBuffer.pop();
						out.write(buffer);
						System.out.print("Sent "+Integer.toHexString((buffer[2]&0xFF)).toUpperCase()+": ");
						for(int i=3;i<13;i++)
							System.out.print(Integer.toHexString((buffer[i]&0xFF)).toUpperCase()+" ");
						System.out.println();
					}
					else
						Thread.yield();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	public void sendMessage(byte[] buffer) {
		// TODO Auto-generated method stub
		
	}
	public void sendGameCommand(GameCommand command) {
		byte data[] = new byte[13];
		Arrays.fill(data, command.getCommand());
		data[0] = (byte) 0xFF;
		data[1] = (byte) 0x00;
		data[2] = (byte) 0xFF;
		sendBuffer.push(data);
	}
}
