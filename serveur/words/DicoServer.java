import java.io.*;
import java.net.*;
import java.util.concurrent.*;
import java.util.Vector;
import java.util.Random;
import java.util.*;
/*
import tries.patricia.interfaces.*;
import tries.patricia.*;
*/

public class  DicoServer{
    public static void main (String args[]) {
	int port          = 5500;
	int capacity      = Integer.parseInt(args[0]);
	echoServer server = new echoServer(port, capacity);
	server.run();
    }
}

class echoServer {
    Vector<echoClient> clients;
    Vector<Socket>     sockets;
    ServerSocket       serv;
    Socket             client;
    int                port, capacity, nbConnectedClients, nbWaitingSocks;
    Set<String>        dico;
    
    echoServer (int p, int c) {
	capacity = c;
	port     = p;
	clients  = new Vector<echoClient>(c);
	sockets  = new Vector<Socket>();
	

	initDico("glaff1.txt");

	for (int i = 0; i < c; i++) {
	    echoClient tmpEcho = new echoClient(this);
	    clients.add(tmpEcho);
	    tmpEcho.start();
	}
	nbConnectedClients = 0;
	nbWaitingSocks     = 0;
	System.out.println("Le serveur de mots est pret");
    }

    public void initDico(String filename){
	dico = new HashSet<String>();//PatriciaFactory.createNode();
	BufferedReader inputStream = null;

	try {
	    inputStream = new BufferedReader( new FileReader(filename));
	    String line;
	    
	    while ((line = inputStream.readLine()) != null)
		dico.add(line); 
	    inputStream.close();	
			
	}catch(Exception e){
	    System.out.println(e);
	}

	
    }
    
    public Socket removeFirstSocket () {
	Socket ret = sockets.get(0);
	sockets.removeElementAt(0);
	return ret;
    }
	
    public void newConnect () {
	nbConnectedClients++;
	nbWaitingSocks--;
	System.out.println(" Thread handled connection.");
	System.out.println("   * " + nbConnectedClients + " connected.");
	System.out.println("   * " + nbWaitingSocks + " waiting.");
    }

    public void clientLeft () {
	nbConnectedClients--;
	System.out.println(" Client left.");
	System.out.println("   * " + nbConnectedClients + " connected.");
	System.out.println("   * " + nbWaitingSocks + " waiting.");
    }

    public Set<String> getDico(){
	return dico;
    }
	
    public int stillWaiting () { return nbWaitingSocks; }
	
    public void run () {
	try {
	    serv = new ServerSocket(port); 
	    
	    while (true) {
		client = serv.accept();
		System.out.println("New connexion at server.");  
		synchronized (this) {
		    sockets.add(client);
		    nbWaitingSocks++;
		    this.notify();
		}
	    }
	} catch (Throwable t) { t.printStackTrace(System.err); }
    }
}

class echoClient extends Thread {
    BufferedReader   inchan;
    DataOutputStream outchan;

    BufferedReader   inchanStat;
    DataOutputStream outchanStat;
    
    echoServer       server;
    Socket           socket;
    Socket           stat;
    int              idC;
    
    echoClient (echoServer s) {
	server = s;
    }
	
    public void run () {
	Socket s;
	boolean cont;
	String param[];
	Set<String> dico = server.getDico();
	while (true) {
	    synchronized (server) {
		if (server.stillWaiting() == 0)
		    try {
			server.wait();
		    } catch (InterruptedException e) { e.printStackTrace(); }
		s = server.removeFirstSocket();
		server.newConnect();
	    }
	    try {
		inchan  = new BufferedReader(
					     new InputStreamReader(s.getInputStream()));
		outchan = new DataOutputStream(s.getOutputStream());
		
		socket  = s;
		
		cont = true;
		while (cont) {
		    
		    String cmd = inchan.readLine();
		    byte[] res;
		    
		    if(cmd == null)
			continue;
		    param= cmd.split(" ");
		    System.out.println(param[0]+" "+param[1]);
		    
		    if(param[0].equals("CHECK") ){
			if( dico.contains(param[1]) ){
			    res="OK\n".getBytes("UTF-8");
			    outchan.write(res,0, res.length);
			}
			else{
			    res="KO\n".getBytes("UTF-8");
			    outchan.write(res,0, res.length);
			}
			outchan.flush(); System.out.println("bytes = "+res.length);
		    }
		    /*
		      switch (param[0]){
		      case "START":
		      System.out.println("Un client va commencer une interaction");
		      int tmp;
			
		      synchronized(server){
		      tmp = server.getCurrentId();
		      server.setCurrentId();
		      }
		      outchan.writeChars( tmp +"\n" );
			
		      break;
			
		      case "PUT":
		      server.getMap().put(param[1], param[2]);
		      System.out.println("ID ="+param[3]);
		      outchanStat.writeChars( "PUT "+ param[3] );
		      outchanStat.flush();
		      break;
		      case "GET":
		      if(server.getMap().containsKey(param[1])){
		      outchan.writeChars( server.getMap().get(param[1])+"\n" );
		      outchanStat.writeChars( "GET "+param[2]  );
		      outchanStat.flush();
		      }
		      else
		      outchan.writeChars( "Key not found\n" );
		      outchan.flush();
		      break;

		      case "QUIT":
		      System.out.println("Un client a terminé son interaction");
		      cont =false;
		      break;
		      default:
		      System.out.println("????");
		      break;
		      }
		    */
	 
		}
		socket.close();
		synchronized (server) {
		    server.clientLeft();
		}
	    } catch (IOException e) { e.printStackTrace(); System.exit(1); }
	}
    }	  
}