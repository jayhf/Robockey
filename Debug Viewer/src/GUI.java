import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.Arrays;

import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;

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
	public void addConnection(RobotCommunicationThread robotConnection) {
		JMenuBar menuBar = new JMenuBar();
		JMenu gameCommands = new JMenu("Game Commands");
		for(GameCommand command:GameCommand.values()){
			JMenuItem menuItem = new JMenuItem(command.toString());
			menuItem.addActionListener((ActionEvent e)->robotConnection.sendGameCommand(command));
			menuItem.setMnemonic(command.getMnemonic());
			gameCommands.add(menuItem);
		}
		gameCommands.setMnemonic(KeyEvent.VK_G);
		menuBar.add(gameCommands);
		JMenu viewerOptions = new JMenu("Viewer Options");
		JMenuItem clearPaths = new JMenuItem("Clear Paths");
		clearPaths.addActionListener((ActionEvent e)->rink.clean());
		viewerOptions.add(clearPaths);
		menuBar.add(viewerOptions);
		frame.add(menuBar,BorderLayout.NORTH);
		frame.pack();
	}
	public void addMouseListener(MouseListener l) {
		rink.addMouseListener(l);
	}
	public void addMouseMotionListener(MouseMotionListener l) {
		rink.addMouseMotionListener(l);
	}
}
