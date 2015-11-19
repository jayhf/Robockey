import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.sql.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Arrays;

public class Logger {
	private static Logger logger = null;
	public static Logger getLogger(){
		if(logger==null)
			logger = new Logger();
		return logger;
	}
	private File logFile;
	private PrintStream log;
	public Logger(){
		Date date = new Date(System.currentTimeMillis());
		DateFormat format = new SimpleDateFormat("mm-dd-yy HH.mm.ss");
		logFile = new File("../../Logs/log "+format.format(date)+".csv");
		try {
			logFile.createNewFile();
			log = new PrintStream(logFile);
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}
	public void write(byte[] data){
		log.println(Arrays.toString(data).replace("[","").replace("]","").replace(" ",""));
	}
	@Override
	public void finalize(){
		log.close();
	}
}
