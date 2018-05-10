package boggle.client;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import boggle.ui.GameWindow;

public class ClientJoueur extends Thread implements ClientEcrivain{
	private BoggleClient client;
	private PrintStream ecrit;
	private List<String> toSend;
	
	public ClientJoueur (BoggleClient c, PrintStream ecriture) {
		client = c;
		ecrit = ecriture;
		toSend = new ArrayList<>();
	}
	
	public synchronized void addToSend(String toAdd) {
		toSend.add(toAdd);
	}
	
	public void setGameWindow(GameWindow g) {
		// Inutile
	}

	public void run() {
		while((! client.isConnected())  ) {	
			synchronized(this) {
				try {
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
		if(toSend.size()>0) {
			String send = toSend.remove(0);
			ecrit.print(send+"\n");
			ecrit.flush();
			System.out.println("Envoi au serveur : "+send);
		}else {
			System.out.println("Le client a arreter avant d'entrer son pseudo, fin ClientJoueur");
			return;
		}
		
		while(true) {
			while(toSend.size()==0) {
				synchronized(this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}	
			String send = toSend.remove(0);
			ecrit.print(send+"\n");
			ecrit.flush();
			System.out.println("ClientJoueur envoi au serveur: "+send);
			if(send.equals("SORT/"+client.getUserName()+"/")){
				break;
			}
				
		}
		System.out.println("Fin ClientJoueur");

	}
}
