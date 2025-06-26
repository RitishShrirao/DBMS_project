#ifndef BOOK_H
#define BOOK_H

#define BOOK_SUCCESS 0
#define BOOK_FAILURE 1

struct Book{
    int book_id;
    char book_name[30];
    char ISBN[30];
};

extern struct PDS_RepoInfo *repoHandle;

// Add the given book into the repository by calling put_rec_by_key
int add_book( struct Book *b );

// Display book info in a single line as a CSV without any spaces
void print_book( struct Book *b );

// Use get_rec_by_key function to retrieve book
int search_book( int book_id, struct Book *b );

// Load all the books from a CSV file
int store_books( char *book_data_file );

int search_book_by_ISBN( char *ISBN, struct Book *b, int *io_count );

/* Return 0 if ISBN of the book matches with ISBN parameter */
/* Return 1 if ISBN of the book does NOT match */
/* Return > 1 in case of any other error */
int match_book_ISBN( void *rec, void *key );

int delete_book( int book_id );

#endif