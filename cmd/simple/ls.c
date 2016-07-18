#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ls: List directory contents.		Author: Warren Toomey */

/* External variables. */
extern int optind;
extern char *optarg;

int longoutput=0;		// Do a long (-l) output
int showdots=0;			// Show dot files
int dircontents=1;		// Show the contents of a directory

void listone(char *entry, struct stat *sbptr)
{
  // Lose the newline on the time string
  char *timestring= ctime(&(sbptr->st_mtime));
  timestring[ strlen(timestring) - 1 ]= '\0';

  if (longoutput) {
    // Print the type of entry as the first character
    if S_ISDIR(sbptr->st_mode)
      printf("d ");
    else if S_ISREG(sbptr->st_mode)
      printf("- ");
    else
      printf("  ");

    // Print out the perms in octal, the inum, the uid/gid, the size and name
    printf("%03o %5d %3d %3d %6ld %s %s\n", sbptr->st_mode & 0777,
	sbptr->st_ino, sbptr->st_uid, sbptr->st_gid, sbptr->st_size,
	timestring, entry);
  } else {
    // Just print out the name
    puts(entry);
  }
}

// We keep a array structure of dirents and their stat buffers
typedef struct {
  struct dirent *dent;
  struct stat sb;
} Namestat;

#define NLISTSIZE 200
Namestat namelist[NLISTSIZE];

// Compare two Namestat structs using the entry names
int namecmp(const void *a, const void *b)
{
  Namestat *aa= (Namestat *)a;
  Namestat *bb= (Namestat *)b;

  return(strcmp(aa->dent->d_name, bb->dent->d_name));
}

// List the entry (if a file), or its contents (if a directory)
void listmany(char *entry)
{
  DIR *D;
  struct dirent *dent;
  struct stat sb;
  int count=0;

  // Ensure the entry exists
  if (stat(entry, &sb)==-1) {
    printf("%s: non-existent\n", entry);
    return;
  }

  // It's a file, just print it out
  if (S_ISREG(sb.st_mode)) {
    listone(entry, &sb);
    return;
  }

  // It's a directory, deal with all of it
  if (S_ISDIR(sb.st_mode)) {
    // Only list the directory, not its contents
    if (dircontents==0) {
      listone(entry, &sb);
      return;
    }

    // Open the directory and move into it
    D= opendir(entry);
    if (D==NULL) {
      printf("%s: unable to opendir\n", entry);
      return;
    }
    chdir(entry);

    // Process each entry
    while ((dent=readdir(D))!=NULL) {

      // Skip empty directory entries
      if (dent->d_name[0]=='\0') continue;

      // Skip dot files
      if ((showdots==0) && (dent->d_name[0]=='.')) continue;

      // Get the file's stats
      if (stat(dent->d_name, &sb)==-1) {
        printf("%s: non-existent\n", dent->d_name);
        continue;
      }

      // and add the file to the array
      namelist[count].dent= dent;
      memcpy(&(namelist[count].sb), &sb, sizeof(sb));
      count++;

    }

    // Sort the array into name order
    qsort(namelist, count, sizeof(Namestat), namecmp);

    // Print each one out
    for (int i=0; i < count; i++) 
      listone(namelist[i].dent->d_name, &(namelist[i].sb));

    // Go back to where we came from. XXX: not always "..", so fix
    closedir(D);
    chdir("..");
  }
}

int main(int argc, char *argv[])
{
  int opt;                      /* option letter from getopt() */

  /* Process any command line flags. */
  while ((opt = getopt(argc, argv, "lad")) != EOF) {
        if (opt == 'l')
                longoutput = 1;
        if (opt == 'a')
                showdots = 1;
        if (opt == 'd')
                dircontents = 0;
  }

  // No further arguments, list the current directory
  if (optind==argc) {
    listmany("."); exit(0);
  }

  // Otherwise, process the arguments left
  for (int i=optind; i<argc; i++) 
    listmany(argv[i]);
  exit(0);
}
