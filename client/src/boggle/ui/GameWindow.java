package boggle.ui;

import java.awt.Desktop;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import boggle.client.BoggleClient;
import boggle.client.ClientEcrivain;
import boggle.client.UserScore;
import javafx.animation.KeyFrame;
import javafx.animation.KeyValue;
import javafx.animation.Timeline;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.geometry.Rectangle2D;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.input.MouseDragEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.FontPosture;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;
import javafx.scene.text.TextFlow;
import javafx.stage.Screen;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;
import javafx.util.Duration;

public class GameWindow{

	private List<Label> cases = new ArrayList<>();
	private StringBuilder motEnConstruction = new StringBuilder();
	private StringBuilder trajectoire = new StringBuilder();
	private final double FONT_SIZE =70;
	private final double GRID_GAP = 50;
	private Stage stage;
	private Scene scene;
	private Text info;
	private BoggleClient c;
	private Label scoreTitle;
	private TableView<UserScore> scoreTable;
	private ObservableList<UserScore> dataList;
	private ClientEcrivain ecriv;
	private VBox chatBox;
	private List<TextFlow> messages;
	private Label afficheMotEnConst;
	private boolean chatActive;
	private boolean journalActive;
	private String urlJournal;


	/* Chaque label ecoute l'evenement Released -> fin de la trajectoire*/
	private EventHandler<MouseEvent> handlerMouseReleased = new EventHandler<MouseEvent>() {
		public void handle(MouseEvent e) {
			System.out.println("DROP = "+motEnConstruction+", TRAJECTOIRE = "+trajectoire);
			synchronized(ecriv) {
				ecriv.addToSend(c.commandeTrouveMot(motEnConstruction.toString(), trajectoire.toString()));
				ecriv.notifyAll();
			}
			motEnConstruction = new StringBuilder();
			trajectoire = new StringBuilder();

			/*On remet la couleur noir */
			for(Label c: cases )
				c.setTextFill(Color.web("#000000"));
			e.consume();
		}
	};

	/* Chaque label ecoute l'evenement DragEntered -> on est sur une case qu'on veut ajouter on change sa couleur*/
	private EventHandler<MouseDragEvent> handlerDragEntered = new EventHandler<MouseDragEvent>() {
		public void handle(MouseDragEvent e) {
			Label l = (Label)e.getSource();
			l.setTextFill(Color.web("#0076a3"));
			motEnConstruction.append(l.getText());
			trajectoire.append(l.getId());
			afficheMotEnConst.setText(afficheMotEnConst.getText()+l.getText());
			e.consume();
		}
	};

	/*Debut de l'ecoute des evenements de Drag & Drop sur chaque label*/
	private EventHandler<MouseEvent> handlerDragDetected = new EventHandler<MouseEvent>() {
		public void handle(MouseEvent arg0) {
			afficheMotEnConst.setText("");
			((Label)arg0.getSource()).startFullDrag();
		}
	};


	public GameWindow(Stage s, BoggleClient client, ClientEcrivain ecrivain, boolean chatActive, String urlJournal) {
		this.stage = s;
		c = client;
		ecriv = ecrivain;
		this.chatActive = chatActive;
		journalActive = true;
		if(urlJournal.length() > 0) {
			journalActive =true;
			this.urlJournal = urlJournal;
		}else {
			journalActive =false;
			this.urlJournal ="";
		}

		stage.setTitle("MIKA-JOJO");
		stage.setHeight(1000);
		stage.setWidth(1000);
		stage.setResizable(false);


		VBox hb= new VBox();

		Label nameLabel = new Label("Entrez votre nom!");
		nameLabel.setPadding(new Insets(0, 0, 10, 0));

		nameLabel.setAlignment(Pos.CENTER);
		// Add Email Text Field
		TextField nameField = new TextField();
		nameField.setPrefHeight(40);
		nameField.setMaxWidth(500);
		Label prblmField = new Label("");
		prblmField.setPadding(new Insets(10, 0, 0, 0));

		nameField.setOnAction(new EventHandler<ActionEvent>(){

			@Override
			public void handle(ActionEvent arg0) {
				System.out.println(nameField.getText());
				boolean nameValide = checkName(nameField.getText());
				if(nameValide) {
					synchronized(ecriv) {
						ecriv.addToSend(client.commandeConnexion(nameField.getText()));
						ecriv.notifyAll();
					}
					setGame();
				}else {
					nameField.clear();
					prblmField.setText("Le nom doit etre de taille entre 1 et 16 inclus, de plus il doit contenir uniquement des chiffres ou des lettres!");
				}
			}

			

		});
		hb.getChildren().addAll(nameLabel, nameField, prblmField);
		hb.setAlignment(Pos.CENTER);

		Scene scene = new Scene(hb, stage.getHeight(), stage.getWidth());  

		Rectangle2D primScreenBounds = Screen.getPrimary().getVisualBounds();
		stage.setX((primScreenBounds.getWidth() - stage.getWidth()) / 2); 
		stage.setY((primScreenBounds.getHeight() - stage.getHeight()) / 4);

		stage.setScene(scene);

		stage.setOnCloseRequest(new EventHandler<WindowEvent>() { 
			@Override 
			public void handle(WindowEvent event) { 
				synchronized(ecriv) {
					c.commandeConnexion("Test");
					ecriv.notifyAll();
				}
				stage.close();
			} 
		});

		stage.show();
	}


	public void changeString(String content) {
		setPlayable(true);
		for(int i = 0 ; i< 4; i++){
			for(int j = 0; j < 4 ; j++){
				cases.get(i*4+j).setText(content.charAt(i+j*4)+""); 
			}
		}
		synchronized(c) {
			c.notifyAll();
		}

	}

	public GridPane newGridPane(String tirage) {
		GridPane gridpane = new GridPane();
		gridpane.setAlignment(Pos.CENTER);
		gridpane.setHgap(GRID_GAP);
		gridpane.setVgap(GRID_GAP);

		for(int i = 0 ; i< 4; i++){
			String lId;	
			for(int j = 0; j < 4 ; j++){
				if(j == 0) {
					lId = "A";
				}else if(j == 1) {
					lId = "B";
				}else if(j == 2) {
					lId = "C";
				}else {
					lId = "D";
				}
				Label tmp = new Label(tirage.charAt(i+j*4)+"");
				tmp.setId(lId+(i+1));

				tmp.setAlignment(Pos.CENTER);
				tmp.setFont( new Font(FONT_SIZE) );
				tmp.addEventHandler(MouseEvent.DRAG_DETECTED, handlerDragDetected);
				tmp.addEventHandler(MouseDragEvent.MOUSE_DRAG_ENTERED, handlerDragEntered);	
				tmp.addEventHandler(MouseEvent.MOUSE_RELEASED, handlerMouseReleased);



				tmp.setPrefSize(80, 80);
				cases.add(tmp);
				gridpane.add(tmp, i,j);
			}
		}
		return gridpane;
	}

	public TableView<UserScore> newScoreTable() {

		TableView<UserScore> table = new TableView<>();
		TableColumn<UserScore, String> userCol = new TableColumn<>("User");
		userCol.setCellValueFactory(
				new PropertyValueFactory<UserScore,String>("fieldUser"));
		userCol.setMinWidth(150);
		userCol.setStyle("-fx-alignment: CENTER;");

		TableColumn<UserScore, Integer> scoreCol = new TableColumn<>("Score");
		scoreCol.setCellValueFactory(
				new PropertyValueFactory<UserScore,Integer>("fieldScore"));
		scoreCol.setMinWidth(100);

		scoreCol.setStyle("-fx-alignment: CENTER;");

		dataList =FXCollections.observableArrayList();
		table.setItems(dataList);
		table.getColumns().addAll(userCol, scoreCol);
		table.setMinHeight(500);

		return table;
	}

	public Label newScoreTitle(int numTour) {
		Label tableTitle = new Label("Score du tour n:"+numTour);
		tableTitle.setFont(new Font(30));
		return tableTitle;
	}

	public void setInfo(String newInfo) {
		info.setText(newInfo);

	}	

	public void setScore() {
		ObservableList<UserScore> dataList = c.getScoreList();
		int tour = c.getNumTour();
		scoreTable.setItems(dataList);
		scoreTitle.setText("Score du tour n:"+tour);
	}
	
	public void setScoreSession() {
		ObservableList<UserScore> dataList = c.getScoreList();
		scoreTable.setItems(dataList);
		scoreTitle.setText("Score Last Session");
	}

	public void setGame() {


		info = new Text("Attente du message de bienvenue du serveur!");
		info.setFont(new Font(40));

		afficheMotEnConst = new Label("");
		afficheMotEnConst.setFont(new Font(40));
		afficheMotEnConst.setTextFill(Color.BLUE);

		VBox VBox_motEnConst_Info = new VBox();
		VBox_motEnConst_Info.getChildren().addAll(info, afficheMotEnConst);
		VBox_motEnConst_Info.setAlignment(Pos.CENTER);

		StackPane stackpane = new StackPane();  	
		stackpane.getChildren().add(VBox_motEnConst_Info);

		GridPane grilleJeu = newGridPane("GAMEDID NOT START");	
		setPlayable(false);
		scoreTable = newScoreTable();
		scoreTitle = newScoreTitle(-1);
		VBox vBoxScore = new VBox();
		vBoxScore.getChildren().addAll(scoreTitle, scoreTable);

		HBox hbox = new HBox();
		hbox.getChildren().addAll(grilleJeu, vBoxScore);
		hbox.setPadding(new Insets(10, 50,10, 50));
		hbox.setSpacing(150);

		VBox vbox = new VBox();

		vbox.getChildren().addAll(stackpane, hbox);

		HBox stackBtn = new HBox();
		stackBtn.setMinSize(1000, 30);
		stackBtn.setPadding(new Insets(0,0,10,930));
		if(journalActive) {
			Button btn = new Button();
			btn.setText("Journal");
			btn.setOnAction(
					new EventHandler<ActionEvent>() {
						@Override
						public void handle(ActionEvent event) {

							if (Desktop.isDesktopSupported()) {
								try {
									Desktop.getDesktop().browse(new URI(urlJournal));
								} catch (IOException e) {
									e.printStackTrace();
								} catch (URISyntaxException e) {
									e.printStackTrace();
								}

							}
						}});
			btn.setAlignment(Pos.TOP_RIGHT);
			stackBtn.getChildren().add(btn);	    
		}
		vbox.getChildren().add(stackBtn);

		if(chatActive) {

			/*VERSION AVEC CHAT*/
			chatBox = new VBox(5);
			messages = new ArrayList<>();
			ScrollPane container = new ScrollPane();

			container.setMinSize(1000, 210);
			container.setMaxSize(1000, 210);
			container.setContent(chatBox); 

			TextField whispToField = new TextField();
			whispToField.setPromptText("All");
			whispToField.setMaxSize(115, 25);
			whispToField.setMinSize(115, 25);

			TextField writingMsgField = new TextField();
			writingMsgField.setPromptText("Envoyer un message");
			writingMsgField.setMaxSize(835, 25);	
			writingMsgField.setMinSize(835, 25);         
			writingMsgField.setOnAction(new EventHandler<ActionEvent>(){
				@Override
				public void handle(ActionEvent arg0) {
					if(writingMsgField.getText().length() != 0) {
						System.out.println(writingMsgField.getText());

						TextFlow flow;
						if(whispToField.getText().length() > 0) {
							boolean nameValide = checkName(whispToField.getText());
							if(nameValide) {
								flow = newPrivateMessageToSend(whispToField.getText(), writingMsgField.getText());
								synchronized(ecriv) {
									ecriv.addToSend(c.commandeEnvoiMessagePrive(whispToField.getText(), writingMsgField.getText()));
									ecriv.notifyAll();
								}
								synchronized(c) {
									c.notifyAll();
								}
								messages.add(flow);
								chatBox.getChildren().add(flow);
							}else {
								whispToField.clear();
								info.setText("Pseudo du message prive incorrecte!");
							}
							
							
						}else {
							//						tmpMsg = new Label(" Jean : "+writingMsgField.getText());
							synchronized(ecriv) {
								ecriv.addToSend(c.commandeEnvoiMessage(writingMsgField.getText()));
								ecriv.notifyAll();
							}
							synchronized(c) {
								c.notifyAll();
							}
						}		
						writingMsgField.clear();
					}
					arg0.consume();
				}

			});


			Label to = new Label("To:");
			to.setPadding(new Insets(1,0,0,5));
			to.setMinSize(30, 25);

			HBox writingBox = new HBox();
			writingBox.getChildren().addAll(to,whispToField,writingMsgField);

			VBox tchat = new VBox();      
			tchat.getChildren().addAll(container,writingBox);

			vbox.getChildren().add(tchat);


			KeyValue initKeyValueChat = new KeyValue(container.vvalueProperty(), 1);
			KeyFrame initFrameChat = new KeyFrame(Duration.ZERO, initKeyValueChat);

			KeyValue endKeyValueChat = new KeyValue(container.vvalueProperty(), 1);
			KeyFrame endFrameChat = new KeyFrame(Duration.seconds(10), endKeyValueChat);

			Timeline timelineChat = new Timeline(initFrameChat, endFrameChat);

			timelineChat.setCycleCount(Timeline.INDEFINITE);
			timelineChat.play();
			/* FIN VERSION AVEC CHAT*/
		}


		scene = new Scene(vbox, stage.getHeight(), stage.getWidth());

		stage.setOnCloseRequest(new EventHandler<WindowEvent>() { 
			@Override 
			public void handle(WindowEvent event) { 
				synchronized(ecriv) {
					ecriv.addToSend(c.commandeDeconnexion());
					ecriv.notifyAll();
				}
				synchronized(c) {
					c.notifyAll();
				}
				stage.close();
			} 
		});

		stage.setScene(scene);


		double sceneWidth = scene.getWidth();
		double msgWidth = info.getLayoutBounds().getWidth();
		KeyValue initKeyValueInfo = new KeyValue(info.translateXProperty(), sceneWidth);
		KeyFrame initFrameInfo = new KeyFrame(Duration.ZERO, initKeyValueInfo);

		KeyValue endKeyValueInfo = new KeyValue(info.translateXProperty(), -1.0 * msgWidth);
		KeyFrame endFrameInfo = new KeyFrame(Duration.seconds(9), endKeyValueInfo);

		Timeline timelineInfo = new Timeline(initFrameInfo, endFrameInfo);

		timelineInfo.setCycleCount(Timeline.INDEFINITE);
		timelineInfo.play();
	}


	public void setPlayable(boolean playable) {
		for(int i=0; i<cases.size(); i++) {
			if(playable) {
				cases.get(i).addEventHandler(MouseEvent.DRAG_DETECTED, handlerDragDetected);
				cases.get(i).addEventHandler(MouseDragEvent.MOUSE_DRAG_ENTERED, handlerDragEntered);	
				cases.get(i).addEventHandler(MouseEvent.MOUSE_RELEASED, handlerMouseReleased);
			}else {
				cases.get(i).removeEventHandler(MouseEvent.DRAG_DETECTED, handlerDragDetected);
				cases.get(i).removeEventHandler(MouseDragEvent.MOUSE_DRAG_ENTERED, handlerDragEntered);	
				cases.get(i).removeEventHandler(MouseEvent.MOUSE_RELEASED, handlerMouseReleased);
			}
		}
	}

	private TextFlow newPrivateMessageToSend(String to, String msg) {

		TextFlow flow = new TextFlow();
		Text t1 = new Text(" A ");
		Text t2 = new Text(to);
		t2.setFill(Color.RED);
		t2.setFont(Font.font("System Regular", FontWeight.BOLD, FontPosture.ITALIC, 12));
		Text t3 = new Text(" : "+msg);
		flow.getChildren().addAll(t1, t2, t3);
		return flow;
	}

	public void receivePublicMessage(String msg) {	
		if(chatActive) {
			TextFlow flow = newPublicMessageReceived(msg);
			messages.add(flow);
			chatBox.getChildren().add(flow);
		}else {
			System.out.println("Reception d'un message public mais l'option du chat n'est pas activee");
		}
	}

	public void receivePrivateMessage(String from, String msg) {
		if(chatActive) {
			TextFlow flow = newPrivateMessageReceived(from, msg);
			messages.add(flow);
			chatBox.getChildren().add(flow);
		}else {
			System.out.println("Reception d'un message prive mais l'option du chat n'est pas activee");
		}
	}

	private TextFlow newPrivateMessageReceived(String from, String msg) {
		TextFlow flow = new TextFlow();
		Text t1 = new Text(" De ");
		Text t2 = new Text(from);
		t2.setFill(Color.RED);
		t2.setFont(Font.font("System Regular", FontWeight.BOLD, FontPosture.ITALIC, 12));
		Text t3 = new Text(" : "+msg);
		flow.getChildren().addAll(t1, t2, t3);
		return flow;
	}

	private TextFlow newPublicMessageReceived(String msg) {
		TextFlow flow = new TextFlow();
		Text t1 = new Text(" Anonyme");
		t1.setFont(Font.font("System Regular", FontWeight.BOLD, 12));	
		t1.setFill(Color.color(Math.random(), Math.random(), Math.random()));
		Text t2 = new Text(" : "+msg);
		flow.getChildren().addAll(t1, t2);
		return flow;
	}

	public void setNewMotEnConst(String mot, String traj) {
		for(Label c: cases )
			c.setTextFill(Color.web("#000000"));
		afficheMotEnConst.setText(mot);
		int i =0;
		int coordCase = 0;
		boolean needReset = false;
		while(i < traj.length()) {
			switch(traj.charAt(i)) {
			case 'A':
				coordCase += 0;
				break;
				
			case 'B':
				coordCase += 1;
				break;
				
			case 'C':
				coordCase += 2;
				break;
				
			case 'D':
				coordCase += 3;
				break;
				
			case '1':
				needReset = true;
				coordCase += 0;
				break;
				
			case '2':
				needReset = true;
				coordCase += 4;
				break;
				
			case '3':
				needReset = true;
				coordCase += 8;
				break;
				
			case '4':
				needReset = true;
				coordCase += 12;
				break;
			}
			
			
			if(needReset) {
				cases.get(coordCase).setTextFill(Color.web("#0076a3"));
				coordCase = 0;
				needReset = false;
			}
			i++;
		}
	}

	
	private boolean checkName(String text) {
		Pattern p = Pattern.compile("^\\w+$");
		Matcher m = p.matcher(text);
		
		return m.matches() && text.length()<=16;
	}
}
