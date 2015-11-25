import java.awt.event.KeyEvent;

public enum GameCommand {
	COMM_TEST(0xA0, "Comm Test",KeyEvent.VK_C), PLAY(0xA1, "Play",KeyEvent.VK_L), PAUSE(0xA4, "Pause",KeyEvent.VK_A),
	HALFTIME(0xA6, "Half Time",KeyEvent.VK_H), GAME_OVER(0xA7,"Game Over",KeyEvent.VK_G);
	private byte value;
	private String name;
	private int mnemonic;
	private GameCommand(int value, String name, int mnemonic){
		this.value = (byte) value;
		this.name = name;
		this.mnemonic = mnemonic;
	}
	public byte getCommand(){
		return value;
	}
	@Override
	public String toString(){
		return name;
	}
	public int getMnemonic() {
		return mnemonic;
	}
}
