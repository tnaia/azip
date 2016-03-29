/*
  Azip, a compressing utility for ascii files.
  Copyright 2016 Tassio Naia
  Contact me through the domain polignu dot org
  using email tns (so to form email at domain),
  replacing dot for a dot).

  This is free software. 
  See COPYING for details.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include<stdio.h>
#include<stdlib.h>


#define banner "This is azip, version 160328.\n"
#define buf_size (1024 * 1024)

unsigned int freq[1<<7];


int main(int argc, char **argv)
{
  FILE *input;
  int in_buffer[buf_size +1];
  unsigned int chars_read = 0;
  int i;
  int llen;
  printf(banner);

  /* Open file */
  if(argc != 2)
    {
      fprintf(stderr, "  Usage: azip <input file>.\n");
      exit(1);
    }

  if((input = fopen(argv[1],"r")) == NULL)
    {
      fprintf(stderr, "  Usage: azip <input file>.\n");
      exit(1);
    }
  
  /* Initialize structures */
  i=0;
  while(i < 1<<7)
    freq[i++] = 0;
  
  /* First pass */
  printf("First pass.\n");

  while((llen = fread(in_buffer,
		      1,
		      buf_size,
		      input)) != 0)
    {
      printf("%s\nllen = %d\n", in_buffer,llen);fflush(stdout);
      i = 0;
      while(i < llen)
	{
	  printf("%c", in_buffer[i]);fflush(stdout);
	  freq[in_buffer[i++]] +=1;
	  chars_read++;
	}
    }
  
  fclose(input);
  
  /* Print file statistics */
  printf("Read %u chars.\n", chars_read);
  
  /* Second pass */

  return 0;
}
