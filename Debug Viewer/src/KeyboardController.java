import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class KeyboardController implements KeyListener, RobotController{
	private Set<Integer> pressedKeys = new HashSet<>();
	private int[] keys;
	public KeyboardController(int... keys){
		this.keys = keys;
	}
	
	@Override
	public void keyPressed(KeyEvent e) {
		pressedKeys.add(e.getKeyCode());
	}
	@Override
	public void keyReleased(KeyEvent e) {
		pressedKeys.remove(e.getKeyCode());
	}

	@Override
	public void keyTyped(KeyEvent e) {}

	@Override
	public ControlParameters getControlParameters() {
		double forward=0;
		double ccw=0;
		if(pressedKeys.contains(keys[0]))
			forward++;
		if(pressedKeys.contains(keys[1]))
			forward--;
		if(pressedKeys.contains(keys[3]))
			ccw--;
		if(pressedKeys.contains(keys[2]))
			ccw++;
		double left=0;
		double right=0;
		if(forward == 0){
			left = ccw;
			right = -ccw;
		}
		else if(ccw == 0){
			left = right = forward;
		}
		else{
			left = (forward + ccw)/2;
			right = (forward - ccw)/2;
		}
		return new ControlParameters(left, right);
	}

	
}
