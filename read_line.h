#ifndef READ_LINE_H
#define READ_LINE_H

#define MAX_BUFFER_LINE (2048)

char *read_line();
void read_line_print_usage();

char g_line_buffer[MAX_BUFFER_LINE];
int g_line_length;

#endif
