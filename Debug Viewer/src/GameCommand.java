
public enum GameCommand {
	COMM_TEST(0xA0, "Comm Test"), PLAY(0xA1, "Play"), PAUSE(0xA4, "Pause"), HALFTIME(0xA6, "Half Time"), GAME_OVER(0xA7,"Game Over");
	private byte value;
	private String name;
	private GameCommand(int value, String name){
		this.value = (byte) value;
		this.name = name;
	}
	public byte getCommand(){
		return value;
	}
	@Override
	public String toString(){
		return name;
	}
}
