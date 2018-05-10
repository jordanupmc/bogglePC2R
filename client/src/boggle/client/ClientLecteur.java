package boggle.client;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.SocketException;

import boggle.ui.GameWindow;
import javafx.application.Platform;

public class ClientLecteur extends Thread{
	private BoggleClient client;
	private DataInputStream lect;
	private GameWindow gw;

	public ClientLecteur (BoggleClient c, DataInputStream lecture, GameWindow gw) {
		client = c;
		lect = lecture;
		this.gw =gw;
	}

	@SuppressWarnings("deprecation")
	public void run() {
		String cmd;
		while(true){
			try {
				cmd = lect.readLine();
				if(cmd == null) {
					System.out.println("Fin du ClientLecteur");
					break;
				}
				//				cmd = "";
				//				char c;
				//				while((c = (char) System.in.read()) != '\n') {cmd+= c;}
				String tmp[] = cmd.split("/");
				switch(tmp[0]) {

				//				BIENVENUE/LIDAREJULTNEATNG/1*usr1*1*usr2*2/
				//				BIENVENUE/ABCDEFGHIJKLMNOP/1*usr1*1*usr2*2/
				case "BIENVENUE":
					if(tmp.length == 3) {
						if(tmp[1].length() != 16) {
							System.out.println("Format incorrecte, tirageFormat: string length 16");
							break;
						}
						if( !client.setScoreList(tmp[2])) {
							System.out.println("Format incorrecte, scoreFormat: numTour*usr1*score1*usr2*score2*... ");
							break;
						}
						client.setTirageCourrant(tmp[1]);
						client.setInGame(true);
						Platform.runLater(() ->gw.setInfo("Bienvenue!"));
						Platform.runLater(() ->gw.setScore());
						Platform.runLater(()-> gw.changeString(tmp[1]));

					}else {
						System.out.println("Format incorrecte, usage: BIENVENUE/tirage/scores/");
					}
					break;

					//				CONNECTE/JEAN/
				case "CONNECTE":
					if(tmp.length == 2) {
						System.out.println(tmp[1]+" vient d'arriver!");
						Platform.runLater(() ->gw.setInfo(tmp[1]+" vient d'arriver!"));
					}else {
						System.out.println("Format incorrecte, usage: CONNECTE/user/");
					}
					break;

					//				DECONNEXION/JEAN/
				case "DECONNEXION":
					if(tmp.length == 2) {
						System.out.println(tmp[1]+" vient de partir!");
						Platform.runLater(() ->gw.setInfo(tmp[1]+" vient de partir!"));

					}else {
						System.out.println("Format incorrecte, usage: DECONNEXION/user/");
					}
					break;

					//				SESSION/
				case "SESSION":
					if(tmp.length == 1) {
						System.out.println("Debut d'une nouvelle session!\n");
						Platform.runLater(() ->gw.setInfo("Debut d'une nouvelle session!"));
					}else {
						System.out.println("Format incorrecte, usage: SESSION/");
					}
					break;

					//				VAINQUEUR/25*usr1*1*usr2*10/
				case "VAINQUEUR":
					if(tmp.length == 2) {
						if( !client.setScoreList(tmp[1])) {
							System.out.println("Format incorrecte, bilanFormat: numTour*usr1*score1*usr2*score2*... ");
							break;
						}
						System.out.println("Fin de la session!\n");
						Platform.runLater(() ->gw.setInfo("Fin de la session!"));
						Platform.runLater(() ->gw.setScoreSession());
					}else {
						System.out.println("Format incorrecte, usage: VAINQUEUR/bilan/");
					}
					break;

					//				TOUR/LIDAREJULTNEATNG/					
				case "TOUR":
					if(tmp.length == 2) {
						if(tmp[1].length() != 16) {
							System.out.println("Format incorrecte, tirageFormat: string length 16");
							break;
						}
						System.out.println("Debut d'un nouveau tour!\n");
						client.setTirageCourrant(tmp[1]);
						client.setInGame(true);
						Platform.runLater(()-> gw.changeString(tmp[1]));
						Platform.runLater(() ->gw.setInfo("Debut d'un nouveau tour!"));

					}else {
						System.out.println("Format incorrecte, usage: TOUR/tirage/");
					}
					break;

					//				MVALIDE/TRIDENT/	
				case "MVALIDE":
					if(tmp.length == 2) {
						System.out.println("Le mot "+tmp[1]+" a ete valide par le serveur!");
						Platform.runLater(() ->gw.setInfo("Le mot "+tmp[1]+" a ete valide par le serveur!"));
					}else {
						System.out.println("Format incorrecte, usage: MVALIDE/mot/");
					}
					break;		

					//				MINVALIDE/AZER/					
				case "MINVALIDE":
					if(tmp.length == 2) {
						System.out.println("Invalidation d'un mot : "+tmp[1]+"!");
						Platform.runLater(() ->gw.setInfo("Invalidation: "+tmp[1]));
					}else {
						System.out.println("Format incorrecte, usage: MINVALIDE/raison/");
					}
					break;

					//				RFIN/					
				case "RFIN":
					if(tmp.length == 1) {
						System.out.println("Expiration du delai imparti a la reflexion!\n");
						client.setInGame(false);
						Platform.runLater(() ->gw.setInfo("Expiration du delai imparti a la reflexion!"));
						Platform.runLater(()-> gw.setPlayable(false));
						synchronized(client) {
							client.notifyAll();
						}
					}else {
						System.out.println("Format incorrecte, usage: RFIN/");
					}
					break;

					//						BILANMOTS/BOIRE VOIR MANGER/1*usr1*25*usr2*30/					
				case "BILANMOTS":
					if(tmp.length == 3) {
						if( !client.setScoreList(tmp[2])) {
							System.out.println("Format incorrecte, scoreFormat: numTour*usr1*score1*usr2*score2*... ");
							break;
						}
						System.out.println("Ensemble des mots proposes et valides: "+tmp[1]+", scores de tous les joueurs : "+tmp[2]+"!\n");
						Platform.runLater(() ->gw.setInfo("Mots valides: "+tmp[1]));
						Platform.runLater(() ->gw.setScore());

					}else {
						System.out.println("Format incorrecte, usage: BILANMOTS/MOTS VALIDES/SCORES/");
					}
					break;
					
					//				RECEPTION/Salut a tous les amis/				
				case "RECEPTION":
					if(tmp.length == 2) {
						System.out.println("Reception d'un message publique : "+tmp[1]);
						Platform.runLater(() ->gw.receivePublicMessage(tmp[1]));

					}else {
						System.out.println("Format incorrecte, usage: RECEPTION/message/");
					}
					break;

					//					PRECEPTION/Vient on triche/Jean/					
				case "PRECEPTION":
					if(tmp.length == 3) {
						System.out.println("Reception d'un message prive de "+tmp[2]+" : "+tmp[1]);
						Platform.runLater(() ->gw.receivePrivateMessage(tmp[2], tmp[1]));
					}else {
						System.out.println("Format incorrecte, usage: PRECEPTION/Message/User/");
					}
					break;

				default:
					System.out.println("Commande invalide "+cmd);

				}		
			}catch(SocketException e) {
				System.out.println("Fin client Lecteur");
				break;
			}catch(IOException e) {
				e.printStackTrace();
			}

		}

	}
}
