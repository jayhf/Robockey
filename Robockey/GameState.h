enum GameState{
	NONE, COMM_TEST, PLAY, PAUSE, HALFTIME, GAME_OVER
};

enum Robot{
	CONTROLLER = 0, ROBOT1 = 1, ROBOT2 = 2, ROBOT3 = 3
};

enum Team{
	RED, BLUE
};

uint8_t redScore;
uint8_t blueScore;
uint8_t getRScore();
uint8_t getBScore();
enum GameState state = NONE;
//Returns the ID of this robot in the range [1,3]
uint8_t getRobotID();

void initGameState();
void updateGameState();
void updateGameState(enum GameState state);

void goalScored(enum Team team);

void updateEnemyLocations(uint8_t *positions);

bool allowedToMove(){
	return state == PLAY;
}