#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pds_sample.h"
#include "author.h"

// Load all the authors from a CSV file
int store_authors( char * author_data_file )
{
    FILE *afptr;
    char author_line[500], token;
    struct Author a, dummy;

    afptr = (FILE *) fopen(author_data_file, "r");
    while(fgets(author_line, sizeof(author_line)-1, afptr)){
        //printf("line:%s",author_line);
        sscanf(author_line, "%d%s%s", &(a.author_id),a.author_name,a.email);
        print_author( &a );
        add_author( &a );
    }   
}

void print_author( struct Author *a )
{
    printf("%d,%s,%s\n", a->author_id,a->author_name,a->email);
}

// Use get_rec_by_ndx_key function to retrieve author
int search_author( int author_id, struct Author *a, int * io_count)
{
    return get_linked_rec_by_key( author_id, a , io_count);
}

// Add the given author into the repository by calling put_rec_by_key
int add_author( struct Author *a )
{
    int status;

    status = put_linked_rec_by_key( a->author_id, a );

    if( status != PDS_SUCCESS ){
        fprintf(stderr, "Unable to add author with key %d. Error %d", a->author_id, status );
        return AUTHOR_FAILURE;
    }
    return status;
}

// Delete author by author id
int delete_author( int author_id )
{
    int status = delete_rec_by_ndx_key( author_id );

    if (status == PDS_DELETE_FAILED)    return AUTHOR_FAILURE;
    else    return AUTHOR_SUCCESS;
}