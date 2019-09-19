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

// Used to control LEDs
#define CHOICE_OFF 0

// Used to check buttons
#define CHOICE_NONE 0

// Defining choice variables for different LEDs/options
#define CHOICE_GREEN  2
#define CHOICE_RED    4
#define CHOICE_YELLOW 8

// Variable to define the number of LEDs being used
#define NO_OF_LEDS 3

// Pin numbers corresponding to each LED
#define LED_GREEN  2
#define LED_RED    3
#define LED_YELLOW 4

// Button pin definitions for respective pin numbers
#define BUTTON_GREEN  8
#define BUTTON_RED    9
#define BUTTON_YELLOW 10

// Buzzer pin definition
#define BUZZER 7

// Number of rounds to succesfully remember before you win. 13 is do-able
#define LEVELS_TO_COMPLETE 13

// Amount of time to press a button before game times out. 3500ms = 3.5 sec
#define ENTRY_TIME_LIMIT 3500

// Contains the combination of buttons as we advance
byte gameBoard[32];

// Counts the number of succesful rounds the player has made it through
byte gameRound = 0;

void setup()
{
  // Enable pull ups on inputs
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW, INPUT_PULLUP);

  // Set up LED pin numbers as output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  // Set up the Buzzer as another output since we will be emitting sound tones
  pinMode(BUZZER, OUTPUT);

  // Make sure everything set up correctly
  play_winner(); 
}

void loop()
{
  // Indicate the start of game play by turning ON all LEDs and then turning them OFF
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_YELLOW);
  delay(1000);
  setLEDs(CHOICE_OFF);
  delay(250);

  // Play memory game and handle result
  if (play_memory() == true)
    play_winner(); // Player won, play winner tones
  else 
    play_loser(); // Player lost, play loser tones
}

// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
boolean play_memory(void)
{
  // This is to ensure we get a close to perfect random number generator
  // Seed the random generator with random analog value read from pin 0
  randomSeed(analogRead(0));

  // Reset the game to the beginning
  gameRound = 0;

  // Should only execute if there are incomplete levels
  while (gameRound < LEVELS_TO_COMPLETE)
  {
    // Add a button to the current moves, then play them back
    add_to_moves();

    // Play back the current game board
    playMoves();

    // Then require the player to repeat the sequence
    for (byte currentMove = 0; currentMove < gameRound; currentMove++)
    {
      // See what button the player presses
      byte choice = wait_for_button();
      
      // If wait timed out, player loses
      if (choice == 0) return false;
      
      // If the choice is incorect, player loses
      if (choice != gameBoard[currentMove]) return false;
    }
    
    // Player was correct, delay before play continues
    delay(1000);
  }

  // Player made it through all the rounds and won!
  return true;
}

// Plays the current contents of the game moves
// i.e. plays the current sequence of LED tones
void playMoves(void)
{
  for (byte currentMove = 0; currentMove < gameRound; currentMove++)
  {
    toner(gameBoard[currentMove], 150);

    // Wait some amount of time between button playback
    delay(150);
  }
}

// Adds a new random button to the game sequence, by sampling the timer. i.e. levelling up
void add_to_moves(void)
{
  // min (included), max (exluded)
  byte newButton = random(0, NO_OF_LEDS);

  // We have to convert this number, 0 to NO_OF_LEDS - 1, to CHOICEs
  // Include a new choice here if an extra LED is added
  if (newButton == 0) newButton = CHOICE_GREEN;
  else if (newButton == 1) newButton = CHOICE_RED;
  else if (newButton == 2) newButton = CHOICE_YELLOW;

  // Add this new button to the game array
  gameBoard[gameRound++] = newButton;
}

// Light up a given LEDs
// This function will display the sequence through LEDs
// Pass in a byte that is made up from CHOICE_RED, CHOICE_YELLOW, etc.
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

  if ((leds & CHOICE_YELLOW) != 0)
    digitalWrite(LED_YELLOW, HIGH);
  else
    digitalWrite(LED_YELLOW, LOW);
}

// Wait for a button to be pressed
// Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
byte wait_for_button(void)
{
  // Store the time we started the loop
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

      // This helps in eliminating switch bounce effects and accidental double taps
      delay(10);
      return button;
    }
  }
  
  // If we get here, we've timed out!
  return CHOICE_NONE;
}

// Returns a '1' bit in the position corresponding to CHOICE_RED, CHOICE_GREEN, etc.
// Checks to see if a button was pressed and returns what LED it was corresponding to
byte checkButton(void)
{
  if (digitalRead(BUTTON_RED) == 1) return(CHOICE_RED); 
  else if (digitalRead(BUTTON_GREEN) == 1) return(CHOICE_GREEN);
  else if (digitalRead(BUTTON_YELLOW) == 1) return(CHOICE_YELLOW);

  // If no button is pressed, return none
  return(CHOICE_NONE);
}

// Light an LED and play tone
// Play a tone specific to each LED to aid in audio and visual cues
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
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(buzz_delay_us);

    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(buzz_delay_us);
  }
}

// Light up winner lights
void play_winner(void)
{
  setLEDs(CHOICE_GREEN);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  winner_sound();
  setLEDs(CHOICE_GREEN);
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
      digitalWrite(BUZZER, LOW);
      delayMicroseconds(x);

      digitalWrite(BUZZER, HIGH);
      delayMicroseconds(x);
    }
  }
}

// Play the loser sound/lights
void play_loser(void)
{
  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);
  setLEDs(CHOICE_YELLOW);
  buzz_sound(255, 1500);
  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);
  setLEDs(CHOICE_YELLOW);
  buzz_sound(255, 1500);
}
