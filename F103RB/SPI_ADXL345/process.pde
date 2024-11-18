import processing.serial.*;

Serial myPort;
int[] accelData = new int[3]; // Pour stocker les valeurs X, Y, Z
int checksum = 0;            // Checksum calculé pendant la réception
int receivedChecksum = 0;    // Checksum reçu
boolean packetStarted = false; // Indique si on est en train de recevoir un paquet
int index = 0;               // Indice pour les données reçues
String c = "";
int idEtudiant  = 0;
int codeSession = 0;
int codeCapteur = 0;
boolean DEBUG = false;
void setup() {
  //fullScreen();
  size(1000, 1000);
  background(255);
  // Ouvrir la communication série (choisir le port correct)
  String portName = Serial.list()[2];
  myPort = new Serial(this, portName, 9600);
}

void draw() {
  // Vérifier s'il y a des données disponibles sur le port série
  while (myPort.available() > 0) {
    int inByte = myPort.read(); // Lire l'octet reçu
    
    if (inByte == 0x7E) { // Début du paquet (Start Byte)
      packetStarted = true;
      index = 0;          // Réinitialiser l'indice pour les données
      checksum = 0;       // Réinitialiser le checksum
      accelData[0] = accelData[1] = accelData[2] = 0;
      idEtudiant = codeSession = codeCapteur = 0;
      if (DEBUG) 
        println("Initial Byte reçu: " + inByte);
      c = String.valueOf(inByte);
      checksum += inByte;
    } else if (inByte == 0x7F && packetStarted) { // Fin du paquet (End Byte)
      // Comparer le checksum calculé et reçu
      if (DEBUG)
        println("Final Byte reçu: " + inByte);
        int check = checksum;
        checksum %= 256;
        
      if (checksum == receivedChecksum) {
        // Afficher les données si elles sont valides
        println("Données valides. X: " + accelData[0] + 
                " Y: " + accelData[1] + 
                " Z: " + accelData[2]);
      } else {
        println("Erreur de checksum. Données corrompues.");
        println("Received Checksum: " + inByte);
        println("Calculated Checksum: " + checksum);
        println("calcul Checksum = "+c);
      }
      
      //plotCenteredBarsWithLimits(idEtudiant, codeSession, codeCapteur);
      if (DEBUG){
        println("Calculated Checksum: " + checksum);
        println("calcul Checksum = "+c +" = "+ check);
        println(check +" % 256 = "+checksum);
      }
      packetStarted = false; // Réinitialiser pour le prochain paquet
    } else if (packetStarted) {
      if (DEBUG)
        println("Index: " + index + ", Byte reçu: " + inByte);
      // Traiter les autres octets du paquet
      if (index == 0) {
        // ID Étudiant (on peut choisir de l'afficher ou non)
        println("ID Étudiant : " + inByte);
        idEtudiant  = inByte;
      } else if (index == 1) {
        // Code de Session (on peut vérifier sa validité ici)
        println("Code de Session : " + inByte);
        codeSession = inByte;
      } else if (index == 2) {
        // Code de Session (on peut vérifier sa validité ici)
        println("Code Capteur: " + inByte);
        codeCapteur = inByte;
      } else if (index >=3 && index <= 8) {
        // Données d'accélération X, Y, Z (16 bits par axe, donc 6 octets en tout)
        int shift = (index % 2 == 0) ? 8 : 0;
        accelData[(index - 3) / 2] |= (inByte << shift); // Combiner les octets faibles et forts
        if (accelData[(index - 3) / 2] > 32767) {
            accelData[(index - 3) / 2] -= 65536;
          }
        if (index == 4)
          println("X: "+accelData[0]);
        if (index == 6)
          println("Y: "+accelData[1]);
          if (index == 8)
          println("Z: "+accelData[2]);
      } else if (index == 9) {
        // Checksum reçu
        receivedChecksum = inByte;
        println("Received Checksum: " + inByte);
        
      }
      // Calculer le checksum à chaque réception d'octet (sauf le checksum lui-même)
      if (index != 9) {
        c += " + " + String.valueOf(inByte);
        checksum += inByte;
      }
      index++; // Passer à l'octet suivant
    }
  }
  plotCenteredBarsWithLimits(idEtudiant, codeSession, codeCapteur);
}

void plotCenteredBarsWithLimits(int idEtudiant, int sessionCode, int codeCapteur) {
  background(255); // Limpa a tela com fundo branco
  accelData[0] *= -1;
  accelData[1] *= -1;
  accelData[2] *= -1;
  // Configurações gerais
  int centerX = width / 2;
  int centerY = height / 2;
  int barWidth = 50;
  int spacing = 100;

  // Define limites
  int m = max(max(accelData[0], accelData[1]),accelData[2]);
  int limit = 400;
  int scaleFactor = max(m / limit, 1);
  // Plota barra para X
  int xHeight = constrain(accelData[0] / scaleFactor, -limit, limit);
  fill(0, 0, 255); // Azul
  rect(centerX - spacing, centerY, barWidth, -xHeight);

  // Plota barra para Y
  int yHeight = constrain(accelData[1] / scaleFactor, -limit, limit);
  fill(0, 255, 0); // Verde
  rect(centerX, centerY, barWidth, -yHeight);

  // Plota barra para Z
  int zHeight = constrain(accelData[2] / scaleFactor, -limit, limit);
  fill(255, 0, 0); // Vermelho
  rect(centerX + spacing, centerY, barWidth, -zHeight);

  // Informações gerais no canto superior esquerdo
  fill(0); // Preto para texto
  textSize(20);
  text("ID Étudiant: " + idEtudiant, 10, 20);
  text("Code Session: " + sessionCode, 10, 40);
  text("Code Capteur: " + codeCapteur, 10, 60);

  // Adiciona valores ao lado das barras
  fill(0); // Preto para texto
  textSize(20);
  int pad_x = accelData[0] > 0? -10 : +20;
  int pad_y = accelData[1] > 0? -10 : +20;
  int pad_z = accelData[2] > 0? -10 : +20;
  text("X: " + accelData[0], centerX - spacing, centerY - xHeight + pad_x);
  text("Y: " + accelData[1], centerX, centerY - yHeight + pad_y);
  text("Z: " + accelData[2], centerX + spacing, centerY - zHeight + pad_z);
}
