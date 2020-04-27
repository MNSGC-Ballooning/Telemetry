/* Code to run the automatic pointing of the antenna for telementry ground station (GS)
 * 
 * ***************************************
 * PINOUT FOR CONNECTOR
 *  1 - no use
 *  2 - 2-4.5V for azimuth (0-450 deg)
 *  3 - 2-4.5V for elevation (0-180 deg)
 *  4 - Az left (ccw)
 *  5 - Az right (cw)
 *  6 - El down
 *  7 - El up
 *  8 - GND
 *  **************************************
 *  - Need to write the pins HIGH when in use and and LOW when not
 *  - Having conflicting pins HIGH (up/down or ccw/cw) will break motor
 *  
 *  **************************************
 *  COMMANDS
 *  ELAZXXXYYY - rotate El to XXX and Az to YYY
 *  ELXXX      - rotate El to XXX
 *  AZYYY      - rotate Az to YYY
 *  **************************************
 *  - left fill with 0 if not 3 digits, i.e. 90 = 090
 *  - need to send angles as degrees NOT RADIANS
 *  
 */

// CONSTANTS *****************************
// Pin definitions
const int UP_PIN = 10;
const int DOWN_PIN = 11;
const int CW_PIN = 9;
const int CCW_PIN = 8;
const int AZ_PIN = A0;
const int EL_PIN = A2;
// max and min angles
const int AZ_MAX = 360;
const int AZ_MIN = 0;
const int EL_MAX = 180;
const int EL_MIN = 0;
// ***************************************

// Variables *****************************
int Az_Current;
int Az_New;
int El_Current;
int El_New;
String cmd = "";
int cmd_flag = 0;
// ***************************************


void setup() 
{
  // open serial object
  Serial.begin(9600);

  // define pinmodes
  pinMode(UP_PIN,OUTPUT); // move
  pinMode(DOWN_PIN,OUTPUT); // move
  pinMode(CW_PIN,OUTPUT); // move
  pinMode(CCW_PIN,OUTPUT); // move
  pinMode(AZ_PIN,INPUT); // read current angle
  pinMode(EL_PIN,INPUT); // read current angle

  // write all pins to low to begin
  digitalWrite(UP_PIN,LOW);
  digitalWrite(DOWN_PIN,LOW);
  digitalWrite(CW_PIN,LOW);
  digitalWrite(CCW_PIN,LOW);

  // request command input (really only need this for testing in arduino)
  //Serial.println("Enter Command: ");

}

void loop() 
{
  // check if command is input
  if(Serial.available()>0)
  {
    cmd = Serial.readString(); // read command
    /*
    Serial.print("Command: ");
    Serial.println(cmd);
    */
    parse_cmd(); // call function to parse the command into angles
    get_angles(); // determine current El and Az angles
    /*
    Serial.print("Current El = ");
    Serial.println(El_Current);
    Serial.print("Current Az = ");
    Serial.println(Az_Current);
    */
    if(cmd_flag > 0)
    {
      set_angles(); // set new angles
      cmd_flag = 0;
    }
    //Serial.println("Enter Command: ");
    Serial.println(String(El_Current) +','+ String(Az_Current));
  }
}


// FUNCTIONS *****************************************


void parse_cmd()
{
  if(cmd.substring(0,4).equals("ELAZ"))
  {
    // get the angles for El and Az
    El_New = cmd.substring(4,7).toInt();
    Az_New = cmd.substring(7,10).toInt();
    /*
    Serial.print("New El = ");
    Serial.println(El_New);
    Serial.print("New Az = ");
    Serial.println(Az_New);
    */
    cmd_flag = 1; //1
    return;
  }
  if(cmd.substring(0,2).equals("EL") ) 
  {
    // get the angle for El
    El_New = cmd.substring(2,5).toInt();
    get_angles();
    Az_New = Az_Current;
    /*
    Serial.print("New El = ");
    Serial.println(El_New);
    Serial.print("New Az = ");
    Serial.println(Az_New);
    */
    cmd_flag = 2; //2
    return;
  }
  if(cmd.substring(0,2).equals("AZ") ) 
  {
    // get the angle for Az
    Az_New = cmd.substring(2,5).toInt();
    get_angles();
    El_New = El_Current;
    /*
    Serial.print("New El = ");
    Serial.println(El_New);
    Serial.print("New Az = ");
    Serial.println(Az_New);
    */
    cmd_flag = 3; //3
    return;
  }
  if(cmd.substring(0,6).equals("getANG"))
  {
    cmd_flag = 0;
    return;
  }
  
}

void get_angles()
{
  int El_ind = 0;
  int Az_ind = 0;
  // read each pin 10 times and average
  for(int i=0; i<10; i++)
  {
    El_ind += analogRead(EL_PIN); // get index (~400 - ~920) (should be)
    delay(5);
  }
  for(int i=0; i<10; i++)
  {
    Az_ind += analogRead(AZ_PIN);
    delay(5);
  }

  El_ind = El_ind/10;
  Az_ind = Az_ind/10;
  /*
  Serial.print("El_ind: ");
  Serial.println(El_ind);
  Serial.print("Az_ind: ");
  Serial.println(Az_ind);
  */
  // find angle from voltage
  El_Current = (0.1392*El_ind) - 0.5377; // from excel data
  Az_Current = (1.354*Az_ind) + 7.9662; // from excel curves
  
}

void set_angles()
{
  // check if angles are above or below max
  if(El_New > EL_MAX) El_New = EL_MAX;
  if(El_New < EL_MIN) El_New = EL_MIN;
  if(Az_New > AZ_MAX) Az_New = AZ_MAX;
  if(Az_New < AZ_MIN) Az_New = AZ_MIN;

  // get current angles
  get_angles();

  if( cmd_flag == 1)
  {
    while( abs(Az_Current-Az_New) > 1  || abs(El_Current-El_New) > 1) 
    {
      // check if Az within 2 degrees of current angle
      if( abs(Az_Current-Az_New) <= 1)
      {
         movement(1); // turn motors off
      }
      else if( Az_Current < Az_New )
      {
        movement(2); // move to right (CW?)
      }
      else if( Az_Current > Az_New )
      {
        movement(3); // move to left (CCW?)
      }
  
      // check if El within 2 degrees of current angle
      if( abs(El_Current-El_New) <= 1)
      {
         movement(4); // turn motors off
      }
      else if( El_Current < El_New )
      {
        movement(5); // move up
      }
      else if( El_Current > El_New )
      {
        movement(6); // move down
      }
      get_angles();
    }
    // turn off motor
    movement(7); // turn it off
  }
  else if( cmd_flag == 2)
  {
    while( abs(Az_Current-Az_New) > 1  || abs(El_Current-El_New) > 1 ) 
    {  
      // check if El within 2 degrees of current angle
      if( abs(El_Current-El_New) <= 1)
      {
         movement(4); // turn motors off
      }
      else if( El_Current < El_New )
      {
        movement(5); // move up
      }
      else if( El_Current > El_New )
      {
        movement(6); // move down
      }
      // update angles
      get_angles();
    }
    // turn off motor
    movement(7); // turn it off
  }
  else if( cmd_flag == 3)
  {
    while( abs(Az_Current-Az_New) > 1  || abs(El_Current-El_New) > 1 ) 
    {
      // check if Az within 2 degrees of current angle
      if( abs(Az_Current-Az_New) <= 1)
      {
         movement(1); // turn motors off
      }
      else if( Az_Current < Az_New )
      {
        movement(2); // move to right (CW?)
      }
      else if( Az_Current > Az_New )
      {
        movement(3); // move to left (CCW?)
      }
      // update angles
      get_angles();
    }
    // turn off motor
    movement(7); // turn it off
  }
}

void movement(int x)
{
  switch(x)
  {
    case 7:
    digitalWrite(UP_PIN,LOW);
    digitalWrite(DOWN_PIN,LOW);
    digitalWrite(CW_PIN,LOW);
    digitalWrite(CCW_PIN,LOW);
    break;

    case 1: // Al off
    digitalWrite(CW_PIN,LOW);
    digitalWrite(CCW_PIN,LOW);
    break;

    case 4: // el off
    digitalWrite(UP_PIN,LOW);
    digitalWrite(DOWN_PIN,LOW);
    break;

    case 2: // right
    digitalWrite(CCW_PIN,LOW);
    digitalWrite(CW_PIN,HIGH);
    break;

    case 3: // left
    digitalWrite(CW_PIN,LOW);
    digitalWrite(CCW_PIN,HIGH);
    break;

    case 5: // up
    digitalWrite(DOWN_PIN,LOW);
    digitalWrite(UP_PIN,HIGH);
    break;

    case 6: // down
    digitalWrite(UP_PIN,LOW);
    digitalWrite(DOWN_PIN,HIGH);
    break;
  }
}
