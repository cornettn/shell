#include "read_line.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "tty_raw_mode.h"

//extern void tty_raw_mode(void);

int g_line_length = 0;
char g_line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change.
// Yours have to be updated.
int g_history_index = 0;
char ** g_history;
int g_history_length = 0;

/*
 *  Prints usage for read_line
 */

void read_line_print_usage() {
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
} /* read_line_print_usage() */


void shift_right(int insert_pos) {
  /* Shift everything to the right of the cursor right */

  for (int i = g_line_length + 1; i > insert_pos; i--) {
    g_line_buffer[i] = g_line_buffer[i - 1];
  }
}

void shift_left(int insert_pos) {
  /* Shift everything to the right of the cursor left */

  for (int i = insert_pos; i < g_line_length; i++) {
    g_line_buffer[i] = g_line_buffer[i + 1];
  }
}

void return_to_position(int insert_pos) {
  char ch = 0;
  for (int i = g_line_length; i > insert_pos; i--) {
    /* Write the left arrow */
    ch = 27;
    write(1, &ch, 1);
    ch = 91;
    write(1, &ch, 1);
    ch = 68;
    write(1, &ch, 1);
  }
}

/*
 * Input a line with some basic editing.
 */

char *read_line() {

  /* Get the attributes of the tty being used before changing to raw mode */

  struct termios *term = (struct termios *) malloc(sizeof(struct termios));
  tcgetattr(0, term);

  // Set terminal in raw mode
  tty_raw_mode();

  g_line_length = 0;
  int insert_pos = 0;

  // Read one line until enter is typed
  while (1) {
    // Read one character in raw mode.
    char ch = '\0';
    read(0, &ch, 1);

    if (ch >= 32 && ch <= 126) {
      // It is a printable character.

      // Write the character to the screen
      write(1, &ch, 1);

      // If max number of character reached return.
      if (g_line_length == (MAX_BUFFER_LINE - 2)) {
        break;
      }

      shift_right(insert_pos);

      /* Add char to buffer */

      g_line_buffer[insert_pos] = ch;
      g_line_length++;
      insert_pos++;

      /* Print the new line */

      for (int i = insert_pos; i < g_line_length; i++) {
        ch = g_line_buffer[i];
        write(1, &ch, 1);
      }

      ch = 32;
      write(1, &ch, 1);

      ch = 8;
      write(1, &ch, 1);

      return_to_position(insert_pos);

    }
    else if (ch == 10) {
      // <Enter> was typed. Return line
      // Print newline
      write(1, &ch, 1);

      if (g_history_length == 0) {
        g_history = (char **) malloc(sizeof(char *));
      }
      else {
        g_history = realloc(g_history, (g_history_length + 1) * sizeof(char *));
      }
      g_history[g_history_length] = (char *) malloc(g_line_length * sizeof(char));
      g_history[g_history_length] = strcpy(g_history[g_history_length], g_line_buffer);
      g_history_length++;
      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      g_line_buffer[0] = 0;
      break;
    }
    else if ((ch == 8) || (ch == 127)) {
      // <backspace> was typed. Remove previous character read.

      if ((insert_pos == 0) || (g_line_length == 0)) {
        continue;
      }

      shift_left(insert_pos);

      // Remove one character from buffer
      g_line_length--;
      insert_pos--;

      // Go back one character
      ch = 8;
      write(1, &ch, 1);

      // Write a space to erase the last character read
      ch = ' ';
      write(1, &ch, 1);

      // Go back one character
      ch = 8;
      write(1, &ch, 1);


      return_to_position(insert_pos);


    }
    else if (ch == 27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1 = '\0';
      char ch2 = '\0';
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if ((ch1 == 91) && (ch2 == 65)) {
        // Up arrow. Print next line in history.

        if (g_history_length == 0 || g_history_index == g_history_length) {
          continue;
        }

        // Erase old line
        // Print backspaces
        int i = 0;
        for (i = 0; i < g_line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // Print spaces on top
        for (i = 0; i < g_line_length; i++) {
          ch = ' ';
          write(1, &ch, 1);
        }

        // Print backspaces
        for (i = 0; i < g_line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // Copy line from history

        if (g_history_index < g_history_length) {
          g_history_index++;
        }

        strcpy(g_line_buffer, g_history[g_history_length - g_history_index]);
        g_line_length = strlen(g_line_buffer);

        // echo line
        write(1, g_line_buffer, g_line_length);
        insert_pos = g_line_length;
      }
      else if ((ch1 == 91) && (ch2 == 68)) {

        /* Left Arrow Key */

        if (insert_pos == 0) {
          continue;
        }

        /* Write the left arrow */

        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

        /* Update insert pos */

        insert_pos--;

      }
      else if ((ch1 == 91) && (ch2 == 67)) {

        /* Right Arrow Key */

        if (insert_pos == g_line_length) {
          continue;
        }

        /* Write the left arrow */

        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

        /* Update insert pos */

        insert_pos++;
      }
      else if ((ch1 == 91) && (ch2 == 66)) {

        /* Down Arrow Key */
        if (g_history_length == 0 || g_history_index == 0) {
          continue;
        }

        // Erase old line
        // Print backspaces
        int i = 0;
        for (i = 0; i < g_line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // Print spaces on top
        for (i = 0; i < g_line_length; i++) {
          ch = ' ';
          write(1, &ch, 1);
        }

        // Print backspaces
        for (i = 0; i < g_line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // Copy line from history

        if (g_history_index - 1 > 0) {
          g_history_index--;
          strcpy(g_line_buffer, g_history[g_history_length - g_history_index]);
        }
        else {
          strcpy(g_line_buffer, "");
        }

        g_line_length = strlen(g_line_buffer);

        // echo line
        write(1, g_line_buffer, g_line_length);
        insert_pos = g_line_length;
      }
    }
  } // while loop

  // Add eol and null char at the end of string
  g_line_buffer[g_line_length] = 10;
  g_line_length++;
  g_line_buffer[g_line_length] = 0;


  tcsetattr(0, TCSANOW, term);

  free(term);

  return g_line_buffer;
} /* read_line() */

