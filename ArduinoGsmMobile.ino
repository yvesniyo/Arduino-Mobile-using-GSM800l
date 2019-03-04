#include <Keypad.h>
#include<SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

SoftwareSerial gsm(13,12);

const byte ROWS = 5;




const byte COLS = 4;
int vib=2;

int VOLUME=4;
bool statusChecking=false;
char keys[ROWS][COLS] = {
  {'F','f','#','*'},
  {'1','2','3','U'},
  {'4','5','6','D'},
  {'7','8','9','E'},
  {'L','0','R','K'}
};
/*For mega
byte colPins[COLS] = {48, 46, 44, 42};
byte rowPins[ROWS] = {32, 34, 36, 38, 40};
*/

byte colPins[COLS] = {3, 4, 5, 6};
byte rowPins[ROWS] = {11, 10, 9, 8, 7};
String scroll="";
int dataScroll=0;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String option="menu";
String number="";
String STATUS="";
int newSMS=0;
String callNumber="",lcddata="";

void setup(){
  lcd.begin(16,2);
  Serial.begin(9600);
  gsm.begin(9600);
  delay(500);
  printMenu();
  pinMode(vib,OUTPUT);

}

void lcdp(String string1,String string2){
  lcd.clear();
  lcd.print(string1);
  lcd.setCursor(0,1);
  //String string=string2.substring(0,16);
  lcd.print(string2);
}

void loop(){

   
   if(Serial.available()>0){
    String data=Serial.readString();
    if(data.indexOf("AT")!=-1){
      gsm.println(data);      
    }
    
    
   }
   
  if(gsm.available()){
    String data=gsm.readString();
    Serial.println(data);
    /*if(statusChecking){       
      if(gsm.readString().indexOf("+CPAS:")!=-1 && gsm.readString().indexOf("3")!=-1){
        //show("Ringing");
        lcdp("Call Status"," Ringing");
      }else if(gsm.readString().indexOf("+CPAS:")!=-1 && gsm.readString().indexOf("4")!=-1){
        //show("Call in progress");
        lcdp("Call Status"," Call In progress");
      }else if(gsm.readString().indexOf("+CPAS:")!=-1 && gsm.readString().indexOf("0")!=-1){
        //show("Call in progress not accessible");
        lcdp("Call Status"," Call unkown");
        statusChecking=false;
      }
    }*/
    if(data.indexOf("+CUSD:")!=-1){
      //lcddata=data;
      //scroll=="yes";
      int index1=data.indexOf("\"");
      int index2=data.lastIndexOf("\"");
      String ussd=data.substring(index1+1,index2);
      delay(100);
      Serial.println(ussd);
      delay(500);
      lcdp("USSD Result",ussd.substring(0,15));
      
    }
    
    if(data.indexOf("+CLIP:")!=-1){
      STATUS="ringing";
      callNumber=getCallNum(data);
      digitalWrite(vib,HIGH);
      lcdp("Call From:",getCallNum(data));
    }if(data.indexOf("+CBC:")!=-1){
      lcd.clear();
      lcd.print("Battery Level");
      lcd.setCursor(0,1);
      String voltage=battery(data);
      int vol=voltage.toInt();
      vol=vol/5*100;
      lcd.print(vol);
      lcd.print(" %");
      //show(vol +"% Power");
    }if(data.indexOf("+CMGR")!=-1){
      //show("Extract Messages.");
      //show("");
      int index1=data.indexOf("\"");
      int index2=data.indexOf(",");
      String statu=data.substring(index1+1,index2-1);
      int index3=data.indexOf(",",index2+1);
      String from=data.substring(index2+2,index3-1);
      int index4=data.indexOf(",",index3+1);
      String none=data.substring(index3+1,index4-1);
      int index5=data.indexOf("\"",index4+1);
      int lstindex1=data.lastIndexOf("\"");
      int lstindex2=data.lastIndexOf("\"",lstindex1-5);
      String date=data.substring(lstindex2+1,lstindex1);
      int index6=data.lastIndexOf("\"");
      String message=data.substring(index6+1,data.length());
      //show("Status: "+statu);
      //show("From: "+from);
      //show("Date: "+date);
      //show("Message: "+message);
      //great("Message","From:"+from,date,message);
      printMenu();
    }else if(data.indexOf("+CMTI:")!=-1){
      newSMS++;
      int index=data.indexOf(",");
      String messageId=data.substring(index+1,data.length());
      showMessage(messageId);
      
    }else if(data.indexOf("ATD")!=-1){
      int index=data.indexOf(";");
      String number=data.substring(3,index);
      callNumber=number;
      lcdp("Calling Tel",number);
      //show("Calling Tel "+number);
      statusChecking=true;
      option="call";
    }else if(data.indexOf("NO DIALTONE")!=-1){
      lcdp("Call Status","NO DIALTONE");
      printMenu();
    }else if(data.indexOf("BUSY")!=-1){
      lcdp("Call Status","User Busy");
      printMenu();
    }else if(data.indexOf("NO CARRIER")!=-1){
      digitalWrite(vib,LOW);
      lcdp("Call Status","Connection Failed");
      printMenu();
    }else if(data.indexOf("NO ANSWER")!=-1){
      lcdp("Call Status","NO ANSWER");
      printMenu();
    }else{
      //lcd.print(data);
      //Serial.println(data);
    }
    
  }
  
  char key = keypad.getKey();
  if (key){
    if(scroll=="yes"){
      if(key=="R"){
        dataScroll+=1;
        String d=lcddata.substring(dataScroll,dataScroll+16);
        lcdp(" > to scroll",d);
      }
      return;
    }
    if(STATUS=="ringing"){
      if(key=='k'){
        lcdp("Calling","From:"+callNumber);
        //statusChecking=true;
        digitalWrite(vib,LOW);
        getCall();
        delay(500);
      }else if(key=='E'){
        digitalWrite(vib,LOW);
        cancelCall();
        delay(500);
        printMenu();
      }
    }
    if(option=="menu"){
      switch(key){
        case'F':
          option="menu";
          printMenu();
          break;
        case'1':
          callMenu();
          option="call";
          break;
        case'2':
          ussdMenu();
          option="ussd";
          break;
        case'3':
          smsMenu();
          option="sms";
          break;
        case'4':
          Exit();
          break;
        case'K':
          if(STATUS=="ringing"){
            getCall();
          }else{
            //show("Choose Correct");
          }
          break;
        default:
          //show("Choose Correct");     
          break;     
      }
    }else if(option=="call"){
      if(key=='K'){
        if(number.length()<3 || number.length() >10){
          lcdp("Error:","Wrong Number");
          delay(1000);
          lcdp("Tel Number",number);
        }else{
          call(number);
          number="";
        }
        
        
      }else if(key=='E'){
        if(STATUS=="calling"){
          digitalWrite(vib,LOW);
          cancelCall();
        }
        number="";
        Exit();
      }else{
        if(key=='L'){
          number=number.substring(0,number.length()-1);
          //show("");
          lcdp("Tel Number",number);
          Serial.print(number);
        }else if(isNum(key)){
          number+=String(key);
          lcdp("Tel Number",number);
          Serial.print(key);
        }else{
          lcd.print("Only Nums");
          //show("Num only");
        } 
      } 
    }else if(option=="ussd"){
      if(key=='K'){
        if(number.indexOf("*")!=-1 && number.indexOf("#")!=-1){
          ussd(number);
          number="";
        }else if(STATUS=="ussdTask"){
          ussd(number);
          number="";
        }else{
          //show("This is not a valid ussd code");
        }
      }else if(key=='E'){
        gsm.println("AT+CUSD=2");
        delay(100);
        Exit();
      }else{
        if(key=='L'){
          number=number.substring(0,number.length()-1);
          //show("");
          lcdp("USSD Codes",number);
          Serial.print(number);
        }else if(isUssd(key)){
          number+=String(key);
          lcdp("USSD Codes",number);
          Serial.print(key);
        }else{
          //show("Code only");
        } 
      }
    }else if(option=="sms"){
      switch(key){
        case'1':
          //show("------------");
          //show("Enter id of message to read");
          option="messageId";
          break;
        case'2':
          //show("------------");//+CPMS
          //show("Total Number of messages on sim are:");
          option="menu";
          break;
        case'3':
          //show("------------");
          //show("Total Number of messages unread:");
          option="menu";
          break;
        case'R':
          lcdp("Message Menu","2.Total SMS");
          option="menu";
          break;
        default:
          //show("Wrong choice");
          break;
      }
    }else if(option=="messageId"){
      if(key=='E'){
        number="";
        delay(100);
        Exit();
      }else if(key=='K'){
        //show("");
        showMessage(number);
        //show("");
        number="";
      }else if(key=='L'){
          number=number.substring(0,number.length()-1);
          //show("");
          Serial.print(number);
      }else if(isNum(key)){
        number+=String(key);
        Serial.print(key);
      }else{
        //show("Enter numbers only");
      }
    }
  }
  

  //End of void loop
}
void reset(){
  gsm.println("AT+CFUN=1,1");
  //show("Trying to reset gsm");
  while(!gsm.available() || gsm.readString().indexOf("OK")== -1){
    delay(10);
  }
  //show("Gsm reset successfully");
  
}
void showMessage(String id){
  //show("Trying to read Mesage:"+ id);
  gsm.println("AT+CMGF=1");
  delay(700);
  gsm.println("AT+CMGR="+ id);
  number="";
  option="menu";
  lcdp("Trying to read","loading...");
  
  
}

bool isNum(char key){
  if(key=='U' || key=='f' || key=='F' || key=='L' || key=='R' || key=='*' || key=='#' || key=='D'){
      return false;
   }else{
    return true;
   }
}
bool isUssd(char key){
  if(key=='U' || key=='f' || key=='F' || key=='L' || key=='R' || key=='D'){
      return false;
   }else{
    return true;
   }
}
void printMenu(){
  option="menu";
  //show("Menu: 1.Call 2.Ussd 3.Sms");
  String menu="1.Call 2.USSD 3.Sms";
  lcdp("Menu 1.Call","2.USSD 3.SMS");
  
}
void callMenu(){
  //show("Enter Number to call");
  lcdp("Call Menu","Enter Tel No");
}
void ussdMenu(){
  //show("Enter Ussd Code");
  lcdp("USSD Menu","Enter USSD");
}
void smsMenu(){
  //show("1. Read Message N0");
  lcdp("Message Menu","1.Read SMS No");
}
void Exit(){
  //show("You are exiting.");
  lcdp("Exiting","You are now Exiting");
  option="menu";
  delay(1000);
  printMenu();
}

void call(String number){
  //show("\nSet to call:"+number);
  lcdp("Set To Call",number);
  gsm.println("ATD"+ number +";");
  STATUS="calling";
  
}

void ussd(String ussd){
  //show("ussd sent "+ussd);
  gsm.println("AT+CUSD=1,\""+ ussd +"\"");
  STATUS="ussdTask";
  lcdp("USSD sent",ussd);
  
}
void cancelCall(){
  gsm.println("ATH");
  //show("Calling Canceled");
  STATUS="";
  delay(1000);
  callMenu();
  lcdp("Calling","Canceled");
}
void getCall(){
  gsm.println("ATA");
  //show("Getting Call");
  lcdp("Answering Call"," .......");
  statusChecking=true;
  
}

String getCallNum(String data){
  int firstD=data.indexOf("\"");
  String number=data.substring(firstD+1,firstD+11);
  return number;
}

String battery(String data){
  int lastD=data.lastIndexOf(",");
  String voltage=data.substring(lastD+1,data.length());
  return voltage;
}
void show(String data){
  //Serial.println(data);
}



