package boggle.client;


import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;

public class BoggleClient {

	private String userName;
	private String scoreCourrant;
	private String tirageCourrant;
	private boolean inGame;
	private boolean isConnected;  
	private ObservableList<UserScore> scoreList;
	private int numTour;
	
	
	public BoggleClient() {
		scoreCourrant ="";
		tirageCourrant ="";
		inGame = false;		
	}
	
	public synchronized int getNumTour() {
		return numTour;
	}
	
	public synchronized  boolean setScoreList(String score) {
		scoreCourrant = score;
		ObservableList<UserScore> dataList =
	                FXCollections.observableArrayList();
		String tmp[] = score.split("\\*");
		
		
		try {
			numTour = Integer.parseInt(tmp[0]);
		}catch(NumberFormatException nfe)  {  
		    return false;  
		 } 
		System.out.println("SCORES : Tour n:"+tmp[0]+"\n");
		int i=1;
		while(i<tmp.length) {
			try {
				dataList.add(new UserScore(tmp[i++], Integer.parseInt(tmp[i++])));
			}catch(NumberFormatException e) {
				System.out.println("Un score n'est pas un entier");
			}
		}
		scoreList = dataList;
		return true;
	}
	
	public synchronized ObservableList<UserScore> getScoreList() {
		return scoreList;
	}
	
	public synchronized String getTirageCourrant() {
		return tirageCourrant;
	}
	
	public String commandeConnexion(String name) {
		userName = name;
		isConnected = true;
		return "CONNEXION/"+userName+"/";
	}
	
	public String commandeDeconnexion() {
		isConnected = false;
		return "SORT/"+userName+"/";
	}
	
	public synchronized boolean isConnected() {
		return isConnected;
	}
	
	public String commandeTrouveMot(String mot, String trajectoire) {
		return "TROUVE/"+mot+"/"+trajectoire+"/";
	}
	
	public String commandeEnvoiMessage(String msg) {
		return "ENVOI/"+msg+"/";
	}
	
	public String commandeEnvoiMessagePrive(String to, String msg) {
		return "PENVOI/"+to+"/"+msg+"/";
	}

	
	public void afficheScore() {
		String tmp[] = scoreCourrant.split("\\*");
		System.out.println("SCORES : Tour n:"+tmp[0]+"\n");
		int i=1;
		while(i<tmp.length) {
			System.out.println(tmp[i++] +" : "+tmp[i++]);
		}
	}
	
	public synchronized boolean isInGame() {
		return inGame;
	}
	
	public synchronized void setTirageCourrant(String tirage) {
		tirageCourrant = tirage;
	}
	
	public synchronized void setInGame(boolean inGame) {
		this.inGame = inGame;
	}

	
	public String getUserName() {
		return userName;
	}

	public void setUserName(String userName) {
		this.userName = userName;
	}


	
}
