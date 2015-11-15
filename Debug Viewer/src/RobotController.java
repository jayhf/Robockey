import java.util.ResourceBundle.Control;

public interface RobotController {
	
	public ControlParameters getControlParameters();
	
	class ControlParameters{
		public double leftSpeed, rightSpeed;

		public ControlParameters(double leftSpeed, double rightSpeed) {
			super();
			this.leftSpeed = leftSpeed;
			this.rightSpeed = rightSpeed;
		}
	}
}
