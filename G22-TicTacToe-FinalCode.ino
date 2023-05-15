#include <LiquidCrystal.h>
#include <Stepper.h>
#include <Servo.h>
Servo myservo; 

int switch_pin = 2;
int sensor [3][3] = {{38,36,34},{32,30,28},{26,24,22}}; // sensor pin numbers

const int stepsPerRevolution = 2038; // Defines the number of steps per rotation
Stepper steppery = Stepper(stepsPerRevolution, 13, 11, 12, 10);
Stepper stepperx = Stepper(stepsPerRevolution, 9, 7, 8, 6);

const int rs = 50, en = 51, d4 = 39, d5 = 37, d6 = 35, d7 = 33;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int position [2][3] = {{4600,2045,0},{0,2060,4900}}; //col and row positions for motor to move on board

int gameBoard [3][3] = {{0,0,0},{0,0,0},{0,0,0}}; //matrix to record the game board

bool firstMove = true;
int turnCount = 0;
long temp; // variabe for ransom first move of robot
long turn; // variable for random start turn

int lostmessage [1][5] = {{"HA HA HA you lost again", "loooser", "Another win for me", "oops I won!","You lost" }}; // messages to show when the robotwins

void setup() 
{
  lcd.begin(16, 2);

  pinMode(2, INPUT);                                       // attach switch to pin 2

  myservo.attach(5);                                        // attaches the servo on pin 5 to the servo object

  Serial.begin(9600);


 // input pin 0 is unconnected, random noise will cause the call to randomSeed() to generate
 // different seed numbers each time the sketch runs.
  randomSeed(analogRead(0)); 
 
  turn = random(1,3);                                       //Randomly generate the starting player's turn
  
  for (int i=0; i<=2 ; i++)                                 //loop to define the pin modes for sensors
  {
    for (int j=0; j<=2; j++)
    {
      pinMode(sensor[i][j], INPUT);
    }
  }
 
}

void loop() {

  if(digitalRead(switch_pin) == HIGH){                         // starts the game only when the switch in on

    if (turn == 1)                                             //If it is player 1's turn to play
    {
      lcd.setCursor(0, 0);
      lcd.print("Your turn");                                  // display its your turn on lcd

      boolean playerInput = false;
      while (!playerInput)                                     //Loop until player 1 has made their play
      {
        for (int i=0; i<=2 ; i++)                              //Check the state of all sensors to see if the player has chosen a position
          for (int j=0; j<=2; j++)
            if (digitalRead(sensor[i][j]) == LOW)              //If a sensor is covered
            {
              if (gameBoard [i][j] == 0)                       //Check that that position is unoccupied
              {
                gameBoard[i][j] = 1;                           //Add the position to the gameboard and change the player's turn
                turn = 2;
                playerInput = true;
              }
            }
      }
      
      displayBoard ();
      Serial.println("");
    }
    else{
      delay(1000);
      lcd.clear();                                           //clear the diplay
      aITurn ();                                                // call the function for robot to make a move
      displayBoard ();
      Serial.println("");

    }
    int winner = evaluate (gameBoard);                         //Check if there has been a winner
    if (winner == 10 || winner == -10)                         //If the score is 10 or -10 then a player has won. 10 is when user wins and -10 when robot wins
    {
      if (winner == 10){
        flashWin();
      }
      if (winner ==  -10){
        flashloss();
      }
      resetBoard ();                                            // reset the game
      
    }
    if (!checkMovesLeft(gameBoard))                            //If there are no more spaces on the board and no-one has won then it is a draw
    {
      flashDraw();
      resetBoard ();                                           // reset the game

    }
    
  }
}
void flashWin ()  {                                            // desplay you won when the user wins
  Serial.println("You won!");
  lcd.setCursor(0, 0);
  lcd.print("You won!");
  delay(10000);
  lcd.clear();
}

void flashloss ()  {                                           // desplay you lost when the user lost
  Serial.println("You lost!");
  lcd.setCursor(0, 0);
  int messagenumber = random(5);

  lcd.print(lostmessage[0][messagenumber]);
  delay(10000);
  lcd.clear();
}

void flashDraw (){                                            // desplay draw if there is no winner
  Serial.println("draw");
  lcd.setCursor(0, 0);
  lcd.print("draw");
  delay(5000);
  lcd.clear();
}


bool checkMovesLeft (int gBoard[3][3])                                //Function to check if there are still empty spaces on the board
{
  for (int i=0; i<=2 ; i++)                                           //Loop through all 9 board positions - i row, j column
    for (int j=0; j<=2; j++)
      if (gBoard[i][j] == 0)
        return true;                                                 //If an empty space is found, return true
  return false;
}

int evaluate (int gBoard[3][3])                                         //Function to evaluate the current board value
{
  for (int i=0; i<=2; i++)                                              //Loop through all rows and check for a win
  {
    if(gBoard[i][0] == gBoard[i][1] && gBoard[i][1] == gBoard[i][2])
    {
      if (gBoard[i][0] == 1)
        return 10;
      else if (gBoard[i][0] == 2)
        return -10;
    }
  }
  for (int j=0; j<=2; j++)                                              //Loop through all columns and check for a win
  {
    if(gBoard[0][j] == gBoard[1][j] && gBoard[1][j] == gBoard[2][j])
    {
      if (gBoard[0][j] == 1)
        return 10;
      else if (gBoard[0][j] == 2)
        return -10;
    }
  }
  if(gBoard[0][0] == gBoard[1][1] && gBoard[1][1] == gBoard[2][2])      //Check first diagonal for a win
  {
    if (gBoard[0][0] == 1)
      return 10;
    else if (gBoard[0][0] == 2)
      return -10;
  }
  if(gBoard[0][2] == gBoard[1][1] && gBoard[1][1] == gBoard[2][0])      //Check second diagonal for a win
  {
    if (gBoard[0][2] == 1)
      return 10;
    else if (gBoard[0][2] == 2)
      return -10;
  }
  return 0;
}


int minimax (int gBoard [3][3], int depth, bool isMax)            //Considers the possible plays and returns a value for the current board position
{
  int score = evaluate (gBoard);

  if (score == 10 || score == -10)                                //If a player has won the game
    return score;

  if (checkMovesLeft (gBoard) == false)                           //If there are no moves left then it must be a draw
    return 0;

  if (isMax)                                                      //If this is the maximizer's turn
  {
    int best = -1000;
    for (int i=0; i<=2; i++)                                      //Go through each cell
      for (int j=0; j<=2; j++)
        if (gBoard[i][j] == 0)                                    //If the space hasn't been played
        {
          gBoard[i][j] = 1;                                       //Play the move
          best = max (best, minimax(gBoard, depth+1, !isMax));
          gBoard[i][j] = 0;                                       //Undo the move
        }
    return best;
  }
  else                                                            //If this is the minimizer's turn
  {
    int best = 1000;
    for (int i=0; i<=2 ; i++)                                     //Go through each cell
      for (int j=0; j<=2; j++)
        if (gBoard[i][j] == 0)                                    //If the space hasn't been played
        {
          gBoard[i][j] = 2;                                       //Play the move
          best = min (best, minimax(gBoard, depth+1, !isMax));
          gBoard[i][j] = 0;                                       //Undo the move
        }
    return best;
  }
}

void aITurn ()
{
  int bestRow = 0;
  int bestCol = 0;
  if (firstMove)
  {
    bool played = false;                                          //the robot plays a random empty corner as the first move
    int temp = random(4);
    if (temp == 0)
    {
      if (gameBoard[0][0] == 0)
      {
        bestRow = 0;
        bestCol = 0;
        played = true;
      }
    }
    else if (temp == 1)
    {
      if (gameBoard[0][2] == 0)
      {
        bestRow = 0;
        bestCol = 2;
        played = true;
      }
    }
    else if (temp == 2)
    {
      if (gameBoard[2][0] == 0)
      {
        bestRow = 2;
        bestCol = 0;
        played = true;
      }
    }
    else
    {
      if (gameBoard[2][2] == 0)
      {
        bestRow = 2;
        bestCol = 2;
        played = true;
      }
    }
    if (!played)
    {
      bestRow = 1;
      bestCol = 1;
    }
    firstMove = false;
  }
  else                                                          // if its not the first move the robot makes the best move
  {
    int bestVal = +1000;
    for (int i=0; i<=2; i++)                                    //Go through each cell
      for (int j=0; j<=2; j++)
        if (gameBoard[i][j] == 0)                               //If the space hasn't been played
        {
          gameBoard[i][j] = 2;                                  //Play the move
 
          int moveVal = minimax(gameBoard, 0, true);
          gameBoard[i][j] = 0;                                  //Undo the move
          if (moveVal < bestVal)                                // compare this potential move to other potential moves
          {
            bestRow = i;
            bestCol = j;
            bestVal = moveVal;
          }
        }
  }
  gameBoard[bestRow][bestCol] = 2;                            //Once all combinations have been explored, play the best move
  makemove(bestCol,bestRow);                                  // calls the function to move motors and diplay the move
  turn = 1;                                                   //Set back to player's turn
}


void drop(){                                                  // function that makes the token dropper drop a token

  myservo.write(90);
  delay(1000);
  myservo.write(170);


}

void makemove(int col , int row)                                // function that makes the motors move to the desired position then drop the token and move back to the origin
{
  steppery.setSpeed(10);
  steppery.step(position[1][row]);
  stepperx.setSpeed(10);
  stepperx.step(position[0][col]);
  drop();
  stepperx.setSpeed(10);
  stepperx.step(-1* position[0][col]);
  steppery.setSpeed(10);
  steppery.step(-1 *position[1][row]);  
}




void displayBoard ()                                          //Function to display the gameboard in serial monitor for debugging
{
  for (int i=0; i<=2; i++)                                    //Go through each cell
  {
    for (int j=0; j<=2; j++)
    {
      Serial.print(gameBoard[i][j]);                          //Display the cell value
      Serial.print(" ");
    }
    Serial.println("");
  }

}

void resetBoard ()                                        //Function to reset the gameboard for the next game
{
  turn = random(1,3);                                     //Reset turn counter
  firstMove = true;                                       //Reset first move
  turnCount = 0;                                          //Reset number of turns counter
  for (int i=0; i<=2; i++){                               //Clear play board
    for (int j=0; j<=2; j++)
    {
      gameBoard [i][j] = 0;
    }}
  Serial.println("Board Reset");                          //Display results on serial monitor for debugging
  displayBoard ();
  Serial.print("Turn: ");
  Serial.print(turn);
  Serial.println(" ");
  lcd.setCursor(0, 0);
  lcd.print("New game");
  delay(3000);
  lcd.clear();
}



//Tic tac toe game open source code based on LED and bottons:  https://www.the-diy-life.com/make-an-arduino-tic-tac-toe-game-with-an-ai-opponent/
//Code and circuit for stepeper motors: https://lastminuteengineers.com/28byj48-stepper-motor-arduino-tutorial/
//code and circuit for the display: https://docs.arduino.cc/learn/electronics/lcd-displays












