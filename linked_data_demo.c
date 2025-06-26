#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pds_sample.h"
#include "book.h"
#include "author.h"

// Include the header file that contains the struct declarations for parent and child records

// Declare global variables for 10 parent and 10 child records

void show_menu();
void setup_data();
void process_option( int option );
void process_option1();
void process_option2();

int main()
{
    int option;
    setup_data();
    do{
        show_menu();
        scanf("%d",&option);
        process_option(option);
    } while (option != 0);

    // Close files before exiting
    int close_status = pds_close();
    if (close_status != PDS_SUCCESS) {
        printf("Closing failed\n");
    }
    return 0;
}

void show_menu()
{
    printf("\nLINKED DATA DEMO\n\n");
    printf("0. Exit\n");
    printf("1. Add linked data\n");
    printf("2. Get linked data\n");
    printf("\nEnter option: ");  
}

void setup_data()
{
    // statically create 10 parent and 10 child records individually
    pds_create("books", "authors");
    pds_open("books", "authors", sizeof(struct Book), sizeof(struct Author));
    
    // Add records using put_rec_by_key and put_linked_rec_by_key
    for (int i = 0; i < 10; i++) {
        struct Book *b = (struct Book *)malloc(sizeof(struct Book));
        struct Author *a = (struct Author *)malloc(sizeof(struct Author));

        b->book_id = i + 1;
        sprintf(b->book_name, "Book %d", i + 1);
        sprintf(b->ISBN, "%d", i + 1);

        a->author_id = i + 1;
        sprintf(a->author_name, "Author %d", i + 1);
        sprintf(a->email, "%d@gmail.com", i + 1);

        put_rec_by_key(b->book_id, b);
        put_linked_rec_by_key(a->author_id, a);
    }
}

void process_option( int option )
{
    switch(option){
        case 0: 
            // do nothing
            break;
        case 1:
            process_option1();
            break;
        case 2:
            process_option2();
            break;
        default:
            printf("Invalid option\n");
            break;
    }
}

void process_option1()
{
    int parent_key, child_key;
    printf("Enter parent key and child key for linking: ");
    scanf("%d%d", &parent_key, &child_key);
    // TODO: Call appropriate PDS functions here to link the two records
    int link_stat = pds_link_rec( parent_key, child_key );
    if (link_stat != PDS_SUCCESS) {
        printf("Linking failed\n");
    }
}

void process_option2()
{
    int parent_key;
    printf("Enter parent key: ");
    scanf("%d", &parent_key);
    // TODO: Call appropriate PDS functions here to fetch arrach of child keys
    // Call your PDS functions to display the list of child keys retrieved for the given parent key
    // Display all field values of each child record in a nicely formatted manner
    int linked_key_result[10];
    int res_size;
    int status = pds_get_linked_rec( parent_key, linked_key_result, &res_size );

    if (status != PDS_SUCCESS) {
        printf("Fetching linked records failed\n");
        return;
    }

    printf("\nLinked records for parent key %d:\n", parent_key);

    for (int i = 0; i < res_size; i++) {
        struct Author a;
        int tmp;
        get_linked_rec_by_key(linked_key_result[i], &a, &tmp);
        printf("\nAuthor ID: %d\n", a.author_id);
        printf("Author Name: %s\n", a.author_name);
        printf("Author Email: %s\n", a.email);
    }
}