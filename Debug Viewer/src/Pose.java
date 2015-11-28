import javafx.geometry.Point2D;

public class Pose {
	public double x=0, y=0, o=0;

	public Pose(double x, double y, double o) {
		super();
		this.x = x;
		this.y = y;
		this.o = o;
	}

	@Override
	public String toString() {
		return "Pose [x=" + x + ", y=" + y + ", o=" + o + "]";
	}
	
	public Point2D toPoint2D(){
		return new Point2D(x, y);
	}
	
}
