import org.junit.Test;

import static org.junit.Assert.*;

import java.util.Arrays;
public class RobotTest {
	@Test
	public void localization(){
		double actualX = 100;
		double actualY = 451;
		double actualO = Math.PI-1;
		short data[] = transformStars(-actualX, -actualY, actualO);
		short result[] = new short[3];
		Robot.javamWiiUpdate(data, result);
		System.out.printf("(%f,%f,%f)\n",result[0]/10.0,result[1]/10.0,result[2]/100.0);
		assertEquals(2*actualX,result[0]/10.0,5);
		assertEquals(2*actualY,result[1]/10.0,5);
		assertEquals(actualO,result[2]*Math.PI/32768,.1);
		short result2[] = new short[3];
		Robot.mWiiUpdate(data, result2);
		System.out.printf("(%f,%f,%f)",result2[0]/10.0,result2[1]/10.0,result2[2]/100.0);
		assertEquals(result[0],result2[0]);
		assertEquals(result[1],result2[1]);
		assertEquals(result[2],result2[2]);
		//assertEquals(0,result[2]/100.0,.1);
	}
	@Test
	public void javaVcppLocalization(){
		short data[] = {805, 508, 0, 856, 490, 0, 887, 524, 0, 814, 579, 0};
		//{634, 684, 0, 683, 709, 0, 1023, 1023, 0, 586, 735, 0};
		short java[] = new short[3];
		short cpp[] = new short[3];
		Robot.javamWiiUpdate(data, java);
		Robot.mWiiUpdate(data, cpp);
		System.out.println(Arrays.toString(java));
		System.out.println(Arrays.toString(cpp));
		assertArrayEquals(java, cpp);
	}
	private static short[] transformStars(double x, double y, double o){
		double data[][] = {{0,14.5},{0,-14.5},{-11.655,8.741},{10.563,2.483}};
		for(int i=0;i<data.length;i++){
			data[i][0]=x+data[i][0]*100/28.3;
			data[i][1]=y+data[i][1]*100/28.3;
		}
		short[] result = new short[12];
		for(int i=0;i<4;i++){
			result[3*i] = i>=data.length?1023:(short) (data[i][0]*Math.cos(o) - data[i][1]*Math.sin(o)+512);
			result[3*i+1] = i>=data.length?1023:(short) (data[i][0]*Math.sin(o) + data[i][1]*Math.cos(o)+384);
		}
		System.out.println(Arrays.toString(result));
		return result;
	}
}