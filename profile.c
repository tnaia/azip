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
#define buf_size (1024 * 1024 +1)

#define alphabet_size (1<<7)
#define tree_size (1<<7)
long unsigned int freq[alphabet_size];

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
  long unsigned int w;
  int left, right;
} tree[tree_size];

int tree_len = 0;
int roots[tree_size];
int roots_len = 0;

void sort_roots() 
{
  return;
}

/* Prints tree rooted at i */
void print_rec(int i)
{
  printf("(");
  if(tree[i].left != -1) {
    print_rec(tree[i].left);
  }
  if(tree[i].right != -1) {
    print_rec(tree[i].right);
  }
  if((tree[i].left == tree[i].right) && tree[i].left == -1) {
    printf("%s/%lu", symbol_name[tree[i].character], tree[i].w);
  } else
    printf("/%lu", tree[i].w);
  printf(")");fflush(stdout);
}

void print_tree()
{
  int i = 0;
  printf("The tree as it stands:\n");
  while(i < roots_len)
    {
      print_rec(roots[i++]);
    }
  printf("\n");
}

void print_tree_linear() 
{
  int i = 0;
  while(i < tree_len)
    {
      
      printf("----tree: %d\n"
             "    left: %d\n"
             "   right: %d\n"
             "  weight: %lu\n",
             i, tree[i].left, tree[i].right, tree[i].w);
      if((tree[i].left == tree[i].right) && tree[i].left == -1)
        printf("    char: %s\n", symbol_name[tree[i].character]);
      i++;
    }
}

int main(int argc, char **argv)
{
  FILE *input;
  char in_buffer[buf_size +1];
  long unsigned int chars_read = 0;
  long unsigned int combined_weight;
  long unsigned int i, j, max, aux;
  size_t llen;
  printf(banner);
  printf("Alphabet size is %d\n", alphabet_size);
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
  while(i < alphabet_size)
    freq[i++] = 0;
  
  /* First pass */
  printf("First pass.\n");

  while((llen = fread(in_buffer,
                      1,
                      60,
                      input)) != 0)
    {
      printf("\nllen = %d\n", (int)llen);fflush(stdout);
      i = 0;
      while(i < (int) llen)
        {
          printf("%c", in_buffer[i]);fflush(stdout);
          freq[in_buffer[i]] +=1;
          i++;
          chars_read++;
        }
    }
  
  fclose(input);
  printf("\nEnd of first pass.\n");  
  i = 0;
  printf("Frequencies\n"
         "===========\n");
  while(i < alphabet_size)
    {
      if(freq[i] > 0)
        {
          printf("%4lu --> %lu (%s)\n", i, freq[i], symbol_name[i]);
        }
      i++;
    }

  /* Print file statistics */
  printf("Read %lu chars.\n", chars_read);
  
  /* Initialize tree */
  i = 0;
  while(i < tree_size)
    {
      tree[i].left = tree[i].right = -1;
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
      printf("tree_len = %d, roots_len = %d, roots = [%d", tree_len,roots_len,roots[0]);
      i = 1;
      while(i < roots_len)
        printf(", %d", roots[i++]);
      printf("]\n");

      print_tree_linear();
      print_tree();

      /* join roots of smallest w */
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

      print_tree_linear();
      print_tree();
      sort_roots();

  /* Second pass */

  return 0;
}
