import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Ellipse2D;

public class Puck {
	private Pose pose;
	public Puck(Pose pose){
		this.pose = pose;
	}
	public Pose getPose(){
		return pose;
	}
	public void paint(Graphics2D g2d) {
		g2d.setColor(Color.DARK_GRAY);
		g2d.fill(new Ellipse2D.Double(pose.x-3.81, pose.y-3.81, 7.62, 7.62));
	}
	public void setPose(Pose pose) {
		this.pose = pose;
	}
}
