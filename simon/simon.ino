/*
 Simon Says is a memory game. Start the game by pressing one of the four buttons. When a button lights up, 
 press the button, repeating the sequence. The sequence will get longer and longer. The game is won after 
 13 rounds.

 Generates random sequence, plays music, and displays button lights.

 Simon tones from Wikipedia
 - A (red, upper left) - 440Hz - 2.272ms - 1.136ms pulse
 - a (green, upper right, an octave higher than A) - 880Hz - 1.136ms, 0.568ms pulse
 - D (blue, lower left, a perfect fourth higher than the upper left) 587.33Hz - 1.702ms - 0.851ms pulse
 - G (yellow, lower right, a perfect fourth higher than the lower left) - 784Hz - 1.276ms - 0.638ms pulse

 Simon Says game originally written in C for the PIC16F88.
 Ported for the ATmega168, then ATmega328, then Arduino 1.0.
 Adapted for UBMS by Drew Coleman.
 */

/****************************************************************\
 Public Constants
\****************************************************************/
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

// Used to control LEDs
#define CHOICE_OFF 0

// Used to check buttons
#define CHOICE_NONE 0

// Defining choice variables for different LEDs/options
#define CHOICE_RED    (1 << 0)
#define CHOICE_GREEN  (1 << 1)
#define CHOICE_BLUE   (1 << 2)
#define CHOICE_YELLOW (1 << 3)

// LED pin definitions
#define LED_RED    9
#define LED_GREEN  8
#define LED_BLUE   3
#define LED_YELLOW 4

// Button pin definitions
#define BUTTON_RED    7
#define BUTTON_GREEN  6
#define BUTTON_BLUE   5
#define BUTTON_YELLOW 2

// Buzzer pin definitions
#define BUZZER1 10
#define BUZZER2 11

// Number of rounds to succesfully remember before you win. 13 is do-able.
#define ROUNDS_TO_WIN 13

// Amount of time to press a button before game times out. 3500ms = 3.5 sec
#define ENTRY_TIME_LIMIT 3500

#define MODE_MEMORY  0
#define MODE_BATTLE  1
#define MODE_BEEGEES 2

// By default, let's play the memory game
byte gameMode = MODE_MEMORY;

// Contains the combination of buttons as we advance
byte gameBoard[32];

// Counts the number of succesful rounds the player has made it through
byte gameRound = 0;

// Setup hardware inputs/outputs. These pins are defined in the hardware_versions header file
void setup()
{
  // Enable pull ups on inputs
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW, INPUT_PULLUP);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  // Mode checking
  // By default, we're going to play the memory game
  gameMode = MODE_MEMORY;

  // Check to see if the lower right button is pressed
  if (checkButton() == CHOICE_YELLOW) play_beegees();

  // Check to see if upper right button is pressed
  if (checkButton() == CHOICE_GREEN)
  {
    // Put game into battle mode
    gameMode = MODE_BATTLE;

    // Turn on the upper right (green) LED
    setLEDs(CHOICE_GREEN);
    toner(CHOICE_GREEN, 150);

    // Turn on the other LEDs until you release button
    setLEDs(CHOICE_RED | CHOICE_BLUE | CHOICE_YELLOW);

    // Wait for user to stop pressing button
    while (checkButton() != CHOICE_NONE);

    // Now do nothing. Battle mode will be serviced in the main routine.
  }

  // After setup is complete, say hello to the world
  play_winner();
}

void loop()
{
  // Blink lights while waiting for user to press a button
  attractMode();

  // Indicate the start of game play
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE | CHOICE_YELLOW);
  delay(1000);
  setLEDs(CHOICE_OFF);
  delay(250);

  if (gameMode == MODE_MEMORY)
  {
    // Play memory game and handle result
    if (play_memory() == true) 
      play_winner(); // Player won, play winner tones
    else 
      play_loser(); // Player lost, play loser tones
  }

  if (gameMode == MODE_BATTLE)
  {
    // Play game until someone loses
    play_battle();

    // Player lost, play loser tones
    play_loser();
  }
}

// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
boolean play_memory(void)
{
  // Seed the random generator with random amount of millis()
  randomSeed(millis());

  // Reset the game to the beginning
  gameRound = 0;

  while (gameRound < ROUNDS_TO_WIN) 
  {
    // Add a button to the current moves, then play them back
    add_to_moves();

    // Play back the current game board
    playMoves();

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0; currentMove < gameRound; currentMove++)
    {
      // See what button the user presses
      byte choice = wait_for_button();

      // If wait timed out, player loses
      if (choice == 0) return false;

      // If the choice is incorect, player loses
      if (choice != gameBoard[currentMove]) return false;
    }

    // Player was correct, delay before playing moves
    delay(1000);
  }

  // Player made it through all the rounds to win!
  return true;
}

// Play the special 2 player battle mode
// A player begins by pressing a button then handing it to the other player
// That player repeats the button and adds one, then passes back.
// This function returns when someone loses
boolean play_battle(void)
{
  // Reset the game frame back to one frame
  gameRound = 0;

  // Loop until someone fails 
  while (1)
  {
    // Wait for user to input next move
    byte newButton = wait_for_button();
    
    // Add this new button to the game array
    gameBoard[gameRound++] = newButton;

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0; currentMove < gameRound; currentMove++)
    {
      byte choice = wait_for_button();

      // If wait timed out, player loses.
      if (choice == 0) return false;

      // If the choice is incorect, player loses.
      if (choice != gameBoard[currentMove]) return false;
    }

    // Give the user an extra 100ms to hand the game to the other player
    delay(100);
  }

  // We should never get here
  return true;
}

// Plays the current contents of the game moves
void playMoves(void)
{
  for (byte currentMove = 0; currentMove < gameRound; currentMove++) 
  {
    toner(gameBoard[currentMove], 150);

    // Wait some amount of time between button playback
    // Shorten this to make game harder
    // 150 works well. 75 gets fast.
    delay(150);
  }
}

// Adds a new random button to the game sequence, by sampling the timer
void add_to_moves(void)
{
  // min (included), max (exluded)
  byte newButton = random(0, 4);

  // We have to convert this number, 0 to 3, to CHOICEs
  if (newButton == 0) newButton = CHOICE_RED;
  else if (newButton == 1) newButton = CHOICE_GREEN;
  else if (newButton == 2) newButton = CHOICE_BLUE;
  else if (newButton == 3) newButton = CHOICE_YELLOW;

  // Add this new button to the game array
  gameBoard[gameRound++] = newButton;
}

/****************************************************************\
 The following functions control the hardware
\****************************************************************/

// Light up a given LED
// Pass in a byte that is made up from CHOICE_RED, CHOICE_YELLOW, etc
void setLEDs(byte leds)
{
  if ((leds & CHOICE_RED) != 0)
    digitalWrite(LED_RED, HIGH);
  else
    digitalWrite(LED_RED, LOW);

  if ((leds & CHOICE_GREEN) != 0)
    digitalWrite(LED_GREEN, HIGH);
  else
    digitalWrite(LED_GREEN, LOW);

  if ((leds & CHOICE_BLUE) != 0)
    digitalWrite(LED_BLUE, HIGH);
  else
    digitalWrite(LED_BLUE, LOW);

  if ((leds & CHOICE_YELLOW) != 0)
    digitalWrite(LED_YELLOW, HIGH);
  else
    digitalWrite(LED_YELLOW, LOW);
}

// Wait for a button to be pressed
// Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
byte wait_for_button(void)
{
  // Remember the time we started the this loop
  long startTime = millis();

  // Loop until too much time has passed
  while ((millis() - startTime) < ENTRY_TIME_LIMIT)
  {
    byte button = checkButton();

    if (button != CHOICE_NONE)
    { 
      // Play the button the user just pressed
      toner(button, 150);

      // Now let's wait for user to release button
      while (checkButton() != CHOICE_NONE);

      // This helps with debouncing and accidental double taps
      delay(10);

      return button;
    }

  }

  // If we get here, we've timed out!
  return CHOICE_NONE;
}

// Returns a '1' bit in the position corresponding to CHOICE_RED, CHOICE_GREEN, etc.
byte checkButton(void)
{
  if (digitalRead(BUTTON_RED) == 0) return(CHOICE_RED); 
  else if (digitalRead(BUTTON_GREEN) == 0) return(CHOICE_GREEN); 
  else if (digitalRead(BUTTON_BLUE) == 0) return(CHOICE_BLUE); 
  else if (digitalRead(BUTTON_YELLOW) == 0) return(CHOICE_YELLOW);

  // If no button is pressed, return none
  return(CHOICE_NONE);
}

// Light an LED and play tone
// Red, upper left:     440Hz - 2.272ms - 1.136ms pulse
// Green, upper right:  880Hz - 1.136ms - 0.568ms pulse
// Blue, lower left:    587.33Hz - 1.702ms - 0.851ms pulse
// Yellow, lower right: 784Hz - 1.276ms - 0.638ms pulse
void toner(byte which, int buzz_length_ms)
{
  // Turn on a given LED
  setLEDs(which);

  // Play the sound associated with the given LED
  switch(which) 
  {
    case CHOICE_RED:
      buzz_sound(buzz_length_ms, 1136); 
      break;
    case CHOICE_GREEN:
      buzz_sound(buzz_length_ms, 568); 
      break;
    case CHOICE_BLUE:
      buzz_sound(buzz_length_ms, 851); 
      break;
    case CHOICE_YELLOW:
      buzz_sound(buzz_length_ms, 638); 
      break;
  }

  // Turn off all LEDs
  setLEDs(CHOICE_OFF);
}

// Toggle buzzer every buzz_delay_us, for a duration of buzz_length_ms
void buzz_sound(int buzz_length_ms, int buzz_delay_us)
{
  // Convert total play time from milliseconds to microseconds
  long buzz_length_us = buzz_length_ms * (long)1000;

  // Loop until the remaining play time is less than a single buzz_delay_us
  while (buzz_length_us > (buzz_delay_us * 2))
  {
    // Decrease the remaining play time
    buzz_length_us -= buzz_delay_us * 2;

    // Toggle the buzzer at various speeds
    digitalWrite(BUZZER1, LOW);
    digitalWrite(BUZZER2, HIGH);
    delayMicroseconds(buzz_delay_us);

    digitalWrite(BUZZER1, HIGH);
    digitalWrite(BUZZER2, LOW);
    delayMicroseconds(buzz_delay_us);
  }
}

// Play the winner sound and lights
void play_winner(void)
{
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  winner_sound();
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  winner_sound();
}

// Play the winner sound
void winner_sound(void)
{
  // Toggle the buzzer at various speeds
  for (byte x = 250; x > 70; x--)
  {
    for (byte y = 0; y < 3; y++)
    {
      digitalWrite(BUZZER2, HIGH);
      digitalWrite(BUZZER1, LOW);
      delayMicroseconds(x);

      digitalWrite(BUZZER2, LOW);
      digitalWrite(BUZZER1, HIGH);
      delayMicroseconds(x);
    }
  }
}

// Play the loser sound/lights
void play_loser(void)
{
  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_YELLOW);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_YELLOW);
  buzz_sound(255, 1500);
}

// Show an "attract mode" display while waiting for user to press button
void attractMode(void)
{
  while (1) 
  {
    setLEDs(CHOICE_RED);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_BLUE);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_GREEN);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_YELLOW);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;
  }
}

/*****************************************************************\
 The following functions are related to Beegees Easter Egg only
\*****************************************************************/

// Notes in the melody. Each note is about an 1/8th note, "0"s are rests.
int melody[] = {
  NOTE_G4, NOTE_A4, 0, NOTE_C5, 0, 0, NOTE_G4, 0, 0, 0,
  NOTE_E4, 0, NOTE_D4, NOTE_E4, NOTE_G4, 0,
  NOTE_D4, NOTE_E4, 0, NOTE_G4, 0, 0,
  NOTE_D4, 0, NOTE_E4, 0, NOTE_G4, 0, NOTE_A4, 0, NOTE_C5, 0
};

// This essentially sets the tempo, 115 is just about right for a disco groove :)
int noteDuration = 115;

// Keeps track of which LED we are on during the beegees loop
int LEDnumber = 0;

// Do nothing but play bad beegees music
// This function is activated when user holds bottom right (yellow) button during power up
void play_beegees()
{
  // Turn on the bottom right (yellow) LED
  setLEDs(CHOICE_YELLOW);
  toner(CHOICE_YELLOW, 150);

  // Turn on the other LEDs until you release button
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE);

  // Wait for user to stop pressing button
  while (checkButton() != CHOICE_NONE);

  // Turn off LEDs
  setLEDs(CHOICE_NONE);

  // Wait a second before playing song
  delay(1000);

  // setup the "BUZZER1" side of the buzzer to stay low, while we play the tone on the other pin.
  digitalWrite(BUZZER1, LOW);

  // Play song until you press a button
  while (checkButton() == CHOICE_NONE)
  {
    // Iterate over the notes of the melody:
    for (int thisNote = 0; thisNote < 32; thisNote++) {
      changeLED();
      tone(BUZZER2, melody[thisNote], noteDuration);
      
      // To distinguish the notes, set a minimum time between them.
      // The note's duration + 30% seems to work well
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      
      // Stop the tone playing:
      noTone(BUZZER2);
    }
  }
}

// Each time this function is called the board moves to the next LED
void changeLED(void)
{
  // Change the LED
  setLEDs(1 << LEDnumber);

  // Goto the next LED
  LEDnumber++;
  
  // Wrap the counter if needed
  if (LEDnumber > 3) LEDnumber = 0;
}
