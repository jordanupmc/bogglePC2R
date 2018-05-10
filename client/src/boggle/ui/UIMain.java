package boggle.ui;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.net.Socket;

import boggle.client.BoggleClient;
import boggle.client.ClientEcrivain;
import boggle.client.ClientJoueur;
import boggle.client.ClientLecteur;
import boggle.client.ClientTricheur;
import javafx.application.Application;
import javafx.stage.Stage;

public class UIMain extends Application {
	
	private static PrintStream canalEcriture;
	private static BoggleClient c;
	private static GameWindow g;
	private static DataInputStream canalLecture;
	private static ClientEcrivain cEnvoyeur;
	private static ClientLecteur cl;
	private static boolean chat;
	private static boolean tricheur;
	private static String journalUrl = "";
	


	@Override
	public void start(Stage stage) throws Exception {	
		
		if(tricheur) {
			cEnvoyeur = new ClientTricheur(c, canalEcriture, "Dictionnaire.txt", chat);
		}else {
			cEnvoyeur = new ClientJoueur(c, canalEcriture);
		}
		
		if(cEnvoyeur instanceof Thread) {
			((Thread) cEnvoyeur).start();
		}else {
			System.out.println("Pas possible : cEnvoyeur not a Thread");
		}	
		g = new GameWindow(stage, c, cEnvoyeur, chat, journalUrl);
		cEnvoyeur.setGameWindow(g);
		cl = new ClientLecteur(c, canalLecture, g);
		cl.start();
		
	}
	
	

	public static void main(String[] args) { 
		String adresse = "localhost";
		int port = 2018;
		int i = 0;
		while(i<args.length) {
			switch(args[i]) {
				case "-serveur":
					i++;
					adresse = args[i];
					i++;
					break;
				
				case "-port":
					i++;
					port = Integer.parseInt(args[i]);
					i++;
					break;
				
				case "-chat":
					chat = true;
					i++;
					break;
					
				case "-cheat":
					tricheur = true;
					i++;
					break;
					
				case "-journal":
					i++;
					journalUrl = args[i];
					i++;
					break;
					
				default:
					System.err.println("Usage : java boggleClient <-serveur hostname(default: localhost)> <-port numport(default: 2018)> <-chat(default: no)> <-cheat(default: no)> <-journal urlJournal)>");
					System.exit(1);
			}
		}
		
		try {
			Socket sock = new Socket(adresse, port);
			canalEcriture = new PrintStream(sock.getOutputStream());
			canalLecture = new DataInputStream(sock.getInputStream());
			c = new BoggleClient();
			launch(args); 
			if(cEnvoyeur instanceof Thread) {
				((Thread) cEnvoyeur).join();
			}else {
				System.out.println("Not possible : cEnvoyeur not a Thread");
			}
			cl.interrupt();
			sock.close();
		} catch (IOException e) {
			e.printStackTrace();
		}catch(InterruptedException e) {
			System.out.println("Client lecteur est fini "+cl.isAlive());
			e.printStackTrace();
		}
	} 
}
