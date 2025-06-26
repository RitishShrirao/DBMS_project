#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pds_sample.h"
#include "book.h"

// Load all the books from a CSV file
int store_books( char * book_data_file )
{
    FILE *bfptr;
    char book_line[500], token;
    struct Book b, dummy;

    bfptr = (FILE *) fopen(book_data_file, "r");
    while(fgets(book_line, sizeof(book_line)-1, bfptr)){
        //printf("line:%s",book_line);
        sscanf(book_line, "%d%s%s", &(b.book_id),b.book_name,b.ISBN);
        print_book( &b );
        add_book( &b );
    }   
}

void print_book( struct Book *b )
{
    printf("%d,%s,%s\n", b->book_id,b->book_name,b->ISBN);
}

// Use get_rec_by_ndx_key function to retrieve book
int search_book( int book_id, struct Book *b )
{
    return get_rec_by_ndx_key( book_id, b );
}

// Add the given book into the repository by calling put_rec_by_key
int add_book( struct Book *b )
{
    int status;

    status = put_rec_by_key( b->book_id, b );

    if( status != PDS_SUCCESS ){
        fprintf(stderr, "Unable to add book with key %d. Error %d", b->book_id, status );
        return BOOK_FAILURE;
    }
    return status;
}

// Use get_rec_by_non_ndx_key function to retrieve book
int search_book_by_ISBN( char *ISBN, struct Book *b, int *io_count )
{
    int status;

    // Call function
    status = get_rec_by_non_ndx_key(ISBN, b, match_book_ISBN, io_count);

    if (status == PDS_REC_NOT_FOUND) {
        return BOOK_FAILURE;
    }
    return BOOK_SUCCESS;
}

// Return 0 if ISBN of the book matches with the ISBN parameter
// Return 1 if ISBN of the book does not match with the ISBN parameter
// Return >1 in case of any other errors
int match_book_ISBN( void * rec, void * key)
{
    // Store the rec in a struct book pointer
    // Store the key in a char pointer
    // Compare the ISBN of the book with the key
    struct Book *b = (struct Book *) rec;
    char *key_ISBN = (char *) key;

    // No memory allocated
    if (b == NULL || key_ISBN == NULL){
        return 2;
    }


    // Compare using strcmp
    if (strcmp(b->ISBN, key_ISBN) == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

int delete_book( int book_id )
{
    int status = delete_rec_by_ndx_key( book_id );

    if (status == PDS_DELETE_FAILED)    return BOOK_FAILURE;
    else    return BOOK_SUCCESS;
}