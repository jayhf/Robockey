import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;

public class RobotDragger extends MouseAdapter{
	private Robot[] robots;
	private Rink rink;
	private volatile Robot selectedRobot = null;
	private double xOffset = 0;
	private double yOffset = 0;
	
	public RobotDragger(Rink rink, Robot...robots){
		this.robots = robots;
		this.rink = rink;
	}
	@Override
	public void mousePressed(MouseEvent e) {
		try {
			AffineTransform transform = rink.getTransform().createInverse();
			Point2D mouseCoords = transform.transform(new Point2D.Double(e.getX(),e.getY()), null);
			for(Robot robot: robots){
				Pose pose = robot.getPose();
				if(mouseCoords.distance(new Point2D.Double(pose.x, pose.y))<7.5){
					selectedRobot = robot;
					xOffset = pose.x - mouseCoords.getX();
					yOffset = pose.y - mouseCoords.getY();
					break;
				}
			}
		} catch (NoninvertibleTransformException e1) {
			System.out.println("Not invertible!");
		}
	}
	@Override
	public void mouseReleased(MouseEvent e) {
		selectedRobot = null;
	}
	@Override
	public void mouseDragged(MouseEvent e){
		try {
			AffineTransform transform = rink.getTransform().createInverse();
			Point2D mouseCoords = transform.transform(new Point2D.Double(e.getX(),e.getY()), null);
			if(selectedRobot!=null)
				selectedRobot.setPose(new Pose(mouseCoords.getX()+xOffset,mouseCoords.getY()+yOffset, 0));
		} catch (NoninvertibleTransformException e1) {
			System.out.println("Not invertible!");
		}
	}
}
