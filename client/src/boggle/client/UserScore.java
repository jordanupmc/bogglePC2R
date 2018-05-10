package boggle.client;

import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;

public class UserScore {
	 private SimpleStringProperty fieldUser;
     private SimpleIntegerProperty fieldScore;
    
     public UserScore(String fUser, int fScore){
         this.fieldUser = new SimpleStringProperty(fUser);
         this.fieldScore = new SimpleIntegerProperty(fScore);
     }
    
     public String getFieldUser() {
         return fieldUser.get();
     }
    
     public double getFieldScore() {
         return fieldScore.get();
     }
    
}
