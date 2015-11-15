import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.KeyListener;

import javax.swing.JFrame;

public class GUI {
	private Rink rink;
	private JFrame frame;
	
	public GUI(Rink rink){
		this.rink = rink;
	}
	public void addKeyListener(KeyListener k){
		frame.addKeyListener(k);
	}
	public void init(){
		frame = new JFrame();
		frame.setTitle("Team Overkill Robockey Viewer");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.add(rink,BorderLayout.CENTER);
		frame.pack();
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		Rectangle bounds = frame.getBounds();
		frame.setLocation((screenSize.width-bounds.width)/2, (screenSize.height-bounds.height)/2);
		frame.setVisible(true);
	}

	public void refresh() {
		rink.repaint();
	}
}
