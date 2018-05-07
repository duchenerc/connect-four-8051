#include "reg932.h"
#include "uart.h"

void init();

void board_construct();
void board_draw();
void player_turn(unsigned char player);
unsigned char check_win(unsigned char player);
void size_select();
void play_note(unsigned char note, unsigned char numb_plays);
void delay_counts(unsigned char low, unsigned char high);
void main_song();
unsigned char draw();

void led_control(unsigned char ctrl);

void print(char* str);
void clear_display();

const unsigned char SPACE_X = 'X';
const unsigned char SPACE_O = 'O';
const unsigned char SPACE_EMPTY = ' ';

const unsigned char CTRL_SIZE = 'a';

unsigned char board[7][6];
unsigned char size;

const unsigned char NUM_LEDS = 9;
sbit led0 = P2^4;
sbit led1 = P0^5;
sbit led2 = P2^7;
sbit led3 = P0^6;
sbit led4 = P1^6;
sbit led5 = P0^4;
sbit led6 = P2^5;
sbit led7 = P0^7;
sbit led8 = P2^6;

const unsigned char NUM_BTNS = 9;
sbit btn0 = P2^0;
sbit btn1 = P0^1;
sbit btn2 = P2^3;
sbit btn3 = P0^2;
sbit btn4 = P1^4;
sbit btn5 = P0^0;
sbit btn6 = P2^1;
sbit btn7 = P0^3;
sbit btn8 = P2^2;

// Speaker
sbit speaker = P1^7;

void main(void)
{
	unsigned char current_player = SPACE_O; // changed to SPACE_X at start

	init();

	main_song();
	
	size_select();

	do
	{
		board_construct();
		board_draw();

		do
		{
			// swap players
			current_player = (current_player == SPACE_X ? SPACE_O : SPACE_X);

			led_control(current_player);
			player_turn(current_player);
			board_draw();

		} while (check_win(current_player) == 0 && (draw() == 0));

		// print win message
		if (draw() == 1)
		{	
			play_note(0,5);
		    play_note(11,5);
		    play_note(8,5);
		    play_note(5,5);
		    play_note(1,5);
		    play_note(0,15);
			print("There was a draw! Good luck next time. Hit any button to try again.");
		}
		else
		{
			uart_transmit(current_player);
			print(" wins! Press any button to play another game.\r\n");
		}
		// wait for user to press to restart
		while (~btn0|~btn1|~btn2|~btn3|~btn4|~btn5|~btn6|~btn7|~btn8);

		// wait till release
		while (btn0&btn1&btn2&btn3&btn4&btn5&btn6&btn7&btn8);

	} while (1);
	
}

unsigned char draw()
{
	unsigned char i;
	unsigned char j;
	unsigned char draw = 1;

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size - 1; j++)
		{
			if (board[i][j] == SPACE_EMPTY)
			{
				draw = 0;
			}
		}
	}
	return draw;
}

void board_construct()
{
	unsigned char i;
	unsigned char j;
	
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size - 1; j++)
		{
			board[i][j] = SPACE_EMPTY;
		}
	}
}

void board_draw()
{
	unsigned char i;
	unsigned char j;

	unsigned char length = 2 * size + 1;
	unsigned char height = length - 2;

	clear_display();
	
	for (j=0; j < height; j++)
	{
		for (i=0; i < length; i++)
		{
			if (j%2 == 0)
			{
				if (i%2 == 0) print("+");
				else print("-");
			}
	
			else
			{
				if (i%2 == 0) print("|");
				else uart_transmit(board[i/2][size-2-j/2]);
			}
		}

		print("\r\n");
	}
	
}

void player_turn(unsigned char player)
{
	unsigned char i;
	unsigned char j;

	// get user input
	do
	{
		
		while (1)
		{
			if (~btn0) { i = 0; break; }
			if (~btn1) { i = 1; break; }
			if (~btn2) { i = 2; break; }
			if (~btn3) { i = 3; break; }
			if (~btn4) { i = 4; break; }
			if (~btn5) { i = 5; break; }
			if (~btn6) { i = 6; break; }
			if (~btn7) { i = 7; break; }
			if (~btn8) { i = 8; break; }
		}
	
	// keep looping on two conditions
	// 1. The pressed button is at an i too wide for the current board size
	// 2. The column is full of pieces already (the top slot isn't empty)
	} while (i >= size || board[i][size-2] != SPACE_EMPTY);

	// place piece
	for (j = 0; j < size-1; j++)
	{
		if (board[i][j] == SPACE_EMPTY)
		{
			board[i][j] = player;
			break;
		}
	}

	// wait for user to release before returning
	while (btn0&btn1&btn2&btn3&btn4&btn5&btn6&btn7&btn8);
}

unsigned char check_win(unsigned char player)
{
	unsigned char i;
	unsigned char j;
	unsigned char iw;
	unsigned char jw;

	// iterate through every space to check
	for (i = 0; i < size; i++)
	{
    	for (j = 0; j < size-1; j++)
		{

			// fast continue if this space isn't the player
			if (board[i][j] != player) continue;

			// horizontal check
			if (i < size-3)
			{
				iw = 1; // already checked the 0th space
				while (board[i+iw][j] == player)
				{
					if (iw >= 3) return 1;
					iw++;
				}
					
			}

			// vertical check
			if (j < size-4)
			{
				jw = 1;
				while (board[i][j+jw] == player)
				{
					if (jw >= 3) return 1;
					jw++;
				}
				
			}

			// positive diagonal check
			if (i < size-3 && j < size-4)
			{
				iw = 1;
				jw = 1;

				while (board[i+iw][j+jw] == player)
				{
					if (iw >= 3) return 1;
					iw++;
					jw++;
				}
			}

			// negative diagonal check
			if (i < size-3 && j >= 3)
			{
				iw = 1;
				jw = 1;

				while (board[i+iw][j-jw] == player) // note the minus
				{
					if (iw >= 3) return 1;
					iw++;
					jw++;
				}
			}


	    }
	}

	return 0;
}

void size_select(void)
{
	clear_display();
	print("Choose a size: 5, 6, or 7\r\n");
	led_control(CTRL_SIZE);
	
	while (1)
	{
		if (~btn0|~btn3|~btn6)
		{
			size = 5;
			break;
		}

		if (~btn1|~btn4|~btn7)
		{
			size = 6;
			break;
		}

		if (~btn2|~btn5|~btn8)
		{
			size = 7;
			break;
		}
	}

	while (btn0&btn1&btn2&btn3&btn4&btn5&btn6&btn7&btn8); // wait for release

}

void init()
{

	// uart setup
	uart_init();
	EA = 1;

	// set pins to bidirectional
	P0M1 = 0;
	P0M2 = 0;
	P1M1 = 0;
	P1M2 = 0;
	P2M1 = 0;
	P2M2 = 0;

}

void led_control(unsigned char ctrl)
{
	// clear LEDs
	led0 = 1;
	led1 = 1;
	led2 = 1;
	led3 = 1;
	led4 = 1;
	led5 = 1;
	led6 = 1;
	led7 = 1;
	led8 = 1;

	switch (ctrl)
	{
		case 'X': // SPACE_X
		led0 = 0;
		led2 = 0;
		led4 = 0;
		led6 = 0;
		led8 = 0;
		break;

		case 'O': // SPACE_O
		led0 = 0;
		led1 = 0;
		led2 = 0;
		led3 = 0;
		led5 = 0;
		led6 = 0;
		led7 = 0;
		led8 = 0;
		break;

		case 'a': // CTRL_SIZE
		led2 = 0;
		led4 = 0;
		led5 = 0;
		led6 = 0;
		led7 = 0;
		led8 = 0;
		break;

		default: break;
	}
}

void print(char* str)
{
	unsigned char i;
	for (i = 0; str[i] != 0; i++)
	{
		uart_transmit(str[i]);
	}
}

void clear_display(void)
{
	print("\033[2J\033[H"); // literally magic
}

void main_song()
{
	play_note(3,3);
	play_note(2,4);
	play_note(1,5);
	play_note(5,3);
	play_note(3,6);
	play_note(8,2);
	play_note(2,1);
	play_note(9,9);
	play_note(12,2);
	play_note(4,8);
	play_note(10,6);
	play_note(12,12);
}



void play_note(unsigned char note, unsigned char numb_plays)
{
  unsigned char i;
  unsigned char j;
  unsigned char length;
  unsigned char low;
  unsigned char high;


  switch(note)
  {
    case 0:  //rest...not a note, lasts same length as the other 1/16th notes
      high = 0x8f;
      low = 0x7d;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < 1; j++)
        {
          delay_counts(low,high);
        }
      }
      break;
    case 1:  //C5
      length = 65;
      high = 0x0fc;
      low = 0x8f;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 3:  //D5
      length = 73;
      high = 0x0fc;
      low = 0x0ef;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 5:  //E5
      length = 82;
      high = 0x0fd;
      low = 0x45;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 6:  //F5
      length = 87;
      high = 0x0fd;
      low = 0x6c;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 8:  //G5
      length = 98;
      high = 0x0fd;
      low = 0xb4;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 10:  //A5
      length = 110;
      high = 0x0fd;
      low = 0x0f4;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 11:  //AS5
      length = 117;
      high = 0x0fe;
      low = 0x11;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
    case 13:  //C6
      length = 131;
      high = 0x0fe;
      low = 0x47;
      for(i = 0; i < numb_plays; i++)
      {
        for(j = 0; j < length; j++)
        {
          speaker = 0;
          delay_counts(low,high);
          speaker = 1;
          delay_counts(low,high);
        }
      }
      break;
  }
  return;
}
void delay_counts(unsigned char low, unsigned char high)
{
  if(low > 0 || high > 0)
  {
    TMOD = 0x01;//timer 0, mode 1
    TL0 = low;
    TH0 = high;   
    TR0 = 1; //start timer

    while(!TF0);
    TR0 = 0; //stop timer
    TF0 = 0; //clear flag
  }
  return;
}
