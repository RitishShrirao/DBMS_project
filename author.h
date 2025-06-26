#ifndef AUTHOR_H
#define AUTHOR_H

#define AUTHOR_SUCCESS 0
#define AUTHOR_FAILURE 1

struct Author{
    int author_id;
    char author_name[30];
    char email[30];
};

extern struct PDS_RepoInfo *repoHandle;

// Add the given author into the repository by calling put_rec_by_key
int add_author( struct Author *a );

// Display author info in a single line as a CSV without any spaces
void print_author( struct Author *a );

// Use get_rec_by_key function to retrieve author
int search_author( int author_id, struct Author *a, int *io_count );

// Load all the authors from a CSV file
int store_authors( char *author_data_file );

int search_author_by_email( char *email, struct Author *a, int *io_count );

/* Return 0 if email of the author matches with email parameter */
/* Return 1 if email of the author does NOT match */
/* Return > 1 in case of any other error */
int match_author_email( void *rec, void *key );

int delete_author( int author_id );

#endif