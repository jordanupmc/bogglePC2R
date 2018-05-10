package boggle.client;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import boggle.ui.GameWindow;
import dico.Dictionnaire;
import javafx.application.Platform;

public class ClientTricheur extends Thread implements ClientEcrivain{
	private BoggleClient client;
	private PrintStream ecrit;
	private List<String> toSend;
	private Dictionnaire dico;
	private char[][] grille;
	private String[][] position;
	private GameWindow gw;
	private Set<String> wordSent;
	private static List<String> phrasesDebutTour = initialisePhrasesDebutTour();
	private static List<String> phrasesFinTour = initialisePhrasesFinTour();
	private Boolean chatActif;


	public ClientTricheur (BoggleClient c, PrintStream ecriture, String dicoFile, Boolean chatActif) {
		this.chatActif = chatActif;
		client = c;
		ecrit = ecriture;
		toSend = new ArrayList<>();
		wordSent = new HashSet<>();
		this.dico = new Dictionnaire(dicoFile);
		grille = new char[4][];
		for(int i=0; i<4; i++) {
			grille[i]=new char[4];
		}
		position = new String[4][];
		for(int i=0; i<4; i++) {
			position[i]=new String[4];
		}

		for(int i=0; i<4; i++) {
			String tmp;
			if(i==0) {
				tmp ="A";
			}else if(i == 1) {
				tmp ="B";
			}else if(i == 2) {
				tmp ="C";
			}else {
				tmp ="D";
			}
			for(int j=0; j<4; j++) {
				position[i][j] = tmp+(j+1);
			}
		}
	}

	public void setGameWindow(GameWindow g) {
		gw = g;
	}



	private void findWordsUtil(boolean[][] visited, int i,int j, String str, String pos){
		visited[i][j] = true;
		str = str + grille[i][j];
		pos = pos + position[i][j];

		if (str.length() >= 3 && dico.motExiste(str) && !wordSent.contains(str)) {
			addToSendTricheur(client.commandeTrouveMot(str, pos));
			wordSent.add(str);
		}

		for (int row=i-1; row<=i+1 && row<4; row++)
			for (int col=j-1; col<=j+1 && col<4; col++)
				if (row>=0 && col>=0 && !visited[row][col])
					findWordsUtil(visited, row, col, str, pos);

		if(str.length()-2 <0) {
			str = "";
			pos = "";
		}else {
			str.substring(0, str.length()-1);
			pos.substring(0, str.length()-2);
		}
		visited[i][j] = false;
	}

	void findWords(){
		boolean[][] visited;
		visited = new boolean[4][];
		for(int i=0; i<4; i++) {
			visited[i]=new boolean[4];
		}

		for(int i=0; i<4; i++) {
			for(int j=0; j<4; j++) {
				visited[i][j] = false;
			}
		}

		String str = "";
		String pos = "";

		for (int i=0; i<4; i++)
			for (int j=0; j<4; j++)
				findWordsUtil(visited, i, j, str, pos);
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
			System.out.println("Le client a arreter avant d'entrer son pseudo, fin ClientTricheur");
			return;
		}

		while(true) {

			// On attend que le client soit en jeu, il a alors une grille jouable
			while(! client.isInGame()) {
				synchronized(client) {
					try {
						client.wait();
						// Permet de quitter le jeu avant de recevoir le bienvenue
						if(toSend.size() > 0) {	
							String send = toSend.remove(0);
							if(send.equals("SORT/"+client.getUserName()+"/")){
								ecrit.print(send+"\n");
								ecrit.flush();
								System.out.println("ClientJoueur envoi au serveur: "+send);
								System.out.println("Fin ClientTricheur");
								return;
							}
						}
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			toSend = new ArrayList<>();
			for(int i=0; i<4; i++) {
				for(int j=0; j<4; j++) {
					grille[i][j] = client.getTirageCourrant().charAt(i*4+j);
				}
			}
			wordSent = new HashSet<>();
			// Le tricheur a 40% de chance d'envoyer un message aleatoire au debut d'un tour, parmi une liste de phrases predefinies
			if(chatActif) {
				if(Math.random()<0.4) {
					addToSendTricheur(client.commandeEnvoiMessage(phrasesDebutTour.get(new Random().nextInt((phrasesDebutTour.size()-1)))));
				}
			}

			findWords();
			// Le tricheur a 40% de chance d'envoyer un message aleatoire a la fin d'un tour, parmi une liste de phrases predefinies
			if(chatActif) {
				if(Math.random()<0.4) {
					addToSendTricheur(client.commandeEnvoiMessage(phrasesFinTour.get(new Random().nextInt(phrasesFinTour.size()-1))));
				}
			}

			while(toSend.size()>0 || client.isInGame()){
				if(toSend.size()>0) {
					String send = toSend.remove(0);
					String tmp[] = send.split("/");		
					if(tmp[0].equals("TROUVE")) {
						Platform.runLater(() ->gw.setNewMotEnConst(tmp[1], tmp[2]));
					}
					ecrit.print(send+"\n");
					ecrit.flush();
					System.out.println("ClientJoueur envoi au serveur: "+send);
					if(send.equals("SORT/"+client.getUserName()+"/")){
						System.out.println("Fin ClientTricheur");
						return;
					}
					try {
						// Pour rendre le tricheur un peu plus humain, il envoi des requetes toutes les 0.5s a 2s.
						Thread.sleep(new Random().nextInt(1500) + 500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}else {
					// On attend une nouvelle requete a envoyee
					try {
						synchronized(client) {
							client.wait();
						}
					}catch(InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
			
			
			

		}
	}


	@Override
	public void addToSend(String toAdd) {
		toSend.add(0, toAdd);
	}
	
	private void addToSendTricheur(String toAdd) {
		toSend.add(toAdd);
	}
	
	private static List<String> initialisePhrasesFinTour() {
		ArrayList<String> res = new ArrayList<>();
		res.add("Cette grille etait vachement facile!");
		res.add("Bien joue a tous !");
		res.add("J'espere avoir un bon score ");
		res.add("Je pensais que ca allait etre plus difficile");
		res.add("C'est passe tellement vite, ce jeu est trop cool !");
		res.add("raaaaaaaaah, avec un peu plus de temps!");
		res.add("Ca s'est bien passe pour vous?");
		res.add("trop durrrrrrrrrr");
		res.add("Nonnnnnn, c'est trop court le temps de reflexion!");
		res.add("Avec un peu de chance, je serais dans	 le milieu du tableau des scores");
		res.add("Si je suis pas le premier apres cette grille, je vous paierai un kebab!");
		res.add("Elle etait pas evidente cette grille");
		res.add("Tellement facile que je vais me permettre d'aller boire de l'eau avant la prochaine");
		res.add("Good game a tous!");
		res.add("Venez pas vous plaindre si apres cette grille je passe loin devant vous au score !");
		res.add("J'espere que vous etes bon, j'ai au moins trouve 20 mots");
		res.add("Apres les mots que je viens de trouver, si vous esperez toujours me battre vous pouvez courrir");
		res.add("Y'a pas de grille plus difficiles parce que la...");
		res.add("Donnez moi un handicap pour la prochaine car la je risque de vous larguez au loin avec mon score");
		res.add("C'est juste une victoire que je viens de vous proposer");

		return res;
	}

	private static List<String> initialisePhrasesDebutTour() {
		ArrayList<String> res = new ArrayList<>();
		res.add("Cette grille a l'air d'etre simple");
		res.add("Ouh je la sens bien cette grille !");
		res.add("Aller bonne chance a vous");
		res.add("Ouh je la sens bien cette grille !");
		res.add("Ah je me vois deja a la premiere place");
		res.add("C'est parti!");
		res.add("Aller ca vient de commencer que j'ai deja trouve 3 mots");
		res.add("Malheuresemnt pour vous je vais devoir jouer cette grille!");
		res.add("Bonne chance a tous");
		res.add("Je viens a peine de regarder la grille mais je me vois deja sur le podium");
		res.add("Je sens que je vais battre mon record de mots sur cette grille");

		return res;
	}

}
