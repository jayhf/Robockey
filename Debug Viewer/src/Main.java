
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.Scanner;

import javax.swing.SwingUtilities;

import static java.awt.event.KeyEvent.*;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.geom.Path2D;
import java.io.File;
import java.io.FileNotFoundException;

public class Main {
	public static void main(String[] args) throws InvocationTargetException, InterruptedException, FileNotFoundException{
		//Robot robot = new Robot(new Pose(60,0,Math.PI),Team.RED);
		//Robot robot2 = new Robot(new Pose(-60,0,0),Team.BLUE);
		//System.out.println(robot.getPosition(new int[]{5}));
		Robot robots[] = new Robot[3];
		for(int i=0;i<3;i++)
			robots[i] = new Robot(new Pose(0,0,0), Team.BLUE);
		Rink rink = new Rink(robots[1]);//,robot2);
		//rink.addRobot(new Robot(new Pose(76,12,Math.PI/2),Team.RED));
		GUI gui = new GUI(rink);
		SwingUtilities.invokeAndWait(()->gui.init());
		short location[] = new short[3];
		//robot.javaLocalize(new short[]{49,330,10,473,368,8,567,367,10,513,402,5},location);
		short data[] = reformatIRData(new short[]{575,498,547,596,478,434,427,438});
		//robot.localize(data);
		System.out.println(Arrays.toString(location));
		//robot.setPose(new Pose(location[0]/10.0*115/1024,location[1]/10.0*115/1024,location[2]/100.0));
		//System.out.println(robot.getPose().o);
		File file = new File("../../mWii Data.csv");
		Scanner s = new Scanner(file);
		s.nextLine();
		while(s.hasNextLine()){
			String[] line = s.nextLine().replace(",", " , ").split(",");
			for(int r = 0;r<3;r++){
				if(line[8*r].trim().isEmpty())
					continue;
				short[] lineData = new short[8];
				for(int i=0;i<8;i++)
					lineData[i] = Short.parseShort(line[8*r+i].trim());
				data = reformatIRData(lineData);
				robots[r].localize(data);
			}
			gui.refresh();
			//Thread.sleep(10);
		}
		gui.refresh();
		s.close();
		/*KeyboardController controller = new KeyboardController(VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT);
		robot.addController(controller);

		KeyboardController controller2 = new KeyboardController(VK_W,VK_S,VK_A,VK_D);
		robot2.addController(controller2);
		
		gui.addKeyListener(controller);
		gui.addKeyListener(controller2);
		gui.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e){
				if(e.getKeyCode()==KeyEvent.VK_MINUS)
					rink.reset();
			}
		});
		
		long time = 0;
		while(true){
			time+= System.nanoTime();
			rink.update(.02);
			SwingUtilities.invokeLater(()->gui.refresh());
			time+=20000000;
			time-=System.nanoTime();
			long sleep = time/10000000;
			time-=sleep*1000000;
			Thread.sleep(sleep);
		}*/
		//System.out.printf("(%d,%d)", location[0],location[1]);
	}

	public static short[] reformatIRData(short[] s) {
		short[] result = new short[12];
		for(int i=0;i<4;i++){
			result[3*i] = s[i];
			result[3*i+1] = s[4+i];
		}
		return result;
	}

}