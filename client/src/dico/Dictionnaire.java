package dico;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;


public class  Dictionnaire{
	
	private Map<String, Boolean> dico;

	public Dictionnaire(String file) {
		dico = new HashMap<>();
		
		try{
			File f = new File(file);
			InputStream ips=new FileInputStream(f);
			InputStreamReader ipsr=new InputStreamReader(ips);
			BufferedReader br=new BufferedReader(ipsr);
			String ligne;

			//parcourt un fichier ligne par ligne
			while ((ligne=br.readLine())!=null){				
				dico.put(ligne, Boolean.TRUE);
			}
			br.close();

		}catch (Exception e){
			System.out.println(e.toString());
		}
//		System.out.println(dico.recherchePAT("TT"));
	}
	
	public boolean motExiste(String mot) {
		return dico.containsKey(mot);
	}

}
