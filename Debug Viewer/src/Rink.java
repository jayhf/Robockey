import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Arc2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Path2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import javax.swing.JComponent;

@SuppressWarnings("serial")
public class Rink extends JComponent{
	private List<Robot> robots=new ArrayList<Robot>();
	private Path2D perimeter;
	private Path2D robotCollisionPolygon;
	private Path2D puckCollisionPolygon;
	private int redScore = 0;
	private int blueScore = 0;
	private Puck puck = new Puck(new Pose(0,0,0));
	public Rink(Robot... robots){
		super.setPreferredSize(new Dimension(1280, 768));
		super.setMinimumSize(new Dimension(1280, 768));
		this.robots.addAll(Arrays.asList(robots));
		//robots.add(new Robot(new Pose(0,10,Math.PI*3/4)));
		perimeter = new Path2D.Double();
		perimeter.moveTo(-115, -60+29.21);
		perimeter.append(new Arc2D.Double(-115, 60-29.21*2, 29.21*2, 29.21*2, 180, 90, Arc2D.OPEN), true);
		perimeter.append(new Arc2D.Double(115-2*29.21, 60-29.21*2, 29.21*2, 29.21*2, 270, 90, Arc2D.OPEN), true);
		perimeter.append(new Arc2D.Double(115-2*29.21, -60, 29.21*2, 29.21*2, 0, 90, Arc2D.OPEN), true);
		perimeter.append(new Arc2D.Double(-115, -60, 29.21*2, 29.21*2, 90, 90, Arc2D.OPEN), true);
		perimeter.closePath();
		robotCollisionPolygon = new Path2D.Double();
		robotCollisionPolygon.moveTo(-107.5, -52.5+29.21);
		robotCollisionPolygon.append(new Arc2D.Double(-107.5, 52.5-29.21*2, 29.21*2, 29.21*2, 180, 90, Arc2D.OPEN), true);
		robotCollisionPolygon.append(new Arc2D.Double(107.5-2*29.21, 52.5-29.21*2, 29.21*2, 29.21*2, 270, 90, Arc2D.OPEN), true);
		robotCollisionPolygon.append(new Arc2D.Double(107.5-2*29.21, -52.5, 29.21*2, 29.21*2, 0, 90, Arc2D.OPEN), true);
		robotCollisionPolygon.append(new Arc2D.Double(-107.5, -52.5, 29.21*2, 29.21*2, 90, 90, Arc2D.OPEN), true);
		robotCollisionPolygon.closePath();
		puckCollisionPolygon = new Path2D.Double();
		puckCollisionPolygon.moveTo(-107.5, -52.5+29.21);
		puckCollisionPolygon.append(new Arc2D.Double(-109.34, 56.19-29.21*2, 29.21*2, 29.21*2, 180, 90, Arc2D.OPEN), true);
		puckCollisionPolygon.append(new Arc2D.Double(109.34-2*29.21, 56.19-29.21*2, 29.21*2, 29.21*2, 270, 90, Arc2D.OPEN), true);
		puckCollisionPolygon.append(new Arc2D.Double(109.34-2*29.21, -56.19, 29.21*2, 29.21*2, 0, 90, Arc2D.OPEN), true);
		puckCollisionPolygon.append(new Arc2D.Double(-109.34, -56.19, 29.21*2, 29.21*2, 90, 90, Arc2D.OPEN), true);
		puckCollisionPolygon.closePath();
	}
	
	@Override
	public void paint(Graphics g){
		Graphics2D g2d = (Graphics2D) g;
		g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		Rectangle bounds = g2d.getClipBounds();
		//Make drawing units centimeters
		double scaleFactor = Math.min(bounds.getWidth()/280, bounds.getHeight()/170);
		g2d.scale(scaleFactor,scaleFactor);
		g2d.translate(140, 85);
		g2d.scale(1, -1);

		
		g2d.setColor(Color.BLUE);
		g2d.fill(new Rectangle2D.Double(-125,-30,10,60));
		g2d.setColor(Color.RED);
		g2d.fill(new Rectangle2D.Double(115,-30,10,60));

		g2d.setColor(Color.BLACK);
		g2d.setStroke(new BasicStroke(.5f));
		g2d.draw(perimeter);

		g2d.scale(1, -1);
		g2d.drawString(""+blueScore, -90, -70);
		g2d.drawString(""+redScore, 100, -70);
		g2d.scale(1, -1);
		
		for(Robot robot:robots)
			robot.paint(g2d);
		
		//puck.paint(g2d);
	}
	
	public void update(double dt){
		for(Robot robot:robots)
			robot.updateCommands();
		for(int i=0;i<100;i++){
			double oldPuckX = puck.getPose().x;
			double oldPuckY = puck.getPose().y;
			for(Robot robot:robots){
				Pose pose = robot.getPose();
				for(Robot robot2:robots){
					if(robot != robot2){
						Pose pose2 = robot2.getPose();
						double distance = Math.hypot(pose.x-pose2.x,pose.y-pose2.y);
						if(distance<15){
							double theta = Math.atan2(pose.y-pose2.y, pose.x-pose2.x);
							pose.x += Math.cos(theta)*(15-distance)/2;
							pose.y += Math.sin(theta)*(15-distance)/2;
							pose2.x -= Math.cos(theta)*(15-distance)/2;
							pose2.y -= Math.sin(theta)*(15-distance)/2;
						}
					}
				}
				Pose puckPose = puck.getPose();
				double distance = Math.hypot(pose.x-puckPose.x,pose.y-puckPose.y);
				if(distance<7.5+3.81){
					double theta = Math.atan2(pose.y-puckPose.y, pose.x-puckPose.x);
					puckPose.x -= Math.cos(theta)*(7.5+3.81-distance);
					puckPose.y -= Math.sin(theta)*(7.5+3.81-distance);
				}
				double oldX = pose.x;
				double oldY = pose.y;
				robot.move(dt/100);
				if(!robotCollisionPolygon.contains(pose.x, pose.y)){
					if(!robotCollisionPolygon.contains(pose.x,oldY)){
						pose.x = oldX;
					}
					if(!robotCollisionPolygon.contains(oldX,pose.y)){
						pose.y = oldY;
					}
				}
				if(!puckCollisionPolygon.contains(puckPose.x, puckPose.y)
						&&(puck.getPose().y>30||puck.getPose().y<-30)){
					if(!puckCollisionPolygon.contains(puckPose.x,oldY)){
						puckPose.x = oldPuckX;
					}
					if(!puckCollisionPolygon.contains(oldX,puckPose.y)){
						puckPose.y = oldPuckY;
					}
				}
			}
			Collections.shuffle(robots);
		}
		if(puck.getPose().x>116)
			blueScore++;
		
		if(puck.getPose().x<-116)
			redScore++;
		
		if(Math.abs(puck.getPose().x)>116){
			reset();
		}
	}
	public void reset(){
		for(Robot robot:robots){
			if(robot.getTeam()==Team.BLUE)
				robot.setPose(new Pose(-Math.random()*60-47.5,Math.random()*105-105/2.0,0));
			
			else
				robot.setPose(new Pose(Math.random()*60+47.5,Math.random()*105-105/2.0,Math.PI));

		}
		puck.setPose(new Pose(0,0,0));
	}

	public void addRobot(Robot robot) {
		robots.add(robot);
	}
}
