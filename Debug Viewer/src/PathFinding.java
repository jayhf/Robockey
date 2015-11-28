import java.awt.geom.Area;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Path2D;
import java.util.Arrays;

public class PathFinding {
	private static final int YMAX = 52, YMIN = -52;
	private static final int XMAX = 110, XMIN = -110;
	public static int getVertices(Pose[] result, Pose[] obstacles, Pose start, Pose end,
			Pose puck, boolean scoringPuck){
		Pose[] allies = Arrays.copyOfRange(obstacles, 0, 3);
		Pose[] enemies = Arrays.copyOfRange(obstacles, 3, 6);
		result[0] = start;
		result[1] = end;
		int resultCount = 2;
		for(int i=0;i<enemies.length;i++){
			Pose pose = enemies[i];
			if(pose.y+10<YMAX)
				resultCount = checkPoint(result, resultCount, obstacles, new Pose(pose.x, (((int)pose.y+18)>>1)+(YMAX>>1),0));
			if(pose.y-10>YMIN)
				resultCount = checkPoint(result, resultCount, obstacles, new Pose(pose.x, (((int)pose.y-18)>>1)+(YMIN>>1),0));
			if(pose.x+10<XMAX)
				resultCount = checkPoint(result, resultCount, obstacles, new Pose((((int)pose.x+18)>>1)+(XMAX>>1),pose.y,0));
			if(pose.x-10>XMIN)
				resultCount = checkPoint(result, resultCount, obstacles, new Pose((((int)pose.x-18)>>1)+(XMIN>>1),pose.y,0));
			
			for(int j=i+1; j<enemies.length;j++){
				Pose pose2 = enemies[j];
				if((pose.x-pose2.x)*(pose.x-pose2.x)+(pose.y-pose2.y)*(pose.y-pose2.y)>400)
					resultCount = checkPoint(result, resultCount, obstacles, new Pose(((int)pose.x>>1)+((int)pose2.x>>1),((int)pose.y>>1)+((int)pose2.y>>1),0));
			}
			
			for(int j=0; j<allies.length;j++){
				Pose pose2 = allies[j];
				if((pose.x-pose2.x)*(pose.x-pose2.x)+(pose.y-pose2.y)*(pose.y-pose2.y)>400)
					resultCount = checkPoint(result, resultCount, obstacles, new Pose(((int)pose.x>>1)+((int)pose2.x>>1),((int)pose.y>>1)+((int)pose2.y>>1),0));
			}
		}
		
		System.out.println(resultCount);
		return resultCount;
	}
	private static int checkPoint(Pose[] result, int resultCount, Pose[] obstacles, Pose newVertex){
		for(int i=0;i<obstacles.length;i++){
			Pose pose = obstacles[i];
			if((pose.x-newVertex.x)*(pose.x-newVertex.x)+(pose.y-newVertex.y)*(pose.y-newVertex.y)<100){
				return resultCount;
			}
		}
		for(int i=0;i<resultCount;i++){
			Pose pose = result[i];
			if((pose.x-newVertex.x)*(pose.x-newVertex.x)+(pose.y-newVertex.y)*(pose.y-newVertex.y)<400){
				return resultCount;
			}
		}
		result[resultCount] = newVertex;
		return resultCount + 1;
	}
	public static Pose nextPose(Pose[] vertices){
		Pose start = vertices[0];
		Pose end = vertices[1];
		return null;
	}
	public static boolean[][] getEdges(Pose[] vertices, int vertexCount, Pose[] obstacles){
		boolean[][] result = new boolean[vertices.length][vertices.length];
		for(int i=0;i<vertexCount;i++){
			int distances[] = new int[2];
			Arrays.fill(distances, Integer.MAX_VALUE);
			int distanceIndices[] = new int[distances.length];
			J:
			for(int j=i+1; j<vertexCount;j++){
				Pose v1 = vertices[i];
				Pose v2 = vertices[j];
				int distance = (int) ((v1.x-v2.x)*(v1.x-v2.x)+(v1.y-v2.y)*(v1.y-v2.y));
				for(Pose obstacle:obstacles){
					double o = Math.atan2((obstacle.y-v1.y), (obstacle.x-v1.x));
					double d = Math.sqrt((obstacle.y-v1.y)*(obstacle.y-v1.y)+(obstacle.x-v1.x)*(obstacle.x-v1.x));
					double x = d*Math.cos(o)+v1.x;
					double y = d*Math.sin(o)+v1.y;
					if(y<7.5&&y>-7.5 && x<d&&x>0){
						continue J;
					}
				}
				result[i][j] = true;
				int index = j;
				for(int k=0;k<distances.length;k++){
					if(distances[k]>distance){
						int nextDistance = distances[k];
						distances[k] = distance;
						distance = nextDistance;
						int nextIndex = distanceIndices[k];
						distanceIndices[k] = index;
						index = nextIndex;
					}
				}
			}
			for(int j=0;j<distances.length;j++){
				result[i][distanceIndices[j]] = true;
			}
		}
		return result;
	}
}
