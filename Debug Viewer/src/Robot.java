import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Path2D;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Robot {
	private static boolean nativesLoaded = false;
	static{
		try{
			System.loadLibrary("simulator");
			nativesLoaded = true;
		} catch (UnsatisfiedLinkError e){
			System.out.println("dll not found");
		}
	}
	private Pose pose;
	private RobotController controller = null;
	private Team team;
	private volatile Path2D path = new Path2D.Double();
	private List<Line2D> directions = new ArrayList<>();
	private int[] lastIRData = new int[16];
	private volatile Color ledColor = new Color(0, true);
	private Pose puckEstimate = null;
	private double puckAngle = 0, puckDistance = 0;
	private boolean robotActive = false;
	public Robot(Pose pose, Team team) {
		this.pose = pose;
		this.team = team;
		if(nativesLoaded){
			Thread robotThread = new Thread(()->run());
			//robotThread.start();
		}
	}
	private Color color = Color.getHSBColor((float) Math.random(), 1, 1);
	public void paint(Graphics2D g){
		if(!robotActive)
			return;
		g.setColor(team==Team.RED?Color.RED:Color.BLUE);
		g.fill(new Ellipse2D.Double(pose.x-7.5, pose.y-7.5, 15, 15));
		g.setColor(Color.BLACK);
		g.draw(new Line2D.Double(pose.x, pose.y, pose.x+7.5*Math.cos(pose.o), pose.y+7.5*Math.sin(pose.o)));
		synchronized (directions) {
			for(Line2D line:directions)
				g.draw(line);
		}
		g.setColor(color);
		synchronized(path){
			g.draw(path);
		}
		for(int i=0;i<16;i++){
			g.setColor(new Color((Color.ORANGE.getRGB()&0xFFFFFF)|(((lastIRData[i]>>2)&0xFF)<<24),true));
			double x = pose.x + 7.5*Math.cos(pose.o+Math.PI*(i-7.5)/8);
			double y = pose.y + 7.5*Math.sin(pose.o+Math.PI*(i-7.5)/8);
			g.fill(new Ellipse2D.Double(x-1.5, y-1.5, 3, 3));
		}
		g.setColor(ledColor);
		g.fill(new Ellipse2D.Double(pose.x-2, pose.y-2, 4, 4));
		if(puckEstimate!=null){
			g.setColor(Color.GRAY);
			g.fill(new Ellipse2D.Double(puckEstimate.x-3.81, puckEstimate.y-3.81, 7.62, 7.62));
			g.setColor(Color.DARK_GRAY);
			g.draw(new Line2D.Double(pose.x, pose.y, pose.x+100*Math.cos(puckAngle+pose.o), pose.y+100*Math.sin(puckAngle+pose.o)));
			g.draw(new Ellipse2D.Double(pose.x-puckDistance, pose.y-puckDistance, puckDistance*2, puckDistance*2));
		}
	}
	
	private RobotController.ControlParameters controls = new RobotController.ControlParameters(0,0);
	//public native void mWiiUpdate(short[] s, short[] location);
	public native void run();
	public static native void mWiiUpdate(short[] irData, short[] location);
	public void localize(short[] irData){
		short[] location = new short[3];
		short[] location2 = new short[3];
		javamWiiUpdate(irData, location2);
		mWiiUpdate(irData, location);
		boolean wrong = (Math.abs(location[0] - location2[0])>100)||(Math.abs(location[1] - location2[1])>100)||(Math.abs(location[2] - location2[2])>100);
		if(wrong){
			System.out.println(Arrays.toString(irData));
			System.out.println(Arrays.toString(location));
			System.out.println(Arrays.toString(location2));
		}
		if(location[0]!=1023 || location[1]!=1023)
			moveTo(new Pose(location[0],location[1],location[2]*Math.PI/32768));
	}
	public static final short toBAMS(float angle){
		return (short)(angle*32768/Math.PI);
	}
	public static final float toFloatAngle(short angle){
		return (float) ((Math.PI/32768)*angle);
	}
	public static void javamWiiUpdate(short[] irData, short[] location){
		float possiblePointsX[] = new float[12];
		float possiblePointsY[] = new float[12];
		short possiblePointsO[] = new short[12];
		int possiblePointCount = 0;
		short irX[] = {(short)(irData[0]),(short)(irData[3]),(short)(irData[6]),(short)(irData[9])};
		short irY[] = {(short)(irData[1]),(short)(irData[4]),(short)(irData[7]),(short)(irData[10])};
		if(irY[1] == 1023){
			System.arraycopy(new short[]{1023,1023,0}, 0, location, 0, 3);
			return;
		}
		byte validPoints = 0;
		byte errorPoints = 0;
		//byte pointCount = (byte) (2 + ((irY[2] != 1023)?1:0)+((irY[3] != 1023)?1:0));
		for(byte i = 0; i<4;i++){
			if(irY[i]==1023)
				continue;
			for(byte j = (byte) (i+1); j<4;j++){
				if(irY[j]==1023)
					continue;
				short dx = (short) (irX[i]-irX[j]);
				short dy = (short) (irY[i]-irY[j]);
				short d = (short) (dx*dx + dy*dy);
				//System.out.println(d);
				byte id;
				if(d>4500){
					if (d > 7500) {
						if (d > 9000)
							continue;
						else
							id = 5;
					} else {
						if (d > 5850)
							id = 4;
						else
							id = 3;
					}
				} else if (d > 2000) {
					if (d > 3100)
						id = 2;
					else
						id = 1;
				} else if (d > 1100)
					id = 0;
				else
					continue;
				/*if(d>5500){
					if(d>8950){
						if(d>11000) continue;
						else id = 5;
					}
					else{
						if(d>7100) id = 4;
						else id = 3;
					}
				}
				else if(d>2550){
					if(d>3800) id = 2;
					else id = 1;
				}
				else if(d>1500) id = 0;
				else continue;*/
				if((validPoints & (1<<id))==0)
					validPoints |= 1<<id;
				else{
					errorPoints |= 1<<id;
					continue;
				}
				short mx = (short) (irX[i]+irX[j]-1024);
				short my = (short) (irY[i]+irY[j]-768);
				short px = dy;
				short py = (short) -dx;
				float cx;
				float cy;
				short co;
				switch(id){
					case 0:
						cx = 2.014857231f;
						cy = 0.011916738f;
						co = 27951;
						break;
					case 1:
						cx = 1.189835575f;
						cy = 0.358868959f;
						co = -23947;
						break;
					case 2:
						cx = 0.761287104f;
						cy = -0.230281866f;
						co = 22225;
						break;
					case 3:
						cx = 0.45522679f;
						cy = 0.177359962f;
						co = -29904;
						break;
					case 4:
						cx = 0.503755633f;
						cy = -0.002927468f;
						co = 11567;
						break;
					case 5:
						cx = 0;
						cy = 0;
						co = -16384;
						break;
					default:
						continue;
				}
				float ox = mx + cy*dx + cx*px;
				float oy = my + cy*dy + cx*py;
				float ox2 = mx - cy*dx - cx*px;
				float oy2 = my - cy*dy - cx*py;
				short o = (short) (toBAMS((float) (Math.atan2(dy,dx)))+co);

				possiblePointsX[possiblePointCount] = ox;
				possiblePointsY[possiblePointCount] = oy;
				possiblePointsO[possiblePointCount] = (short) (o+32768);
				possiblePointsX[possiblePointCount+1] = ox2;
				possiblePointsY[possiblePointCount+1] = oy2;
				possiblePointsO[possiblePointCount+1] = o;
				possiblePointCount+=2;
			}
		}
		/*validPoints &= ~errorPoints;
		short x = 0;
		short y = 0;
		short o = 0;
		for(int i=0;i<6;i++){
			if((validPoints&1<<i)!=0){

			}
		}*/
		//System.out.println("\n");
		//System.out.println(Arrays.toString(possiblePointsX));
		//System.out.println(Arrays.toString(possiblePointsY));
		//System.out.println(Arrays.toString(possiblePointsO));
		float ox = 0;
		float oy = 0;
		short oo = 0;
		if(possiblePointCount == 0){
			System.arraycopy(new short[]{1023,1023,0}, 0, location, 0, 3);
			return;
		}
		else if(possiblePointCount == 2){
			float d1 = possiblePointsX[0]*possiblePointsX[0]+possiblePointsY[0]*possiblePointsY[0];
			float d2 = possiblePointsX[1]*possiblePointsX[1]+possiblePointsY[1]*possiblePointsY[1];
			if(d1<d2){
				ox = possiblePointsX[1];
				oy = possiblePointsY[1];
				oo = possiblePointsO[1];
			}
			else{
				ox = possiblePointsX[0];
				oy = possiblePointsY[0];
				oo = possiblePointsO[0];
			}
		}
		else{
			boolean stop = false;
			int scores[] = new int[12];
			for(int i=0;i<possiblePointCount && !stop;i++){
				for(int j=i+1;j<possiblePointCount;j++){
					float dx = possiblePointsX[i]-possiblePointsX[j];
					float dy = possiblePointsY[i]-possiblePointsY[j];
					short dTheta = (short) (possiblePointsO[i]-possiblePointsO[j]);
					if((dTheta>-16384&&dTheta<16384)&&dx*dx+dy*dy<400)
						scores[j]++;
				}
			}
			int maxScore = 0;
			int maxScoreIndex = 0;
			for(int i=0;i<possiblePointCount;i++)
				if(scores[i]>maxScore){
					maxScore = scores[i];
					maxScoreIndex = i;
				}
			//System.out.println(Arrays.toString(scores));

			//System.out.printf("%d: (%f,%f,%d)\n",maxScoreIndex, possiblePointsX[maxScoreIndex],possiblePointsY[maxScoreIndex],possiblePointsO[maxScoreIndex]);

			int originCount = 0;
			for(int i=0;i<possiblePointCount;i++){
				float dx = possiblePointsX[i]-possiblePointsX[maxScoreIndex];
				float dy = possiblePointsY[i]-possiblePointsY[maxScoreIndex];
				short dTheta = (short) (possiblePointsO[i]-possiblePointsO[maxScoreIndex]);
				if((dTheta>-16384&&dTheta<16384)&&dx*dx+dy*dy<400){
					originCount++;
					ox += possiblePointsX[i];
					oy += possiblePointsY[i];
					if(i!=maxScoreIndex)
						oo += dTheta;
					//System.out.println("O"+possiblePointsO[i]);
				}
			}
			ox/=originCount;
			oy/=originCount;
			oo=(short) ((oo/originCount)+possiblePointsO[maxScoreIndex]);

		}
		//System.out.printf("(%f,%f,%d)\n",ox,oy,oo);
		float coso = (float) Math.cos(toFloatAngle(oo));
		float sino = (float) Math.sin(toFloatAngle(oo));
		float rx = (-ox*coso - oy *sino)*(115f/768);
		float ry = (ox*sino - oy *coso)*(115f/768);
		//System.out.printf("(%f,%f,%d)\n",rx,ry,oo);
		System.arraycopy(new short[]{(short)rx,(short)ry,(short) -oo}, 0, location, 0, 3);

	}
	public void moveTo(Pose pose){
		synchronized(path){
			this.pose = pose;
			if(path.getCurrentPoint()==null)
				path.moveTo(pose.x, pose.y);
			else
				path.lineTo(pose.x, pose.y);
			}
		synchronized (directions) {
			directions.add(new Line2D.Double(pose.x, pose.y, pose.x+7.5*Math.cos(pose.o), pose.y+7.5*Math.sin(pose.o)));
		}
	}
	public void move(double dt) {
		if(controller != null){
			pose.x += 75*dt*Math.cos(pose.o)*(controls.leftSpeed+controls.rightSpeed)/2;
			pose.y += 75*dt*Math.sin(pose.o)*(controls.leftSpeed+controls.rightSpeed)/2;
			pose.o += 6*dt*(controls.leftSpeed-controls.rightSpeed);
		}
	}
	
	public void addController(RobotController controller) {
		this.controller = controller;
	}
	
	public Pose getPose() {
		return pose;
	}
	
	public void setPose(Pose pose) {
		this.pose = pose;
	}
	
	public void updateCommands() {
		System.out.println("A"+controller);
		controls = controller.getControlParameters();
	}
	public void setLED(int value){
		if(Math.random()>0)
			return;
		switch(value){
			case 1:
				ledColor = new Color(255, 128, 128);
				break;
			case 2:
				ledColor = new Color(0, 255, 255);
				break;
			case 3:
				ledColor = new Color(255, 0, 255);
				break;
			case 0:
			default:
				ledColor = new Color(0,true);
				break;
		}
	}
	public Team getTeam() {
		return team;
	}
	double v = 0;
	public void receivedDebugMessage(ByteBuffer buffer) {
		robotActive = true;
		byte id = buffer.get();
		switch(id){
			case 0x10:
				moveTo(new Pose(buffer.getShort(),buffer.getShort(),buffer.getShort()*Math.PI/32768));
				//System.out.println(getPose());
				break;
			case 0x11:
				System.out.print("ADC "+id+": [");
				for(int i=0;i<8;i++){
					lastIRData[i]=((0xFF&buffer.get())<<2);
					//System.out.print(lastIRData[i]+",");
				}
				break;
			case 0x12:
				for(int i=0;i<8;i++){
					lastIRData[i+8]=((0xFF&buffer.get())<<2);
					//System.out.print(lastIRData[i+8]+",");
				}
				int max = 0, maxIndex = 0;
				for(int i=0;i<16;i++){
					if(max < lastIRData[i]){
						maxIndex = i;
						max = lastIRData[i];
					}
				}
				int max2 = Math.max(lastIRData[(maxIndex+15)%16], lastIRData[(maxIndex+1)%16]);
				System.out.print(v= v*.95+.05*(max+max2));
				System.out.print("]\n");
				break;
			case 0x13:
				System.out.println("Battery: "+buffer.getShort());
				break;
			case 0x14:
				puckAngle = buffer.getShort()*Math.PI/32768;
				puckEstimate = new Pose(buffer.get(),buffer.get(),0);
				System.out.println("Resistor: "+buffer.get());
				System.out.println(puckEstimate);
				puckDistance =  0xFF& buffer.get();
				System.out.println("Distance " + puckDistance);
				break;
			case 0x20:
				System.out.print("Angle: "+(buffer.getShort()*180/32768+'\t'));
				System.out.print(" Distance: "+buffer.getShort()+'\t');
				System.out.print("("+((buffer.get())<<3)+","+((buffer.get())<<3)+")");
				System.out.println(" ("+buffer.get()+","+buffer.get()+")");
				break;
			case 0x21:
				System.out.print("x: "+buffer.getShort()+ ", ");
				System.out.print("y: "+buffer.getShort() + ", ");
				System.out.print("d: "+buffer.getShort()+", ");
				System.out.print("w: "+buffer.getShort()+"\n");
				
				break;
		}
	}
	public void clean() {
		synchronized(path){
			Path2D newPath = new Path2D.Double();
			newPath.moveTo(pose.x, pose.y);
			path = newPath;
		}
		synchronized(directions){
			directions.clear();
		}
	}
	
}
