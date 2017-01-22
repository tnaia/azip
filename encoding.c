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
#include<assert.h>

#define DEBUG 0
#define banner "This is azip, version 160329.\n"
#define buf_size (1024 * 1024 +1)

#define alphabet_size (1<<7)
#define max_code_len (alphabet_size/8)
#define tree_size (2*alphabet_size)
long unsigned int freq[alphabet_size];
int encoding[alphabet_size];

char * symbol_name[alphabet_size] = { 
"nul","soh - start of heading","stx - start of text","etx - end of text","eot - end of transmission","enq - enquiry","ack - acknowledgement","bel - bell","bs - backspace","ht - horizontal tab",
  "nl - line feed/new line","vt - vertical tab","np - form feed/new page","cr - carriage return","so - shift out","si - shift in","dle - data link escape","dc1 - device control 1","dc2 - device control -2","dc3 - device control 3",
  "dc4 - device control 4","nak - negative acknowledgement","syn - synchronous idle","etb - end of transmission block","can - cancel","em - end of medium","sub - substitute","esc - escape","fs - file separator","gs - group separator",
"rs - record separator","us - unit separator","sp - space","!","\"","#","$","%","&","'",
  "(",")","*","+",",","-",".","/","0","1",
  "2","3","4","5","6","7","8","9",":",";",
  "<","=",">","?","@","A","B","C","D","E",
  "F","G","H","I","J","K","L","M","N","O",
  "P","Q","R","S","T","U","V","W","X","Y",
  "Z","[","\\","]","^","_","`","a","b","c",
  "d","e","f","g","h","i","j","k","l","m",
  "n","o","p","q","r","s","t","u","v","w",
  "x","y","z","{","|","}","~","del - delete" };

struct node {
  char character;
  char code[max_code_len];
  int code_len;
  long unsigned int w;
  int left, right;
} tree[tree_size];



int tree_len = 0;
int roots[tree_size];
int roots_len = 0;

void output_code(int i);
void print_7bit_char(char c);
void print_encoding_tree();
void output_bit(char bit);
void print_code_stderr_only(int i);

/* untested */
void print_7bit_char_stderr_only(char c)
{
  int i=6;
  if(DEBUG)
    while(i>=0)
      {
        fprintf(stderr,"%d", (c >> i)%2);
        i--;
      }
}

void output_7bit_char(char c)
{
  int i=6;
  while(i>=0)
    {
      if(DEBUG) fprintf(stderr,"%d", (c >> i)%2);
      output_bit((c >> i)%2);
      i--;
    }
}

void print_tree_rec(int i)
{
  if(tree[i].left != -1)
    {
      if(DEBUG) fprintf(stderr,"0");
      output_bit(0);
      print_tree_rec(tree[i].left);
      print_tree_rec(tree[i].right);
    }
  else
    {
      if(DEBUG) fprintf(stderr,"1");
      output_bit(1);
      output_7bit_char(tree[i].character);
    }
}

void prefix_zero(int i)
{
  if(DEBUG)
    {
      fprintf(stderr,"Prefixing code of length %d with  0: [", tree[i].code_len);
      print_code_stderr_only(i);
      fprintf(stderr,"] --> [");
    }
  tree[i].code_len++;

  if(DEBUG)
    {
      print_code_stderr_only(i);
      fprintf(stderr,"]\n");
    }
  if(tree[i].left != -1) prefix_zero(tree[i].left);
  if(tree[i].right != -1) prefix_zero(tree[i].right);
}

void prefix_one(int i)
{
  int j= 0;

  if(DEBUG)
    {
      fprintf(stderr,"Prefixing code of length %d with  1: [", tree[i].code_len);
      print_code_stderr_only(i);
      fprintf(stderr,"] --> [");
    }

  j = max_code_len -1 -tree[i].code_len/8;
  tree[i].code[j] = tree[i].code[j] + (1<<((tree[i].code_len % 8) + 1));
  tree[i].code_len++;

  if(DEBUG)
    {
      print_code_stderr_only(i);
      fprintf(stderr,"]\n");
    }
  if(tree[i].left != -1) prefix_one(tree[i].left);
  if(tree[i].right != -1) prefix_one(tree[i].right);
}

void print_code_stderr_only(int i)
{
  int j, aux;
  j = max_code_len -1 -tree[i].code_len/8;
  aux = tree[i].code_len % 8;
  while(aux > 0)
    {
      if(((tree[i].code[j] >>aux) % 2)  > 0) fprintf(stderr,"1");
      else fprintf(stderr,"0");
      aux--;
    }

  while(++j < max_code_len)
    {
      aux = 8;
      while(aux > 0)
        {
          if(((tree[i].code[j] >>aux) % 2)  > 0) fprintf(stderr,"1");
          else fprintf(stderr,"0");
          aux--;
        }
    }
}

void output_code(int i)
{
  int j, aux;
  j = max_code_len -1 -tree[i].code_len/8;
  aux = tree[i].code_len % 8;
  while(aux > 0)
    {
      if(((tree[i].code[j] >>aux) % 2)  > 0) 
        {
          if(DEBUG) fprintf(stderr,"1");
          output_bit(1);
        }
      else 
        {
          if(DEBUG) fprintf(stderr,"0");
          output_bit(0);
        }
      aux--;
    }

  while(++j < max_code_len)
    {
      aux = 8;
      while(aux > 0)
        {
          if(((tree[i].code[j] >>aux) % 2)  > 0) 
            {
              if(DEBUG) fprintf(stderr,"1");
              output_bit(1);
            }
          else 
            {
              if(DEBUG) fprintf(stderr,"0");
              output_bit(0);
            }
          aux--;
        }
    }
}
/* Prints tree rooted at i */
void print_rec(int i)
{
  fprintf(stderr,"(");
  if(tree[i].left != -1) {
    print_rec(tree[i].left);
  }
  if(tree[i].right != -1) {
    print_rec(tree[i].right);
  }
  if((tree[i].left == tree[i].right) && tree[i].left == -1) {
    fprintf(stderr,"%s[", symbol_name[tree[i].character]);
    print_7bit_char_stderr_only(tree[i].character);
    fprintf(stderr,"-->");
    print_code_stderr_only(i);
    fprintf(stderr,"]/%lu", tree[i].w);
  } else
    fprintf(stderr,"/%lu", tree[i].w);
  fprintf(stderr,")");fflush(stdout);
}

void print_tree()
{
  int i = 0;
  fprintf(stderr,"The tree as it stands:\n");
  while(i < roots_len)
    {
      print_rec(roots[i++]);
    }
  fprintf(stderr,"\n");
}

void print_tree_linear() 
{
  int i = 0;
  fprintf(stderr,"\n"
         "Node  lft  rgt           weight  char\n"
         "----  ---  ---  ---------------  -------------------------------\n");
  while(i < tree_len)
    {
      
      fprintf(stderr," %3d  "
             "%3d  "
             "%3d  "
             "%15lu  ",
             i, tree[i].left, tree[i].right, tree[i].w);
      if((tree[i].left == tree[i].right) && tree[i].left == -1)
        fprintf(stderr,"%s\n", symbol_name[tree[i].character]);
      else fprintf(stderr,"\n");
      i++;
    }
}

FILE* output;
unsigned char out_buffer[buf_size +max_code_len +1];
unsigned long int next_bit = 0;

void print_out_buffer()
{
  int j = 0;
  fprintf(stderr, "[out buffer: ");
  while(j < buf_size * 8)
    {
      if(j == next_bit) fprintf(stderr, ".");
      if(((out_buffer[j/8] >> (7-(j%8))) % 2) == 1) fprintf(stderr, "1");
      else fprintf(stderr, "0");
      j++;
    }
  if(j == next_bit)
    fprintf(stderr,".] next_bit = %lu\n", next_bit);
  else
    fprintf(stderr,"] next_bit = %lu\n", next_bit);
}

void output_bit(char bit)
{
  char aux;

  aux = next_bit % 8;
  /* we reset when we start a new byte */      
  if(aux == 0) 
    {
      if(DEBUG) fprintf(stderr, "[zeroing next byte]");
      out_buffer[next_bit/8] = 0;
    }
  if(bit) out_buffer[next_bit/8] += 1<<(7-aux);

  next_bit++;

  if(DEBUG) print_out_buffer();

  if(next_bit == 8 * buf_size) 
    {
      /* Flush output */
      if(fwrite(out_buffer, 1, buf_size, output) != buf_size)
        {
          fprintf(stderr, "! Cannot output!\n");
          exit(1);
        }

      next_bit = 0;
      if(DEBUG) fprintf(stderr,"[out_buffer flushed]\n");
    }
}


int main(int argc, char **argv)
{
  FILE *input;
  char in_buffer[buf_size +1];
  long unsigned int chars_read = 0, chars_seen = 0;
  long unsigned int combined_weight;
  long unsigned int i, j, max, aux;
  size_t llen;
  fprintf(stderr,banner);
  fprintf(stderr,"Alphabet size is %d.\n", alphabet_size);
  /* Open file */
  if(argc != 2)
    {
      fprintf(stderr, "  Usage: azip <input file>.\n");
      exit(1);
    }

  if((input = fopen(argv[1],"r")) == NULL)
    {
      fprintf(stderr, "! I cannot open your file.\n");
      exit(1);
    }
  
  /* Initialize structures */
  i=0;
  while(i < alphabet_size)
    freq[i++] = 0;
  
  /* First pass */
  fprintf(stderr,"First pass...");

  while((llen = fread(in_buffer,
                      1,
                      buf_size,
                      input)) != 0)
    {
      if(DEBUG)
        {
          fprintf(stderr,"\nllen = %d\n", (int)llen);fflush(stdout);
        }
      i = 0;
      while(i < (int) llen)
        {
          if(DEBUG)
            {
              fprintf(stderr,"%c", in_buffer[i]);fflush(stdout);
            }
          /* Only support 7-bit ASCII */
          assert(in_buffer[i] >= 0);
          freq[in_buffer[i]] +=1;
          i++;
          chars_read++;
        }
    }
  
  fclose(input);
  fprintf(stderr,"[ DONE ]\n");  
  i = 0;
  fprintf(stderr,"\n"
         " char       frequency  description\n"
         "-----  --------------  -------------------------------\n");
  while(i < alphabet_size)
    {
      if(freq[i] > 0)
        {
          chars_seen++;
          fprintf(stderr,"%4lu  %15lu  %s\n", i, freq[i], symbol_name[i]);
        }
      i++;
    }

  /* Print file statistics */
  fprintf(stderr,"The file has %lu chars (saw %lu distinct).\n", chars_read, chars_seen);
  /* Initialize tree */
  i = 0;
  while(i < tree_size)
    {
      tree[i].left = tree[i].right = -1;
      j = 0;
      while(j < max_code_len)
        {
          tree[i].code[j] = 0;
          j++;
        }
      tree[i].code_len = 0;
      
      i++;
    }

  i = 0;
  while(i < alphabet_size)
    {
      if(freq[i] > 0)
        {
          tree[tree_len].character = i;
          tree[tree_len].w = freq[i];
          roots[tree_len] = tree_len;
          tree_len++;
        }
      i++;
    }
  roots_len = tree_len;

  /* Build tree */

  /* Pre-sort roots */
  i = 0;
  while(i < roots_len -1)
    {
      j = i+1;
      max = i;
      while(j < roots_len)
        {
          if(tree[roots[j]].w > tree[roots[max]].w) max = j;
          j++;
        }
      aux = roots[i];
      roots[i] = roots[max];
      roots[max] = aux;
      i++;
    }


  while(roots_len > 1)
    {
      if(DEBUG)
        {
          fprintf(stderr,"tree_len = %d, roots_len = %d, roots = [%d", tree_len,roots_len,roots[0]);
          i = 1;
          while(i < roots_len)
            fprintf(stderr,", %d", roots[i++]);
          fprintf(stderr,"]\n");
          
          print_tree_linear();
          print_tree();
        }

      /* join roots of smallest w */
      prefix_zero(roots[roots_len -2]);
      prefix_one (roots[roots_len -1]);

      tree[tree_len].left = roots[roots_len -2];
      tree[tree_len].right = roots[roots_len -1];
      combined_weight = tree[roots[roots_len -2]].w + tree[roots[roots_len -1]].w;
      tree[tree_len].w = combined_weight;
      roots[roots_len -2] = tree_len;
      tree_len++;
      roots_len--;

      /* Sort roots */
      i = 0;
      j = roots_len -1;
      while(j - i >1)
        {
          aux = (j + i)/2;
          if(combined_weight > tree[roots[aux]].w) j = aux;
          else i = aux;
        }



      aux = roots[roots_len -1];
      j = roots_len -1;
      while(j > i)
        {
          roots[j] = roots[j -1];
          j--;
        }
      roots[i] = aux;
    }

  if(DEBUG)
    {
      print_tree_linear();
      print_tree();
    }

  /* Build coding table */
  i = 0;
  while(i < chars_seen)
    {
      encoding[tree[i].character] = i;
      i++;
    }

  /* Second pass and output */

  output = stdout;

  i = 0;
  while(i < buf_size)
    out_buffer[i++] = 0xff;

  print_tree_rec(roots[0]);
  if((input = fopen(argv[1],"r")) == NULL)
    {
      fprintf(stderr, "! I cannot not open your file (on second pass).\n");
      exit(1);
    }

  while((llen = fread(in_buffer,
                      1,
                      buf_size,
                      input)) > 0)
    {
      i = 0;
      while(i < (int) llen)
        {
          output_code(encoding[in_buffer[i]]);
          i++;
        }
    }

  /* flush output buffer */

  /* Ensure at least one padding bit */
  if(next_bit % 8 == 0) out_buffer[next_bit/8] = 0;

  if(fwrite(out_buffer, 1, next_bit/8 +1, output) != next_bit/8 + 1)
    {
      fprintf(stderr, "! Cannot output!\n");
      exit(1);
    }
  if(DEBUG) fprintf(stderr,"[out_buffer flushed]");
         
  fclose(input);  
  return 0;
}
