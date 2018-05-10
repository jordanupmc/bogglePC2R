package dico;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.HashSet;
import java.util.Set;


public class BuildDico {

	public static void main(String[] args) {
		Set<String> set = new HashSet<>();
		String line;
		try {
			BufferedReader br = new BufferedReader(new FileReader("glaff-1.2.2.txt"));
			BufferedWriter bw = new BufferedWriter(new FileWriter("Dictionnaire.txt"));

			while ((line = br.readLine()) != null) {
				String tmp[] = line.split("\\|");
				String stringNormal = new String(tmp[0].getBytes(), "utf8");
				String noAccent = Normalizer.normalize(stringNormal, Form.NFD).replaceAll("[\u0300-\u036F]", "");
				String upper = noAccent.toUpperCase();
				if((upper.length() > 3) && (upper.length() <= 16) &&  (! noAccent.contains("?")) && (! set.contains(upper))){
					set.add(upper);
					bw.write(upper);
					bw.newLine();
					bw.flush();
				}
			}
			br.close();
			br = new BufferedReader(new FileReader("oldiesSubLexicon.txt"));			
			while ((line = br.readLine()) != null) {
				String tmp[] = line.split("\\|");
				String stringNormal = new String(tmp[0].getBytes(), "utf8");
				String noAccent = Normalizer.normalize(stringNormal, Form.NFD).replaceAll("[\u0300-\u036F]", "");
				String upper = noAccent.toUpperCase();
				if((upper.length() > 3) && (upper.length() <= 16) &&  (! noAccent.contains("?")) && (! set.contains(upper))){
					set.add(upper);
					bw.write(upper);
					bw.newLine();
					bw.flush();
				}
			}
			br.close();
			bw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}

}
