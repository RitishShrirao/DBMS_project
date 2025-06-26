#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds_sample.h"
// #include "contact.h"
#include "book.h"
#include "author.h"

#define TREPORT(a1,a2) printf("Status: %s - %s\n\n",a1,a2); fflush(stdout);

void process_line( char *test_case );

int main(int argc, char *argv[])
{
	FILE *cfptr;
	char test_case[50];
	int i = 1;

	if( argc != 2 ){
		fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
		exit(1);
	}

	cfptr = (FILE *) fopen(argv[1], "r");
	while(fgets(test_case, sizeof(test_case)-1, cfptr)){
		// printf("line:%s",test_case);
		if( !strcmp(test_case,"\n") || !strcmp(test_case,"") )
			continue;
		printf("Test case:%d\n", i++);
		process_line( test_case );
	}
}

void process_line( char *test_case )
{
	char repo_name[30], linked_repo_name[30];
	char command[25], param1[15], param2[15], param3[15], info[1024];
	int book_id, status, rec_size, expected_status;
	int author_id, linked_rec_size;
	struct Book testBook;
	struct Author testAuthor;
	int expected_linked_key_result[100];

	// strcpy(testBook.book_name, "dummy name");
	// strcpy(testBook.ISBN, "dummy number");

	rec_size = sizeof(struct Book);
	linked_rec_size = sizeof(struct Author);

	sscanf(test_case, "%s%s%s", command, param1, param2);

	// scan for the third parameter if the command is CREATE or OPEN (linked_repo_name)
	if (!strcmp(command, "CREATE") || !strcmp(command, "OPEN") || !strcmp(command, "LINK"))
	{
		sscanf(test_case, "%s%s%s%s", command, param1, param2, param3);
	}
	else if (!strcmp(command, "SEARCH_LINKED_BY_PARENT"))
	{
		sscanf(test_case, "%s%s%s", command, param1, param2);
		int expected_res_size;
		sscanf(param2, "%d", &expected_res_size);

		char tmp[10];
		int running_strlen = strlen(command) + strlen(param1) + strlen(param2) + 3;
		// printf("Expected res_size=%d\n", expected_res_size);	// Testing

		if (expected_res_size <= 0)
		{
			return;
		}

		for (int i = 0; i < expected_res_size; i++)
		{
			sscanf(test_case + running_strlen, "%s", tmp);
			sscanf(tmp, "%d", &expected_linked_key_result[i]);
			running_strlen += strlen(tmp);
			// printf("%d\n", expected_linked_key_result[i]);	// Testing
		}
	}
	
	printf("Test case: %s", test_case); fflush(stdout);
	if( !strcmp(command,"CREATE") ){
		strcpy(repo_name, param1);
		strcpy(linked_repo_name, param2);

		if( !strcmp(param3,"0") )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		status = pds_create( repo_name , linked_repo_name );
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_create returned status %d",status);
			TREPORT("FAIL", info);
		}
	}
	else if( !strcmp(command,"OPEN") ){
		strcpy(repo_name, param1);
		strcpy(linked_repo_name, param2);

		if( !strcmp(param3,"0") )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		status = pds_open( repo_name, linked_repo_name, rec_size, linked_rec_size);
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_open returned status %d",status);
			TREPORT("FAIL", info);
		}
	}
	else if( !strcmp(command,"STORE") ){
		if( !strcmp(param2,"0") )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		sscanf(param1, "%d", &book_id);
		testBook.book_id = book_id;
		sprintf(testBook.ISBN, "ISBN-of-%d", book_id);
		sprintf(testBook.book_name, "Name-of-%d", book_id);
		status = add_book( &testBook );
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"add_book returned status %d",status);
			TREPORT("FAIL", info);
		}
	}
	else if (!strcmp(command, "STORE_LINKED"))
	{
		if (!strcmp(param2, "0"))
			expected_status = AUTHOR_SUCCESS;
		else
			expected_status = AUTHOR_FAILURE;

		sscanf(param1, "%d", &author_id);
		
		testAuthor.author_id = author_id;
		sprintf(testAuthor.author_name, "Author-of-%d", author_id);
		sprintf(testAuthor.email, "Email-of-%d", author_id);
		status = add_author(&testAuthor);
		if (status == PDS_SUCCESS)
			status = AUTHOR_SUCCESS;
		else
			status = AUTHOR_FAILURE;
		if (status == expected_status)
		{
			TREPORT("PASS", "");
		}
		else
		{
			sprintf(info, "add_author returned status %d", status);
			TREPORT("FAIL", info);
		}
	}
	else if( !strcmp(command,"NDX_SEARCH") ){
		if( !strcmp(param2,"0") )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		sscanf(param1, "%d", &book_id);
		testBook.book_id = -1;
		status = search_book( book_id, &testBook );
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status != expected_status ){
			sprintf(info,"search key: %d; Got status %d",book_id, status);
			TREPORT("FAIL", info);
		}
		else{
			// Check if the retrieved values match
			char expected_ISBN[30];
    		sprintf(expected_ISBN, "ISBN-of-%d", book_id);
			char expected_name[30];
    		sprintf(expected_name, "Name-of-%d", book_id);
			if( expected_status == 0 ){
				if (testBook.book_id == book_id && 
					strcmp(testBook.book_name,expected_name) == 0 &&
					strcmp(testBook.ISBN, expected_ISBN) == 0){
						TREPORT("PASS", "");
				}
				else{
					sprintf(info,"Book data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}",
						book_id, expected_name, expected_ISBN, 
						testBook.book_id, testBook.book_name, testBook.ISBN
					);
					TREPORT("FAIL", info);
				}
			}
			else
				TREPORT("PASS", "");
		}
	}
	
	else if (!strcmp(command,"SEARCH_LINKED") ){
		if (!strcmp(param2,"-1"))
			expected_status = AUTHOR_FAILURE;
		else
			expected_status = AUTHOR_SUCCESS;

		sscanf(param1, "%d", &author_id);
		testAuthor.author_id = -1;
		int io_count, expected_io;

		sscanf(param2, "%d", &expected_io);

		status = search_author(author_id, &testAuthor, &io_count);
		if (status != PDS_SUCCESS)
		{
			sprintf(info, "search key: %d; Got status %d", author_id, status);
			TREPORT("FAIL", info);
		}
		else if (io_count != expected_io && status == PDS_SUCCESS)
		{
			sprintf(info, "Num I/O not matching for author %d... Expected:%d Got:%d",
				author_id, expected_io, io_count
			);
			TREPORT("FAIL", info);
		}
		else
		{
			// Check if the retrieved values match
			char expected_email[30], expected_name[30];
			sprintf(expected_email, "Email-of-%d", author_id);
			sprintf(expected_name, "Author-of-%d", author_id);
			if (expected_status == PDS_SUCCESS)
			{
				if (testAuthor.author_id == author_id &&
					strcmp(testAuthor.author_name, expected_name) == 0 &&
					strcmp(testAuthor.email, expected_email) == 0)
				{
					TREPORT("PASS", "");
				}
				else
				{
					sprintf(info, "Author data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}\n",
						author_id, expected_name, expected_email,
						testAuthor.author_id, testAuthor.author_name, testAuthor.email
					);
					TREPORT("FAIL", info);
				}
			}
			else
				TREPORT("PASS", "");
		}
	}
	else if (!strcmp(command, "SEARCH_LINKED_BY_PARENT")){
		int linked_key_result[100];
		int res_size, expected_res_size;
		sscanf(param1, "%d", &book_id);
		sscanf(param2, "%d", &expected_res_size);
		status = pds_get_linked_rec(book_id, linked_key_result, &res_size);

		// expected status = -1 in case of errors
		if (!strcmp(param2, "-1"))
			sscanf(param2, "%d", &expected_status);
		else
			expected_status = PDS_SUCCESS;
		
		if (status != expected_status)
		{
			sprintf(info, "Fetching linked records failed for parent key %d. Expected status %d, Got status %d", book_id, expected_status, status);
			TREPORT("FAIL", info);
		}
		else
		{
			if (status == PDS_SUCCESS && res_size != expected_res_size)
			{
				sprintf(info, "Fetching linked records failed for parent key %d. Expected size %d, Got size %d", book_id, expected_res_size, res_size);
				TREPORT("FAIL", info);
			}
			else
			{
				for (int i = 0; i < res_size; i++)
				{
					// printf("Linked key : %d\n", linked_key_result[i]);	// Testing

					// Check all expected_linked_keys in linked keys or not
					int found = 0;
					for (int j = 0; j < expected_res_size; j++)
					{
						if (expected_linked_key_result[i] == linked_key_result[j])
						{
							found = 1;
							break;
						}
					}

					if (found == 0)
					{
						sprintf(info, "Expected key %d not found in linked keys", linked_key_result[i]);
						TREPORT("FAIL", info);
						return;
					}
				}

				// Checking all linked keys in expected keys or not
				for (int i = 0; i < res_size; i++)
				{
					int found = 0;
					for (int j = 0; j < expected_res_size; j++)
					{
						if (linked_key_result[i] == expected_linked_key_result[j])
						{
							found = 1;
							break;
						}
					}

					if (found == 0)
					{
						sprintf(info, "Linked key %d not found in expected keys", linked_key_result[i]);
						TREPORT("FAIL", info);
						return;
					}
				}
				TREPORT("PASS", "");
			}
		}
	}
	else if ( !strcmp(command, "LINK")){
		if (!strcmp(param3, "0"))
			expected_status = PDS_SUCCESS;
		else
			sscanf(param3, "%d", &expected_status);
		sscanf(param1, "%d", &book_id);
		sscanf(param2, "%d", &author_id);

		status = pds_link_rec(book_id, author_id);

		if (status != expected_status)
		{
			sprintf(info, "Parent key: %d, Child key: %d could not be linked. Expected status : %d, Got status : %d", book_id, author_id, expected_status, status);
			TREPORT("FAIL", info);
		}
		else
			TREPORT("PASS", "");
	}
	else if( !strcmp(command,"NON_NDX_SEARCH") ){
		char ISBN_num[30], expected_name[30], expected_ISBN[30];
		int expected_io, actual_io;
		
		if( strcmp(param2,"-1") == 0 )
			expected_status = BOOK_FAILURE;
		else
			expected_status = BOOK_SUCCESS;

		sscanf(param1, "%s", ISBN_num);
		sscanf(param2, "%d", &expected_io);
		testBook.book_id = -1;
		actual_io = 0;
		status = search_book_by_ISBN( ISBN_num, &testBook, &actual_io );
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status != expected_status ){
			sprintf(info,"search key: %d; Got status %d",book_id, status);
			TREPORT("FAIL", info);
		}
		else{
			// Check if the retrieved values match
			// Check if num block accesses match too
			// Extract the expected book_id from the ISBN number
			sscanf(ISBN_num+sizeof("ISBN-of"), "%d", &book_id);
			sprintf(expected_name,"Name-of-%d",book_id);
			sprintf(expected_ISBN,"ISBN-of-%d",book_id);
			if( expected_status == 0 ){
				if (testBook.book_id == book_id && 
					strcmp(testBook.book_name, expected_name) == 0 &&
					strcmp(testBook.ISBN, expected_ISBN) == 0 ){
						if( expected_io == actual_io ){
							TREPORT("PASS", "");
						}
						else{
							sprintf(info,"Num I/O not matching for book %d... Expected:%d Got:%d",
								book_id, expected_io, actual_io
							);
							TREPORT("FAIL", info);
						}
				}
				else{
					sprintf(info,"Book data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}",
						book_id, expected_name, expected_ISBN, 
						testBook.book_id, testBook.book_name, testBook.ISBN
					);
					TREPORT("FAIL", info);
				}
			}
			else
				TREPORT("PASS", "");
		}
	}
	else if( !strcmp(command,"NDX_DELETE") ){
		if( strcmp(param2,"0") == 0 )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		sscanf(param1, "%d", &book_id);
		testBook.book_id = -1;
		status = delete_book( book_id );
		if( status != expected_status ){
			sprintf(info,"delete key: %d; Got status %d",book_id, status);
			TREPORT("FAIL", info);
		}
		else{
			TREPORT("PASS", "");
		}
	}
	else if( !strcmp(command,"CLOSE") ){
		if( !strcmp(param1,"0") )
			expected_status = BOOK_SUCCESS;
		else
			expected_status = BOOK_FAILURE;

		status = pds_close();
		if(status == PDS_SUCCESS)
			status = BOOK_SUCCESS;
		else
			status = BOOK_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_close returned status %d",status);
			TREPORT("FAIL", info);
		}
	}
}